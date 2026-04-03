#include "page_registry.h"
#include "preprocessor.h"

#if !FEATURE_MODULE_TABLE_SWITCHING

module_page_maps_t getTableSwitchingPageMaps()
{
  return { nullptr, 0U };
}

module_storage_maps_t getTableSwitchingStorageMaps()
{
  return { nullptr, 0U };
}

#endif
