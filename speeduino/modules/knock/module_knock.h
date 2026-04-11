#pragma once

#include "modules/core/module_registry.h"

void module_knock_init_post_pin_mapping(void);
void module_knock_tick_10hz(void);
void module_knock_on_engine_stop(void);

module_page_maps_t getKnockPageMaps();
module_page_descriptors_t getKnockPageDescriptors();
module_storage_maps_t getKnockStorageMaps();
