/** @file
 * Legacy EEPROM Storage updates.
 */

#include "storage/storage.h"
#include "storage/pages.h"
#include "data/tune_registry.h"
#include "data/table_registry.h"
#include "engine/sensors.h"
#include "support/units.h"

#pragma GCC optimize ("Os")

void runLegacyStorageUpdates(void)
{
  // Only the latest update for small flash devices must be retained.
#ifndef SMALL_FLASH_MODE
  if(loadEEPROMVersion() == 2)
  {
    auto table_it = ignitionTable.values.begin();
    for(uint8_t x=0; x<ignitionTable.values.num_rows; x++)
    {
      auto row = *table_it;
      while (!row.at_end())
      {
        *row = *row + 40;
        ++row;
      }
      ++table_it;
    }
    saveAllPages();
    saveEEPROMVersion(3);
  }

  if(loadEEPROMVersion() == 3)
  {
    configPage9.speeduino_tsCanId = 0;
    configPage9.true_address = 256;
    configPage9.realtime_base_address = 336;
    if(configPage4.sparkDur == UINT8_MAX) { configPage4.sparkDur = 10; }
    saveAllPages();
    saveEEPROMVersion(4);
  }

  if(loadEEPROMVersion() == 4)
  {
    configPage10.crankingEnrichBins[0] = 0;
    configPage10.crankingEnrichBins[1] = 40;
    configPage10.crankingEnrichBins[2] = 70;
    configPage10.crankingEnrichBins[3] = 100;
    configPage10.crankingEnrichValues[0] = 100 + configPage2.crankingPct;
    configPage10.crankingEnrichValues[1] = 100 + configPage2.crankingPct;
    configPage10.crankingEnrichValues[2] = 100 + configPage2.crankingPct;
    configPage10.crankingEnrichValues[3] = 100 + configPage2.crankingPct;
    saveAllPages();
    saveEEPROMVersion(5);
  }

  if(loadEEPROMVersion() == 5)
  {
    constexpr uint16_t EEPROM_CONFIG10_END_V6 = 2094U;
    for(uint16_t x=0U; x < 1152U; x++)
    {
      uint16_t endMem = EEPROM_CONFIG10_END_V6 - x;
      uint16_t startMem = endMem - 128U;
      (void)update(getStorageAPI(), endMem, getStorageAPI().read(startMem));
    }
    for(uint16_t x=0; x < 352U; x++)
    {
      uint16_t endMem = EEPROM_CONFIG10_END_V6 - 1152U - x;
      uint16_t startMem = endMem - 64U;
      (void)update(getStorageAPI(), endMem, getStorageAPI().read(startMem));
    }
    saveEEPROMVersion(6);
    loadAllPages();
  }

  if(loadEEPROMVersion() == 6)
  {
    constexpr uint16_t EEPROM_CONFIG10_END_V7 = 2094U;
    for(uint16_t x=0U; x < 529U; x++)
    {
      uint16_t endMem = EEPROM_CONFIG10_END_V7 - x;
      uint16_t startMem = endMem - 82U;
      (void)update(getStorageAPI(), endMem, getStorageAPI().read(startMem));
    }
    saveEEPROMVersion(7);
    loadAllPages();
  }

  if (loadEEPROMVersion() == 7)
  {
    configPage10.flexBoostBins[0] = 0;
    configPage10.flexBoostAdj[0]  = (int8_t)configPage2.aeColdPct;
    configPage10.flexFuelBins[0] = 0;
    configPage10.flexFuelAdj[0]  = configPage2.idleUpPin;
    configPage10.flexAdvBins[0] = 0;
    configPage10.flexAdvAdj[0]  = configPage2.aeTaperMin;
    for (uint8_t x = 1; x < 6; x++)
    {
      uint8_t pct = x * 20;
      configPage10.flexBoostBins[x] = pct;
      configPage10.flexFuelBins[x] = pct;
      configPage10.flexAdvBins[x] = pct;
      configPage10.flexBoostAdj[x] = (((configPage2.aeColdTaperMin - (int8_t)configPage2.aeColdPct) * pct) / 100) + (int8_t)configPage2.aeColdPct;
      configPage10.flexFuelAdj[x] = (((configPage2.idleUpAdder - configPage2.idleUpPin) * pct) / 100) + configPage2.idleUpPin;
      configPage10.flexAdvAdj[x] = (((configPage2.aeTaperMax - configPage2.aeTaperMin) * pct) / 100) + configPage2.aeTaperMin;
    }
    saveAllPages();
    saveEEPROMVersion(8);
  }

  if (loadEEPROMVersion() == 8)
  {
    configPage2.fuelAlgorithm = (LoadSource)configPage2.legacyMAP;
    configPage2.ignAlgorithm = (LoadSource)configPage2.legacyMAP;
    configPage4.boostType = 1;
    saveAllPages();
    saveEEPROMVersion(9);
  }

  if(loadEEPROMVersion() == 9)
  {
    for (byte AuxinChan = 0; AuxinChan <16 ; AuxinChan++) { configPage9.caninput_sel[AuxinChan] = 0; }
    configPage4.ADCFILTER_TPS  = ADCFILTER_TPS_DEFAULT;
    configPage4.ADCFILTER_CLT  = ADCFILTER_CLT_DEFAULT;
    configPage4.ADCFILTER_IAT  = ADCFILTER_IAT_DEFAULT;
    configPage4.ADCFILTER_O2   = ADCFILTER_O2_DEFAULT;
    configPage4.ADCFILTER_BAT  = ADCFILTER_BAT_DEFAULT;
    configPage4.ADCFILTER_MAP  = ADCFILTER_MAP_DEFAULT;
    configPage4.ADCFILTER_BARO = ADCFILTER_BARO_DEFAULT;
    saveAllPages();
    saveEEPROMVersion(10);
  }

  if(loadEEPROMVersion() == 10)
  {
    configPage2.primePulse[0] = configPage2.aeColdTaperMax / 5;
    configPage2.primePulse[1] = configPage2.aeColdTaperMax / 5;
    configPage2.primePulse[2] = configPage2.aeColdTaperMax / 5;
    configPage2.primePulse[3] = configPage2.aeColdTaperMax / 5;
    configPage2.primeBins[0] = 0;
    configPage2.primeBins[1] = 40;
    configPage2.primeBins[2] = 70;
    configPage2.primeBins[3] = 100;
    configPage2.asePct[0] = configPage2.aeColdTaperMin;
    configPage2.asePct[1] = configPage2.aeColdTaperMin;
    configPage2.asePct[2] = configPage2.aeColdTaperMin;
    configPage2.asePct[3] = configPage2.aeColdTaperMin;
    configPage2.aseCount[0] = 10;
    configPage2.aseCount[1] = 10;
    configPage2.aseCount[2] = 10;
    configPage2.aseCount[3] = 10;
    configPage2.aseBins[0] = 0;
    configPage2.aseBins[1] = 20;
    configPage2.aseBins[2] = 60;
    configPage2.aseBins[3] = 80;
    configPage4.cltAdvBins[0] = 0;
    configPage4.cltAdvBins[1] = 30;
    configPage4.cltAdvBins[2] = 60;
    configPage4.cltAdvBins[3] = 70;
    configPage4.cltAdvBins[4] = 85;
    configPage4.cltAdvBins[5] = 100;
    configPage4.cltAdvValues[0] = 0;
    configPage4.cltAdvValues[1] = 0;
    configPage4.cltAdvValues[2] = 0;
    configPage4.cltAdvValues[3] = 0;
    configPage4.cltAdvValues[4] = 0;
    configPage4.cltAdvValues[5] = 0;
    if(configPage2.tachoDuration > 6) { configPage2.tachoDuration = 3; }
    configPage2.aeMode = AE_MODE_TPS;
    configPage2.maeThresh = configPage2.taeThresh;
    configPage4.maeRates[0] = 75;
    configPage4.maeRates[1] = 75;
    configPage4.maeRates[2] = 75;
    configPage4.maeRates[3] = 75;
    configPage4.maeBins[0] = 7;
    configPage4.maeBins[1] = 12;
    configPage4.maeBins[2] = 20;
    configPage4.maeBins[3] = 40;
    configPage10.fuel2Mode = 0;
    saveAllPages();
    saveEEPROMVersion(11);
  }

  if(loadEEPROMVersion() == 11)
  {
    configPage4.batVoltCorrect = 0;
    configPage2.legacyMAP = 0;
    configPage10.fuel2Mode = 0;
    configPage10.fuel2SwitchVariable = 0;
    configPage10.fuel2SwitchValue = 7000;
    saveAllPages();
    saveEEPROMVersion(12);
  }

  if(loadEEPROMVersion() == 12)
  {
    configPage4.baroFuelBins[0] = 80;
    configPage4.baroFuelBins[1] = 85;
    configPage4.baroFuelBins[2] = 90;
    configPage4.baroFuelBins[3] = 95;
    configPage4.baroFuelBins[4] = 100;
    configPage4.baroFuelBins[5] = 105;
    configPage4.baroFuelBins[6] = 110;
    configPage4.baroFuelBins[7] = 115;
    configPage4.baroFuelValues[0] = 100;
    configPage4.baroFuelValues[1] = 100;
    configPage4.baroFuelValues[2] = 100;
    configPage4.baroFuelValues[3] = 100;
    configPage4.baroFuelValues[4] = 100;
    configPage4.baroFuelValues[5] = 100;
    configPage4.baroFuelValues[6] = 100;
    configPage4.baroFuelValues[7] = 100;
    configPage2.idleAdvEnabled = IDLEADVANCE_MODE_OFF;
    configPage2.idleAdvTPS = 5;
    configPage2.idleAdvRPM = 20;
    configPage4.idleAdvBins[0] = 30;
    configPage4.idleAdvBins[1] = 40;
    configPage4.idleAdvBins[2] = 50;
    configPage4.idleAdvBins[3] = 60;
    configPage4.idleAdvBins[4] = 70;
    configPage4.idleAdvBins[5] = 80;
    configPage4.idleAdvValues[0] = 15;
    configPage4.idleAdvValues[1] = 15;
    configPage4.idleAdvValues[2] = 15;
    configPage4.idleAdvValues[3] = 15;
    configPage4.idleAdvValues[4] = 15;
    configPage4.idleAdvValues[5] = 15;
    saveAllPages();
    saveEEPROMVersion(13);
  }

  if(loadEEPROMVersion() == 13)
  {
    configPage10.crankingEnrichValues[0] /= 5;
    configPage10.crankingEnrichValues[1] /= 5;
    configPage10.crankingEnrichValues[2] /= 5;
    configPage10.crankingEnrichValues[3] /= 5;
    configPage2.injAng[0] = configPage2.injAng[0];
    configPage2.injAng[1] = configPage2.injAng[0];
    configPage2.injAng[2] = configPage2.injAng[0];
    configPage2.injAng[3] = configPage2.injAng[0];
    configPage2.injAngRPM[0] = 5;
    configPage2.injAngRPM[1] = 25;
    configPage2.injAngRPM[2] = 45;
    configPage2.injAngRPM[3] = 65;
    configPage2.dfcoDelay = 0;
    configPage2.dfcoMinCLT = temperatureAddOffset(40);
    for (int i=0; i<6; i++) { configPage10.flexAdvAdj[i] += 40; }
    configPage2.aeColdPct = 100;
    configPage2.aeColdTaperMin = 40;
    configPage2.aeColdTaperMax = 100;
    if(configPage6.idleKP >= 8) { configPage6.idleKP = UINT8_MAX; } else { configPage6.idleKP = configPage6.idleKP<<5; }
    if(configPage6.idleKI >= 8) { configPage6.idleKI = UINT8_MAX; } else { configPage6.idleKI = configPage6.idleKI<<5; }
    if(configPage6.idleKD >= 8) { configPage6.idleKD = UINT8_MAX; } else { configPage6.idleKD = configPage6.idleKD<<5; }
    if(configPage10.vvtCLKP >= 8) { configPage10.vvtCLKP = UINT8_MAX; } else { configPage10.vvtCLKP = configPage10.vvtCLKP<<5; }
    if(configPage10.vvtCLKI >= 8) { configPage10.vvtCLKI = UINT8_MAX; } else { configPage10.vvtCLKI = configPage10.vvtCLKI<<5; }
    if(configPage10.vvtCLKD >= 8) { configPage10.vvtCLKD = UINT8_MAX; } else { configPage10.vvtCLKD = configPage10.vvtCLKD<<5; }
    configPage10.crankingEnrichTaper = 1;
    configPage2.aseTaperTime = 1;
    configPage2.SoftLimitMode = SOFT_LIMIT_FIXED;
    configPage2.vssMode = VSS_MODE_OFF;
    saveAllPages();
    saveEEPROMVersion(14);
  }

  if(loadEEPROMVersion() == 14)
  {
    constexpr uint16_t EEPROM_CALIBRATION_O2_OLD = 2559U;
    constexpr uint16_t EEPROM_CALIBRATION_IAT_OLD = 3071U;
    constexpr uint16_t EEPROM_CALIBRATION_CLT_OLD = 3583U;
    for(uint16_t x=0U; x<((uint16_t)CALIBRATION_TABLE_SIZE/16U); ++x)
    {
      uint16_t y = EEPROM_CALIBRATION_CLT_OLD + (x * 16U);
      cltCalibrationTable.values[x] = getStorageAPI().read(y);
      cltCalibrationTable.axis[x] = (x * 32U);
      y = EEPROM_CALIBRATION_IAT_OLD + (x * 16U);
      iatCalibrationTable.values[x] = getStorageAPI().read(y);
      iatCalibrationTable.axis[x] = (x * 32U);
      y = EEPROM_CALIBRATION_O2_OLD + (x * 16U);
      o2CalibrationTable.values[x] = getStorageAPI().read(y);
      o2CalibrationTable.axis[x] = (x * 32U);
    }
    saveAllCalibrationTables();
    configPage10.oilPressureProtEnbl = false;
    configPage10.oilPressureEnable = false;
    configPage10.fuelPressureEnable = false;
    configPage10.wmiEnabled = 0;
    configPage10.wmiMode = 0;
    configPage10.wmiOffset = 0;
    configPage10.wmiIndicatorEnabled = 0;
    configPage10.wmiEmptyEnabled = 0;
    configPage10.wmiAdvEnabled = 0;
    for(int i=0; i<6; i++)
    {
      configPage10.wmiAdvBins[i] = i*100/2;
      configPage10.wmiAdvAdj[i] = OFFSET_IGNITION;
    }
    configPage13.outputPin[0] = 0;
    configPage13.outputPin[1] = 0;
    configPage13.outputPin[2] = 0;
    configPage13.outputPin[3] = 0;
    configPage13.outputPin[4] = 0;
    configPage13.outputPin[5] = 0;
    configPage13.outputPin[6] = 0;
    configPage13.outputPin[7] = 0;
    configPage2.multiplyMAP = configPage2.crkngAddCLTAdv;
    configPage2.aeApplyMode = 0;
    configPage2.primingDelay = 0;
    configPage2.aseTaperTime = 10;
    saveAllPages();
    saveEEPROMVersion(15);
  }

  if(loadEEPROMVersion() == 15)
  {
    configPage10.spark2Mode = 0;
    saveAllPages();
    saveEEPROMVersion(16);
  }
#endif
}
