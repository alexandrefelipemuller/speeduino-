#pragma once

#include "data/config9_domains.h"
#include "data/config_pages.h"
#include "data/statuses.h"
#include "support/table3d.h"

void module_secondary_serial_init(const secondary_serial_config_t &config);
void module_secondary_serial_poll(const secondary_serial_config_t &config);
void module_secondary_serial_tick_30hz(const secondary_serial_config_t &config);

void module_comms_extended_init(void);
void module_comms_extended_poll(uint8_t internal_can_enabled, uint8_t can_wbo_enabled);
void module_comms_extended_tick_50hz(void);
void module_comms_extended_tick_30hz(void);
void module_comms_extended_tick_15hz(void);
void module_comms_extended_tick_10hz(void);
void module_comms_extended_tick_4hz(uint8_t sensor_status, statuses &current, const can_extended_config_t &config);

void module_logging_init(const config13 &page13);
void module_logging_tick_30hz(const config13 &page13);
void module_logging_tick_10hz(const config13 &page13);
void module_logging_tick_4hz(const config13 &page13);
void module_logging_tick_1hz(const statuses &current, const config13 &page13);

void module_table_switching_apply(const config2 &page2, const config10 &page10, const table3d16RpmLoad &fuel_table2, const table3d16RpmLoad &ignition_table2, statuses &current);

void module_advanced_engine_on_engine_stop(const config4 &page4);
void module_fan_aircon_init(uint8_t fan_pin);
void module_fan_aircon_tick_10hz(void);
void module_fan_aircon_tick_1hz(void);
void module_programmable_io_init_post_pin_mapping(void);
void module_programmable_io_tick_10hz(void);
void module_nitrous_init_post_pin_mapping(void);
void module_nitrous_tick_4hz(void);
void module_knock_init_post_pin_mapping(void);
void module_knock_tick_10hz(void);
void module_knock_on_engine_stop(void);
void module_wmi_tick_30hz(void);
void module_launch_flatshift_tick_15hz(void);
void module_advanced_engine_tick_10hz(void);
statuses::scheduler_cut_t module_engine_protection_scheduler_cut(statuses &current, const config2 &page2, const config4 &page4, const config6 &page6, const config9 &page9, const config10 &page10);
