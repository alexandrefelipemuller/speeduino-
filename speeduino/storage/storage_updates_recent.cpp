/** @file
 * Recent EEPROM Storage updates.
 */

#include "storage/storage.h"
#include "storage/pages.h"
#include "storage/updates.h"
#include "modules/wmi/wmi_storage.h"
#include "config_pages.h"
#include "data/tune_registry.h"
#include "data/table_registry.h"
#include "comms/comms_CAN.h"
#include "engine/sensors.h"
#include "support/units.h"

#pragma GCC optimize ("Os")

static constexpr uint8_t CURRENT_DATA_VERSION = 27U;

void updateTableU16toU8(table2D_u16_u8_32 &targetTable, uint16_t u16EEpromBinAddress)
{
  uint16_t oldValues[32];
  static_assert(targetTable.size()==32U, "Calibration size change - fix this!");
  (void)loadObject(getStorageAPI(), u16EEpromBinAddress, targetTable.axis);
  (void)loadObject(getStorageAPI(), u16EEpromBinAddress+sizeof(oldValues), oldValues);
  for (uint8_t i = 0; i < targetTable.size(); i++) { targetTable.values[i] = (uint8_t)oldValues[i]; }
}

void upgradeV25toV26(void)
{
  if(loadEEPROMVersion() == 25U)
  {
    constexpr uint16_t OLD_VALUE_SIZE = sizeof(table2D_u16_u16_32::values);
    constexpr uint16_t STORAGE_END = 0xFFF;
    constexpr uint16_t V25_EEPROM_CALIBRATION_CLT_VALUES = STORAGE_END-OLD_VALUE_SIZE;
    constexpr uint16_t V25_EEPROM_CALIBRATION_CLT_BINS =  V25_EEPROM_CALIBRATION_CLT_VALUES-(uint16_t)sizeof(decltype(o2CalibrationTable)::axis);
    constexpr uint16_t V25_EEPROM_CALIBRATION_IAT_VALUES = V25_EEPROM_CALIBRATION_CLT_BINS-OLD_VALUE_SIZE;
    constexpr uint16_t V25_EEPROM_CALIBRATION_IAT_BINS = V25_EEPROM_CALIBRATION_IAT_VALUES-(uint16_t)sizeof(decltype(o2CalibrationTable)::axis);
    constexpr uint16_t V25_EEPROM_CALIBRATION_O2_VALUES = V25_EEPROM_CALIBRATION_IAT_BINS-(uint16_t)sizeof(decltype(o2CalibrationTable)::values);
    constexpr uint16_t V25_EEPROM_CALIBRATION_O2_BINS =   V25_EEPROM_CALIBRATION_O2_VALUES-(uint16_t)sizeof(decltype(o2CalibrationTable)::axis);
    constexpr uint16_t V25_EEPROM_LAST_BARO = (V25_EEPROM_CALIBRATION_O2_BINS-(uint16_t)1);
    updateTableU16toU8(cltCalibrationTable, V25_EEPROM_CALIBRATION_CLT_BINS);
    updateTableU16toU8(iatCalibrationTable, V25_EEPROM_CALIBRATION_IAT_BINS);
    (void)loadObject(getStorageAPI(), V25_EEPROM_CALIBRATION_O2_BINS, o2CalibrationTable.axis);
    (void)loadObject(getStorageAPI(), V25_EEPROM_CALIBRATION_O2_VALUES, o2CalibrationTable.values);
    saveLastBaro(getStorageAPI().read(V25_EEPROM_LAST_BARO));
    saveAllCalibrationTables();
    saveEEPROMVersion(26);
  }
}

static void upgradeV26toV27(void)
{
  if(loadEEPROMVersion() == 26U)
  {
    configPage6.afrLoadSource = AFR_LOAD_PRIMARY;
    saveAllPages();
    saveEEPROMVersion(27);
  }
}

