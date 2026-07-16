#include "storage/page_registry.h"

#include "modules/comms_extended/module_comms_extended.h"

module_page_maps_t getCommsExtendedPageMaps()
{
  return { nullptr, 0U };
}

module_page_descriptors_t getCommsExtendedPageDescriptors()
{
  return { nullptr, 0U };
}

module_storage_maps_t getCommsExtendedStorageMaps()
{
  return { nullptr, 0U };
}
