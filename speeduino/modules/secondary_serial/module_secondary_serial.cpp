#include "module_secondary_serial.h"

#include "core_constants.h"
#include "preprocessor.h"

#if FEATURE_MODULE_SECONDARY_SERIAL

#include "secondary_serial.h"

void module_secondary_serial_init(const secondary_serial_config_t &config)
{
  if (config.is_enabled() == 1U) { secondarySerial.begin(115200); }
}

void module_secondary_serial_poll(const secondary_serial_config_t &config)
{
  if (config.is_enabled() == 1U)
  {
    #ifndef CORE_AVR
      if (secondarySerial.available() > 0) { secondserial_Command(); }
    #else
      if (secondarySerial.available() > SERIAL_BUFFER_THRESHOLD) { secondserial_Command(); }
    #endif
  }
}

void module_secondary_serial_tick_30hz(const secondary_serial_config_t &config)
{
  #ifdef CORE_AVR
    if ((config.is_enabled() == 1U) && (secondarySerial.available() > 0))
    {
      secondserial_Command();
    }
  #else
    UNUSED(config);
  #endif
}

#endif
