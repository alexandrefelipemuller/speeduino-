#include "page_registry.h"
#include "preprocessor.h"

#if !FEATURE_MODULE_COMMS_EXTENDED

module_page_maps_t getCommsExtendedPageMaps()
{
  return { nullptr, 0U };
}

module_storage_maps_t getCommsExtendedStorageMaps()
{
  return { nullptr, 0U };
}

#endif
