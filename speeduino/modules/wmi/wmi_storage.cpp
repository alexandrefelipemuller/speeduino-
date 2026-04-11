/** @file
 * WMI EEPROM storage helpers.
 */

#include "modules/wmi/wmi_storage.h"

#if FEATURE_MODULE_WMI
#include "data/table_registry.h"
#include "data/tune_registry.h"
#include "modules/wmi/wmi.h"
#include "storage/pages.h"
#include "storage/storage.h"
#include "storage/updates.h"
#include "support/table3d.h"
#include "support/utilities.h"

namespace {
static constexpr uint16_t EEPROM_CONFIG12_MAP = 2387;
static constexpr uint16_t EEPROM_CONFIG12_MAP2 = 2469;
static constexpr uint16_t EEPROM_CONFIG12_MAP3 = 2551;

struct write_location
{
  uint16_t address;
  uint16_t writesRemaining;
};

static inline uint16_t write_range(const byte *pStart, const byte *pEnd, uint16_t address, uint16_t writesRemaining)
{
  return updateBlockLimitWriteOps(getStorageAPI(), address, pStart, pEnd, writesRemaining);
}

static inline write_location write(const table_row_iterator &row, const write_location &location)
{
  return { (uint16_t)(location.address + row.size()), updateBlockLimitWriteOps(getStorageAPI(), location.address, &*row, row.end(), location.writesRemaining) };
}

static inline write_location write(table_value_iterator it, write_location location)
{
  while (location.writesRemaining > 0U && !it.at_end())
  {
    location = write(it.operator*(), location);
    ++it;
  }
  return location;
}

static inline write_location write(table_axis_iterator it, write_location location)
{
  while (location.writesRemaining > 0U && !it.at_end())
  {
    if (update(getStorageAPI(), location.address, it.operator*())) { --location.writesRemaining; }
    ++location.address;
    ++it;
  }
  return location;
}

static inline uint16_t writeTable(table3d_t *pTable, TableType key, uint16_t address, uint16_t writesRemaining)
{
  return write(y_rbegin(pTable, key),
                write(x_begin(pTable, key),
                  write(rows_begin(pTable, key), { address, writesRemaining }))).writesRemaining;
}

static inline uint16_t load_range(uint16_t address, byte *pFirst, const byte *pLast)
{
  return loadBlock(getStorageAPI(), address, pFirst, pLast);
}

static inline uint16_t load(table_row_iterator row, uint16_t address)
{
  return load_range(address, &*row, row.end());
}

static inline uint16_t load(table_value_iterator it, uint16_t address)
{
  while (!it.at_end())
  {
    address = load(*it, address); // cppcheck-suppress misra-c2012-17.2
    ++it;
  }
  return address;
}

static inline uint16_t load(table_axis_iterator it, uint16_t address)
{
  while (!it.at_end())
  {
    *it = getStorageAPI().read(address);
    ++address;
    ++it;
  }
  return address;
}

static inline uint16_t loadTable(table3d_t *pTable, TableType key, uint16_t address)
{
  return load(y_rbegin(pTable, key),
                load(x_begin(pTable, key),
                  load(rows_begin(pTable, key), address)));
}
} // namespace

void module_wmi_save_pages(uint16_t &writes_remaining)
{
  writes_remaining = writeTable(&wmiTable, decltype(wmiTable)::type_key, EEPROM_CONFIG12_MAP, writes_remaining);
  writes_remaining = writeTable(&vvt2Table, decltype(vvt2Table)::type_key, EEPROM_CONFIG12_MAP2, writes_remaining);
  writes_remaining = writeTable(&dwellTable, decltype(dwellTable)::type_key, EEPROM_CONFIG12_MAP3, writes_remaining);
}

void module_wmi_load_pages(void)
{
  (void)loadTable(&wmiTable, decltype(wmiTable)::type_key, EEPROM_CONFIG12_MAP);
  (void)loadTable(&vvt2Table, decltype(vvt2Table)::type_key, EEPROM_CONFIG12_MAP2);
  (void)loadTable(&dwellTable, decltype(dwellTable)::type_key, EEPROM_CONFIG12_MAP3);
}

void module_wmi_upgrade_v22(void)
{
  if(configPage10.wmiMode >= WMI_MODE_OPENLOOP) { multiplyTableValue(wmiMapPage, 2); }
}
#endif
