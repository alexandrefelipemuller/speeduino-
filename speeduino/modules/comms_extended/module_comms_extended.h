#pragma once

#include "config9_domains.h"
#include "statuses.h"

void module_comms_extended_init(void);
void module_comms_extended_poll(uint8_t internal_can_enabled, uint8_t can_wbo_enabled);
void module_comms_extended_tick_50hz(void);
void module_comms_extended_tick_30hz(void);
void module_comms_extended_tick_15hz(void);
void module_comms_extended_tick_10hz(void);
void module_comms_extended_tick_4hz(uint8_t sensor_status, statuses &current, const can_extended_config_t &config);
