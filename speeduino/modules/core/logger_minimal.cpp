#include "modules/logging/logger.h"
#include "modules/logging/logger_private.h"
#include "maths.h"
#include "utilities.h"
#include "preprocessor.h"
#include "units.h"
#include "board_definition.h"

#if !FEATURE_MODULE_LOGGING

extern struct statuses currentStatus;
extern struct config2 configPage2;

byte buildEngineStatus(const statuses &current)
{
  bool bits[] = {
    current.engineIsRunning,
    current.engineIsCranking,
    current.aseIsActive,
    current.wueIsActive,
    current.isAcceleratingTPS,
    current.isDeceleratingTPS,
  };
  return setStatusBits(0U, bits);
}

byte buildSdCardStatus(const statuses &current)
{
  bool bits[] = {
    current.sdCardPresent,
    current.sdCardType == 1U,
    current.sdCardReady,
    current.sdCardLogging,
    current.sdCardError,
    false,
    current.sdCardFS == 1U,
    current.sdCardUnused,
  };
  return setStatusBits(0U, bits);
}

byte getTSLogEntry(uint16_t byteNum)
{
  byte statusValue = 0U;

  switch(byteNum)
  {
    case 0: statusValue = currentStatus.secl; break;
    case 1: statusValue = buildStatus1(currentStatus); break;
    case 2: statusValue = buildEngineStatus(currentStatus); break;
    case 3: statusValue = currentStatus.syncLossCounter; break;
    case 4: statusValue = lowByte(currentStatus.MAP); break;
    case 5: statusValue = highByte(currentStatus.MAP); break;
    case 6: statusValue = temperatureAddOffset(currentStatus.IAT); break;
    case 7: statusValue = temperatureAddOffset(currentStatus.coolant); break;
    case 8: statusValue = currentStatus.batCorrection; break;
    case 9: statusValue = currentStatus.battery10; break;
    case 10: statusValue = currentStatus.O2; break;
    case 11: statusValue = currentStatus.egoCorrection; break;
    case 12: statusValue = currentStatus.iatCorrection; break;
    case 13: statusValue = currentStatus.wueCorrection; break;
    case 14: statusValue = lowByte(currentStatus.RPM); break;
    case 15: statusValue = highByte(currentStatus.RPM); break;
    case 16: statusValue = lowByte(currentStatus.AEamount >> 1U); break;
    case 17: statusValue = lowByte(currentStatus.corrections); break;
    case 18: statusValue = highByte(currentStatus.corrections); break;
    case 19: statusValue = currentStatus.VE1; break;
    case 20: statusValue = currentStatus.VE2; break;
    case 21: statusValue = currentStatus.afrTarget; break;
    case 22: statusValue = lowByte(currentStatus.tpsDOT); break;
    case 23: statusValue = highByte(currentStatus.tpsDOT); break;
    case 24: statusValue = currentStatus.advance; break;
    case 25: statusValue = currentStatus.TPS; break;
    case 26: statusValue = lowByte(currentStatus.loopsPerSecond); break;
    case 27: statusValue = highByte(currentStatus.loopsPerSecond); break;
    case 28: currentStatus.freeRAM = freeRam(); statusValue = lowByte(currentStatus.freeRAM); break;
    case 29: currentStatus.freeRAM = freeRam(); statusValue = highByte(currentStatus.freeRAM); break;
    case 30: statusValue = lowByte(currentStatus.boostTarget >> 1U); break;
    case 31: statusValue = lowByte(div100(currentStatus.boostDuty)); break;
    case 32: statusValue = buildStatus2(currentStatus); break;
    case 33: statusValue = lowByte(currentStatus.rpmDOT); break;
    case 34: statusValue = highByte(currentStatus.rpmDOT); break;
    case 35: statusValue = currentStatus.ethanolPct; break;
    case 36: statusValue = currentStatus.flexCorrection; break;
    case 37: statusValue = currentStatus.flexIgnCorrection; break;
    case 38: statusValue = currentStatus.idleLoad; break;
    case 39: statusValue = buildTestOutput(currentStatus); break;
    case 40: statusValue = currentStatus.O2_2; break;
    case 41: statusValue = currentStatus.baro; break;
    case 42: statusValue = lowByte(currentStatus.canin[0]); break;
    case 43: statusValue = highByte(currentStatus.canin[0]); break;
    case 44: statusValue = lowByte(currentStatus.canin[1]); break;
    case 45: statusValue = highByte(currentStatus.canin[1]); break;
    case 46: statusValue = lowByte(currentStatus.canin[2]); break;
    case 47: statusValue = highByte(currentStatus.canin[2]); break;
    case 48: statusValue = lowByte(currentStatus.canin[3]); break;
    case 49: statusValue = highByte(currentStatus.canin[3]); break;
    case 50: statusValue = lowByte(currentStatus.canin[4]); break;
    case 51: statusValue = highByte(currentStatus.canin[4]); break;
    case 52: statusValue = lowByte(currentStatus.canin[5]); break;
    case 53: statusValue = highByte(currentStatus.canin[5]); break;
    case 54: statusValue = lowByte(currentStatus.canin[6]); break;
    case 55: statusValue = highByte(currentStatus.canin[6]); break;
    case 56: statusValue = lowByte(currentStatus.canin[7]); break;
    case 57: statusValue = highByte(currentStatus.canin[7]); break;
    case 58: statusValue = lowByte(currentStatus.canin[8]); break;
    case 59: statusValue = highByte(currentStatus.canin[8]); break;
    case 60: statusValue = lowByte(currentStatus.canin[9]); break;
    case 61: statusValue = highByte(currentStatus.canin[9]); break;
    case 62: statusValue = lowByte(currentStatus.canin[10]); break;
    case 63: statusValue = highByte(currentStatus.canin[10]); break;
    case 64: statusValue = lowByte(currentStatus.canin[11]); break;
    case 65: statusValue = highByte(currentStatus.canin[11]); break;
    case 66: statusValue = lowByte(currentStatus.canin[12]); break;
    case 67: statusValue = highByte(currentStatus.canin[12]); break;
    case 68: statusValue = lowByte(currentStatus.canin[13]); break;
    case 69: statusValue = highByte(currentStatus.canin[13]); break;
    case 70: statusValue = lowByte(currentStatus.canin[14]); break;
    case 71: statusValue = highByte(currentStatus.canin[14]); break;
    case 72: statusValue = lowByte(currentStatus.canin[15]); break;
    case 73: statusValue = highByte(currentStatus.canin[15]); break;
    case 74: statusValue = currentStatus.tpsADC; break;
    case 75: statusValue = 0U; break;
    case 76: statusValue = lowByte(currentStatus.PW1); break;
    case 77: statusValue = highByte(currentStatus.PW1); break;
    case 78: statusValue = lowByte(currentStatus.PW2); break;
    case 79: statusValue = highByte(currentStatus.PW2); break;
    case 80: statusValue = lowByte(currentStatus.PW3); break;
    case 81: statusValue = highByte(currentStatus.PW3); break;
    case 82: statusValue = lowByte(currentStatus.PW4); break;
    case 83: statusValue = highByte(currentStatus.PW4); break;
    case 84: statusValue = buildStatus3(currentStatus); break;
    case 85: statusValue = buildEngineProtectStatus(currentStatus); break;
    case 86: statusValue = lowByte(currentStatus.fuelLoad); break;
    case 87: statusValue = highByte(currentStatus.fuelLoad); break;
    case 88: statusValue = lowByte(currentStatus.ignLoad); break;
    case 89: statusValue = highByte(currentStatus.ignLoad); break;
    case 90: statusValue = lowByte(currentStatus.dwell); break;
    case 91: statusValue = highByte(currentStatus.dwell); break;
    case 92: statusValue = currentStatus.CLIdleTarget; break;
    case 93: statusValue = lowByte(currentStatus.mapDOT); break;
    case 94: statusValue = highByte(currentStatus.mapDOT); break;
    case 95: statusValue = lowByte(currentStatus.vvt1Angle); break;
    case 96: statusValue = highByte(currentStatus.vvt1Angle); break;
    case 97: statusValue = currentStatus.vvt1TargetAngle; break;
    case 98: statusValue = lowByte(currentStatus.vvt1Duty); break;
    case 99: statusValue = lowByte(currentStatus.flexBoostCorrection); break;
    case 100: statusValue = highByte(currentStatus.flexBoostCorrection); break;
    case 101: statusValue = currentStatus.baroCorrection; break;
    case 102: statusValue = currentStatus.VE; break;
    case 103: statusValue = currentStatus.ASEValue; break;
    case 104: statusValue = lowByte(currentStatus.vss); break;
    case 105: statusValue = highByte(currentStatus.vss); break;
    case 106: statusValue = currentStatus.gear; break;
    case 107: statusValue = currentStatus.fuelPressure; break;
    case 108: statusValue = currentStatus.oilPressure; break;
    case 109: statusValue = currentStatus.wmiPW; break;
    case 110: statusValue = buildStatus4(currentStatus); break;
    case 111: statusValue = lowByte(currentStatus.vvt2Angle); break;
    case 112: statusValue = highByte(currentStatus.vvt2Angle); break;
    case 113: statusValue = currentStatus.vvt2TargetAngle; break;
    case 114: statusValue = lowByte(currentStatus.vvt2Duty); break;
    case 115: statusValue = currentStatus.outputsStatus; break;
    case 116: statusValue = temperatureAddOffset(currentStatus.fuelTemp); break;
    case 117: statusValue = currentStatus.fuelTempCorrection; break;
    case 118: statusValue = currentStatus.advance1; break;
    case 119: statusValue = currentStatus.advance2; break;
    case 120: statusValue = buildSdCardStatus(currentStatus); break;
    case 121: statusValue = lowByte(currentStatus.EMAP); break;
    case 122: statusValue = highByte(currentStatus.EMAP); break;
    case 123: statusValue = currentStatus.fanDuty; break;
    case 124: statusValue = buildAirConStatus(currentStatus); break;
    case 125: statusValue = lowByte(currentStatus.actualDwell); break;
    case 126: statusValue = highByte(currentStatus.actualDwell); break;
    case 127: statusValue = buildStatus5(currentStatus); break;
    case 128: statusValue = currentStatus.knockCount; break;
    case 129: statusValue = currentStatus.knockRetard; break;
    case 130: statusValue = lowByte(currentStatus.PW5); break;
    case 131: statusValue = highByte(currentStatus.PW5); break;
    case 132: statusValue = lowByte(currentStatus.PW6); break;
    case 133: statusValue = highByte(currentStatus.PW6); break;
    case 134: statusValue = lowByte(currentStatus.PW7); break;
    case 135: statusValue = highByte(currentStatus.PW7); break;
    case 136: statusValue = lowByte(currentStatus.PW8); break;
    case 137: statusValue = highByte(currentStatus.PW8); break;
    case 138: statusValue = currentStatus.systemTemp; break;
    default: statusValue = 0U; break;
  }

  return statusValue;
}

