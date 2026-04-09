#include "page_registry.h"

#include "prog_mem_support.h"
static constexpr page_map_t emptyPageMap = { nullptr, 0U };
static constexpr uint8_t STORAGE_MAP_CAPACITY = 64U;

static page_map_t findPageMapInDescriptors(const module_page_descriptors_t &descriptors, uint8_t pageNumber)
{
  for (uint8_t i = 0; i < descriptors.count; ++i)
  {
    const page_descriptor_t descriptor = copyObject_P(&descriptors.descriptors[i]);
    if (descriptor.pageNumber == pageNumber)
    {
      return descriptor.map;
    }
  }

  return emptyPageMap;
}

static page_map_t findPageMapFromProviders(uint8_t pageNumber)
{
  const module_descriptor_t *modules = getRegisteredModules();
  const uint8_t moduleCount = getRegisteredModuleCount();

  for (uint8_t moduleIndex = 0; moduleIndex < moduleCount; ++moduleIndex)
  {
    const page_map_t map = findPageMapInDescriptors(modules[moduleIndex].page_descriptors, pageNumber);
    if (map.searchMap != nullptr)
    {
      return map;
    }
  }

  return emptyPageMap;
}

page_map_t getPageMap(uint8_t pageNumber)
{
  if (pageNumber >= MAX_PAGE_NUM)
  {
    return findPageMapFromProviders(0U);
  }

  return findPageMapFromProviders(pageNumber);
}

static const entity_storage_map_t *copyStorageMapBlock(const module_storage_maps_t &moduleMaps,
                                                       entity_storage_map_t *target,
                                                       uint8_t &offset)
{
  for (uint8_t i = 0; i < moduleMaps.count; ++i)
  {
    target[offset] = copyObject_P(&moduleMaps.maps[i]);
    ++offset;
  }
  return target;
}

const entity_storage_map_t *getEntityStorageMap()
{
  static entity_storage_map_t aggregatedStorageMaps[STORAGE_MAP_CAPACITY];
  static bool isInitialised = false;

  if (!isInitialised)
  {
    uint8_t offset = 0U;
    const module_descriptor_t *modules = getRegisteredModules();
    const uint8_t moduleCount = getRegisteredModuleCount();

    for (uint8_t moduleIndex = 0; moduleIndex < moduleCount; ++moduleIndex)
    {
      (void)copyStorageMapBlock(modules[moduleIndex].storage_maps, aggregatedStorageMaps, offset);
    }
    isInitialised = true;
  }

  return aggregatedStorageMaps;
}

uint8_t getEntityStorageMapSize()
{
  const module_descriptor_t *modules = getRegisteredModules();
  const uint8_t moduleCount = getRegisteredModuleCount();

  uint8_t count = 0U;
  for (uint8_t moduleIndex = 0; moduleIndex < moduleCount; ++moduleIndex)
  {
    count += modules[moduleIndex].storage_maps.count;
  }
  return count;
}
