#pragma once

#include "modules/fan_aircon/fan_aircon.h"

void module_fan_aircon_init(uint8_t fan_pin);
void module_fan_aircon_tick_10hz(void);
void module_fan_aircon_tick_1hz(void);
