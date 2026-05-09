#include "modules/etb/etb.h"

#include <Arduino.h>

#include "boards/board_definition.h"
#include "data/etb_status.h"
#include "data/runtime_state.h"
#include "data/tune_registry.h"
#include "support/maths.h"
#include "support/preprocessor.h"
#include "support/utilities.h"

#if FEATURE_MODULE_ETB
namespace {
static byte etbOpenPin = 0U;
static byte etbClosePin = 0U;
static byte etbPedal1Pin = 0U;
static byte etbPedal2Pin = 0U;
static byte etbThrottle2Pin = 0U;
static bool etbConfigured = false;
static int16_t etbIntegral = 0;
static int16_t etbLastError = 0;

static inline bool is_valid_raw_pin(byte rawPin)
{
  return (rawPin != 0U) && (rawPin < BOARD_MAX_IO_PINS);
}

static inline byte translate_pin(byte rawPin)
{
  return is_valid_raw_pin(rawPin) ? pinTranslate(rawPin) : 0U;
}

static inline uint8_t read_sensor_percent(byte pin, byte minRaw, byte maxRaw, bool invert)
{
  if (pin == 0U)
  {
    return 0U;
  }

  const uint16_t raw10 = (uint16_t)analogRead(pin);
  const uint16_t raw8 = (uint16_t)map(raw10, 0, 1023, 0, 255);
  const uint16_t clipped = clamp(raw8, (uint16_t)minRaw, (uint16_t)maxRaw);
  uint8_t percent = (uint8_t)map(clipped, minRaw, maxRaw, 0, 200);
  if (invert)
  {
    percent = (uint8_t)(200U - percent);
  }
  return percent;
}

static inline uint8_t interpolate_curve(uint8_t pedalPercent)
{
  const uint8_t clipped = clamp(pedalPercent, (uint8_t)0U, (uint8_t)200U);
  const uint16_t scaled = (uint16_t)clipped * (etbCurvePoints - 1U);
  const uint8_t index = (uint8_t)(scaled / 200U);
  const uint8_t nextIndex = (uint8_t)min<uint8_t>(index + 1U, etbCurvePoints - 1U);
  const uint8_t lower = configPage16.etbTargetCurve[index];
  const uint8_t upper = configPage16.etbTargetCurve[nextIndex];
  const uint16_t remainder = scaled % 200U;
  const uint8_t interpolated = (uint8_t)(lower + (((uint16_t)(upper - lower)) * remainder) / 200U);
  return clamp(interpolated, configPage16.etbTargetMin, configPage16.etbTargetMax);
}

static inline void motor_off(void)
{
  if (etbOpenPin != 0U)
  {
    digitalWrite(etbOpenPin, configPage16.etbOpenPolarity ? HIGH : LOW);
  }
  if (etbClosePin != 0U)
  {
    digitalWrite(etbClosePin, configPage16.etbClosePolarity ? HIGH : LOW);
  }
  currentEtbStatus.open_duty = 0U;
  currentEtbStatus.close_duty = 0U;
}

static inline void motor_drive(bool opening, uint8_t duty)
{
  const uint8_t openDuty = opening ? duty : 0U;
  const uint8_t closeDuty = opening ? 0U : duty;

  currentEtbStatus.open_duty = openDuty;
  currentEtbStatus.close_duty = closeDuty;

  if (etbOpenPin != 0U)
  {
    if (openDuty == 0U)
    {
      digitalWrite(etbOpenPin, configPage16.etbOpenPolarity ? HIGH : LOW);
    }
    else
    {
      analogWrite(etbOpenPin, configPage16.etbOpenPolarity ? (255U - openDuty) : openDuty);
    }
  }

  if (etbClosePin != 0U)
  {
    if (closeDuty == 0U)
    {
      digitalWrite(etbClosePin, configPage16.etbClosePolarity ? HIGH : LOW);
    }
    else
    {
      analogWrite(etbClosePin, configPage16.etbClosePolarity ? (255U - closeDuty) : closeDuty);
    }
  }
}

static inline void reset_controller(void)
{
  etbIntegral = 0;
  etbLastError = 0;
}

static inline bool pedal_plausible(uint8_t pedal1, uint8_t pedal2)
{
  return (uint8_t)abs((int16_t)pedal1 - (int16_t)pedal2) <= configPage16.etbPedalMismatch;
}

static inline bool throttle_plausible(uint8_t throttle1, uint8_t throttle2)
{
  return (uint8_t)abs((int16_t)throttle1 - (int16_t)throttle2) <= configPage16.etbThrottleMismatch;
}
} // namespace

