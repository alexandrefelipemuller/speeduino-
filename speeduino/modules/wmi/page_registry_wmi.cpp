#include "storage/page_registry.h"

#include "support/preprocessor.h"
#include "data/table_registry.h"
#include "modules/wmi/wmi.h"

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

static constexpr uint16_t EEPROM_CONFIG12_MAP = 2387;
static constexpr uint16_t EEPROM_CONFIG12_MAP2 = 2469;
static constexpr uint16_t EEPROM_CONFIG12_MAP3 = 2551;

#if FEATURE_MODULE_WMI
static constexpr entity_t wmiPageMap[] PROGMEM = {
  makeTableEntity(&wmiTable),
  makeTableEntity(&vvt2Table),
  makeTableEntity(&dwellTable),
};

static constexpr page_map_t wmiPageMaps[] PROGMEM = {
  { wmiPageMap, _countof(wmiPageMap) },
};

static constexpr page_descriptor_t wmiPageDescriptors[] PROGMEM = {
  { wmiMapPage, wmiPageMaps[0] },
};

static constexpr entity_storage_map_t wmiStorageMaps[] PROGMEM = {
  { &wmiTable, EEPROM_CONFIG12_MAP },
  { &vvt2Table, EEPROM_CONFIG12_MAP2 },
  { &dwellTable, EEPROM_CONFIG12_MAP3 },
};
#else
static constexpr page_map_t wmiPageMaps[] PROGMEM = {};
static constexpr page_descriptor_t wmiPageDescriptors[] PROGMEM = {};
static constexpr entity_storage_map_t wmiStorageMaps[] PROGMEM = {};
#endif

module_page_maps_t getWmiPageMaps()
{
  return { wmiPageMaps, _countof(wmiPageMaps) };
}

module_page_descriptors_t getWmiPageDescriptors()
{
  return { wmiPageDescriptors, _countof(wmiPageDescriptors) };
}

module_storage_maps_t getWmiStorageMaps()
{
  return { wmiStorageMaps, _countof(wmiStorageMaps) };
}
