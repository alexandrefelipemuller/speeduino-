#include "modules/vvt/vvt.h"

#include "data/advanced_engine_status.h"
#include "engine/auxiliaries.h"
#include "boards/board_definition.h"
#include "engine/decoders.h"
#include "support/maths.h"
#include "modules/services/aux_pwm.h"
#include "data/pin_registry.h"
#include "support/port_pin.h"
#include "data/runtime_state.h"
#include "src/PID_v1/PID_v1.h"
#include "data/table_registry.h"
#include "data/tune_registry.h"
#include "support/units.h"

static long vvt1_pwm_value;
static long vvt2_pwm_value;
static volatile unsigned int vvt1_pwm_cur_value;
static volatile unsigned int vvt2_pwm_cur_value;
static long vvt_pid_target_angle;
static long vvt2_pid_target_angle;
static long vvt_pid_current_angle;
static long vvt2_pid_current_angle;
static volatile bool vvt1_pwm_state;
static volatile bool vvt2_pwm_state;
static volatile bool vvt1_max_pwm;
static volatile bool vvt2_max_pwm;
static volatile char nextVVT;
static byte vvtCounter;
static uint32_t vvtWarmTime;
static bool vvtIsHot;
static bool vvtTimeHold;

uint16_t vvt_pwm_max_count;

static integerPID vvtPID(&vvt_pid_current_angle, &currentAdvancedEngineStatus.vvt1_duty, &vvt_pid_target_angle, configPage10.vvtCLKP, configPage10.vvtCLKI, configPage10.vvtCLKD, configPage6.vvtPWMdir);
static integerPID vvt2PID(&vvt2_pid_current_angle, &currentAdvancedEngineStatus.vvt2_duty, &vvt2_pid_target_angle, configPage10.vvtCLKP, configPage10.vvtCLKI, configPage10.vvtCLKD, configPage4.vvt2PWMdir);

#define VVT_TIME_DELAY_MULTIPLIER  50

static inline uint16_t calculateVvtPwmMaxCount(uint8_t configuredFrequency)
{
  uint8_t frequency = (configuredFrequency == 0U) ? 1U : configuredFrequency;
#if defined(CORE_AVR)
  const uint32_t result = MICROS_PER_SEC / (16U * (uint32_t)frequency * 2U);
#elif defined(CORE_TEENSY35)
  const uint32_t result = MICROS_PER_SEC / (32U * (uint32_t)frequency * 2U);
#elif defined(CORE_TEENSY41)
  const uint32_t result = MICROS_PER_SEC / (2U * (uint32_t)frequency * 2U);
#else
  const uint32_t result = MICROS_PER_SEC / (2U * (uint32_t)frequency * 2U);
#endif
  return (result > UINT16_MAX) ? UINT16_MAX : (uint16_t)result;
}

#if(defined(CORE_TEENSY) || defined(CORE_STM32))

#define VVT1_PIN_LOW()          (digitalWrite(pinVVT_1, LOW))
#define VVT1_PIN_HIGH()         (digitalWrite(pinVVT_1, HIGH))
#define VVT2_PIN_LOW()          (digitalWrite(pinVVT_2, LOW))
#define VVT2_PIN_HIGH()         (digitalWrite(pinVVT_2, HIGH))

static inline void initialiseVvtPins(uint8_t pin1, uint8_t pin2)
{
  pinMode(pin1, OUTPUT);
  pinMode(pin2, OUTPUT);
}

#else

static port_register_t vvt1_pin_port;
static pin_mask_t vvt1_pin_mask;
static port_register_t vvt2_pin_port;
static pin_mask_t vvt2_pin_mask;

#define VVT1_PIN_LOW()          ATOMIC() { *vvt1_pin_port &= ~(vvt1_pin_mask);   }
#define VVT1_PIN_HIGH()         ATOMIC() { *vvt1_pin_port |= (vvt1_pin_mask);    }
#define VVT2_PIN_LOW()          ATOMIC() { *vvt2_pin_port &= ~(vvt2_pin_mask);   }
#define VVT2_PIN_HIGH()         ATOMIC() { *vvt2_pin_port |= (vvt2_pin_mask);    }

static inline void initialiseVvtPins(uint8_t pin1, uint8_t pin2)
{
  pinMode(pin1, OUTPUT);
  vvt1_pin_port = portOutputRegister(digitalPinToPort(pin1));
  vvt1_pin_mask = digitalPinToBitMask(pin1);
  pinMode(pin2, OUTPUT);
  vvt2_pin_port = portOutputRegister(digitalPinToPort(pin2));
  vvt2_pin_mask = digitalPinToBitMask(pin2);
}