void module_etb_init_post_pin_mapping(void)
{
  currentEtbStatus.enabled = configPage16.etbEnabled;
  currentEtbStatus.fault = false;
  currentEtbStatus.fault_code = ETB_FAULT_DISABLED;
  currentEtbStatus.pedal_percent = 0U;
  currentEtbStatus.throttle_percent = 0U;
  currentEtbStatus.target_percent = 0U;
  currentEtbStatus.error = 0;
  currentEtbStatus.pedal1_adc = 0U;
  currentEtbStatus.pedal2_adc = 0U;
  currentEtbStatus.throttle2_adc = 0U;

  etbOpenPin = translate_pin(configPage16.etbOpenPin);
  etbClosePin = translate_pin(configPage16.etbClosePin);
  etbPedal1Pin = translate_pin(configPage16.etbPedal1Pin);
  etbPedal2Pin = translate_pin(configPage16.etbPedal2Pin);
  etbThrottle2Pin = translate_pin(configPage16.etbThrottle2Pin);

  if (!currentEtbStatus.enabled)
  {
    motor_off();
    return;
  }

  if ((etbOpenPin == 0U) || (etbClosePin == 0U) || (etbPedal1Pin == 0U) || (etbPedal2Pin == 0U))
  {
    currentEtbStatus.fault = true;
    currentEtbStatus.fault_code = ETB_FAULT_CONFIG;
    motor_off();
    return;
  }

  pinMode(etbOpenPin, OUTPUT);
  pinMode(etbClosePin, OUTPUT);
  pinMode(etbPedal1Pin, INPUT);
  pinMode(etbPedal2Pin, INPUT);
  if (etbThrottle2Pin != 0U)
  {
    pinMode(etbThrottle2Pin, INPUT);
  }

  motor_off();
  reset_controller();
  etbConfigured = true;
}

void module_etb_tick_200hz(void)
{
  if (!etbConfigured || !currentEtbStatus.enabled)
  {
    motor_off();
    return;
  }

  const uint8_t pedal1 = read_sensor_percent(etbPedal1Pin, configPage16.etbPedal1Min, configPage16.etbPedal1Max, false);
  const uint8_t pedal2 = read_sensor_percent(etbPedal2Pin, configPage16.etbPedal2Min, configPage16.etbPedal2Max, configPage16.etbPedal2Invert);
  const uint8_t throttle1 = currentStatus.TPS;
  const uint8_t throttle2 = (etbThrottle2Pin != 0U) ? read_sensor_percent(etbThrottle2Pin, configPage16.etbThrottle2Min, configPage16.etbThrottle2Max, configPage16.etbThrottle2Invert) : throttle1;

  currentEtbStatus.pedal1_adc = analogRead(etbPedal1Pin);
  currentEtbStatus.pedal2_adc = analogRead(etbPedal2Pin);
  currentEtbStatus.throttle2_adc = (etbThrottle2Pin != 0U) ? analogRead(etbThrottle2Pin) : 0U;

  if (!pedal_plausible(pedal1, pedal2) || !throttle_plausible(throttle1, throttle2))
  {
    currentEtbStatus.fault = true;
    currentEtbStatus.fault_code = ETB_FAULT_MISMATCH;
    motor_off();
    reset_controller();
    return;
  }

  currentEtbStatus.fault = false;
  currentEtbStatus.fault_code = ETB_FAULT_DISABLED;

  const uint8_t pedalPercent = (uint8_t)(((uint16_t)pedal1 + (uint16_t)pedal2) / 2U);
  const uint8_t throttlePercent = (uint8_t)(((uint16_t)throttle1 + (uint16_t)throttle2) / 2U);
  const uint8_t targetPercent = interpolate_curve(pedalPercent);
  const int16_t error = (int16_t)targetPercent - (int16_t)throttlePercent;

  currentEtbStatus.pedal_percent = pedalPercent;
  currentEtbStatus.throttle_percent = throttlePercent;
  currentEtbStatus.target_percent = targetPercent;
  currentEtbStatus.error = error;

  if (abs(error) <= configPage16.etbDeadband)
  {
    motor_off();
    return;
  }

  const int16_t errorDelta = error - etbLastError;
  etbLastError = error;
  etbIntegral = clamp((int16_t)(etbIntegral + error), (int16_t)-1000, (int16_t)1000);

  int32_t drive = (int32_t)configPage16.etbKP * abs(error);
  drive += (int32_t)configPage16.etbKI * abs(etbIntegral);
  drive += (int32_t)configPage16.etbKD * abs(errorDelta);
  drive = drive / 4L;
  drive = clamp(drive, (int32_t)configPage16.etbMinDuty, (int32_t)255L);

  motor_drive(error > 0, (uint8_t)drive);
}

void module_etb_on_engine_stop(void)
{
  motor_off();
  reset_controller();
  currentEtbStatus.enabled = configPage16.etbEnabled;
  currentEtbStatus.fault = false;
  currentEtbStatus.fault_code = ETB_FAULT_DISABLED;
}
#else
void module_etb_init_post_pin_mapping(void) {}
void module_etb_tick_200hz(void) {}
void module_etb_on_engine_stop(void) {}
#endif
