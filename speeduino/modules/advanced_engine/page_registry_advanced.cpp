#include "storage/page_registry.h"

#include "modules/advanced_engine/module_advanced_engine.h"
#include "support/preprocessor.h"

#if FEATURE_MODULE_ADVANCED_ENGINE
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
  return { nullptr, 0U };
}
#else
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
  return { nullptr, 0U };
}
#endif
