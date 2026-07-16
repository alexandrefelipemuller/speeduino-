/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
*/
/** @file
 * Lower level ConfigPage*, Table2D, Table3D and EEPROM storage operations.
 */

#include "storage/storage.h"
#include "storage/page_registry.h"
#include "storage/pages.h"
#include "modules/boost/boost.h"
#include "modules/comms_extended/module_comms_extended.h"
#include "modules/sd_logging/module_sd_logging.h"
#include "modules/table_switching/module_table_switching.h"
#include "data/runtime_state.h"
#include "data/table_registry.h"
#include "data/tune_registry.h"
#include "engine/sensors.h"
#include "support/preprocessor.h"
#include "support/unit_testing.h"
#include "support/utilities.h"

#if defined(CORE_AVR)
#pragma GCC push_options
// This minimizes RAM usage at no performance cost
#pragma GCC optimize ("Os")
#endif

// Calibration data is stored at the end of the EEPROM (This is in case any further calibration tables are needed as they are large blocks)
constexpr uint16_t EEPROM_DATA_VERSION = 0;
constexpr uint16_t STORAGE_END = 0xFFF;
constexpr uint16_t EEPROM_CALIBRATION_CLT_VALUES = STORAGE_END-(uint16_t)sizeof(decltype(cltCalibrationTable)::values);
constexpr uint16_t EEPROM_CALIBRATION_CLT_BINS =  EEPROM_CALIBRATION_CLT_VALUES-(uint16_t)sizeof(decltype(cltCalibrationTable)::axis);
constexpr uint16_t EEPROM_CALIBRATION_IAT_VALUES = EEPROM_CALIBRATION_CLT_BINS-(uint16_t)sizeof(decltype(iatCalibrationTable)::values);
constexpr uint16_t EEPROM_CALIBRATION_IAT_BINS = EEPROM_CALIBRATION_IAT_VALUES-(uint16_t)sizeof(decltype(iatCalibrationTable)::axis);
constexpr uint16_t EEPROM_CALIBRATION_O2_VALUES = EEPROM_CALIBRATION_IAT_BINS-(uint16_t)sizeof(decltype(o2CalibrationTable)::values);
constexpr uint16_t EEPROM_CALIBRATION_O2_BINS = EEPROM_CALIBRATION_O2_VALUES-(uint16_t)sizeof(decltype(o2CalibrationTable)::axis);
constexpr uint16_t EEPROM_LAST_BARO = EEPROM_CALIBRATION_O2_BINS-(uint16_t)1;

constexpr uint16_t EEPROM_CONFIG1_MAP    = 3;
constexpr uint16_t EEPROM_CONFIG2_START  = 291;
constexpr uint16_t EEPROM_CONFIG3_MAP    = 421;
constexpr uint16_t EEPROM_CONFIG4_START  = 709;
constexpr uint16_t EEPROM_CONFIG5_MAP    = 839;
constexpr uint16_t EEPROM_CONFIG6_START  = 1127;
constexpr uint16_t EEPROM_CONFIG7_MAP1   = 1257;
constexpr uint16_t EEPROM_CONFIG7_MAP2   = 1339;
constexpr uint16_t EEPROM_CONFIG7_MAP3   = 1421;
constexpr uint16_t EEPROM_CONFIG8_MAP1   = 1503;
constexpr uint16_t EEPROM_CONFIG8_MAP2   = 1553;
constexpr uint16_t EEPROM_CONFIG8_MAP3   = 1603;
constexpr uint16_t EEPROM_CONFIG8_MAP4   = 1653;
constexpr uint16_t EEPROM_CONFIG9_START  = 1710;
constexpr uint16_t EEPROM_CONFIG10_START = 1902;
constexpr uint16_t EEPROM_CONFIG11_MAP   = 2096;
constexpr uint16_t EEPROM_CONFIG12_MAP   = 2387;
constexpr uint16_t EEPROM_CONFIG12_MAP2  = 2469;
constexpr uint16_t EEPROM_CONFIG12_MAP3  = 2551;
constexpr uint16_t EEPROM_CONFIG13_START = 2580;
constexpr uint16_t EEPROM_CONFIG14_MAP   = 2710;
// This is OUT OF ORDER as Page 8 was expanded to add fuel trim tables 5-8.
constexpr uint16_t EEPROM_CONFIG8_MAP5   = 3001;
constexpr uint16_t EEPROM_CONFIG8_MAP6   = 3051;
constexpr uint16_t EEPROM_CONFIG8_MAP7   = 3101;
constexpr uint16_t EEPROM_CONFIG8_MAP8   = 3151;
// Page 15 added after OUT OF ORDER page 8
constexpr uint16_t EEPROM_CONFIG15_MAP   = 3199;
constexpr uint16_t EEPROM_CONFIG15_START = 3281;

#if defined(UNIT_TEST)
extern const uint16_t MAX_PAGE_ADDRESS = EEPROM_LAST_BARO-sizeof(uint8_t);
extern const uint16_t STORAGE_SIZE = STORAGE_END;
#endif

