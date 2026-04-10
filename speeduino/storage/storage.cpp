/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
*/
/** @file
 * Lower level ConfigPage*, Table2D, Table3D and EEPROM storage operations.
 */

#include "storage/storage.h"
#include "data/runtime_state.h"
#include "engine/sensors.h"
#include "data/table_registry.h"
#include "support/utilities.h"
#include "support/preprocessor.h"
#include "support/unit_testing.h"

#if defined(CORE_AVR)
#pragma GCC push_options
// This minimizes RAM usage at no performance cost
#pragma GCC optimize ("Os") 
#endif

// LCOV_EXCL_START
// Exclude from coverage: these are only here for completeness
// and will be replaced during normal firmware startup
static byte default_read(uint16_t address) { UNUSED(address); return 0U; }
static void default_write(uint16_t address, byte value) { UNUSED(address); UNUSED(value); }
static uint16_t default_length(void) { return 0U; }
static uint16_t default_write_size(const statuses &current) { UNUSED(current); return 8U; }
// LCOV_EXCL_STOP

static storage_api_t externalApi = {
    .read = default_read,
    .write = default_write,
    .length = default_length,
    .getMaxWriteBlockSize = default_write_size,
  };

// Calibration data is stored at the end of the EEPROM (This is in case any further calibration tables are needed as they are large blocks)
constexpr uint16_t EEPROM_DATA_VERSION = 0;
constexpr uint16_t STORAGE_END = 0xFFF;
constexpr uint16_t EEPROM_CALIBRATION_CLT_VALUES = STORAGE_END-(uint16_t)sizeof(decltype(cltCalibrationTable)::values);
constexpr uint16_t EEPROM_CALIBRATION_CLT_BINS =  EEPROM_CALIBRATION_CLT_VALUES-(uint16_t)sizeof(decltype(cltCalibrationTable)::axis);
constexpr uint16_t EEPROM_CALIBRATION_IAT_VALUES = EEPROM_CALIBRATION_CLT_BINS-(uint16_t)sizeof(decltype(iatCalibrationTable)::values);
constexpr uint16_t EEPROM_CALIBRATION_IAT_BINS = EEPROM_CALIBRATION_IAT_VALUES-(uint16_t)sizeof(decltype(iatCalibrationTable)::axis);
constexpr uint16_t EEPROM_CALIBRATION_O2_VALUES = EEPROM_CALIBRATION_IAT_BINS-(uint16_t)sizeof(decltype(o2CalibrationTable)::values);
constexpr uint16_t EEPROM_CALIBRATION_O2_BINS =   EEPROM_CALIBRATION_O2_VALUES-(uint16_t)sizeof(decltype(o2CalibrationTable)::axis);
constexpr uint16_t EEPROM_LAST_BARO = (EEPROM_CALIBRATION_O2_BINS-(uint16_t)1);

// LCOV_EXCL_START
// Exclude simple getter/setter from code coverage
void setStorageAPI(const storage_api_t &api) 
{
  externalApi = api;
}

/** @brief Provide global access to the raw storage API */
const storage_api_t& getStorageAPI(void)
{
  return externalApi;
}
// LCOV_EXCL_STOP

#if defined(UNIT_TEST)
#endif

// LCOV_EXCL_START
// Exclude simple getter/setter from code coverage
bool isEepromWritePending(void)
{
  return currentStatus.burnPending;
}
void setEepromWritePending(bool isPending)
{
  currentStatus.burnPending = isPending;
}
// LCOV_EXCL_STOP

void loadAllCalibrationTables(void)
{
  // If you modify this function be sure to also modify saveAllCalibrationTables();
  // it should be a mirror image of this function.

  (void)loadObject(getStorageAPI(), EEPROM_CALIBRATION_O2_BINS, o2CalibrationTable.axis);
  (void)loadObject(getStorageAPI(), EEPROM_CALIBRATION_O2_VALUES, o2CalibrationTable.values);
  
  (void)loadObject(getStorageAPI(), EEPROM_CALIBRATION_IAT_BINS, iatCalibrationTable.axis);
  (void)loadObject(getStorageAPI(), EEPROM_CALIBRATION_IAT_VALUES, iatCalibrationTable.values);

  (void)loadObject(getStorageAPI(), EEPROM_CALIBRATION_CLT_BINS, cltCalibrationTable.axis);
  (void)loadObject(getStorageAPI(), EEPROM_CALIBRATION_CLT_VALUES, cltCalibrationTable.values);
}

