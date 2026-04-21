#include "storage/page_registry.h"

#include "modules/can/module_can.h"
#include "support/preprocessor.h"
#include "data/tune_registry.h"

static constexpr entity_t makeRawEntity(config_page_t *pEntity, uint16_t entitySize)
{
  return entity_t(pEntity, entitySize);
}

static constexpr uint16_t EEPROM_CONFIG9_START = 1710;

#if FEATURE_MODULE_CAN
static constexpr entity_t canBusPageMap[] PROGMEM = {
  makeRawEntity(&configPage9, sizeof(configPage9)),
};

static constexpr page_map_t canPageMaps[] PROGMEM = {
  { canBusPageMap, _countof(canBusPageMap) },
};

static constexpr page_descriptor_t canPageDescriptors[] PROGMEM = {
  { canbusPage, canPageMaps[0] },
};

static constexpr entity_storage_map_t canStorageMaps[] PROGMEM = {
  { &configPage9, EEPROM_CONFIG9_START },
};
#else
static constexpr page_map_t canPageMaps[] PROGMEM = {};
static constexpr page_descriptor_t canPageDescriptors[] PROGMEM = {};
static constexpr entity_storage_map_t canStorageMaps[] PROGMEM = {};
#endif

module_page_maps_t getCanPageMaps()
{
  return { canPageMaps, _countof(canPageMaps) };
}

module_page_descriptors_t getCanPageDescriptors()
{
  return { canPageDescriptors, _countof(canPageDescriptors) };
}

module_storage_maps_t getCanStorageMaps()
{
  return { canStorageMaps, _countof(canStorageMaps) };
}
