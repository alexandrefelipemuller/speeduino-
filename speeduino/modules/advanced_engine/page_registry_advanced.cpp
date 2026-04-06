#include "page_registry.h"

#include "pages.h"
#include "preprocessor.h"
#include "table_registry.h"
#include "tune_registry.h"

template <typename table_t>
static constexpr uint16_t get_table_value_end(void)
{
  return table_t::xaxis_t::length * table_t::yaxis_t::length;
}

template <typename table_t>
static constexpr uint16_t get_table_axisx_end(void)
{
  return get_table_value_end<table_t>() + table_t::xaxis_t::length;
}

template <typename table_t>
static constexpr uint16_t getTableSize(void)
{
  return get_table_axisx_end<table_t>() + table_t::yaxis_t::length;
}

template <typename table_t>
static constexpr entity_t makeTableEntity(table_t *pTable)
{
  return entity_t((table3d_t *)pTable, table_t::type_key, getTableSize<table_t>());
}

static constexpr entity_t makeRawEntity(config_page_t *pEntity, uint16_t entitySize)
{
  return entity_t(pEntity, entitySize);
}

static constexpr entity_t makeEmptyEntity(uint16_t entitySize)
{
  return entity_t(EntityType::NoEntity, entitySize);
}

static constexpr uint16_t EEPROM_CONFIG7_MAP1 = 1257;
static constexpr uint16_t EEPROM_CONFIG7_MAP2 = 1339;
static constexpr uint16_t EEPROM_CONFIG7_MAP3 = 1421;
static constexpr uint16_t EEPROM_CONFIG10_START = 1902;
static constexpr uint16_t EEPROM_CONFIG12_MAP = 2387;
static constexpr uint16_t EEPROM_CONFIG12_MAP2 = 2469;
static constexpr uint16_t EEPROM_CONFIG12_MAP3 = 2551;
static constexpr uint16_t EEPROM_CONFIG14_MAP = 2710;
static constexpr uint16_t EEPROM_CONFIG15_MAP = 3199;
static constexpr uint16_t EEPROM_CONFIG15_START = 3281;

#if FEATURE_MODULE_ADVANCED_ENGINE
static constexpr entity_t boostVvtPageMap[] PROGMEM = {
  makeTableEntity(&boostTable),
  makeTableEntity(&vvtTable),
  makeTableEntity(&stagingTable),
};
static constexpr entity_t warmUpPageMap[] PROGMEM = {
  makeRawEntity(&configPage10, sizeof(configPage10)),
};
static constexpr entity_t wmiPageMap[] PROGMEM = {
  makeTableEntity(&wmiTable),
  makeTableEntity(&vvt2Table),
  makeTableEntity(&dwellTable),
  makeEmptyEntity(8U),
};
static constexpr entity_t boostVvt2PageMap[] PROGMEM = {
  makeTableEntity(&boostTableLookupDuty),
  makeRawEntity(&configPage15, sizeof(configPage15)),
};

static constexpr page_map_t advancedPageMaps[] PROGMEM = {
  { boostVvtPageMap, _countof(boostVvtPageMap) },
  { warmUpPageMap, _countof(warmUpPageMap) },
  { wmiPageMap, _countof(wmiPageMap) },
  { boostVvt2PageMap, _countof(boostVvt2PageMap) },
};

static constexpr page_descriptor_t advancedPageDescriptors[] PROGMEM = {
  { boostvvtPage, advancedPageMaps[0] },
  { warmupPage, advancedPageMaps[1] },
  { wmiMapPage, advancedPageMaps[2] },
  { boostvvtPage2, advancedPageMaps[3] },
};

static constexpr entity_storage_map_t advancedStorageMaps[] PROGMEM = {
  { &boostTable, EEPROM_CONFIG7_MAP1 },
  { &vvtTable, EEPROM_CONFIG7_MAP2 },
  { &stagingTable, EEPROM_CONFIG7_MAP3 },
  { &configPage10, EEPROM_CONFIG10_START },
  { &wmiTable, EEPROM_CONFIG12_MAP },
  { &vvt2Table, EEPROM_CONFIG12_MAP2 },
  { &dwellTable, EEPROM_CONFIG12_MAP3 },
  { &ignitionTable2, EEPROM_CONFIG14_MAP },
  { &boostTableLookupDuty, EEPROM_CONFIG15_MAP },
  { &configPage15, EEPROM_CONFIG15_START },
};
#else
static constexpr page_map_t advancedPageMaps[] PROGMEM = {};
static constexpr page_descriptor_t advancedPageDescriptors[] PROGMEM = {};
static constexpr entity_storage_map_t advancedStorageMaps[] PROGMEM = {};
#endif

module_page_maps_t getAdvancedEnginePageMaps()
{
  return { advancedPageMaps, _countof(advancedPageMaps) };
}

module_page_descriptors_t getAdvancedEnginePageDescriptors()
{
  return { advancedPageDescriptors, _countof(advancedPageDescriptors) };
}

module_storage_maps_t getAdvancedEngineStorageMaps()
{
  return { advancedStorageMaps, _countof(advancedStorageMaps) };
}
