#include "modules/boost/boost.h"

#include "data/advanced_engine_status.h"
#include "data/pin_registry.h"
#include "support/atomic.h"
#include "boards/board_definition.h"
#include "support/maths.h"
#include "support/port_pin.h"
#include "data/runtime_state.h"
#include "src/PID_v1/PID_v1.h"
#include "support/table2d.h"
#include "data/table_registry.h"
#include "data/tune_registry.h"
#include "support/utilities.h"

constexpr uint8_t SIMPLE_BOOST_P = 1U;
constexpr uint8_t SIMPLE_BOOST_I = 1U;
constexpr uint8_t SIMPLE_BOOST_D = 1U;

static byte boostCounter;

#if defined(CORE_TEENSY) || defined(CORE_STM32)

#define BOOST_PIN_LOW()         (digitalWrite(pinBoost, LOW))
#define BOOST_PIN_HIGH()        (digitalWrite(pinBoost, HIGH))

static void initializeBoostPin(uint8_t pin)
{
  pinMode(pin, OUTPUT);
}

#else

static port_register_t boost_pin_port;
static pin_mask_t boost_pin_mask;

#define BOOST_PIN_LOW()         ATOMIC() { *boost_pin_port &= ~(boost_pin_mask); }
#define BOOST_PIN_HIGH()        ATOMIC() { *boost_pin_port |= (boost_pin_mask);  }

static void initializeBoostPin(uint8_t pin)
{
  pinMode(pin, OUTPUT);
  boost_pin_port = portOutputRegister(digitalPinToPort(pin));
  boost_pin_mask = digitalPinToBitMask(pin);
}

#endif

static long boost_pwm_target_value;
static volatile bool boost_pwm_state;
static volatile unsigned int boost_pwm_cur_value = 0;

uint16_t boost_pwm_max_count;

constexpr table2D_u8_s16_6 flexBoostTable(&configPage10.flexBoostBins, &configPage10.flexBoostAdj);

static integerPID_ideal boostPID(
  &currentStatus.MAP,
  &currentAdvancedEngineStatus.boost_duty,
  &currentAdvancedEngineStatus.boost_target,
  &configPage10.boostSens,
  &configPage10.boostIntv,
  configPage6.boostKP,
  configPage6.boostKI,
  configPage6.boostKD,
  DIRECT);

void initialiseBoost(uint8_t pin)
{
  initializeBoostPin(pin);
  boostPID.SetOutputLimits(configPage2.boostMinDuty, configPage2.boostMaxDuty);
  if(configPage6.boostMode == BOOST_MODE_SIMPLE) { boostPID.SetTunings(SIMPLE_BOOST_P, SIMPLE_BOOST_I, SIMPLE_BOOST_D); }
  else { boostPID.SetTunings(configPage6.boostKP, configPage6.boostKI, configPage6.boostKD); }

  currentAdvancedEngineStatus.boost_duty = 0;
  boostCounter = 0;
}

