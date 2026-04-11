#include "storage/page_registry.h"

#include "modules/advanced_engine/module_advanced_engine.h"
#include "support/preprocessor.h"
#include "data/table_registry.h"

#if FEATURE_MODULE_ADVANCED_ENGINE
static constexpr entity_storage_map_t advancedStorageMaps[] PROGMEM = {
  { &ignitionTable2, 2710 },
};
#else
static constexpr entity_storage_map_t advancedStorageMaps[] PROGMEM = {};
#endif

module_page_maps_t getAdvancedEnginePageMaps()
{
  return { nullptr, 0U };
}

module_page_descriptors_t getAdvancedEnginePageDescriptors()
{
  return { nullptr, 0U };
}

module_storage_maps_t getAdvancedEngineStorageMaps()
{
  return { advancedStorageMaps, _countof(advancedStorageMaps) };
}
