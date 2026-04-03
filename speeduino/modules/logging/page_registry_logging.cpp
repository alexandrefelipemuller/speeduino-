#include "page_registry.h"

#include "pages.h"
#include "preprocessor.h"
#include "tune_registry.h"

static constexpr entity_t makeRawEntity(config_page_t *pEntity, uint16_t entitySize)
{
  return entity_t(pEntity, entitySize);
}

static constexpr uint16_t EEPROM_CONFIG13_START = 2580;

#if FEATURE_MODULE_LOGGING
static constexpr entity_t progOutsPageMap[] PROGMEM = {
  makeRawEntity(&configPage13, sizeof(configPage13)),
};

static constexpr page_map_t loggingPageMaps[] PROGMEM = {
  { progOutsPageMap, _countof(progOutsPageMap) },
};

static constexpr entity_storage_map_t loggingStorageMaps[] PROGMEM = {
  { &configPage13, EEPROM_CONFIG13_START },
};
#else
static constexpr page_map_t loggingPageMaps[] PROGMEM = {};
static constexpr entity_storage_map_t loggingStorageMaps[] PROGMEM = {};
#endif

module_page_maps_t getLoggingPageMaps()
{
  return { loggingPageMaps, _countof(loggingPageMaps) };
}

module_storage_maps_t getLoggingStorageMaps()
{
  return { loggingStorageMaps, _countof(loggingStorageMaps) };
}