static void boostByGear(void)
{
  if(configPage4.boostType == OPEN_LOOP_BOOST)
  {
    if(configPage9.boostByGearEnabled == 1)
    {
      uint16_t combinedBoost = 0;
      switch (currentStatus.gear)
      {
        case 1: combinedBoost = (((uint16_t)configPage9.boostByGear1 * (uint16_t)get3DTableValue(&boostTable, (currentStatus.TPS * 2U), currentStatus.RPM))) << 2; break;
        case 2: combinedBoost = (((uint16_t)configPage9.boostByGear2 * (uint16_t)get3DTableValue(&boostTable, (currentStatus.TPS * 2U), currentStatus.RPM))) << 2; break;
        case 3: combinedBoost = (((uint16_t)configPage9.boostByGear3 * (uint16_t)get3DTableValue(&boostTable, (currentStatus.TPS * 2U), currentStatus.RPM))) << 2; break;
        case 4: combinedBoost = (((uint16_t)configPage9.boostByGear4 * (uint16_t)get3DTableValue(&boostTable, (currentStatus.TPS * 2U), currentStatus.RPM))) << 2; break;
        case 5: combinedBoost = (((uint16_t)configPage9.boostByGear5 * (uint16_t)get3DTableValue(&boostTable, (currentStatus.TPS * 2U), currentStatus.RPM))) << 2; break;
        case 6: combinedBoost = (((uint16_t)configPage9.boostByGear6 * (uint16_t)get3DTableValue(&boostTable, (currentStatus.TPS * 2U), currentStatus.RPM))) << 2; break;
        default: return;
      }
      currentAdvancedEngineStatus.boost_duty = (combinedBoost <= 10000U) ? combinedBoost : 10000U;
    }
    else if(configPage9.boostByGearEnabled == 2)
    {
      switch (currentStatus.gear)
      {
        case 1: currentAdvancedEngineStatus.boost_duty = configPage9.boostByGear1 * 2 * 100; break;
        case 2: currentAdvancedEngineStatus.boost_duty = configPage9.boostByGear2 * 2 * 100; break;
        case 3: currentAdvancedEngineStatus.boost_duty = configPage9.boostByGear3 * 2 * 100; break;
        case 4: currentAdvancedEngineStatus.boost_duty = configPage9.boostByGear4 * 2 * 100; break;
        case 5: currentAdvancedEngineStatus.boost_duty = configPage9.boostByGear5 * 2 * 100; break;
        case 6: currentAdvancedEngineStatus.boost_duty = configPage9.boostByGear6 * 2 * 100; break;
        default: break;
      }
    }
  }
  else if(configPage4.boostType == CLOSED_LOOP_BOOST)
  {
    if(configPage9.boostByGearEnabled == 1)
    {
      uint16_t combinedBoost = 0;
      switch (currentStatus.gear)
      {
        case 1: combinedBoost = ((((uint16_t)configPage9.boostByGear1 * (uint16_t)get3DTableValue(&boostTable, (currentStatus.TPS * 2U), currentStatus.RPM)) / 100)) << 2; break;
        case 2: combinedBoost = ((((uint16_t)configPage9.boostByGear2 * (uint16_t)get3DTableValue(&boostTable, (currentStatus.TPS * 2U), currentStatus.RPM)) / 100)) << 2; break;
        case 3: combinedBoost = ((((uint16_t)configPage9.boostByGear3 * (uint16_t)get3DTableValue(&boostTable, (currentStatus.TPS * 2U), currentStatus.RPM)) / 100)) << 2; break;
        case 4: combinedBoost = ((((uint16_t)configPage9.boostByGear4 * (uint16_t)get3DTableValue(&boostTable, (currentStatus.TPS * 2U), currentStatus.RPM)) / 100)) << 2; break;
        case 5: combinedBoost = ((((uint16_t)configPage9.boostByGear5 * (uint16_t)get3DTableValue(&boostTable, (currentStatus.TPS * 2U), currentStatus.RPM)) / 100)) << 2; break;
        case 6: combinedBoost = ((((uint16_t)configPage9.boostByGear6 * (uint16_t)get3DTableValue(&boostTable, (currentStatus.TPS * 2U), currentStatus.RPM)) / 100)) << 2; break;
        default: return;
      }
      currentAdvancedEngineStatus.boost_target = (combinedBoost <= 511U) ? combinedBoost : 511U;
    }
    else if(configPage9.boostByGearEnabled == 2)
    {
      switch (currentStatus.gear)
      {
        case 1: currentAdvancedEngineStatus.boost_target = (configPage9.boostByGear1 << 1); break;
        case 2: currentAdvancedEngineStatus.boost_target = (configPage9.boostByGear2 << 1); break;
        case 3: currentAdvancedEngineStatus.boost_target = (configPage9.boostByGear3 << 1); break;
        case 4: currentAdvancedEngineStatus.boost_target = (configPage9.boostByGear4 << 1); break;
        case 5: currentAdvancedEngineStatus.boost_target = (configPage9.boostByGear5 << 1); break;
        case 6: currentAdvancedEngineStatus.boost_target = (configPage9.boostByGear6 << 1); break;
        default: break;
      }
    }
  }
}

