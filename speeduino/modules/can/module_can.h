#pragma once

#include "modules/core/module_interfaces.h"

constexpr uint8_t canbusPage = 9;

void module_can_init(void);
void module_can_poll(uint8_t internal_can_enabled, uint8_t can_wbo_enabled);
void module_can_tick_50hz(void);
void module_can_tick_30hz(void);
void module_can_tick_15hz(void);
void module_can_tick_10hz(void);
void module_can_tick_4hz(uint8_t sensor_status, statuses &current, const can_extended_config_t &config);
void module_can_on_engine_stop(void);

module_page_maps_t getCanPageMaps();
module_page_descriptors_t getCanPageDescriptors();
module_storage_maps_t getCanStorageMaps();
