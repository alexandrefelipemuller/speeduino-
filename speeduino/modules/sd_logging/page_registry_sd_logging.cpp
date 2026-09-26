#include "storage/page_registry.h"

#include "modules/sd_logging/module_sd_logging.h"
#include "support/preprocessor.h"
#include "data/tune_registry.h"

static constexpr entity_t makeRawEntity(config_page_t *pEntity, uint16_t entitySize)
{
  return entity_t(pEntity, entitySize);
}

static constexpr uint16_t EEPROM_CONFIG13_START = 2580;

#if FEATURE_MODULE_SD_LOGGING
static constexpr entity_t progOutsPageMap[] PROGMEM = {
  makeRawEntity(&configPage13, sizeof(configPage13)),
};

static constexpr page_map_t sdLoggingPageMaps[] PROGMEM = {
  { progOutsPageMap, _countof(progOutsPageMap) },
};

static constexpr page_descriptor_t sdLoggingPageDescriptors[] PROGMEM = {
  { progOutsPage, sdLoggingPageMaps[0] },
};

static constexpr entity_storage_map_t sdLoggingStorageMaps[] PROGMEM = {
  { &configPage13, EEPROM_CONFIG13_START },
};
#else
static constexpr page_map_t sdLoggingPageMaps[] PROGMEM = {};
static constexpr page_descriptor_t sdLoggingPageDescriptors[] PROGMEM = {};
static constexpr entity_storage_map_t sdLoggingStorageMaps[] PROGMEM = {};
#endif

module_page_maps_t getSdLoggingPageMaps()
{
  return { sdLoggingPageMaps, _countof(sdLoggingPageMaps) };
}

module_page_descriptors_t getSdLoggingPageDescriptors()
{
  return { sdLoggingPageDescriptors, _countof(sdLoggingPageDescriptors) };
}

module_storage_maps_t getSdLoggingStorageMaps()
{
  return { sdLoggingStorageMaps, _countof(sdLoggingStorageMaps) };
}