#endif

void vvt1On(void) { VVT1_PIN_HIGH(); }
void vvt1Off(void) { VVT1_PIN_LOW(); }
void vvt2On(void) { VVT2_PIN_HIGH(); }
void vvt2Off(void) { VVT2_PIN_LOW(); }

uint16_t vvtGetPwmMaxCount(void)
{
  return vvt_pwm_max_count;
}

void vvtSetVvt2PwmValue(long pwm_value)
{
  vvt2_pwm_value = pwm_value;
}

void vvtSetVvt2PwmState(bool pwm_state)
{
  vvt2_pwm_state = pwm_state;
}

void vvtSetVvt2MaxPwm(bool max_pwm)
{
  vvt2_max_pwm = max_pwm;
}

static void auxPwmEnableVvtTimer(void)
{
  ENABLE_VVT_TIMER();
}

static void auxPwmDisableVvtTimer(void)
{
  DISABLE_VVT_TIMER();
}

void initialiseVVT(uint8_t pin1, uint8_t pin2)
{
  static const aux_pwm_backend aux_pwm_vvt_backend = {
    &vvtGetPwmMaxCount,
    &vvtSetVvt2PwmValue,
    &vvtSetVvt2PwmState,
    &vvtSetVvt2MaxPwm,
    &vvt2On,
    &vvt2Off,
    &auxPwmEnableVvtTimer,
    &auxPwmDisableVvtTimer
  };
  auxPwmRegisterBackend(aux_pwm_vvt_backend);

  initialiseVvtPins(pin1, pin2);

  if(configPage6.vvtEnabled > 0)
  {
    currentAdvancedEngineStatus.vvt1_angle = 0;
    currentAdvancedEngineStatus.vvt2_angle = 0;

    vvt_pwm_max_count = calculateVvtPwmMaxCount(configPage6.vvtFreq);

    if(configPage6.vvtMode == VVT_MODE_CLOSED_LOOP)
    {
      vvtPID.SetOutputLimits(configPage10.vvtCLminDuty, configPage10.vvtCLmaxDuty);
      vvtPID.SetTunings(configPage10.vvtCLKP, configPage10.vvtCLKI, configPage10.vvtCLKD);
      vvtPID.SetSampleTime(33);
      vvtPID.SetMode(AUTOMATIC);
      if(configPage10.vvt2Enabled == 1)
      {
        vvt2PID.SetOutputLimits(configPage10.vvtCLminDuty, configPage10.vvtCLmaxDuty);
        vvt2PID.SetTunings(configPage10.vvtCLKP, configPage10.vvtCLKI, configPage10.vvtCLKD);
        vvt2PID.SetSampleTime(33);
        vvt2PID.SetMode(AUTOMATIC);
      }
    }

    vvt1_pwm_value = 0;
    vvt2_pwm_value = 0;
    ENABLE_VVT_TIMER();
    currentAdvancedEngineStatus.vvt1_angle_error = false;
    currentAdvancedEngineStatus.vvt2_angle_error = false;
    vvtTimeHold = false;
    if(currentStatus.coolant >= temperatureRemoveOffset(configPage4.vvtMinClt)) { vvtIsHot = true; }
  }

  if((configPage6.vvtEnabled == 0) && (configPage10.wmiEnabled >= 1))
  {
    vvt_pwm_max_count = calculateVvtPwmMaxCount(configPage6.vvtFreq);
    currentAdvancedEngineStatus.wmi_tank_empty = false;
    currentAdvancedEngineStatus.wmi_pw = 0;
    vvt1_pwm_value = 0;
    vvt2_pwm_value = 0;
    ENABLE_VVT_TIMER();
  }

  currentAdvancedEngineStatus.vvt1_duty = 0;
  currentAdvancedEngineStatus.vvt2_duty = 0;
  vvtCounter = 0;
}

