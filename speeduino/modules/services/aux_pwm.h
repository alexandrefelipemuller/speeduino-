#pragma once

#include <stdint.h>

struct aux_pwm_backend
{
  uint16_t (*get_max_count)();
  void (*set_secondary_pwm_value)(long pwm_value);
  void (*set_secondary_pwm_state)(bool pwm_state);
  void (*set_secondary_max_pwm)(bool max_pwm);
  void (*secondary_on)();
  void (*secondary_off)();
  void (*enable_timer)();
  void (*disable_timer)();
};

void auxPwmRegisterBackend(const aux_pwm_backend &backend);
uint16_t auxPwmGetMaxCount(void);
void auxPwmSetSecondaryPwmValue(long pwm_value);
void auxPwmSetSecondaryPwmState(bool pwm_state);
void auxPwmSetSecondaryMaxPwm(bool max_pwm);
void auxPwmSecondaryOn(void);
void auxPwmSecondaryOff(void);
void auxPwmEnableTimer(void);
void auxPwmDisableTimer(void);
