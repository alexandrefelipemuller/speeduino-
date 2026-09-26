#include "storage/page_registry.h"

#include "modules/logging/module_logging.h"

module_page_maps_t getLoggingPageMaps()
{
  return { nullptr, 0U };
}

module_page_descriptors_t getLoggingPageDescriptors()
{
  return { nullptr, 0U };
}

module_storage_maps_t getLoggingStorageMaps()
{
  return { nullptr, 0U };
}