void vvtControl(void)
{
  if((configPage6.vvtEnabled == 1) && (currentStatus.coolant >= temperatureRemoveOffset(configPage4.vvtMinClt)) && (currentStatus.engineIsRunning))
  {
    if(vvtTimeHold == false)
    {
      vvtWarmTime = runSecsX10;
      vvtTimeHold = true;
    }

    if(configPage4.TrigPattern == 9) { currentAdvancedEngineStatus.vvt1_angle = getCamAngle_Miata9905(); }

    if((vvtIsHot == true) || ((runSecsX10 - vvtWarmTime) >= (configPage4.vvtDelay * VVT_TIME_DELAY_MULTIPLIER)))
    {
      vvtIsHot = true;

      if((configPage6.vvtMode == VVT_MODE_OPEN_LOOP) || (configPage6.vvtMode == VVT_MODE_ONOFF))
      {
        if(configPage6.vvtLoadSource == VVT_LOAD_TPS) { currentAdvancedEngineStatus.vvt1_duty = get3DTableValue(&vvtTable, (currentStatus.TPS * 2U), currentStatus.RPM); }
        else { currentAdvancedEngineStatus.vvt1_duty = get3DTableValue(&vvtTable, (uint16_t)currentStatus.MAP, currentStatus.RPM); }
        if((configPage6.vvtMode == VVT_MODE_ONOFF) && (currentAdvancedEngineStatus.vvt1_duty < 200)) { currentAdvancedEngineStatus.vvt1_duty = 0; }
        vvt1_pwm_value = halfPercentage(currentAdvancedEngineStatus.vvt1_duty, vvt_pwm_max_count);

        if(configPage10.vvt2Enabled == 1)
        {
          if(configPage6.vvtLoadSource == VVT_LOAD_TPS) { currentAdvancedEngineStatus.vvt2_duty = get3DTableValue(&vvt2Table, (currentStatus.TPS * 2U), currentStatus.RPM); }
          else { currentAdvancedEngineStatus.vvt2_duty = get3DTableValue(&vvt2Table, (uint16_t)currentStatus.MAP, currentStatus.RPM); }
          if((configPage6.vvtMode == VVT_MODE_ONOFF) && (currentAdvancedEngineStatus.vvt2_duty < 200)) { currentAdvancedEngineStatus.vvt2_duty = 0; }
          vvt2_pwm_value = halfPercentage(currentAdvancedEngineStatus.vvt2_duty, vvt_pwm_max_count);
        }
      }
      else if(configPage6.vvtMode == VVT_MODE_CLOSED_LOOP)
      {
        if(configPage6.vvtLoadSource == VVT_LOAD_TPS) { currentAdvancedEngineStatus.vvt1_target_angle = get3DTableValue(&vvtTable, (currentStatus.TPS * 2U), currentStatus.RPM); }
        else { currentAdvancedEngineStatus.vvt1_target_angle = get3DTableValue(&vvtTable, (uint16_t)currentStatus.MAP, currentStatus.RPM); }

        if((vvtCounter & 31) == 1)
        {
          vvtPID.SetTunings(configPage10.vvtCLKP, configPage10.vvtCLKI, configPage10.vvtCLKD);
          vvtPID.SetControllerDirection(configPage6.vvtPWMdir);
        }

        if(currentAdvancedEngineStatus.vvt1_angle <= configPage10.vvtCLMinAng || currentAdvancedEngineStatus.vvt1_angle > configPage10.vvtCLMaxAng)
        {
          currentAdvancedEngineStatus.vvt1_duty = 0;
          vvt1_pwm_value = halfPercentage(currentAdvancedEngineStatus.vvt1_duty, vvt_pwm_max_count);
          currentAdvancedEngineStatus.vvt1_angle_error = true;
        }
        else if((configPage6.vvtCLUseHold > 0) && (currentAdvancedEngineStatus.vvt1_target_angle == currentAdvancedEngineStatus.vvt1_angle))
        {
          currentAdvancedEngineStatus.vvt1_duty = configPage10.vvtCLholdDuty;
          vvt1_pwm_value = halfPercentage(currentAdvancedEngineStatus.vvt1_duty, vvt_pwm_max_count);
          vvtPID.Initialize();
          currentAdvancedEngineStatus.vvt1_angle_error = false;
        }
        else
        {
          vvt_pid_target_angle = (unsigned long)currentAdvancedEngineStatus.vvt1_target_angle;
          vvt_pid_current_angle = (long)currentAdvancedEngineStatus.vvt1_angle;
          if(vvtPID.Compute(true) == true) { vvt1_pwm_value = halfPercentage(currentAdvancedEngineStatus.vvt1_duty, vvt_pwm_max_count); }
          currentAdvancedEngineStatus.vvt1_angle_error = false;
        }

        if(configPage10.vvt2Enabled == 1)
        {
          if(configPage6.vvtLoadSource == VVT_LOAD_TPS) { currentAdvancedEngineStatus.vvt2_target_angle = get3DTableValue(&vvt2Table, (currentStatus.TPS * 2U), currentStatus.RPM); }
          else { currentAdvancedEngineStatus.vvt2_target_angle = get3DTableValue(&vvt2Table, (uint16_t)currentStatus.MAP, currentStatus.RPM); }

          if((vvtCounter & 31) == 1)
          {
            vvt2PID.SetTunings(configPage10.vvtCLKP, configPage10.vvtCLKI, configPage10.vvtCLKD);
            vvt2PID.SetControllerDirection(configPage4.vvt2PWMdir);
          }

          if(currentAdvancedEngineStatus.vvt2_angle <= configPage10.vvtCLMinAng || currentAdvancedEngineStatus.vvt2_angle > configPage10.vvtCLMaxAng)
          {
            currentAdvancedEngineStatus.vvt2_duty = 0;
            vvt2_pwm_value = halfPercentage(currentAdvancedEngineStatus.vvt2_duty, vvt_pwm_max_count);
            currentAdvancedEngineStatus.vvt2_angle_error = true;
          }
          else if((configPage6.vvtCLUseHold > 0) && (currentAdvancedEngineStatus.vvt2_target_angle == currentAdvancedEngineStatus.vvt2_angle))
          {
            currentAdvancedEngineStatus.vvt2_duty = configPage10.vvtCLholdDuty;
            vvt2_pwm_value = halfPercentage(currentAdvancedEngineStatus.vvt2_duty, vvt_pwm_max_count);
            vvt2PID.Initialize();
            currentAdvancedEngineStatus.vvt2_angle_error = false;
          }
          else
          {
            vvt2_pid_target_angle = (unsigned long)currentAdvancedEngineStatus.vvt2_target_angle;
            vvt2_pid_current_angle = (long)currentAdvancedEngineStatus.vvt2_angle;
            if(vvt2PID.Compute(true) == true) { vvt2_pwm_value = halfPercentage(currentAdvancedEngineStatus.vvt2_duty, vvt_pwm_max_count); }
            currentAdvancedEngineStatus.vvt2_angle_error = false;
          }
        }
        vvtCounter++;
      }

      if(configPage10.wmiEnabled == 0)
      {
        if((currentAdvancedEngineStatus.vvt1_duty == 0) && (currentAdvancedEngineStatus.vvt2_duty == 0))
        {
          vvt1Off();
          vvt2Off();
          vvt1_pwm_state = false;
          vvt1_max_pwm = false;
          vvt2_pwm_state = false;
          vvt2_max_pwm = false;
          DISABLE_VVT_TIMER();
        }
        else if((currentAdvancedEngineStatus.vvt1_duty >= 200) && (currentAdvancedEngineStatus.vvt2_duty >= 200))
        {
          vvt1On();
          vvt2On();
          vvt1_pwm_state = true;
          vvt1_max_pwm = true;
          vvt2_pwm_state = true;
          vvt2_max_pwm = true;
          DISABLE_VVT_TIMER();
        }
        else
        {
          ENABLE_VVT_TIMER();
          if(currentAdvancedEngineStatus.vvt1_duty < 200) { vvt1_max_pwm = false; }
          if(currentAdvancedEngineStatus.vvt2_duty < 200) { vvt2_max_pwm = false; }
        }
      }
      else
      {
        if(currentAdvancedEngineStatus.vvt1_duty == 0)
        {
          vvt1Off();
          vvt1_pwm_state = false;
          vvt1_max_pwm = false;
        }
        else if(currentAdvancedEngineStatus.vvt1_duty >= 200)
        {
          vvt1On();
          vvt1_pwm_state = true;
          vvt1_max_pwm = true;
        }
        else
        {
          ENABLE_VVT_TIMER();
          if(currentAdvancedEngineStatus.vvt1_duty < 200) { vvt1_max_pwm = false; }
        }
      }
    }
  }
  else
  {
    if(configPage10.wmiEnabled == 0)
    {
      DISABLE_VVT_TIMER();
      currentAdvancedEngineStatus.vvt2_duty = 0;
      vvt2_pwm_value = 0;
      vvt2_pwm_state = false;
      vvt2_max_pwm = false;
    }
    currentAdvancedEngineStatus.vvt1_duty = 0;
    vvt1_pwm_value = 0;
    vvt1_pwm_state = false;
    vvt1_max_pwm = false;
    vvtTimeHold = false;
  }
}

