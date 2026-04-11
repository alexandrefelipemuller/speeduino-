#include "modules/fan_aircon/fan_aircon.h"

#include "data/advanced_engine_status.h"
#include "engine/auxiliaries.h"
#include "support/atomic.h"
#include "boards/board_definition.h"
#include "support/maths.h"
#include "data/pin_registry.h"
#include "support/port_pin.h"
#include "data/runtime_state.h"
#include "support/table2d.h"
#include "data/tune_registry.h"
#include "support/units.h"
#include "support/utilities.h"

bool pinIsOutput(byte pin);

static port_register_t aircon_req_pin_port;
static pin_mask_t aircon_req_pin_mask;
static port_register_t aircon_comp_pin_port;
static pin_mask_t aircon_comp_pin_mask;
static port_register_t aircon_fan_pin_port;
static pin_mask_t aircon_fan_pin_mask;
static port_register_t fan_pin_port;
static pin_mask_t fan_pin_mask;

static bool acIsEnabled;
static bool acStandAloneFanIsEnabled;
static uint8_t acStartDelay;
static uint8_t acTPSLockoutDelay;
static uint8_t acRPMLockoutDelay;
static uint8_t acAfterEngineStartDelay;
static bool waitedAfterCranking;

#if defined(PWM_FAN_AVAILABLE)
volatile bool fan_pwm_state;
uint16_t fan_pwm_max_count;
volatile unsigned int fan_pwm_cur_value;
static long fan_pwm_value;
#endif

constexpr table2D_u8_u8_4 fanPWMTable(&configPage6.fanPWMBins, &configPage9.PWMFanDuty);

static uint8_t getAirConRequestPinMode(const config15 &page15)
{
  if(page15.airConReqPol)
  {
    return INPUT;
  }
  return INPUT_PULLUP;
}

static void initAirConRequestPin(const config15 &page15, uint8_t pin)
{
  pinMode(pin, getAirConRequestPinMode(page15));
  aircon_req_pin_port = portInputRegister(digitalPinToPort(pin));
  aircon_req_pin_mask = digitalPinToBitMask(pin);
}

#define READ_AIRCON_REQ_PIN()    ((*aircon_req_pin_port & aircon_req_pin_mask) ? true : false)

#if defined(CORE_TEENSY) || defined(CORE_STM32)
#define AIRCON_PIN_LOW()        (digitalWrite(pinAirConComp, LOW))
#define AIRCON_PIN_HIGH()       (digitalWrite(pinAirConComp, HIGH))
#define AIRCON_FAN_PIN_LOW()    (digitalWrite(pinAirConFan, LOW))
#define AIRCON_FAN_PIN_HIGH()   (digitalWrite(pinAirConFan, HIGH))
#define FAN_PIN_LOW()           (digitalWrite(pinFan, LOW))
#define FAN_PIN_HIGH()          (digitalWrite(pinFan, HIGH))

static void initAirConCompressorPin(uint8_t pin) { pinMode(pin, OUTPUT); }
static void initAirConFanPin(uint8_t pin) { pinMode(pin, OUTPUT); }
static void initialiseFanPin(uint8_t pin) { UNUSED(pin); }
#else
#define AIRCON_PIN_LOW()        *aircon_comp_pin_port &= ~(aircon_comp_pin_mask)
#define AIRCON_PIN_HIGH()       *aircon_comp_pin_port |= (aircon_comp_pin_mask)
#define AIRCON_FAN_PIN_LOW()    *aircon_fan_pin_port &= ~(aircon_fan_pin_mask)
#define AIRCON_FAN_PIN_HIGH()   *aircon_fan_pin_port |= (aircon_fan_pin_mask)
#define FAN_PIN_LOW()           *fan_pin_port &= ~(fan_pin_mask)
#define FAN_PIN_HIGH()          *fan_pin_port |= (fan_pin_mask)

static void initAirConCompressorPin(uint8_t pin)
{
  pinMode(pin, OUTPUT);
  aircon_comp_pin_port = portOutputRegister(digitalPinToPort(pin));
  aircon_comp_pin_mask = digitalPinToBitMask(pin);
}

