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

// Maps an entity to its storage start address on the EEPROM.
//
// This is *THE* single source of truth for mapping the tune
// (I.e page entities) to EEPROM locations.
uint16_t getEntityStartAddress(page_iterator_t iter)
{
  const entity_storage_map_t *pMapEntry = getEntityStorageMap();
  const entity_storage_map_t *entityMapEnd = pMapEntry + getEntityStorageMapSize();

  while ((pMapEntry != entityMapEnd) && (iter.entity.pRaw != pgm_read_ptr(&pMapEntry->pEntity))) {
    ++pMapEntry;
  }
  uint16_t address = 0U;
  if (pMapEntry != entityMapEnd) {
    address = pgm_read_word(&(pMapEntry->eepromStartAddress));
  }
  return address;
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

void savePage(uint8_t pageNum)
{
  uint16_t writesRemaining = getStorageAPI().getMaxWriteBlockSize(currentStatus);

  switch(pageNum)
  {
    case veMapPage:
      writesRemaining = writeTable(&fuelTable, decltype(fuelTable)::type_key, EEPROM_CONFIG1_MAP, writesRemaining);
      break;
    case veSetPage:
      writesRemaining = write_range((byte *)&configPage2, (byte *)&configPage2+sizeof(configPage2), EEPROM_CONFIG2_START, writesRemaining);
      break;
    case ignMapPage:
      writesRemaining = writeTable(&ignitionTable, decltype(ignitionTable)::type_key, EEPROM_CONFIG3_MAP, writesRemaining);
      break;
    case ignSetPage:
      writesRemaining = write_range((byte *)&configPage4, (byte *)&configPage4+sizeof(configPage4), EEPROM_CONFIG4_START, writesRemaining);
      break;
    case afrMapPage:
      writesRemaining = writeTable(&afrTable, decltype(afrTable)::type_key, EEPROM_CONFIG5_MAP, writesRemaining);
      break;
    case afrSetPage:
      writesRemaining = write_range((byte *)&configPage6, (byte *)&configPage6+sizeof(configPage6), EEPROM_CONFIG6_START, writesRemaining);
      break;
    case boostvvtPage:
      writesRemaining = writeTable(&boostTable, decltype(boostTable)::type_key, EEPROM_CONFIG7_MAP1, writesRemaining);
      writesRemaining = writeTable(&vvtTable, decltype(vvtTable)::type_key, EEPROM_CONFIG7_MAP2, writesRemaining);
      writesRemaining = writeTable(&stagingTable, decltype(stagingTable)::type_key, EEPROM_CONFIG7_MAP3, writesRemaining);
      break;
    case seqFuelPage:
      writesRemaining = writeTable(&trim1Table, decltype(trim1Table)::type_key, EEPROM_CONFIG8_MAP1, writesRemaining);
      writesRemaining = writeTable(&trim2Table, decltype(trim2Table)::type_key, EEPROM_CONFIG8_MAP2, writesRemaining);
      writesRemaining = writeTable(&trim3Table, decltype(trim3Table)::type_key, EEPROM_CONFIG8_MAP3, writesRemaining);
      writesRemaining = writeTable(&trim4Table, decltype(trim4Table)::type_key, EEPROM_CONFIG8_MAP4, writesRemaining);
      writesRemaining = writeTable(&trim5Table, decltype(trim5Table)::type_key, EEPROM_CONFIG8_MAP5, writesRemaining);
      writesRemaining = writeTable(&trim6Table, decltype(trim6Table)::type_key, EEPROM_CONFIG8_MAP6, writesRemaining);
      writesRemaining = writeTable(&trim7Table, decltype(trim7Table)::type_key, EEPROM_CONFIG8_MAP7, writesRemaining);
      writesRemaining = writeTable(&trim8Table, decltype(trim8Table)::type_key, EEPROM_CONFIG8_MAP8, writesRemaining);
      break;
    case canbusPage:
      writesRemaining = write_range((byte *)&configPage9, (byte *)&configPage9+sizeof(configPage9), EEPROM_CONFIG9_START, writesRemaining);
      break;
    case warmupPage:
      writesRemaining = write_range((byte *)&configPage10, (byte *)&configPage10+sizeof(configPage10), EEPROM_CONFIG10_START, writesRemaining);
      break;
    case fuelMap2Page:
      writesRemaining = writeTable(&fuelTable2, decltype(fuelTable2)::type_key, EEPROM_CONFIG11_MAP, writesRemaining);
      break;
    case wmiMapPage:
      writesRemaining = writeTable(&wmiTable, decltype(wmiTable)::type_key, EEPROM_CONFIG12_MAP, writesRemaining);
      writesRemaining = writeTable(&vvt2Table, decltype(vvt2Table)::type_key, EEPROM_CONFIG12_MAP2, writesRemaining);
      writesRemaining = writeTable(&dwellTable, decltype(dwellTable)::type_key, EEPROM_CONFIG12_MAP3, writesRemaining);
      break;
    case progOutsPage:
      writesRemaining = write_range((byte *)&configPage13, (byte *)&configPage13+sizeof(configPage13), EEPROM_CONFIG13_START, writesRemaining);
      break;
    case ignMap2Page:
      writesRemaining = writeTable(&ignitionTable2, decltype(ignitionTable2)::type_key, EEPROM_CONFIG14_MAP, writesRemaining);
      break;
    case boostvvtPage2:
      writesRemaining = writeTable(&boostTableLookupDuty, decltype(boostTableLookupDuty)::type_key, EEPROM_CONFIG15_MAP, writesRemaining);
      writesRemaining = write_range((byte *)&configPage15, (byte *)&configPage15+sizeof(configPage15), EEPROM_CONFIG15_START, writesRemaining);
      break;
    default:
      break;
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

void loadAllPages(void)
{
  (void)loadTable(&fuelTable, decltype(fuelTable)::type_key, EEPROM_CONFIG1_MAP);
  (void)load_range(EEPROM_CONFIG2_START, (byte *)&configPage2, (byte *)&configPage2+sizeof(configPage2));
  (void)loadTable(&ignitionTable, decltype(ignitionTable)::type_key, EEPROM_CONFIG3_MAP);
  (void)load_range(EEPROM_CONFIG4_START, (byte *)&configPage4, (byte *)&configPage4+sizeof(configPage4));
  (void)loadTable(&afrTable, decltype(afrTable)::type_key, EEPROM_CONFIG5_MAP);
  (void)load_range(EEPROM_CONFIG6_START, (byte *)&configPage6, (byte *)&configPage6+sizeof(configPage6));
  (void)loadTable(&boostTable, decltype(boostTable)::type_key, EEPROM_CONFIG7_MAP1);
  (void)loadTable(&vvtTable, decltype(vvtTable)::type_key,  EEPROM_CONFIG7_MAP2);
  (void)loadTable(&stagingTable, decltype(stagingTable)::type_key, EEPROM_CONFIG7_MAP3);
  (void)loadTable(&trim1Table, decltype(trim1Table)::type_key, EEPROM_CONFIG8_MAP1);
  (void)loadTable(&trim2Table, decltype(trim2Table)::type_key, EEPROM_CONFIG8_MAP2);
  (void)loadTable(&trim3Table, decltype(trim3Table)::type_key, EEPROM_CONFIG8_MAP3);
  (void)loadTable(&trim4Table, decltype(trim4Table)::type_key, EEPROM_CONFIG8_MAP4);
  (void)loadTable(&trim5Table, decltype(trim5Table)::type_key, EEPROM_CONFIG8_MAP5);
  (void)loadTable(&trim6Table, decltype(trim6Table)::type_key, EEPROM_CONFIG8_MAP6);
  (void)loadTable(&trim7Table, decltype(trim7Table)::type_key, EEPROM_CONFIG8_MAP7);
  (void)loadTable(&trim8Table, decltype(trim8Table)::type_key, EEPROM_CONFIG8_MAP8);
  (void)load_range(EEPROM_CONFIG9_START, (byte *)&configPage9, (byte *)&configPage9+sizeof(configPage9));
  (void)load_range(EEPROM_CONFIG10_START, (byte *)&configPage10, (byte *)&configPage10+sizeof(configPage10));
  (void)loadTable(&fuelTable2, decltype(fuelTable2)::type_key, EEPROM_CONFIG11_MAP);
  (void)loadTable(&wmiTable, decltype(wmiTable)::type_key, EEPROM_CONFIG12_MAP);
  (void)loadTable(&vvt2Table, decltype(vvt2Table)::type_key, EEPROM_CONFIG12_MAP2);
  (void)loadTable(&dwellTable, decltype(dwellTable)::type_key, EEPROM_CONFIG12_MAP3);
  (void)load_range(EEPROM_CONFIG13_START, (byte *)&configPage13, (byte *)&configPage13+sizeof(configPage13));
  (void)loadTable(&ignitionTable2, decltype(ignitionTable2)::type_key, EEPROM_CONFIG14_MAP);
  (void)loadTable(&boostTableLookupDuty, decltype(boostTableLookupDuty)::type_key, EEPROM_CONFIG15_MAP);
  (void)load_range(EEPROM_CONFIG15_START, (byte *)&configPage15, (byte *)&configPage15+sizeof(configPage15));
}

#if defined(CORE_AVR)
#pragma GCC pop_options
#endif
