#pragma once

#include <stdint.h>

#include "modules/core/module_registry.h"

constexpr uint8_t etbPage = 16U;
constexpr uint8_t etbCurvePoints = 16U;

constexpr uint8_t ETB_FAULT_DISABLED = 0U;
constexpr uint8_t ETB_FAULT_CONFIG = 1U;
constexpr uint8_t ETB_FAULT_PEDAL = 2U;
constexpr uint8_t ETB_FAULT_THROTTLE = 3U;
constexpr uint8_t ETB_FAULT_MISMATCH = 4U;

void module_etb_init_post_pin_mapping(void);
void module_etb_tick_200hz(void);
void module_etb_on_engine_stop(void);

module_page_maps_t getEtbPageMaps();
module_page_descriptors_t getEtbPageDescriptors();
module_storage_maps_t getEtbStorageMaps();