void runRecentStorageUpdates(void)
{
  if(loadEEPROMVersion() == 16)
  {
    constexpr uint16_t EEPROM_CONFIG14_END_V16 = 2998U;
    constexpr uint16_t EEPROM_CONFIG13_START_V16 = 2580U;
    constexpr uint16_t SHIFT_DISTANCE = 112U;
    for(uint16_t x=EEPROM_CONFIG14_END_V16; x>=EEPROM_CONFIG13_START_V16; x--)
    {
      (void)update(getStorageAPI(), x, getStorageAPI().read(x-SHIFT_DISTANCE));
    }
    configPage6.iacPWMrun = false;
    configPage2.useDwellMap = 0;
    saveAllPages();
    saveEEPROMVersion(17);
  }

  if(loadEEPROMVersion() == 17)
  {
    auto table_it = vvtTable.values.begin();
    while (!table_it.at_end())
    {
      auto row = *table_it;
      while (!row.at_end())
      {
        *row = *row << 1;
        ++row;
      }
      ++table_it;
    }
    configPage10.vvtCLholdDuty = configPage10.vvtCLholdDuty << 1;
    configPage10.vvtCLminDuty = configPage10.vvtCLminDuty << 1;
    configPage10.vvtCLmaxDuty = configPage10.vvtCLmaxDuty << 1;
    configPage10.vvt2Enabled = 0;
    configPage4.vvt2PWMdir = 0;
    configPage10.TrigEdgeThrd = 0;
    if(configPage6.tachoMode == 1) { configPage6.vvtMode = VVT_MODE_ONOFF; }
    configPage10.vvtCLMinAng = 0;
    configPage10.vvtCLMaxAng = 200;
    configPage4.ANGLEFILTER_VVT = 0;
    configPage2.idleAdvDelay *= 2;
    configPage2.mapSwitchPoint = 0;
    configPage9.boostByGearEnabled = 0;
    configPage13.outputTimeLimit[0] = 0;
    configPage13.outputTimeLimit[1] = 0;
    configPage13.outputTimeLimit[2] = 0;
    configPage13.outputTimeLimit[3] = 0;
    configPage13.outputTimeLimit[4] = 0;
    configPage13.outputTimeLimit[5] = 0;
    configPage13.outputTimeLimit[6] = 0;
    configPage13.outputTimeLimit[7] = 0;
    saveAllPages();
    saveEEPROMVersion(18);
  }

  if(loadEEPROMVersion() == 18)
  {
    configPage2.fanEnable = configPage6.fanUnused;
    configPage2.idleAdvTPS *= 2;
    configPage2.iacTPSlimit *= 2;
    configPage4.floodClear *= 2;
    configPage4.dfcoTPSThresh *= 2;
    configPage6.egoTPSMax *= 2;
    configPage10.lnchCtrlTPS *= 2;
    configPage10.wmiTPS *= 2;
    configPage10.n2o_minTPS *= 2;
    if(configPage10.fuel2SwitchVariable == FUEL2_CONDITION_TPS) { configPage10.fuel2SwitchValue *= 2; }
    if(configPage10.spark2SwitchVariable == SPARK2_CONDITION_TPS) { configPage10.spark2SwitchVariable *= 2; }
    if(configPage2.fuelAlgorithm == LOAD_SOURCE_TPS)
    {
      multiplyTableLoad(&fuelTable,  fuelTable.type_key,  4);
      multiplyTableLoad(&afrTable,   afrTable.type_key,   4);
      multiplyTableLoad(&trim1Table, trim1Table.type_key, 4);
      multiplyTableLoad(&trim2Table, trim2Table.type_key, 4);
      multiplyTableLoad(&trim3Table, trim3Table.type_key, 4);
      multiplyTableLoad(&trim4Table, trim4Table.type_key, 4);
      multiplyTableLoad(&trim5Table, trim5Table.type_key, 4);
      multiplyTableLoad(&trim6Table, trim6Table.type_key, 4);
      multiplyTableLoad(&trim7Table, trim7Table.type_key, 4);
      multiplyTableLoad(&trim8Table, trim8Table.type_key, 4);
      if(configPage4.sparkMode == IGN_MODE_ROTARY)
      {
        for(uint8_t x = 0; x < 8; x++) { configPage10.rotarySplitBins[x] *= 2; }
      }
    }
    if(configPage2.ignAlgorithm == LOAD_SOURCE_TPS) { multiplyTableLoad(&ignitionTable, ignitionTable.type_key, 4); }
    if(configPage10.fuel2Algorithm == LOAD_SOURCE_TPS) { multiplyTableLoad(&fuelTable2, fuelTable2.type_key, 4); }
    if(configPage10.spark2Algorithm == LOAD_SOURCE_TPS) { multiplyTableLoad(&ignitionTable2, ignitionTable2.type_key, 4); }
    multiplyTableLoad(&boostTable, boostTable.type_key, 2);
    if(configPage6.vvtLoadSource == VVT_LOAD_TPS)
    {
      multiplyTableLoad(&vvtTable, vvtTable.type_key, 2);
      multiplyTableLoad(&vvt2Table, vvt2Table.type_key, 2);
    }
    else
    {
      divideTableLoad(&vvtTable, vvtTable.type_key, 2);
      divideTableLoad(&vvt2Table, vvt2Table.type_key, 2);
    }
    configPage4.vvtDelay = 0;
    configPage4.vvtMinClt = 0;
    configPage13.onboard_log_csv_separator = 0;
    configPage13.onboard_log_file_style = 0;
    configPage13.onboard_log_file_rate = 0;
    configPage13.onboard_log_filenaming = 0;
    configPage13.onboard_log_storage = 0;
    configPage13.onboard_log_trigger_boot = 0;
    configPage13.onboard_log_trigger_RPM = 0;
    configPage13.onboard_log_trigger_prot = 0;
    configPage13.onboard_log_trigger_Vbat = 0;
    configPage13.onboard_log_trigger_Epin = 0;
    configPage13.onboard_log_tr1_duration = 0;
    configPage13.onboard_log_tr2_thr_on = 0;
    configPage13.onboard_log_tr2_thr_off = 0;
    configPage13.onboard_log_tr3_thr_RPM = 0;
    configPage13.onboard_log_tr3_thr_MAP = 0;
    configPage13.onboard_log_tr3_thr_Oil = 0;
    configPage13.onboard_log_tr3_thr_AFR = 0;
    configPage13.onboard_log_tr4_thr_on = 0;
    configPage13.onboard_log_tr4_thr_off = 0;
    configPage13.onboard_log_tr5_Epin_pin = 0;
    saveAllPages();
    saveEEPROMVersion(19);
  }

  if(loadEEPROMVersion() == 19)
  {
    if( configPage4.inj4cylPairing > INJ_PAIR_14_23 ) { configPage4.inj4cylPairing = 0; }
    if( configPage2.nCylinders == 4 )
    {
      if ( configPage2.injLayout == INJ_SEQUENTIAL ) { configPage4.inj4cylPairing = INJ_PAIR_13_24; }
      else { configPage4.inj4cylPairing = INJ_PAIR_14_23; }
    }
    configPage9.hardRevMode = 1;
    configPage6.tachoMode = 0;
    configPage4.CANBroadcastProtocol = CAN_BROADCAST_PROTOCOL_OFF;
    configPage15.boostDCWhenDisabled = 0;
    configPage15.boostControlEnable = EN_BOOST_CONTROL_BARO;
    auto table_it = boostTableLookupDuty.values.begin();
    while (!table_it.at_end())
    {
      auto row = *table_it;
      while (!row.at_end()) { *row = 50*2; ++row; }
      ++table_it;
    }
    auto table_X = boostTableLookupDuty.axisX.begin();
    uint16_t i = 0;
    while (!table_X.at_end()) { ++i; *table_X = 1000+(500*i); ++table_X; }
    auto table_Y = boostTableLookupDuty.axisY.begin();
    i = 0;
    while (!table_Y.at_end()) { ++i; *table_Y = (120 + 10*i); ++table_Y; }
    configPage9.afrProtectEnabled = AFR_PROTECT_OFF;
    configPage9.afrProtectMinMAP = 90;
    configPage9.afrProtectMinRPM = 40;
    configPage9.afrProtectMinTPS = 160;
    configPage9.afrProtectDeviation = 14;
    saveAllPages();
    saveEEPROMVersion(20);
  }

  if(loadEEPROMVersion() == 20)
  {
    configPage2.taeMinChange = 4;
    configPage2.maeMinChange = 2;
    configPage2.decelAmount = 100;
    for (uint8_t y = 0; y < sizeof(configPage13.outputPin); y++)
    {
      if ((configPage13.firstDataIn[y] > 22) && (configPage13.firstDataIn[y] < 240)) {configPage13.firstDataIn[y]++;}
      if ((configPage13.firstDataIn[y] > 92) && (configPage13.firstDataIn[y] < 240)) {configPage13.firstDataIn[y]++;}
      if ((configPage13.secondDataIn[y] > 22) && (configPage13.secondDataIn[y] < 240)) {configPage13.secondDataIn[y]++;}
      if ((configPage13.secondDataIn[y] > 92) && (configPage13.secondDataIn[y] < 240)) {configPage13.secondDataIn[y]++;}
    }
    configPage15.airConEnable = 0;
    configPage10.oilPressureProtTime = 0;
    configPage9.iacStepperPower = 0;
    saveAllPages();
    saveEEPROMVersion(21);
  }

  if(loadEEPROMVersion() == 21)
  {
    configPage15.rollingProtRPMDelta[0]   = -30;
    configPage15.rollingProtRPMDelta[1]   = -20;
    configPage15.rollingProtRPMDelta[2]   = -10;
    configPage15.rollingProtRPMDelta[3]   = -5;
    configPage15.rollingProtCutPercent[0] = 50;
    configPage15.rollingProtCutPercent[1] = 65;
    configPage15.rollingProtCutPercent[2] = 80;
    configPage15.rollingProtCutPercent[3] = 95;
    configPage4.dfcoHyster = configPage4.dfcoHyster / 2;
    saveAllPages();
    saveEEPROMVersion(22);
  }

  if(loadEEPROMVersion() == 22)
  {
    module_wmi_upgrade_v22();
    configPage13.hwTestInjDuration = 8;
    configPage13.hwTestIgnDuration = 4;
    configPage9.dfcoTaperEnable = 0;
    configPage9.dfcoTaperTime = 10;
    configPage9.dfcoTaperFuel = 100;
    configPage9.dfcoTaperAdvance = 20;
    configPage9.egoMAPMax = 255;
    configPage9.egoMAPMin = 0;
    configPage2.canWBO = 0;
    saveAllPages();
    saveEEPROMVersion(23);
  }

  if(loadEEPROMVersion() == 23)
  {
    configPage10.knock_mode = KNOCK_MODE_OFF;
    if(configPage2.unused1_126_1 == true) { configPage4.CANBroadcastProtocol = CAN_BROADCAST_PROTOCOL_BMW; }
    if(configPage2.unused1_126_2 == true) { configPage4.CANBroadcastProtocol = CAN_BROADCAST_PROTOCOL_VAG; }
    configPage10.lnchCtrlVss = 255;
    configPage2.flexFreqLow = 50;
    configPage2.flexFreqHigh = 150;
    uint8_t origBoostIntv = ((uint8_t *)&configPage10)[27];
    ((uint8_t *)&configPage10)[27] = ((uint8_t *)&configPage10)[26];
    ((uint8_t *)&configPage10)[26] = ((uint8_t *)&configPage10)[25];
    ((uint8_t *)&configPage10)[25] = origBoostIntv;
    uint8_t origlnchCtrlTPS= ((uint8_t *)&configPage10)[32];
    for(byte x=32U; x<74U; x++) { ((uint8_t *)&configPage10)[x] = ((uint8_t *)&configPage10)[x+1]; }
    ((uint8_t *)&configPage10)[74] = origlnchCtrlTPS;
    saveAllPages();
    saveEEPROMVersion(24);
  }

  if(loadEEPROMVersion() == 24) { saveEEPROMVersion(25); }
  upgradeV25toV26();
  upgradeV26toV27();

  if( (loadEEPROMVersion() == 0) || (loadEEPROMVersion() == 255) )
  {
    configPage9.true_address = 0x200;
    configPage13.outputPin[0] = 0;
    configPage13.outputPin[1] = 0;
    configPage13.outputPin[2] = 0;
    configPage13.outputPin[3] = 0;
    configPage13.outputPin[4] = 0;
    configPage13.outputPin[5] = 0;
    configPage13.outputPin[6] = 0;
    configPage13.outputPin[7] = 0;
    configPage4.FILTER_FLEX = FILTER_FLEX_DEFAULT;
    saveEEPROMVersion(CURRENT_DATA_VERSION);
  }

  if( loadEEPROMVersion() > CURRENT_DATA_VERSION ) { saveEEPROMVersion(CURRENT_DATA_VERSION); }
}