void vvtInterrupt(void)
{
  if(((vvt1_pwm_state == false) || (vvt1_max_pwm == true)) && ((vvt2_pwm_state == false) || (vvt2_max_pwm == true)))
  {
    if((vvt1_pwm_value > 0) && (vvt1_max_pwm == false))
    {
      #if defined(CORE_TEENSY41)
      vvt1Off();
      #else
      vvt1On();
      #endif
      vvt1_pwm_state = true;
    }
    if((vvt2_pwm_value > 0) && (vvt2_max_pwm == false))
    {
      #if defined(CORE_TEENSY41)
      vvt2Off();
      #else
      vvt2On();
      #endif
      vvt2_pwm_state = true;
    }

    if((vvt1_pwm_state == true) && ((vvt1_pwm_value <= vvt2_pwm_value) || (vvt2_pwm_state == false)))
    {
      SET_COMPARE(VVT_TIMER_COMPARE, VVT_TIMER_COUNTER + vvt1_pwm_value);
      vvt1_pwm_cur_value = vvt1_pwm_value;
      vvt2_pwm_cur_value = vvt2_pwm_value;
      if(vvt1_pwm_value == vvt2_pwm_value) { nextVVT = 2; }
      else { nextVVT = 0; }
    }
    else if(vvt2_pwm_state == true)
    {
      SET_COMPARE(VVT_TIMER_COMPARE, VVT_TIMER_COUNTER + vvt2_pwm_value);
      vvt1_pwm_cur_value = vvt1_pwm_value;
      vvt2_pwm_cur_value = vvt2_pwm_value;
      nextVVT = 1;
    }
    else
    {
      SET_COMPARE(VVT_TIMER_COMPARE, VVT_TIMER_COUNTER + vvt_pwm_max_count);
    }
  }
  else
  {
    if(nextVVT == 0)
    {
      if(vvt1_pwm_value < (long)vvt_pwm_max_count)
      {
        #if defined(CORE_TEENSY41)
        vvt1On();
        #else
        vvt1Off();
        #endif
        vvt1_pwm_state = false;
        vvt1_max_pwm = false;
      }
      else { vvt1_max_pwm = true; }
      nextVVT = 1;
      if(vvt2_pwm_state == true) { SET_COMPARE(VVT_TIMER_COMPARE, VVT_TIMER_COUNTER + (vvt2_pwm_cur_value - vvt1_pwm_cur_value)); }
      else
      {
        SET_COMPARE(VVT_TIMER_COMPARE, VVT_TIMER_COUNTER + (vvt_pwm_max_count - vvt1_pwm_cur_value));
        nextVVT = 2;
      }
    }
    else if(nextVVT == 1)
    {
      if(vvt2_pwm_value < (long)vvt_pwm_max_count)
      {
        #if defined(CORE_TEENSY41)
        vvt2On();
        #else
        vvt2Off();
        #endif
        vvt2_pwm_state = false;
        vvt2_max_pwm = false;
      }
      else { vvt2_max_pwm = true; }
      nextVVT = 0;
      if(vvt1_pwm_state == true) { SET_COMPARE(VVT_TIMER_COMPARE, VVT_TIMER_COUNTER + (vvt1_pwm_cur_value - vvt2_pwm_cur_value)); }
      else
      {
        SET_COMPARE(VVT_TIMER_COMPARE, VVT_TIMER_COUNTER + (vvt_pwm_max_count - vvt2_pwm_cur_value));
        nextVVT = 2;
      }
    }
    else
    {
      if(vvt1_pwm_value < (long)vvt_pwm_max_count)
      {
        #if defined(CORE_TEENSY41)
        vvt1On();
        #else
        vvt1Off();
        #endif
        vvt1_pwm_state = false;
        vvt1_max_pwm = false;
        SET_COMPARE(VVT_TIMER_COMPARE, VVT_TIMER_COUNTER + (vvt_pwm_max_count - vvt1_pwm_cur_value));
      }
      else { vvt1_max_pwm = true; }
      if(vvt2_pwm_value < (long)vvt_pwm_max_count)
      {
        #if defined(CORE_TEENSY41)
        vvt2On();
        #else
        vvt2Off();
        #endif
        vvt2_pwm_state = false;
        vvt2_max_pwm = false;
        SET_COMPARE(VVT_TIMER_COMPARE, VVT_TIMER_COUNTER + (vvt_pwm_max_count - vvt2_pwm_cur_value));
      }
      else { vvt2_max_pwm = true; }
    }
  }
}
