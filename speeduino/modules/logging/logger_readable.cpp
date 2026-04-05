#include "logger.h"
#include "advanced_engine_status.h"
#include "can_aux_status.h"
#include "logger_private.h"
#include "maths.h"
#include "utilities.h"

extern struct statuses currentStatus;

int16_t getReadableLogEntry(uint16_t logIndex)
{
  int16_t statusValue = 0;

  switch(logIndex)
  {
    case 0: statusValue = currentStatus.secl; break;
    case 1: statusValue = buildStatus1(currentStatus); break;
    case 2: statusValue = buildEngineStatus(currentStatus); break;
    case 3: statusValue = currentStatus.syncLossCounter; break;
    case 4: statusValue = currentStatus.MAP; break;
    case 5: statusValue = currentStatus.IAT; break;
    case 6: statusValue = currentStatus.coolant; break;
    case 7: statusValue = currentStatus.batCorrection; break;
    case 8: statusValue = currentStatus.battery10; break;
    case 9: statusValue = currentStatus.O2; break;
    case 10: statusValue = currentStatus.egoCorrection; break;
    case 11: statusValue = currentStatus.iatCorrection; break;
    case 12: statusValue = currentStatus.wueCorrection; break;
    case 13: statusValue = currentStatus.RPM; break;
    case 14: statusValue = currentStatus.AEamount; break;
    case 15: statusValue = currentStatus.corrections; break;
    case 16: statusValue = currentStatus.VE1; break;
    case 17: statusValue = currentStatus.VE2; break;
    case 18: statusValue = currentStatus.afrTarget; break;
    case 19: statusValue = currentStatus.tpsDOT; break;
    case 20: statusValue = currentStatus.advance; break;
    case 21: statusValue = currentStatus.TPS; break;
    case 22:
      if(currentStatus.loopsPerSecond > 60000U) { currentStatus.loopsPerSecond = 60000U; }
      statusValue = currentStatus.loopsPerSecond;
      break;
    case 23:
      currentStatus.freeRAM = freeRam();
      statusValue = currentStatus.freeRAM;
      break;
    case 24: statusValue = currentAdvancedEngineStatus.boost_target; break;
    case 25: statusValue = currentAdvancedEngineStatus.boost_duty; break;
    case 26: statusValue = buildStatus2(currentStatus); break;
    case 27: statusValue = currentStatus.rpmDOT; break;
    case 28: statusValue = currentStatus.ethanolPct; break;
    case 29: statusValue = currentStatus.flexCorrection; break;
    case 30: statusValue = currentStatus.flexIgnCorrection; break;
    case 31: statusValue = currentStatus.idleLoad; break;
    case 32: statusValue = buildTestOutput(currentStatus); break;
    case 33: statusValue = currentStatus.O2_2; break;
    case 34: statusValue = currentStatus.baro; break;
    case 35: statusValue = currentCanAuxStatus.values[0]; break;
    case 36: statusValue = currentCanAuxStatus.values[1]; break;
    case 37: statusValue = currentCanAuxStatus.values[2]; break;
    case 38: statusValue = currentCanAuxStatus.values[3]; break;
    case 39: statusValue = currentCanAuxStatus.values[4]; break;
    case 40: statusValue = currentCanAuxStatus.values[5]; break;
    case 41: statusValue = currentCanAuxStatus.values[6]; break;
    case 42: statusValue = currentCanAuxStatus.values[7]; break;
    case 43: statusValue = currentCanAuxStatus.values[8]; break;
    case 44: statusValue = currentCanAuxStatus.values[9]; break;
    case 45: statusValue = currentCanAuxStatus.values[10]; break;
    case 46: statusValue = currentCanAuxStatus.values[11]; break;
    case 47: statusValue = currentCanAuxStatus.values[12]; break;
    case 48: statusValue = currentCanAuxStatus.values[13]; break;
    case 49: statusValue = currentCanAuxStatus.values[14]; break;
    case 50: statusValue = currentCanAuxStatus.values[15]; break;
    case 51: statusValue = currentStatus.tpsADC; break;
    case 52: statusValue = 0U; break;
    case 53: statusValue = currentStatus.PW1; break;
    case 54: statusValue = currentStatus.PW2; break;
    case 55: statusValue = currentStatus.PW3; break;
    case 56: statusValue = currentStatus.PW4; break;
    case 57: statusValue = buildStatus3(currentStatus); break;
    case 58: statusValue = buildEngineProtectStatus(currentStatus); break;
    case 59: break;
    case 60: statusValue = currentStatus.fuelLoad; break;
    case 61: statusValue = currentStatus.ignLoad; break;
    case 62: statusValue = (int16_t)currentStatus.dwell; break;
    case 63: statusValue = currentStatus.CLIdleTarget; break;
    case 64: statusValue = currentStatus.mapDOT; break;
    case 65: statusValue = currentAdvancedEngineStatus.vvt1_angle; break;
    case 66: statusValue = currentAdvancedEngineStatus.vvt1_target_angle; break;
    case 67: statusValue = currentAdvancedEngineStatus.vvt1_duty; break;
    case 68: statusValue = currentAdvancedEngineStatus.flex_boost_correction; break;
    case 69: statusValue = currentStatus.baroCorrection; break;
    case 70: statusValue = currentStatus.VE; break;
    case 71: statusValue = currentStatus.ASEValue; break;
    case 72: statusValue = currentStatus.vss; break;
    case 73: statusValue = currentStatus.gear; break;
    case 74: statusValue = currentStatus.fuelPressure; break;
    case 75: statusValue = currentStatus.oilPressure; break;
    case 76: statusValue = currentAdvancedEngineStatus.wmi_pw; break;
    case 77: statusValue = buildStatus4(currentStatus); break;
    case 78: statusValue = currentAdvancedEngineStatus.vvt2_angle; break;
    case 79: statusValue = currentAdvancedEngineStatus.vvt2_target_angle; break;
    case 80: statusValue = currentAdvancedEngineStatus.vvt2_duty; break;
    case 81: statusValue = currentStatus.outputsStatus; break;
    case 82: statusValue = currentStatus.fuelTemp; break;
    case 83: statusValue = currentStatus.fuelTempCorrection; break;
    case 84: statusValue = currentStatus.advance1; break;
    case 85: statusValue = currentStatus.advance2; break;
    case 86: statusValue = buildSdCardStatus(currentStatus); break;
    case 87: statusValue = currentStatus.EMAP; break;
    case 88: statusValue = currentAdvancedEngineStatus.fan_duty; break;
    case 89: statusValue = buildAirConStatus(currentStatus); break;
    case 90: statusValue = currentStatus.actualDwell; break;
    case 91: statusValue = buildStatus5(currentStatus); break;
    case 92: statusValue = currentStatus.knockCount; break;
    case 93: statusValue = currentStatus.knockRetard; break;
    case 94: statusValue = currentStatus.PW5; break;
    case 95: statusValue = currentStatus.PW6; break;
    case 96: statusValue = currentStatus.PW7; break;
    case 97: statusValue = currentStatus.PW8; break;
    case 98: statusValue = currentStatus.systemTemp; break;
    default: statusValue = 0; break;
  }

  return statusValue;
}

#if defined(FPU_MAX_SIZE) && FPU_MAX_SIZE >= 32
float getReadableFloatLogEntry(uint16_t logIndex)
{
  float statusValue = 0.0F;

  switch(logIndex)
  {
    case 8: statusValue = currentStatus.battery10 / 10.0F; break;
    case 9: statusValue = currentStatus.O2 / 10.0F; break;
    case 18: statusValue = currentStatus.afrTarget / 10.0F; break;
    case 21: statusValue = currentStatus.TPS / 2.0F; break;
    case 33: statusValue = currentStatus.O2_2 / 10.0F; break;
    case 53: statusValue = currentStatus.PW1 / 1000.0F; break;
    case 54: statusValue = currentStatus.PW2 / 1000.0F; break;
    case 55: statusValue = currentStatus.PW3 / 1000.0F; break;
    case 56: statusValue = currentStatus.PW4 / 1000.0F; break;
    default: statusValue = getReadableLogEntry(logIndex); break;
  }

  return statusValue;
}
#endif
