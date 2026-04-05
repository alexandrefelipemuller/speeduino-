#pragma once

#include "modules/core/module_registry.h"

page_map_t getPageMap(uint8_t pageNumber);
const entity_storage_map_t *getEntityStorageMap();
uint8_t getEntityStorageMapSize();
