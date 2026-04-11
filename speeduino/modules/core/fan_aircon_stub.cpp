#include "modules/core/module_interfaces.h"

#include "engine/auxiliaries.h"
#include "support/preprocessor.h"

#if !FEATURE_MODULE_FAN_AIRCON

uint16_t fan_pwm_max_count;

void initialiseFan(uint8_t)
{
}

void initialiseAirCon(void)
{
}

void fanControl(void)
{
}

void airConControl(void)
{
}

void fanOn(void)
{
}

void fanOff(void)
{
}

void fanInterrupt(void)
{
}

void module_fan_aircon_init(uint8_t)
{
}

void module_fan_aircon_tick_10hz(void)
{
}

void module_fan_aircon_tick_1hz(void)
{
}

#endif