void boostControl(void)
{
  if(configPage6.boostEnabled == 1)
  {
    if(configPage4.boostType == OPEN_LOOP_BOOST)
    {
      if((configPage9.boostByGearEnabled > 0) && isExternalVssMode(configPage2)) { boostByGear(); }
      else { currentAdvancedEngineStatus.boost_duty = get3DTableValue(&boostTable, (currentStatus.TPS * 2U), currentStatus.RPM) * 2 * 100; }

      if(currentAdvancedEngineStatus.boost_duty > 10000) { currentAdvancedEngineStatus.boost_duty = 10000; }
      if(currentAdvancedEngineStatus.boost_duty == 0) { DISABLE_BOOST_TIMER(); BOOST_PIN_LOW(); }
      else { boost_pwm_target_value = ((unsigned long)(currentAdvancedEngineStatus.boost_duty) * boost_pwm_max_count) / 10000; }
    }
    else if(configPage4.boostType == CLOSED_LOOP_BOOST)
    {
      if((boostCounter & 7) == 1)
      {
        if((configPage9.boostByGearEnabled > 0) && isExternalVssMode(configPage2)) { boostByGear(); }
        else { currentAdvancedEngineStatus.boost_target = get3DTableValue(&boostTable, (currentStatus.TPS * 2U), currentStatus.RPM) << 1; }

        if(configPage2.flexEnabled == 1)
        {
          currentAdvancedEngineStatus.flex_boost_correction = table2D_getValue(&flexBoostTable, currentStatus.ethanolPct);
          currentAdvancedEngineStatus.boost_target += currentAdvancedEngineStatus.flex_boost_correction;
          currentAdvancedEngineStatus.boost_target = min(currentAdvancedEngineStatus.boost_target, (uint16_t)511U);
        }
        else
        {
          currentAdvancedEngineStatus.flex_boost_correction = 0;
        }
      }

      if(((configPage15.boostControlEnable == EN_BOOST_CONTROL_BARO) && (currentStatus.MAP >= currentStatus.baro)) ||
         ((configPage15.boostControlEnable == EN_BOOST_CONTROL_FIXED) && (currentStatus.MAP >= configPage15.boostControlEnableThreshold)))
      {
        if(currentAdvancedEngineStatus.boost_target > 0)
        {
          if((boostCounter & 15) == 1)
          {
            boostPID.SetOutputLimits(configPage2.boostMinDuty, configPage2.boostMaxDuty);
            if(configPage6.boostMode == BOOST_MODE_SIMPLE) { boostPID.SetTunings(SIMPLE_BOOST_P, SIMPLE_BOOST_I, SIMPLE_BOOST_D); }
            else { boostPID.SetTunings(configPage6.boostKP, configPage6.boostKI, configPage6.boostKD); }
          }

          const bool pid_computed = boostPID.Compute(get3DTableValue(&boostTableLookupDuty, currentAdvancedEngineStatus.boost_target, currentStatus.RPM) * 100 / 2);
          if(currentAdvancedEngineStatus.boost_duty == 0) { DISABLE_BOOST_TIMER(); BOOST_PIN_LOW(); }
          else if(pid_computed) { boost_pwm_target_value = ((unsigned long)(currentAdvancedEngineStatus.boost_duty) * boost_pwm_max_count) / 10000; }
        }
        else
        {
          boostDisable();
        }
      }
      else
      {
        boostPID.Initialize();
        currentAdvancedEngineStatus.boost_duty = configPage15.boostDCWhenDisabled * 100;
        boost_pwm_target_value = ((unsigned long)(currentAdvancedEngineStatus.boost_duty) * boost_pwm_max_count) / 10000;
        ENABLE_BOOST_TIMER();
        if(currentAdvancedEngineStatus.boost_duty == 0) { boostDisable(); }
      }
    }

    if(currentAdvancedEngineStatus.boost_duty >= 10000)
    {
      DISABLE_BOOST_TIMER();
      BOOST_PIN_HIGH();
    }
    else if(currentAdvancedEngineStatus.boost_duty > 0)
    {
      ENABLE_BOOST_TIMER();
    }
  }
  else
  {
    DISABLE_BOOST_TIMER();
    currentAdvancedEngineStatus.flex_boost_correction = 0;
  }

  boostCounter++;
}

void boostDisable(void)
{
  boostPID.Initialize();
  currentAdvancedEngineStatus.boost_duty = 0;
  DISABLE_BOOST_TIMER();
  BOOST_PIN_LOW();
}

void boostInterrupt(void)
{
  if(boost_pwm_state == true)
  {
    #if defined(CORE_TEENSY41)
    BOOST_PIN_HIGH();
    #else
    BOOST_PIN_LOW();
    #endif
    SET_COMPARE(BOOST_TIMER_COMPARE, BOOST_TIMER_COUNTER + (boost_pwm_max_count - boost_pwm_cur_value));
    boost_pwm_state = false;
  }
  else
  {
    #if defined(CORE_TEENSY41)
    BOOST_PIN_LOW();
    #else
    BOOST_PIN_HIGH();
    #endif
    SET_COMPARE(BOOST_TIMER_COMPARE, BOOST_TIMER_COUNTER + boost_pwm_target_value);
    boost_pwm_cur_value = boost_pwm_target_value;
    boost_pwm_state = true;
  }
}
