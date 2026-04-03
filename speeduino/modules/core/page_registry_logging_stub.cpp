#include "page_registry.h"
#include "preprocessor.h"

#if !FEATURE_MODULE_LOGGING

module_page_maps_t getLoggingPageMaps()
{
  return { nullptr, 0U };
}

module_storage_maps_t getLoggingStorageMaps()
{
  return { nullptr, 0U };
}

#endif
