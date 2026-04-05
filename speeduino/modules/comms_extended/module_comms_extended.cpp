#include "module_comms_extended.h"

#include "can_aux_status.h"
#include "preprocessor.h"

#if FEATURE_MODULE_COMMS_EXTENDED

#include "core_constants.h"
#include "comms_CAN.h"
#include "can_transport.h"
#include "utilities.h"
#include "sensors.h"

void module_comms_extended_init(void)
{
  #if defined(NATIVE_CAN_AVAILABLE) && !defined(UNIT_TEST)
    initCAN();
  #endif
}

void module_comms_extended_poll(uint8_t internal_can_enabled, uint8_t can_wbo_enabled)
{
  #if defined(NATIVE_CAN_AVAILABLE)
    if (internal_can_enabled == 1U)
    {
      while (CAN_read())
      {
        can_Command();
        readAuxCanBus();
        if (can_wbo_enabled > 0U) { receiveCANwbo(); }
      }
    }
  #else
    UNUSED(internal_can_enabled);
    UNUSED(can_wbo_enabled);
  #endif
}

void module_comms_extended_tick_50hz(void)
{
  #if defined(NATIVE_CAN_AVAILABLE)
    sendCANBroadcast(50);
  #endif
}

void module_comms_extended_tick_30hz(void)
{
  #if defined(NATIVE_CAN_AVAILABLE)
    sendCANBroadcast(30);
  #endif
}

void module_comms_extended_tick_15hz(void)
{
  #if defined(NATIVE_CAN_AVAILABLE)
    sendCANBroadcast(15);
  #endif
}

void module_comms_extended_tick_10hz(void)
{
  #if defined(NATIVE_CAN_AVAILABLE)
    sendCANBroadcast(10);
  #endif
}

void module_comms_extended_tick_4hz(uint8_t sensor_status, statuses &current, const can_extended_config_t &config)
{
  UNUSED(current);
  if(BIT_CHECK(sensor_status, BIT_SENSORS_AUX_ENBL))
  {
    for (byte AuxinChan = 0; AuxinChan < 16; AuxinChan++)
    {
      currentCanAuxStatus.current_channel = AuxinChan;

      if (((config.caninput_selection(currentCanAuxStatus.current_channel) & 12U) == 4U)
          && (((config.is_secondary_serial_enabled() == 1U) && ((config.is_internal_can_enabled() == 0U) && (config.is_internal_can_available() == 1U)))
              || ((config.is_secondary_serial_enabled() == 1U) && ((config.is_internal_can_enabled() == 1U) && (config.is_internal_can_available() == 1U))
                  && ((config.caninput_selection(currentCanAuxStatus.current_channel) & 64U) == 0U))
              || ((config.is_secondary_serial_enabled() == 1U) && ((config.is_internal_can_enabled() == 1U) && (config.is_internal_can_available() == 0U)))))
      {
        if (config.is_secondary_serial_enabled() == 1U)
        {
          sendCancommand(2, 0, currentCanAuxStatus.current_channel, 0, ((config.caninput_source_address(currentCanAuxStatus.current_channel) & 2047U) + 0x100U));
        }
      }
      else if (((config.caninput_selection(currentCanAuxStatus.current_channel) & 12U) == 4U)
          && (((config.is_secondary_serial_enabled() == 1U) && ((config.is_internal_can_enabled() == 1U) && (config.is_internal_can_available() == 1U))
              && ((config.caninput_selection(currentCanAuxStatus.current_channel) & 64U) == 64U))
              || ((config.is_secondary_serial_enabled() == 0U) && ((config.is_internal_can_enabled() == 1U) && (config.is_internal_can_available() == 1U))
                  && ((config.caninput_selection(currentCanAuxStatus.current_channel) & 128U) == 128U))))
      {
        #if defined(CORE_STM32) || defined(CORE_TEENSY)
          if (config.is_internal_can_enabled() == 1U)
          {
            sendCancommand(3, config.speeduino_ts_can_id(), currentCanAuxStatus.current_channel, 0, ((config.caninput_source_address(currentCanAuxStatus.current_channel) & 2047U) + 0x100U));
          }
        #endif
      }
      else if ((((config.is_secondary_serial_enabled() == 1U) || ((config.is_internal_can_enabled() == 1U) && (config.is_internal_can_available() == 1U)))
                  && ((config.caninput_selection(currentCanAuxStatus.current_channel) & 12U) == 8U))
              || (((config.is_secondary_serial_enabled() == 0U) && ((config.is_internal_can_enabled() == 1U) && (config.is_internal_can_available() == 0U)))
                  && ((config.caninput_selection(currentCanAuxStatus.current_channel) & 3U) == 2U))
              || (((config.is_secondary_serial_enabled() == 0U) && (config.is_internal_can_enabled() == 0U))
                  && ((config.caninput_selection(currentCanAuxStatus.current_channel) & 3U) == 2U)))
      {
        currentCanAuxStatus.values[currentCanAuxStatus.current_channel] =
            readAuxanalog(pinTranslateAnalog(config.aux_input_analog_pin(currentCanAuxStatus.current_channel) & 63U));
      }
      else if ((((config.is_secondary_serial_enabled() == 1U) || ((config.is_internal_can_enabled() == 1U) && (config.is_internal_can_available() == 1U)))
                  && ((config.caninput_selection(currentCanAuxStatus.current_channel) & 12U) == 12U))
              || (((config.is_secondary_serial_enabled() == 0U) && ((config.is_internal_can_enabled() == 1U) && (config.is_internal_can_available() == 0U)))
                  && ((config.caninput_selection(currentCanAuxStatus.current_channel) & 3U) == 3U))
              || (((config.is_secondary_serial_enabled() == 0U) && (config.is_internal_can_enabled() == 0U))
                  && ((config.caninput_selection(currentCanAuxStatus.current_channel) & 3U) == 3U)))
      {
        currentCanAuxStatus.values[currentCanAuxStatus.current_channel] =
            readAuxdigital((config.aux_input_digital_pin(currentCanAuxStatus.current_channel) & 63U) + 1U);
      }
      else
      {
        // Channel not active in the current configuration.
      }
    }
  }
}

#endif