static void initAirConFanPin(uint8_t pin)
{
  pinMode(pin, OUTPUT);
  aircon_fan_pin_port = portOutputRegister(digitalPinToPort(pin));
  aircon_fan_pin_mask = digitalPinToBitMask(pin);
}

static void initialiseFanPin(uint8_t pin)
{
  fan_pin_port = portOutputRegister(digitalPinToPort(pin));
  fan_pin_mask = digitalPinToBitMask(pin);
}
#endif

#define AIRCON_ON()             ATOMIC() { ((((configPage15.airConCompPol)==1)) ? AIRCON_PIN_LOW() : AIRCON_PIN_HIGH()); currentAdvancedEngineStatus.aircon_compressor_on = true; }
#define AIRCON_OFF()            ATOMIC() { ((((configPage15.airConCompPol)==1)) ? AIRCON_PIN_HIGH() : AIRCON_PIN_LOW()); currentAdvancedEngineStatus.aircon_compressor_on = false; }
#define AIRCON_FAN_ON()         ATOMIC() { ((((configPage15.airConFanPol)==1)) ? AIRCON_FAN_PIN_LOW() : AIRCON_FAN_PIN_HIGH()); currentAdvancedEngineStatus.aircon_fan_on = true; }
#define AIRCON_FAN_OFF()        ATOMIC() { ((((configPage15.airConFanPol)==1)) ? AIRCON_FAN_PIN_HIGH() : AIRCON_FAN_PIN_LOW()); currentAdvancedEngineStatus.aircon_fan_on = false; }

static inline void checkAirConCoolantLockout(void)
{
  const int offTemp = temperatureRemoveOffset(configPage15.airConClTempCut);
  if (currentStatus.coolant > offTemp)
  {
    currentAdvancedEngineStatus.aircon_clt_lockout = true;
  }
  else if (currentStatus.coolant < (offTemp - 1))
  {
    currentAdvancedEngineStatus.aircon_clt_lockout = false;
  }
}

static inline void checkAirConTPSLockout(void)
{
  if (currentStatus.TPS > configPage15.airConTPSCut)
  {
    currentAdvancedEngineStatus.aircon_tps_lockout = true;
    acTPSLockoutDelay = 0;
  }
  else if (currentAdvancedEngineStatus.aircon_tps_lockout && (currentStatus.TPS <= configPage15.airConTPSCut))
  {
    if (acTPSLockoutDelay >= configPage15.airConTPSCutTime)
    {
      currentAdvancedEngineStatus.aircon_tps_lockout = false;
    }
    else
    {
      acTPSLockoutDelay++;
    }
  }
  else
  {
    acTPSLockoutDelay = 0;
  }
}

static inline void checkAirConRPMLockout(void)
{
  if ((currentStatus.RPM < (configPage15.airConMinRPMdiv10 * 10)) ||
      (currentStatus.RPMdiv100 > configPage15.airConMaxRPMdiv100))
  {
    currentAdvancedEngineStatus.aircon_rpm_lockout = true;
    acRPMLockoutDelay = 0;
  }
  else if ((currentStatus.RPM >= (configPage15.airConMinRPMdiv10 * 10)) &&
           (currentStatus.RPMdiv100 <= configPage15.airConMaxRPMdiv100))
  {
    if (acRPMLockoutDelay >= configPage15.airConRPMCutTime)
    {
      currentAdvancedEngineStatus.aircon_rpm_lockout = false;
    }
    else
    {
      acRPMLockoutDelay++;
    }
  }
  else
  {
    acRPMLockoutDelay = 0;
  }
}

static bool readAirConRequest(void)
{
  if (!acIsEnabled)
  {
    return false;
  }

  currentAdvancedEngineStatus.aircon_requested = (READ_AIRCON_REQ_PIN() == configPage15.airConReqPol);
  return currentAdvancedEngineStatus.aircon_requested;
}

