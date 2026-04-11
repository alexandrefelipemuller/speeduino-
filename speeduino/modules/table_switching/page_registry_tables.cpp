#include "storage/page_registry.h"

#include "modules/table_switching/module_table_switching.h"
#include "support/preprocessor.h"
#include "data/table_registry.h"

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

static constexpr uint16_t EEPROM_CONFIG8_MAP1 = 1503;
static constexpr uint16_t EEPROM_CONFIG8_MAP2 = 1553;
static constexpr uint16_t EEPROM_CONFIG8_MAP3 = 1603;
static constexpr uint16_t EEPROM_CONFIG8_MAP4 = 1653;
static constexpr uint16_t EEPROM_CONFIG8_MAP5 = 3001;
static constexpr uint16_t EEPROM_CONFIG8_MAP6 = 3051;
static constexpr uint16_t EEPROM_CONFIG8_MAP7 = 3101;
static constexpr uint16_t EEPROM_CONFIG8_MAP8 = 3151;
static constexpr uint16_t EEPROM_CONFIG11_MAP = 2096;

#if FEATURE_MODULE_TABLE_SWITCHING
static constexpr entity_t sequentialPageMap[] PROGMEM = {
  makeTableEntity(&trim1Table),
  makeTableEntity(&trim2Table),
  makeTableEntity(&trim3Table),
  makeTableEntity(&trim4Table),
  makeTableEntity(&trim5Table),
  makeTableEntity(&trim6Table),
  makeTableEntity(&trim7Table),
  makeTableEntity(&trim8Table),
};
static constexpr entity_t fuel2PageMap[] PROGMEM = {
  makeTableEntity(&fuelTable2),
};
static constexpr entity_t ign2PageMap[] PROGMEM = {
  makeTableEntity(&ignitionTable2),
};

static constexpr page_map_t tableSwitchingPageMaps[] PROGMEM = {
  { sequentialPageMap, _countof(sequentialPageMap) },
  { fuel2PageMap, _countof(fuel2PageMap) },
  { ign2PageMap, _countof(ign2PageMap) },
};

static constexpr page_descriptor_t tableSwitchingPageDescriptors[] PROGMEM = {
  { seqFuelPage, tableSwitchingPageMaps[0] },
  { fuelMap2Page, tableSwitchingPageMaps[1] },
  { ignMap2Page, tableSwitchingPageMaps[2] },
};

static constexpr entity_storage_map_t tableSwitchingStorageMaps[] PROGMEM = {
  { &trim1Table, EEPROM_CONFIG8_MAP1 },
  { &trim2Table, EEPROM_CONFIG8_MAP2 },
  { &trim3Table, EEPROM_CONFIG8_MAP3 },
  { &trim4Table, EEPROM_CONFIG8_MAP4 },
  { &trim5Table, EEPROM_CONFIG8_MAP5 },
  { &trim6Table, EEPROM_CONFIG8_MAP6 },
  { &trim7Table, EEPROM_CONFIG8_MAP7 },
  { &trim8Table, EEPROM_CONFIG8_MAP8 },
  { &fuelTable2, EEPROM_CONFIG11_MAP },
};
#else
static constexpr page_map_t tableSwitchingPageMaps[] PROGMEM = {};
static constexpr page_descriptor_t tableSwitchingPageDescriptors[] PROGMEM = {};
static constexpr entity_storage_map_t tableSwitchingStorageMaps[] PROGMEM = {};
#endif

module_page_maps_t getTableSwitchingPageMaps()
{
  return { tableSwitchingPageMaps, _countof(tableSwitchingPageMaps) };
}

module_page_descriptors_t getTableSwitchingPageDescriptors()
{
  return { tableSwitchingPageDescriptors, _countof(tableSwitchingPageDescriptors) };
}

module_storage_maps_t getTableSwitchingStorageMaps()
{
  return { tableSwitchingStorageMaps, _countof(tableSwitchingStorageMaps) };
}
