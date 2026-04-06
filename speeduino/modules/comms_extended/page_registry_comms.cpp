#include "page_registry.h"

#include "pages.h"
#include "preprocessor.h"
#include "tune_registry.h"

static constexpr entity_t makeRawEntity(config_page_t *pEntity, uint16_t entitySize)
{
  return entity_t(pEntity, entitySize);
}

static constexpr uint16_t EEPROM_CONFIG9_START = 1710;

#if FEATURE_MODULE_COMMS_EXTENDED
static constexpr entity_t canBusPageMap[] PROGMEM = {
  makeRawEntity(&configPage9, sizeof(configPage9)),
};

static constexpr page_map_t commsExtendedPageMaps[] PROGMEM = {
  { canBusPageMap, _countof(canBusPageMap) },
};

static constexpr page_descriptor_t commsExtendedPageDescriptors[] PROGMEM = {
  { canbusPage, commsExtendedPageMaps[0] },
};

static constexpr entity_storage_map_t commsExtendedStorageMaps[] PROGMEM = {
  { &configPage9, EEPROM_CONFIG9_START },
};
#else
static constexpr page_map_t commsExtendedPageMaps[] PROGMEM = {};
static constexpr page_descriptor_t commsExtendedPageDescriptors[] PROGMEM = {};
static constexpr entity_storage_map_t commsExtendedStorageMaps[] PROGMEM = {};
#endif

module_page_maps_t getCommsExtendedPageMaps()
{
  return { commsExtendedPageMaps, _countof(commsExtendedPageMaps) };
}

module_page_descriptors_t getCommsExtendedPageDescriptors()
{
  return { commsExtendedPageDescriptors, _countof(commsExtendedPageDescriptors) };
}

module_storage_maps_t getCommsExtendedStorageMaps()
{
  return { commsExtendedStorageMaps, _countof(commsExtendedStorageMaps) };
}
