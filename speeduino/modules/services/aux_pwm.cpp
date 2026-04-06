#include "modules/services/aux_pwm.h"

static aux_pwm_backend registered_backend = {};

void auxPwmRegisterBackend(const aux_pwm_backend &backend)
{
  registered_backend = backend;
}

uint16_t auxPwmGetMaxCount(void)
{
  if(registered_backend.get_max_count != nullptr) { return registered_backend.get_max_count(); }
  return 0;
}

void auxPwmSetSecondaryPwmValue(long pwm_value)
{
  if(registered_backend.set_secondary_pwm_value != nullptr) { registered_backend.set_secondary_pwm_value(pwm_value); }
}

void auxPwmSetSecondaryPwmState(bool pwm_state)
{
  if(registered_backend.set_secondary_pwm_state != nullptr) { registered_backend.set_secondary_pwm_state(pwm_state); }
}

void auxPwmSetSecondaryMaxPwm(bool max_pwm)
{
  if(registered_backend.set_secondary_max_pwm != nullptr) { registered_backend.set_secondary_max_pwm(max_pwm); }
}

void auxPwmSecondaryOn(void)
{
  if(registered_backend.secondary_on != nullptr) { registered_backend.secondary_on(); }
}

void auxPwmSecondaryOff(void)
{
  if(registered_backend.secondary_off != nullptr) { registered_backend.secondary_off(); }
}

void auxPwmEnableTimer(void)
{
  if(registered_backend.enable_timer != nullptr) { registered_backend.enable_timer(); }
}

void auxPwmDisableTimer(void)
{
  if(registered_backend.disable_timer != nullptr) { registered_backend.disable_timer(); }
}
