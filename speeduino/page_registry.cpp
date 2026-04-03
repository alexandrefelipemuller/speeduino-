#include "page_registry.h"

#include "prog_mem_support.h"
static constexpr page_map_t emptyPageMap = { nullptr, 0U };

static page_map_t copyPageMapOrEmpty(const module_page_maps_t &moduleMaps, uint8_t index)
{
  if (index < moduleMaps.count)
  {
    return copyObject_P(&moduleMaps.maps[index]);
  }
  return emptyPageMap;
}

page_map_t getPageMap(uint8_t pageNumber)
{
  if (pageNumber == 0U || pageNumber >= MAX_PAGE_NUM)
  {
    return copyPageMapOrEmpty(getCorePageMaps(), 0U);
  }

  if (pageNumber <= afrSetPage)
  {
    return copyPageMapOrEmpty(getCorePageMaps(), pageNumber);
  }

  switch (pageNumber)
  {
    case boostvvtPage:
      return copyPageMapOrEmpty(getAdvancedEnginePageMaps(), 0U);
    case seqFuelPage:
      return copyPageMapOrEmpty(getTableSwitchingPageMaps(), 0U);
    case canbusPage:
      return copyPageMapOrEmpty(getCommsExtendedPageMaps(), 0U);
    case warmupPage:
      return copyPageMapOrEmpty(getAdvancedEnginePageMaps(), 1U);
    case fuelMap2Page:
      return copyPageMapOrEmpty(getTableSwitchingPageMaps(), 1U);
    case wmiMapPage:
      return copyPageMapOrEmpty(getAdvancedEnginePageMaps(), 2U);
    case progOutsPage:
      return copyPageMapOrEmpty(getLoggingPageMaps(), 0U);
    case ignMap2Page:
      return copyPageMapOrEmpty(getTableSwitchingPageMaps(), 2U);
    case boostvvtPage2:
      return copyPageMapOrEmpty(getAdvancedEnginePageMaps(), 3U);
    default:
      return copyPageMapOrEmpty(getCorePageMaps(), 0U);
  }
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
  static entity_storage_map_t aggregatedStorageMaps[64];
  static bool isInitialised = false;

  if (!isInitialised)
  {
    uint8_t offset = 0U;
    (void)copyStorageMapBlock(getCoreStorageMaps(), aggregatedStorageMaps, offset);
    (void)copyStorageMapBlock(getAdvancedEngineStorageMaps(), aggregatedStorageMaps, offset);
    (void)copyStorageMapBlock(getTableSwitchingStorageMaps(), aggregatedStorageMaps, offset);
    (void)copyStorageMapBlock(getCommsExtendedStorageMaps(), aggregatedStorageMaps, offset);
    (void)copyStorageMapBlock(getLoggingStorageMaps(), aggregatedStorageMaps, offset);
    isInitialised = true;
  }

  return aggregatedStorageMaps;
}

uint8_t getEntityStorageMapSize()
{
  return getCoreStorageMaps().count
       + getAdvancedEngineStorageMaps().count
       + getTableSwitchingStorageMaps().count
       + getCommsExtendedStorageMaps().count
       + getLoggingStorageMaps().count;
}
