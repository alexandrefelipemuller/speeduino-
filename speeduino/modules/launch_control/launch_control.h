#pragma once

#include <stdint.h>

void module_launch_control_init_post_pin_mapping(void);
void module_launch_control_tick_10hz(void);
void module_launch_control_on_engine_stop(void);

int8_t launch_control_soft_ignition_correction(int8_t advance);
uint8_t launch_control_fuel_correction(void);
