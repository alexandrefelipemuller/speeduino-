#pragma once

#include "pages.h"

struct page_map_t
{
  const entity_t *searchMap;
  uint8_t mapSize;
};

struct entity_storage_map_t
{
  const void *pEntity;
  uint16_t eepromStartAddress;
};

struct module_page_maps_t
{
  const page_map_t *maps;
  uint8_t count;
};

struct module_storage_maps_t
{
  const entity_storage_map_t *maps;
  uint8_t count;
};

module_page_maps_t getCorePageMaps();
module_storage_maps_t getCoreStorageMaps();

module_page_maps_t getAdvancedEnginePageMaps();
module_storage_maps_t getAdvancedEngineStorageMaps();

module_page_maps_t getTableSwitchingPageMaps();
module_storage_maps_t getTableSwitchingStorageMaps();

module_page_maps_t getCommsExtendedPageMaps();
module_storage_maps_t getCommsExtendedStorageMaps();

module_page_maps_t getLoggingPageMaps();
module_storage_maps_t getLoggingStorageMaps();