void initialiseAirCon(void)
{
  if ((configPage15.airConEnable) &&
      !pinIsReserved(pinAirConRequest) &&
      !pinIsReserved(pinAirConComp) &&
      !pinIsOutput(pinAirConRequest))
  {
    acAfterEngineStartDelay = 0;
    waitedAfterCranking = false;
    acStartDelay = 0;
    acTPSLockoutDelay = 0;
    acRPMLockoutDelay = 0;

    currentAdvancedEngineStatus.aircon_requested = false;
    currentAdvancedEngineStatus.aircon_compressor_on = false;
    currentAdvancedEngineStatus.aircon_rpm_lockout = false;
    currentAdvancedEngineStatus.aircon_tps_lockout = false;
    currentAdvancedEngineStatus.aircon_turning_on = false;
    currentAdvancedEngineStatus.aircon_clt_lockout = false;
    currentAdvancedEngineStatus.aircon_fan_on = false;

    initAirConRequestPin(configPage15, pinAirConRequest);
    initAirConCompressorPin(pinAirConComp);
    AIRCON_OFF();

    if ((configPage15.airConFanEnabled) && (pinIsReserved(pinAirConFan)))
    {
      initAirConFanPin(pinAirConFan);
      AIRCON_FAN_OFF();
      acStandAloneFanIsEnabled = true;
    }
    else
    {
      acStandAloneFanIsEnabled = false;
    }

    acIsEnabled = true;
  }
  else
  {
    acIsEnabled = false;
  }
}

void airConControl(void)
{
  if (!acIsEnabled)
  {
    return;
  }

  if (currentStatus.engineIsRunning)
  {
    if (acAfterEngineStartDelay >= configPage15.airConAfterStartDelay)
    {
      waitedAfterCranking = true;
    }
    else
    {
      acAfterEngineStartDelay++;
    }
  }
  else
  {
    acAfterEngineStartDelay = 0;
    waitedAfterCranking = false;
  }

  checkAirConCoolantLockout();
  checkAirConTPSLockout();
  checkAirConRPMLockout();

  if (readAirConRequest() &&
      waitedAfterCranking &&
      !currentAdvancedEngineStatus.aircon_tps_lockout &&
      !currentAdvancedEngineStatus.aircon_rpm_lockout &&
      !currentAdvancedEngineStatus.aircon_clt_lockout)
  {
    currentAdvancedEngineStatus.aircon_turning_on = true;

    if (acStandAloneFanIsEnabled)
    {
      AIRCON_FAN_ON();
    }

    if (acStartDelay >= configPage15.airConCompOnDelay)
    {
      AIRCON_ON();
    }
    else
    {
      acStartDelay++;
    }
  }
  else
  {
    currentAdvancedEngineStatus.aircon_turning_on = false;

    if (acStandAloneFanIsEnabled)
    {
      AIRCON_FAN_OFF();
    }

    AIRCON_OFF();
    acStartDelay = 0;
  }
}

void fanOn(void)
{
  ATOMIC() {
    ((configPage6.fanInv) ? FAN_PIN_LOW() : FAN_PIN_HIGH());
  }
}

void fanOff(void)
{
  ATOMIC() {
    ((configPage6.fanInv) ? FAN_PIN_HIGH() : FAN_PIN_LOW());
  }
}

void initialiseFan(uint8_t fan_pin)
{
  pinMode(pinFan, OUTPUT);
  initialiseFanPin(fan_pin);
  fanOff();
  currentAdvancedEngineStatus.fan_on = false;
  currentAdvancedEngineStatus.fan_duty = 0;

#if defined(PWM_FAN_AVAILABLE)
  DISABLE_FAN_TIMER();
  if (configPage2.fanEnable == 2)
  {
#if defined(CORE_TEENSY)
    fan_pwm_max_count = (uint16_t)(MICROS_PER_SEC / (32U * configPage6.fanFreq * 2U));
#endif
    fan_pwm_value = 0;
  }
#endif
}

