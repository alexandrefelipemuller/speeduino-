#include "page_registry.h"

#include "pages.h"
#include "prog_mem_support.h"
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

static constexpr uint16_t EEPROM_CONFIG1_MAP = 3;
static constexpr uint16_t EEPROM_CONFIG2_START = 291;
static constexpr uint16_t EEPROM_CONFIG3_MAP = 421;
static constexpr uint16_t EEPROM_CONFIG4_START = 709;
static constexpr uint16_t EEPROM_CONFIG5_MAP = 839;
static constexpr uint16_t EEPROM_CONFIG6_START = 1127;
static constexpr uint16_t CONFIG_PAGE6_TUNE_SIZE = 128U;

static constexpr entity_t pageZeroMap[] PROGMEM = {
  makeEmptyEntity(0U),
};
static constexpr entity_t veSetPageMap[] PROGMEM = {
  makeRawEntity(&configPage2, sizeof(configPage2)),
};
static constexpr entity_t vePageMap[] PROGMEM = {
  makeTableEntity(&fuelTable),
};
static constexpr entity_t ignPageMap[] PROGMEM = {
  makeTableEntity(&ignitionTable),
};
static constexpr entity_t ignSetPageMap[] PROGMEM = {
  makeRawEntity(&configPage4, sizeof(configPage4)),
};
static constexpr entity_t afrPageMap[] PROGMEM = {
  makeTableEntity(&afrTable),
};
static constexpr entity_t afrSetPageMap[] PROGMEM = {
  makeRawEntity(&configPage6, CONFIG_PAGE6_TUNE_SIZE),
};

static constexpr page_map_t corePageMaps[] PROGMEM = {
  { pageZeroMap, _countof(pageZeroMap) },
  { veSetPageMap, _countof(veSetPageMap) },
  { vePageMap, _countof(vePageMap) },
  { ignPageMap, _countof(ignPageMap) },
  { ignSetPageMap, _countof(ignSetPageMap) },
  { afrPageMap, _countof(afrPageMap) },
  { afrSetPageMap, _countof(afrSetPageMap) },
};

static constexpr page_descriptor_t corePageDescriptors[] PROGMEM = {
  { 0U, corePageMaps[0] },
  { veSetPage, corePageMaps[1] },
  { veMapPage, corePageMaps[2] },
  { ignMapPage, corePageMaps[3] },
  { ignSetPage, corePageMaps[4] },
  { afrMapPage, corePageMaps[5] },
  { afrSetPage, corePageMaps[6] },
};

static constexpr entity_storage_map_t coreStorageMaps[] PROGMEM = {
  { &fuelTable, EEPROM_CONFIG1_MAP },
  { &configPage2, EEPROM_CONFIG2_START },
  { &ignitionTable, EEPROM_CONFIG3_MAP },
  { &configPage4, EEPROM_CONFIG4_START },
  { &afrTable, EEPROM_CONFIG5_MAP },
  { &configPage6, EEPROM_CONFIG6_START },
};

module_page_maps_t getCorePageMaps()
{
  return { corePageMaps, _countof(corePageMaps) };
}

module_page_descriptors_t getCorePageDescriptors()
{
  return { corePageDescriptors, _countof(corePageDescriptors) };
}

module_storage_maps_t getCoreStorageMaps()
{
  return { coreStorageMaps, _countof(coreStorageMaps) };
}