/** Write calibration tables to EEPROM.
This takes the values in the 3 calibration tables (Coolant, Inlet temp and O2)
and saves them to the EEPROM.
*/
void saveAllCalibrationTables(void)
{
  // If you modify this function be sure to also modify loadAllCalibrationTables();
  // it should be a mirror image of this function.

  saveCalibrationTable(SensorCalibrationTable::O2Sensor);
  saveCalibrationTable(SensorCalibrationTable::IntakeAirTempSensor);
  saveCalibrationTable(SensorCalibrationTable::CoolantSensor);
}

void saveCalibrationTable(SensorCalibrationTable sensor)
{
  if(sensor == SensorCalibrationTable::O2Sensor)
  {
    updateObject(getStorageAPI(), o2CalibrationTable.axis, EEPROM_CALIBRATION_O2_BINS);
    updateObject(getStorageAPI(), o2CalibrationTable.values, EEPROM_CALIBRATION_O2_VALUES);
  }
  else if(sensor == SensorCalibrationTable::IntakeAirTempSensor)
  {
    updateObject(getStorageAPI(), iatCalibrationTable.axis, EEPROM_CALIBRATION_IAT_BINS);
    updateObject(getStorageAPI(), iatCalibrationTable.values, EEPROM_CALIBRATION_IAT_VALUES);
  }
  else if(sensor == SensorCalibrationTable::CoolantSensor)
  {
    updateObject(getStorageAPI(), cltCalibrationTable.axis, EEPROM_CALIBRATION_CLT_BINS);
    updateObject(getStorageAPI(), cltCalibrationTable.values, EEPROM_CALIBRATION_CLT_VALUES);
  } else {
    // Unknown sensor identifier - do nothing but keep MISRA checker happy
  }
}

TESTABLE_INLINE_STATIC uint16_t getSensorCalibrationCrcAddress(SensorCalibrationTable sensor) {
  constexpr uint16_t EEPROM_CALIBRATION_CLT_CRC = 3674;
  constexpr uint16_t EEPROM_CALIBRATION_IAT_CRC = 3678;
  constexpr uint16_t EEPROM_CALIBRATION_O2_CRC = 3682;

  switch(sensor)
  {
    case SensorCalibrationTable::O2Sensor:
      return EEPROM_CALIBRATION_O2_CRC;
    case SensorCalibrationTable::IntakeAirTempSensor:
      return EEPROM_CALIBRATION_IAT_CRC;
    case SensorCalibrationTable::CoolantSensor:
    default: //Obviously should never happen
      return EEPROM_CALIBRATION_CLT_CRC;
  }
  return EEPROM_CALIBRATION_CLT_CRC;
}

// LCOV_EXCL_START
// Exclude simple wrappers from code coverage
void saveCalibrationCrc(SensorCalibrationTable sensor, uint32_t calibrationCRC)
{
  updateObject(getStorageAPI(), calibrationCRC, getSensorCalibrationCrcAddress(sensor));
}

/** Retrieves and returns the 4 byte CRC32 checksum for a given calibration page from EEPROM. */
uint32_t loadCalibrationCrc(SensorCalibrationTable sensor)
{
  uint32_t crc32_val;
  return loadObject(getStorageAPI(), getSensorCalibrationCrcAddress(sensor), crc32_val);
}
// LCOV_EXCL_STOP

// Utility functions.
// By having these in this file, it prevents other files from calling EEPROM functions directly. This is useful due to differences in the EEPROM libraries on different devces

// LCOV_EXCL_START
// Exclude simple getter/setter from code coverage
uint8_t loadLastBaro(void)
{ 
  return getStorageAPI().read(EEPROM_LAST_BARO); 
}
void saveLastBaro(uint8_t newValue)
{ 
  (void)update(getStorageAPI(), EEPROM_LAST_BARO, newValue); 
}
// LCOV_EXCL_STOP

// LCOV_EXCL_START
// Exclude simple getter/setter from code coverage
uint8_t loadEEPROMVersion(void)
{ 
  return getStorageAPI().read(EEPROM_DATA_VERSION); 
}
void saveEEPROMVersion(uint8_t newVersion)
{ 
  (void)update(getStorageAPI(), EEPROM_DATA_VERSION, newVersion); 
}
// LCOV_EXCL_STOP

#if defined(CORE_AVR)
#pragma GCC pop_options
#endif
