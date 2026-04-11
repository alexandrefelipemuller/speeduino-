#pragma once

#include "storage/pages.h"
#include "data/statuses.h"

struct page_map_t
{
  const entity_t *searchMap;
  uint8_t mapSize;
};

struct page_descriptor_t
{
  uint8_t pageNumber;
  page_map_t map;
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

struct module_page_descriptors_t
{
  const page_descriptor_t *descriptors;
  uint8_t count;
};

struct module_storage_maps_t
{
  const entity_storage_map_t *maps;
  uint8_t count;
};

using module_page_save_hook_t = void (*)(uint16_t &writesRemaining);
using module_page_load_hook_t = void (*)(void);

struct module_page_storage_descriptor_t
{
  uint8_t pageNumber;
  module_page_save_hook_t save;
  module_page_load_hook_t load;
};

struct module_page_storage_t
{
  const module_page_storage_descriptor_t *descriptors;
  uint8_t count;
};

enum class module_hook_phase_t : uint8_t
{
  init_pre_pin_mapping,
  init_post_pin_mapping,
  poll,
  on_engine_stop,
  tick_50hz,
  tick_30hz,
  tick_15hz,
  tick_10hz,
  tick_4hz,
  tick_1hz,
};

struct module_runtime_context_t
{
  uint8_t sensor_status = 0U;
  statuses *current_status = nullptr;
  const statuses *current_status_const = nullptr;
};

using module_hook_t = void (*)(module_runtime_context_t &context);
using module_table_switching_hook_t = void (*)(statuses &current);
using module_scheduler_cut_hook_t = statuses::scheduler_cut_t (*)(statuses &current);

struct module_hook_descriptor_t
{
  module_hook_phase_t phase;
  module_hook_t hook;
};

struct module_hooks_t
{
  const module_hook_descriptor_t *hooks;
  uint8_t count;
};

struct module_descriptor_t
{
  module_page_descriptors_t page_descriptors;
  module_storage_maps_t storage_maps;
  module_page_storage_t page_storage;
  module_hooks_t hooks;
  module_table_switching_hook_t apply_table_switching;
  module_scheduler_cut_hook_t get_scheduler_cut;
};

module_page_maps_t getCorePageMaps();
module_storage_maps_t getCoreStorageMaps();
module_page_descriptors_t getCorePageDescriptors();

module_page_maps_t getBoostPageMaps();
module_storage_maps_t getBoostStorageMaps();
module_page_descriptors_t getBoostPageDescriptors();

module_page_maps_t getKnockPageMaps();
module_storage_maps_t getKnockStorageMaps();
module_page_descriptors_t getKnockPageDescriptors();

module_page_maps_t getAdvancedEnginePageMaps();
module_storage_maps_t getAdvancedEngineStorageMaps();
module_page_descriptors_t getAdvancedEnginePageDescriptors();

module_page_maps_t getWmiPageMaps();
module_storage_maps_t getWmiStorageMaps();
module_page_descriptors_t getWmiPageDescriptors();

module_page_maps_t getTableSwitchingPageMaps();
module_storage_maps_t getTableSwitchingStorageMaps();
module_page_descriptors_t getTableSwitchingPageDescriptors();

module_page_maps_t getCommsExtendedPageMaps();
module_storage_maps_t getCommsExtendedStorageMaps();
module_page_descriptors_t getCommsExtendedPageDescriptors();

module_page_maps_t getLoggingPageMaps();
module_storage_maps_t getLoggingStorageMaps();
module_page_descriptors_t getLoggingPageDescriptors();

const module_descriptor_t *getRegisteredModules();
uint8_t getRegisteredModuleCount();
bool core_modules_save_page(uint8_t pageNumber, uint16_t &writesRemaining);
void core_modules_load_pages(void);
