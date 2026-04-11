#include "modules/fan_aircon/module_fan_aircon.h"

#include "support/preprocessor.h"

#if FEATURE_MODULE_FAN_AIRCON

void module_fan_aircon_init(uint8_t fan_pin)
{
  advanced_engine_fan_aircon_init(fan_pin);
}

void module_fan_aircon_tick_10hz(void)
{
  advanced_engine_fan_aircon_tick_10hz();
}

void module_fan_aircon_tick_1hz(void)
{
  advanced_engine_fan_aircon_tick_1hz();
}

#else

void module_fan_aircon_init(uint8_t) {}
void module_fan_aircon_tick_10hz(void) {}
void module_fan_aircon_tick_1hz(void) {}

#endif
