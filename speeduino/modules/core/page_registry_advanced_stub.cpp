#include "page_registry.h"
#include "preprocessor.h"

#if !FEATURE_MODULE_ADVANCED_ENGINE

module_page_maps_t getAdvancedEnginePageMaps()
{
  return { nullptr, 0U };
}

module_storage_maps_t getAdvancedEngineStorageMaps()
{
  return { nullptr, 0U };
}

#endif
