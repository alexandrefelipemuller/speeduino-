#pragma once

#include <stdint.h>

#include "modules/core/module_registry.h"

constexpr uint8_t boostvvtPage = 7;
constexpr uint8_t warmupPage = 10;
constexpr uint8_t boostvvtPage2 = 15;

void initialiseBoost(uint8_t pin);
void boostControl(void);
void boostDisable(void);
void boostInterrupt(void);

extern uint16_t boost_pwm_max_count;

module_page_maps_t getBoostPageMaps();
module_page_descriptors_t getBoostPageDescriptors();
module_storage_maps_t getBoostStorageMaps();