uint8_t getLegacySecondarySerialLogEntry(uint16_t byteNum)
{
  uint8_t statusValue = 0U;

  switch(byteNum)
  {
    default:
    case 0: statusValue = currentStatus.secl; break;
    case 1: statusValue = buildStatus1(currentStatus); break;
    case 2: statusValue = buildEngineStatus(currentStatus); break;
    case 3: statusValue = (byte)div100(currentStatus.dwell); break;
    case 4: statusValue = lowByte(currentStatus.MAP); break;
    case 5: statusValue = highByte(currentStatus.MAP); break;
    case 6: statusValue = temperatureAddOffset(currentStatus.IAT); break;
    case 7: statusValue = temperatureAddOffset(currentStatus.coolant); break;
    case 8: statusValue = currentStatus.batCorrection; break;
    case 9: statusValue = currentStatus.battery10; break;
    case 10: statusValue = currentStatus.O2; break;
    case 11: statusValue = currentStatus.egoCorrection; break;
    case 12: statusValue = currentStatus.iatCorrection; break;
    case 13: statusValue = currentStatus.wueCorrection; break;
    case 14: statusValue = lowByte(currentStatus.RPM); break;
    case 15: statusValue = highByte(currentStatus.RPM); break;
    case 16: statusValue = currentStatus.AEamount; break;
    case 17: statusValue = currentStatus.corrections; break;
    case 18: statusValue = currentStatus.VE; break;
    case 19: statusValue = currentStatus.afrTarget; break;
    case 20: statusValue = lowByte(currentStatus.PW1); break;
    case 21: statusValue = highByte(currentStatus.PW1); break;
    case 22: statusValue = (uint8_t)(currentStatus.tpsDOT / 10); break;
    case 23: statusValue = currentStatus.advance; break;
    case 24: statusValue = currentStatus.TPS; break;
    case 25: statusValue = lowByte(currentStatus.loopsPerSecond); break;
    case 26: statusValue = highByte(currentStatus.loopsPerSecond); break;
    case 27: currentStatus.freeRAM = freeRam(); statusValue = lowByte(currentStatus.freeRAM); break;
    case 28: currentStatus.freeRAM = freeRam(); statusValue = highByte(currentStatus.freeRAM); break;
    case 29: statusValue = currentStatus.boostTarget / 2U; break;
    case 30: statusValue = currentStatus.boostDuty / 100U; break;
    case 31: statusValue = buildStatus2(currentStatus); break;
    case 32: statusValue = lowByte(currentStatus.rpmDOT); break;
    case 33: statusValue = highByte(currentStatus.rpmDOT); break;
    case 34: statusValue = currentStatus.ethanolPct; break;
    case 35: statusValue = currentStatus.flexCorrection; break;
    case 36: statusValue = currentStatus.flexIgnCorrection; break;
    case 37: statusValue = currentStatus.idleLoad; break;
    case 38: statusValue = buildTestOutput(currentStatus); break;
    case 39: statusValue = currentStatus.O2_2; break;
    case 40: statusValue = currentStatus.baro; break;
    case 41: statusValue = lowByte(currentStatus.canin[0]); break;
    case 42: statusValue = highByte(currentStatus.canin[0]); break;
    case 43: statusValue = lowByte(currentStatus.canin[1]); break;
    case 44: statusValue = highByte(currentStatus.canin[1]); break;
    case 45: statusValue = lowByte(currentStatus.canin[2]); break;
    case 46: statusValue = highByte(currentStatus.canin[2]); break;
    case 47: statusValue = lowByte(currentStatus.canin[3]); break;
    case 48: statusValue = highByte(currentStatus.canin[3]); break;
    case 49: statusValue = lowByte(currentStatus.canin[4]); break;
    case 50: statusValue = highByte(currentStatus.canin[4]); break;
    case 51: statusValue = lowByte(currentStatus.canin[5]); break;
    case 52: statusValue = highByte(currentStatus.canin[5]); break;
    case 53: statusValue = lowByte(currentStatus.canin[6]); break;
    case 54: statusValue = highByte(currentStatus.canin[6]); break;
    case 55: statusValue = lowByte(currentStatus.canin[7]); break;
    case 56: statusValue = highByte(currentStatus.canin[7]); break;
    case 57: statusValue = lowByte(currentStatus.canin[8]); break;
    case 58: statusValue = highByte(currentStatus.canin[8]); break;
    case 59: statusValue = lowByte(currentStatus.canin[9]); break;
    case 60: statusValue = highByte(currentStatus.canin[9]); break;
    case 61: statusValue = lowByte(currentStatus.canin[10]); break;
    case 62: statusValue = highByte(currentStatus.canin[10]); break;
    case 63: statusValue = lowByte(currentStatus.canin[11]); break;
    case 64: statusValue = highByte(currentStatus.canin[11]); break;
    case 65: statusValue = lowByte(currentStatus.canin[12]); break;
    case 66: statusValue = highByte(currentStatus.canin[12]); break;
    case 67: statusValue = lowByte(currentStatus.canin[13]); break;
    case 68: statusValue = highByte(currentStatus.canin[13]); break;
    case 69: statusValue = lowByte(currentStatus.canin[14]); break;
    case 70: statusValue = highByte(currentStatus.canin[14]); break;
    case 71: statusValue = lowByte(currentStatus.canin[15]); break;
    case 72: statusValue = highByte(currentStatus.canin[15]); break;
    case 73: statusValue = currentStatus.tpsADC; break;
    case 74: statusValue = 0U; break;
    case 75: statusValue = currentStatus.launchCorrection; break;
    case 76: statusValue = lowByte(currentStatus.PW2); break;
    case 77: statusValue = highByte(currentStatus.PW2); break;
    case 78: statusValue = lowByte(currentStatus.PW3); break;
    case 79: statusValue = highByte(currentStatus.PW3); break;
    case 80: statusValue = lowByte(currentStatus.PW4); break;
    case 81: statusValue = highByte(currentStatus.PW4); break;
    case 82: statusValue = buildStatus3(currentStatus); break;
    case 83: statusValue = buildEngineProtectStatus(currentStatus); break;
    case 84: statusValue = lowByte(currentStatus.fuelLoad); break;
    case 85: statusValue = highByte(currentStatus.fuelLoad); break;
    case 86: statusValue = lowByte(currentStatus.ignLoad); break;
    case 87: statusValue = highByte(currentStatus.ignLoad); break;
    case 88: statusValue = lowByte(currentStatus.injAngle); break;
    case 89: statusValue = highByte(currentStatus.injAngle); break;
    case 90: statusValue = currentStatus.idleLoad; break;
    case 91: statusValue = currentStatus.CLIdleTarget; break;
    case 92: statusValue = currentStatus.mapDOT / 10; break;
    case 93: statusValue = (int8_t)currentStatus.vvt1Angle; break;
    case 94: statusValue = currentStatus.vvt1TargetAngle; break;
    case 95: statusValue = currentStatus.vvt1Duty; break;
    case 96: statusValue = lowByte(currentStatus.flexBoostCorrection); break;
    case 97: statusValue = highByte(currentStatus.flexBoostCorrection); break;
    case 98: statusValue = currentStatus.baroCorrection; break;
    case 99: statusValue = currentStatus.ASEValue; break;
    case 100: statusValue = lowByte(currentStatus.vss); break;
    case 101: statusValue = highByte(currentStatus.vss); break;
    case 102: statusValue = currentStatus.gear; break;
    case 103: statusValue = currentStatus.fuelPressure; break;
    case 104: statusValue = currentStatus.oilPressure; break;
    case 105: statusValue = currentStatus.wmiPW; break;
    case 106: statusValue = buildStatus4(currentStatus); break;
    case 107: statusValue = (int8_t)currentStatus.vvt2Angle; break;
    case 108: statusValue = currentStatus.vvt2TargetAngle; break;
    case 109: statusValue = currentStatus.vvt2Duty; break;
    case 110: statusValue = currentStatus.outputsStatus; break;
    case 111: statusValue = temperatureAddOffset(currentStatus.fuelTemp); break;
    case 112: statusValue = currentStatus.fuelTempCorrection; break;
    case 113: statusValue = currentStatus.VE1; break;
    case 114: statusValue = currentStatus.VE2; break;
    case 115: statusValue = currentStatus.advance1; break;
    case 116: statusValue = currentStatus.advance2; break;
    case 117: statusValue = currentStatus.nitrous_status; break;
    case 118: statusValue = buildSdCardStatus(currentStatus); break;
    case 119: statusValue = lowByte(currentStatus.EMAP); break;
    case 120: statusValue = highByte(currentStatus.EMAP); break;
    case 121: statusValue = currentStatus.fanDuty; break;
    case 122: statusValue = buildAirConStatus(currentStatus); break;
  }

  return statusValue;
}

bool is2ByteEntry(uint8_t key)
{
  static constexpr byte PROGMEM fsIntIndex[] = {4, 14, 17, 22, 26, 28, 33, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62, 64, 66, 68, 70, 72, 76, 78, 80, 82, 86, 88, 90, 93, 95, 99, 104, 111, 121, 125, 130, 132, 134, 136 };

  unsigned int bot = 0U;
  unsigned int mid = _countof(fsIntIndex);

  while (mid > 1U)
  {
    if (key >= pgm_read_byte(&fsIntIndex[bot + mid / 2U]))
    {
      bot += mid++ / 2U;
    }
    mid /= 2U;
  }

  return key == pgm_read_byte(&fsIntIndex[bot]);
}

#endif