static const void *getEntityPointer(const entity_t &entity)
{
  if (entity.type == EntityType::Table)
  {
    return entity.pTable;
  }
  if (entity.type == EntityType::Raw)
  {
    return entity.pRaw;
  }
  return nullptr;
}

// Maps an entity to its storage start address on the EEPROM.
//
// This is *THE* single source of truth for mapping the tune
// (I.e page entities) to EEPROM locations.
static uint16_t findEntityStartAddress(const entity_t &entity)
{
  const void *entityPointer = getEntityPointer(entity);
  if (entityPointer == nullptr)
  {
    return 0U;
  }

  const entity_storage_map_t *pMapEntry = getEntityStorageMap();
  const entity_storage_map_t *entityMapEnd = pMapEntry + getEntityStorageMapSize();

  while ((pMapEntry != entityMapEnd) && (entityPointer != pMapEntry->pEntity)) {
    ++pMapEntry;
  }
  uint16_t address = 0U;
  if (pMapEntry != entityMapEnd) {
    address = pMapEntry->eepromStartAddress;
  }
  return address;
}

#if defined(UNIT_TEST)
uint16_t getEntityStartAddress(page_iterator_t iter)
{
  return findEntityStartAddress(iter.entity);
}
#endif

//  ================================= Internal write support ===============================
struct write_location{
  uint16_t address;
  uint16_t writesRemaining;
};

static inline uint16_t write_range(const byte *pStart, const byte *pEnd, uint16_t address, uint16_t writesRemaining)
{
  return updateBlockLimitWriteOps(getStorageAPI(), address, pStart, pEnd, writesRemaining);
}

static inline write_location write(const table_row_iterator &row, const write_location &location)
{
  return { (uint16_t)(location.address+row.size()), updateBlockLimitWriteOps(getStorageAPI(), location.address, &*row, row.end(), location.writesRemaining) };
}

static inline write_location write(table_value_iterator it, write_location location)
{
  while (location.writesRemaining>0U && !it.at_end())
  {
    location = write(it.operator*(), location);
    ++it;
  }
  return location;
}

static inline write_location write(table_axis_iterator it, write_location location)
{
  while (location.writesRemaining>0U && !it.at_end())
  {
    if (update(getStorageAPI(), location.address, it.operator*())) {
      --location.writesRemaining;
    }
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

void saveAllPages(void)
{
  setEepromWritePending(false);

  uint8_t page = MIN_PAGE_NUM;
  while (page<MAX_PAGE_NUM && !isEepromWritePending())
  {
    savePage(page);
    ++page;
  }
}

static uint16_t writeEntity(const page_iterator_t &iter, uint16_t address, uint16_t writesRemaining)
{
  if (iter.entity.type == EntityType::Raw)
  {
    return write_range((byte *)iter.entity.pRaw, (byte *)iter.entity.pRaw + iter.entity.size, address, writesRemaining);
  }
  if (iter.entity.type == EntityType::Table)
  {
    return writeTable(iter.entity.pTable, iter.entity.table_key, address, writesRemaining);
  }
  return writesRemaining;
}

static bool saveMappedPage(uint8_t pageNum, uint16_t &writesRemaining)
{
  bool savedAny = false;
  page_iterator_t iter = page_begin(pageNum);
  while ((iter.entity.type != EntityType::End) && (writesRemaining > 0U))
  {
    uint16_t address = findEntityStartAddress(iter.entity);
    if ((address == 0U) && (getEntityPointer(iter.entity) != nullptr))
    {
      return false;
    }
    if (address != 0U)
    {
      writesRemaining = writeEntity(iter, address, writesRemaining);
      savedAny = true;
    }
    iter = advance(iter);
  }
  return savedAny;
}

void savePage(uint8_t pageNum)
{
  uint16_t writesRemaining = getStorageAPI().getMaxWriteBlockSize(currentStatus);

  if (!core_modules_save_page(pageNum, writesRemaining))
  {
    (void)saveMappedPage(pageNum, writesRemaining);
  }

  setEepromWritePending(writesRemaining==0U);
}

//  ================================= Internal read support ===============================
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

static void loadEntity(const page_iterator_t &iter, uint16_t address)
{
  if (iter.entity.type == EntityType::Raw)
  {
    (void)load_range(address, (byte *)iter.entity.pRaw, (byte *)iter.entity.pRaw + iter.entity.size);
  }
  else if (iter.entity.type == EntityType::Table)
  {
    (void)loadTable(iter.entity.pTable, iter.entity.table_key, address);
  }
  else
  {
  }
}

static void loadMappedPage(uint8_t pageNum)
{
  page_iterator_t iter = page_begin(pageNum);
  while (iter.entity.type != EntityType::End)
  {
    uint16_t address = findEntityStartAddress(iter.entity);
    if (address != 0U)
    {
      loadEntity(iter, address);
    }
    iter = advance(iter);
  }
}

void loadAllPages(void)
{
  for (uint8_t page = MIN_PAGE_NUM; page < MAX_PAGE_NUM; ++page)
  {
    loadMappedPage(page);
  }
  core_modules_load_pages();
}

#if defined(CORE_AVR)
#pragma GCC pop_options
#endif
