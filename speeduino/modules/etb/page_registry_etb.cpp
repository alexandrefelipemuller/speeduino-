#include "storage/page_registry.h"

#include "modules/etb/etb.h"
#include "support/preprocessor.h"
#include "data/tune_registry.h"

static constexpr entity_t makeRawEntity(config_page_t *pEntity, uint16_t entitySize)
{
  return entity_t(pEntity, entitySize);
}

#if FEATURE_MODULE_ETB
static constexpr entity_t etbPageMap[] PROGMEM = {
  makeRawEntity(&configPage16, sizeof(configPage16)),
};

static constexpr page_map_t etbPageMaps[] PROGMEM = {
  { etbPageMap, _countof(etbPageMap) },
};

static constexpr page_descriptor_t etbPageDescriptors[] PROGMEM = {
  { etbPage, etbPageMaps[0] },
};

static constexpr entity_storage_map_t etbStorageMaps[] PROGMEM = {
  { &configPage16, EEPROM_CONFIG16_START },
};
#else
static constexpr page_map_t etbPageMaps[] PROGMEM = {};
static constexpr page_descriptor_t etbPageDescriptors[] PROGMEM = {};
static constexpr entity_storage_map_t etbStorageMaps[] PROGMEM = {};
#endif

module_page_maps_t getEtbPageMaps()
{
  return { etbPageMaps, _countof(etbPageMaps) };
}

module_page_descriptors_t getEtbPageDescriptors()
{
  return { etbPageDescriptors, _countof(etbPageDescriptors) };
}

module_storage_maps_t getEtbStorageMaps()
{
  return { etbStorageMaps, _countof(etbStorageMaps) };
}