void fanControl(void)
{
  if (configPage2.fanEnable == 1)
  {
    const int onTemp = temperatureRemoveOffset(configPage6.fanSP);
    const int offTemp = onTemp - configPage6.fanHyster;
    const bool fanPermit = configPage2.fanWhenOff ? true : currentStatus.engineIsRunning;

    if (fanPermit &&
        ((currentStatus.coolant >= onTemp) ||
         ((configPage15.airConTurnsFanOn) == 1 && currentAdvancedEngineStatus.aircon_turning_on)))
    {
      if (currentStatus.engineIsCranking && (configPage2.fanWhenCranking == 0))
      {
        fanOff();
        currentAdvancedEngineStatus.fan_on = false;
      }
      else
      {
        fanOn();
        currentAdvancedEngineStatus.fan_on = true;
      }
    }
    else if ((currentStatus.coolant <= offTemp) || (!fanPermit))
    {
      fanOff();
      currentAdvancedEngineStatus.fan_on = false;
    }
  }
  else if (configPage2.fanEnable == 2)
  {
    const bool fanPermit = configPage2.fanWhenOff ? true : currentStatus.engineIsRunning;
    if (fanPermit)
    {
      if (currentStatus.engineIsCranking && (configPage2.fanWhenCranking == 0))
      {
        currentAdvancedEngineStatus.fan_duty = 0;
        currentAdvancedEngineStatus.fan_on = false;
#if defined(PWM_FAN_AVAILABLE)
        DISABLE_FAN_TIMER();
#endif
      }
      else
      {
        byte tempFanDuty = table2D_getValue(&fanPWMTable, temperatureAddOffset(currentStatus.coolant));
        if ((configPage15.airConTurnsFanOn) == 1 && currentAdvancedEngineStatus.aircon_turning_on)
        {
          if (tempFanDuty < configPage15.airConPwmFanMinDuty)
          {
            tempFanDuty = configPage15.airConPwmFanMinDuty;
          }
        }
        currentAdvancedEngineStatus.fan_duty = tempFanDuty;
#if defined(PWM_FAN_AVAILABLE)
        fan_pwm_value = halfPercentage(currentAdvancedEngineStatus.fan_duty, fan_pwm_max_count);
        if (currentAdvancedEngineStatus.fan_duty > 0)
        {
          ENABLE_FAN_TIMER();
          currentAdvancedEngineStatus.fan_on = true;
        }
#endif
      }
    }
    else
    {
      currentAdvancedEngineStatus.fan_duty = 0;
      currentAdvancedEngineStatus.fan_on = false;
    }

#if defined(PWM_FAN_AVAILABLE)
    if (currentAdvancedEngineStatus.fan_duty == 0)
    {
      fanOff();
      currentAdvancedEngineStatus.fan_on = false;
      DISABLE_FAN_TIMER();
    }
    else if (currentAdvancedEngineStatus.fan_duty == 200)
    {
      fanOn();
      currentAdvancedEngineStatus.fan_on = true;
      DISABLE_FAN_TIMER();
    }
#else
    if (currentAdvancedEngineStatus.fan_duty == 0)
    {
      fanOff();
      currentAdvancedEngineStatus.fan_on = false;
    }
    else if (currentAdvancedEngineStatus.fan_duty > 0)
    {
      fanOn();
      currentAdvancedEngineStatus.fan_on = true;
    }
#endif
  }
}

#if defined(PWM_FAN_AVAILABLE)
void fanInterrupt(void)
{
  if (fan_pwm_state == true)
  {
    fanOff();
    FAN_TIMER_COMPARE = FAN_TIMER_COUNTER + (fan_pwm_max_count - fan_pwm_cur_value);
    fan_pwm_state = false;
  }
  else
  {
    fanOn();
    FAN_TIMER_COMPARE = FAN_TIMER_COUNTER + fan_pwm_value;
    fan_pwm_cur_value = fan_pwm_value;
    fan_pwm_state = true;
  }
}
#endif

void advanced_engine_fan_aircon_init(uint8_t fan_pin)
{
  initialiseFan(fan_pin);
  initialiseAirCon();
}

void advanced_engine_fan_aircon_tick_10hz(void)
{
  airConControl();
}

void advanced_engine_fan_aircon_tick_1hz(void)
{
  fanControl();
}
