#include "modules/boost/boost.h"

#include "data/table_registry.h"
#include "data/tune_registry.h"
#include "support/preprocessor.h"

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

static constexpr uint16_t EEPROM_CONFIG7_MAP1 = 1257;
static constexpr uint16_t EEPROM_CONFIG7_MAP2 = 1339;
static constexpr uint16_t EEPROM_CONFIG7_MAP3 = 1421;
static constexpr uint16_t EEPROM_CONFIG10_START = 1902;
static constexpr uint16_t EEPROM_CONFIG15_MAP = 3199;
static constexpr uint16_t EEPROM_CONFIG15_START = 3281;

#if FEATURE_MODULE_BOOST
static constexpr entity_t boostVvtPageMap[] PROGMEM = {
  makeTableEntity(&boostTable),
  makeTableEntity(&vvtTable),
  makeTableEntity(&stagingTable),
};

static constexpr entity_t warmUpPageMap[] PROGMEM = {
  makeRawEntity(&configPage10, sizeof(configPage10)),
};

static constexpr entity_t boostVvt2PageMap[] PROGMEM = {
  makeTableEntity(&boostTableLookupDuty),
  makeRawEntity(&configPage15, sizeof(configPage15)),
};

static constexpr page_map_t boostPageMaps[] PROGMEM = {
  { boostVvtPageMap, _countof(boostVvtPageMap) },
  { warmUpPageMap, _countof(warmUpPageMap) },
  { boostVvt2PageMap, _countof(boostVvt2PageMap) },
};

static constexpr page_descriptor_t boostPageDescriptors[] PROGMEM = {
  { boostvvtPage, boostPageMaps[0] },
  { warmupPage, boostPageMaps[1] },
  { boostvvtPage2, boostPageMaps[2] },
};

static constexpr entity_storage_map_t boostStorageMaps[] PROGMEM = {
  { &boostTable, EEPROM_CONFIG7_MAP1 },
  { &vvtTable, EEPROM_CONFIG7_MAP2 },
  { &stagingTable, EEPROM_CONFIG7_MAP3 },
  { &configPage10, EEPROM_CONFIG10_START },
  { &boostTableLookupDuty, EEPROM_CONFIG15_MAP },
  { &configPage15, EEPROM_CONFIG15_START },
};
#else
static constexpr page_map_t boostPageMaps[] PROGMEM = {};
static constexpr page_descriptor_t boostPageDescriptors[] PROGMEM = {};
static constexpr entity_storage_map_t boostStorageMaps[] PROGMEM = {};
#endif

module_page_maps_t getBoostPageMaps()
{
  return { boostPageMaps, _countof(boostPageMaps) };
}

module_page_descriptors_t getBoostPageDescriptors()
{
  return { boostPageDescriptors, _countof(boostPageDescriptors) };
}

module_storage_maps_t getBoostStorageMaps()
{
  return { boostStorageMaps, _countof(boostStorageMaps) };
}
