#pragma once

#include "data/statuses.h"

void core_modules_init_pre_pin_mapping(void);
void core_modules_init_post_pin_mapping(void);
void core_modules_poll(void);
void core_modules_on_engine_stop(void);
void core_modules_tick_50hz(void);
void core_modules_tick_30hz(void);
void core_modules_tick_15hz(void);
void core_modules_tick_10hz(void);
void core_modules_tick_4hz(uint8_t sensor_status, statuses &current);
void core_modules_tick_1hz(const statuses &current);
void core_modules_apply_table_switching(statuses &current);
statuses::scheduler_cut_t core_modules_get_scheduler_cut(statuses &current);
