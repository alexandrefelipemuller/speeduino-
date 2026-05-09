#include "logger.h"
#include "data/advanced_engine_status.h"
#include "data/can_aux_status.h"
#include "data/logger_status.h"
#include "logger_private.h"
#include "support/maths.h"
#include "support/utilities.h"
#include "support/preprocessor.h"
#include "support/units.h"
#include "boards/board_definition.h" 

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

/** 
 * Returns a numbered byte-field (partial field in case of multi-byte fields) from "current status" structure in the format expected by TunerStudio
 * Notes on fields:
 * - Numbered field will be fields from @ref currentStatus, but not at all in the internal order of strct (e.g. field RPM value, number 14 will be
 *   2nd field in struct)
 * - The fields stored in multi-byte types will be accessed lowbyte and highbyte separately (e.g. PW1 will be broken into numbered byte-fields 75,76)
 * - Values have the value offsets and shifts expected by TunerStudio. They will not all be a 'human readable value'
 * @param byteNum - byte-Field number. This is not the entry number (As some entries have multiple byets), but the byte number that is needed
 * @return Field value in 1 byte size struct fields or 1 byte partial value (chunk) on multibyte fields.
 */
byte getTSLogEntry(uint16_t byteNum)
{
  byte statusValue = 0;

  switch(byteNum)
  {
    case 0: statusValue = currentStatus.secl; break; //secl is simply a counter that increments each second. Used to track unexpected resets (Which will reset this count to 0)
    case 1: statusValue = buildStatus1(currentStatus); break; //status1 Bitfield
    case 2: statusValue = buildEngineStatus(currentStatus); break; //Engine Status Bitfield
    case 3: statusValue = currentStatus.syncLossCounter; break;
    case 4: statusValue = lowByte(currentStatus.MAP); break; //2 bytes for MAP
    case 5: statusValue = highByte(currentStatus.MAP); break;
    case 6: statusValue = temperatureAddOffset(currentStatus.IAT); break; //mat
    case 7: statusValue = temperatureAddOffset(currentStatus.coolant); break; //Coolant ADC
    case 8: statusValue = currentStatus.batCorrection; break; //Battery voltage correction (%)
    case 9: statusValue = currentStatus.battery10; break; //battery voltage
    case 10: statusValue = currentStatus.O2; break; //O2
    case 11: statusValue = currentStatus.egoCorrection; break; //Exhaust gas correction (%)
    case 12: statusValue = currentStatus.iatCorrection; break; //Air temperature Correction (%)
    case 13: statusValue = currentStatus.wueCorrection; break; //Warmup enrichment (%)
    case 14: statusValue = lowByte(currentStatus.RPM); break; //rpm HB
    case 15: statusValue = highByte(currentStatus.RPM); break; //rpm LB
    case 16: statusValue = lowByte(currentStatus.AEamount >> 1U); break; //TPS acceleration enrichment (%) divided by 2 (Can exceed 255)
    case 17: statusValue = lowByte(currentStatus.corrections); break; //Total GammaE (%)
    case 18: statusValue = highByte(currentStatus.corrections); break; //Total GammaE (%)
    case 19: statusValue = currentStatus.VE1; break; //VE 1 (%)
    case 20: statusValue = currentStatus.VE2; break; //VE 2 (%)
    case 21: statusValue = currentStatus.afrTarget; break;
    case 22: statusValue = lowByte(currentStatus.tpsDOT); break; //TPS DOT
    case 23: statusValue = highByte(currentStatus.tpsDOT); break; //TPS DOT
    case 24: statusValue = currentStatus.advance; break;
    case 25: statusValue = currentStatus.TPS; break; // TPS (0% to 100%)
    
    case 26: 
      statusValue = lowByte(currentStatus.loopsPerSecond); 
      break;
    case 27: 
      statusValue = highByte(currentStatus.loopsPerSecond); 
      break;
    
    case 28: 
      currentLoggerStatus.free_ram = freeRam();
      statusValue = lowByte(currentLoggerStatus.free_ram); //(byte)((currentStatus.loopsPerSecond >> 8) & 0xFF);
      break; 
    case 29: 
      currentLoggerStatus.free_ram = freeRam();
      statusValue = highByte(currentLoggerStatus.free_ram); 
      break;

    case 30: statusValue = lowByte(currentAdvancedEngineStatus.boost_target >> 1U); break; //Divide boost target by 2 to fit in a byte
    case 31: statusValue = lowByte(div100(currentAdvancedEngineStatus.boost_duty)); break;
    case 32: statusValue = buildStatus2(currentStatus); break; //Spark related bitfield

    //rpmDOT must be sent as a signed integer
    case 33: statusValue = lowByte(currentStatus.rpmDOT); break;
    case 34: statusValue = highByte(currentStatus.rpmDOT); break;

    case 35: statusValue = currentStatus.ethanolPct; break; //Flex sensor value (or 0 if not used)
    case 36: statusValue = currentStatus.flexCorrection; break; //Flex fuel correction (% above or below 100)
    case 37: statusValue = currentStatus.flexIgnCorrection; break; //Ignition correction (Increased degrees of advance) for flex fuel

    case 38: statusValue = currentStatus.idleLoad; break;
    case 39: statusValue = buildTestOutput(currentStatus); break;

    case 40: statusValue = currentStatus.O2_2; break; //O2
    case 41: statusValue = currentStatus.baro; break; //Barometer value

    case 42: statusValue = lowByte(currentCanAuxStatus.values[0]); break;
    case 43: statusValue = highByte(currentCanAuxStatus.values[0]); break;
    case 44: statusValue = lowByte(currentCanAuxStatus.values[1]); break;
    case 45: statusValue = highByte(currentCanAuxStatus.values[1]); break;
    case 46: statusValue = lowByte(currentCanAuxStatus.values[2]); break;
    case 47: statusValue = highByte(currentCanAuxStatus.values[2]); break;
    case 48: statusValue = lowByte(currentCanAuxStatus.values[3]); break;
    case 49: statusValue = highByte(currentCanAuxStatus.values[3]); break;
    case 50: statusValue = lowByte(currentCanAuxStatus.values[4]); break;
    case 51: statusValue = highByte(currentCanAuxStatus.values[4]); break;
    case 52: statusValue = lowByte(currentCanAuxStatus.values[5]); break;
    case 53: statusValue = highByte(currentCanAuxStatus.values[5]); break;
    case 54: statusValue = lowByte(currentCanAuxStatus.values[6]); break;
    case 55: statusValue = highByte(currentCanAuxStatus.values[6]); break;
    case 56: statusValue = lowByte(currentCanAuxStatus.values[7]); break;
    case 57: statusValue = highByte(currentCanAuxStatus.values[7]); break;
    case 58: statusValue = lowByte(currentCanAuxStatus.values[8]); break;
    case 59: statusValue = highByte(currentCanAuxStatus.values[8]); break;
    case 60: statusValue = lowByte(currentCanAuxStatus.values[9]); break;
    case 61: statusValue = highByte(currentCanAuxStatus.values[9]); break;
    case 62: statusValue = lowByte(currentCanAuxStatus.values[10]); break;
    case 63: statusValue = highByte(currentCanAuxStatus.values[10]); break;
    case 64: statusValue = lowByte(currentCanAuxStatus.values[11]); break;
    case 65: statusValue = highByte(currentCanAuxStatus.values[11]); break;
    case 66: statusValue = lowByte(currentCanAuxStatus.values[12]); break;
    case 67: statusValue = highByte(currentCanAuxStatus.values[12]); break;
    case 68: statusValue = lowByte(currentCanAuxStatus.values[13]); break;
    case 69: statusValue = highByte(currentCanAuxStatus.values[13]); break;
    case 70: statusValue = lowByte(currentCanAuxStatus.values[14]); break;
    case 71: statusValue = highByte(currentCanAuxStatus.values[14]); break;
    case 72: statusValue = lowByte(currentCanAuxStatus.values[15]); break;
    case 73: statusValue = highByte(currentCanAuxStatus.values[15]); break;

    case 74: statusValue = currentStatus.tpsADC; break;
    case 75: statusValue = 0U /*getNextError()*/; break;

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
    case 95: statusValue = lowByte(currentAdvancedEngineStatus.vvt1_angle); break; //2 bytes for vvt1Angle
    case 96: statusValue = highByte(currentAdvancedEngineStatus.vvt1_angle); break;
    case 97: statusValue = currentAdvancedEngineStatus.vvt1_target_angle; break;
    case 98: statusValue = lowByte(currentAdvancedEngineStatus.vvt1_duty); break;
    case 99: statusValue = lowByte(currentAdvancedEngineStatus.flex_boost_correction); break;
    case 100: statusValue = highByte(currentAdvancedEngineStatus.flex_boost_correction); break;
    case 101: statusValue = currentStatus.baroCorrection; break;
    case 102: statusValue = currentStatus.VE; break; //Current VE (%). Can be equal to VE1 or VE2 or a calculated value from both of them
    case 103: statusValue = currentStatus.ASEValue; break; //Current ASE (%)
    case 104: statusValue = lowByte(currentStatus.vss); break;
    case 105: statusValue = highByte(currentStatus.vss); break;
    case 106: statusValue = currentStatus.gear; break;
    case 107: statusValue = currentStatus.fuelPressure; break;
    case 108: statusValue = currentStatus.oilPressure; break;
    case 109: statusValue = currentAdvancedEngineStatus.wmi_pw; break;
    case 110: statusValue = buildStatus4(currentStatus); break;
    case 111: statusValue = lowByte(currentAdvancedEngineStatus.vvt2_angle); break; //2 bytes for vvt2Angle
    case 112: statusValue = highByte(currentAdvancedEngineStatus.vvt2_angle); break;
    case 113: statusValue = currentAdvancedEngineStatus.vvt2_target_angle; break;
    case 114: statusValue = lowByte(currentAdvancedEngineStatus.vvt2_duty); break;
    case 115: statusValue = currentStatus.outputsStatus; break;
    case 116: statusValue = temperatureAddOffset(currentStatus.fuelTemp); break; //Fuel temperature from flex sensor
    case 117: statusValue = currentStatus.fuelTempCorrection; break; //Fuel temperature Correction (%)
    case 118: statusValue = currentStatus.advance1; break; //advance 1 (%)
    case 119: statusValue = currentStatus.advance2; break; //advance 2 (%)
    case 120: statusValue = buildSdCardStatus(currentStatus); break; //SD card status
    case 121: statusValue = lowByte(currentStatus.EMAP); break; //2 bytes for EMAP
    case 122: statusValue = highByte(currentStatus.EMAP); break;
    case 123: statusValue = currentAdvancedEngineStatus.fan_duty; break;
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
    case 139: statusValue = buildEtbStatus(currentStatus); break;
    case 140: statusValue = currentEtbStatus.fault_code; break;
    case 141: statusValue = currentEtbStatus.pedal_percent; break;
    case 142: statusValue = currentEtbStatus.throttle_percent; break;
    case 143: statusValue = currentEtbStatus.target_percent; break;
    case 144: statusValue = currentEtbStatus.open_duty; break;
    case 145: statusValue = currentEtbStatus.close_duty; break;
    default: statusValue = 0; // MISRA check
  }

  return statusValue;
}

uint8_t getLegacySecondarySerialLogEntry(uint16_t byteNum)
{
  uint8_t statusValue = 0;

  switch(byteNum)
  {
    default:
    case 0: statusValue = currentStatus.secl; break; //secl is simply a counter that increments each second. Used to track unexpected resets (Which will reset this count to 0)
    case 1: statusValue = buildStatus1(currentStatus); break; //status1 Bitfield, inj1Status(0), inj2Status(1), inj3Status(2), inj4Status(3), DFCOOn(4), boostCutFuel(5), toothLog1Ready(6), toothLog2Ready(7)
    case 2: statusValue = buildEngineStatus(currentStatus); break; //Engine Status Bitfield, running(0), crank(1), ase(2), warmup(3), tpsaccaen(4), tpsacden(5), mapaccaen(6), mapaccden(7)
    case 3: statusValue = (byte)div100(currentStatus.dwell); break; //Dwell in ms * 10
    case 4: statusValue = lowByte(currentStatus.MAP); break; //2 bytes for MAP
    case 5: statusValue = highByte(currentStatus.MAP); break;
    case 6: statusValue = temperatureAddOffset(currentStatus.IAT); break; //mat
    case 7: statusValue = temperatureAddOffset(currentStatus.coolant); break; //Coolant ADC
    case 8: statusValue = currentStatus.batCorrection; break; //Battery voltage correction (%)
    case 9: statusValue = currentStatus.battery10; break; //battery voltage
    case 10: statusValue = currentStatus.O2; break; //O2
    case 11: statusValue = currentStatus.egoCorrection; break; //Exhaust gas correction (%)
    case 12: statusValue = currentStatus.iatCorrection; break; //Air temperature Correction (%)
    case 13: statusValue = currentStatus.wueCorrection; break; //Warmup enrichment (%)
    case 14: statusValue = lowByte(currentStatus.RPM); break; //rpm HB
    case 15: statusValue = highByte(currentStatus.RPM); break; //rpm LB
    case 16: statusValue = currentStatus.AEamount; break; //acceleration enrichment (%)
    case 17: statusValue = currentStatus.corrections; break; //Total GammaE (%)
    case 18: statusValue = currentStatus.VE; break; //Current VE 1 (%)
    case 19: statusValue = currentStatus.afrTarget; break;
    case 20: statusValue = lowByte(currentStatus.PW1); break; //Pulsewidth 1 multiplied by 10 in ms. Have to convert from uS to mS.
    case 21: statusValue = highByte(currentStatus.PW1); break; //Pulsewidth 1 multiplied by 10 in ms. Have to convert from uS to mS.
    case 22: statusValue = (uint8_t)(currentStatus.tpsDOT / 10); break; //TPS DOT
    case 23: statusValue = currentStatus.advance; break;
    case 24: statusValue = currentStatus.TPS; break; // TPS (0% to 100%)
    case 25: statusValue = lowByte(currentStatus.loopsPerSecond); break;
    case 26: statusValue = highByte(currentStatus.loopsPerSecond); break;

    case 27: currentLoggerStatus.free_ram = freeRam(); statusValue = lowByte(currentLoggerStatus.free_ram); break; //(byte)((currentStatus.loopsPerSecond >> 8) & 0xFF); break;
    case 28: currentLoggerStatus.free_ram = freeRam(); statusValue = highByte(currentLoggerStatus.free_ram); break;

    case 29: statusValue = currentAdvancedEngineStatus.boost_target / 2U; break; //Divide boost target by 2 to fit in a byte
    case 30: statusValue = currentAdvancedEngineStatus.boost_duty / 100U; break;
    case 31: statusValue = buildStatus2(currentStatus); break; //Spark related bitfield, launchHard(0), launchSoft(1), hardLimitOn(2), softLimitOn(3), boostCutSpark(4), error(5), idleControlOn(6), sync(7)
    case 32: statusValue = lowByte(currentStatus.rpmDOT); break;
    case 33: statusValue = highByte(currentStatus.rpmDOT); break;
    case 34: statusValue = currentStatus.ethanolPct; break; //Flex sensor value (or 0 if not used)
    case 35: statusValue = currentStatus.flexCorrection; break; //Flex fuel correction (% above or below 100)
    case 36: statusValue = currentStatus.flexIgnCorrection; break; //Ignition correction (Increased degrees of advance) for flex fuel
    case 37: statusValue = currentStatus.idleLoad; break;
    case 38: statusValue = buildTestOutput(currentStatus); break; // testEnabled(0), testActive(1)
    case 39: statusValue = currentStatus.O2_2; break; //O2
    case 40: statusValue = currentStatus.baro; break; //Barometer value
    case 41: statusValue = lowByte(currentCanAuxStatus.values[0]); break;
    case 42: statusValue = highByte(currentCanAuxStatus.values[0]); break;
    case 43: statusValue = lowByte(currentCanAuxStatus.values[1]); break;
    case 44: statusValue = highByte(currentCanAuxStatus.values[1]); break;
    case 45: statusValue = lowByte(currentCanAuxStatus.values[2]); break;
    case 46: statusValue = highByte(currentCanAuxStatus.values[2]); break;
    case 47: statusValue = lowByte(currentCanAuxStatus.values[3]); break;
    case 48: statusValue = highByte(currentCanAuxStatus.values[3]); break;
    case 49: statusValue = lowByte(currentCanAuxStatus.values[4]); break;
    case 50: statusValue = highByte(currentCanAuxStatus.values[4]); break;
    case 51: statusValue = lowByte(currentCanAuxStatus.values[5]); break;
    case 52: statusValue = highByte(currentCanAuxStatus.values[5]); break;
    case 53: statusValue = lowByte(currentCanAuxStatus.values[6]); break;
    case 54: statusValue = highByte(currentCanAuxStatus.values[6]); break;
    case 55: statusValue = lowByte(currentCanAuxStatus.values[7]); break;
    case 56: statusValue = highByte(currentCanAuxStatus.values[7]); break;
    case 57: statusValue = lowByte(currentCanAuxStatus.values[8]); break;
    case 58: statusValue = highByte(currentCanAuxStatus.values[8]); break;
    case 59: statusValue = lowByte(currentCanAuxStatus.values[9]); break;
    case 60: statusValue = highByte(currentCanAuxStatus.values[9]); break;
    case 61: statusValue = lowByte(currentCanAuxStatus.values[10]); break;
    case 62: statusValue = highByte(currentCanAuxStatus.values[10]); break;
    case 63: statusValue = lowByte(currentCanAuxStatus.values[11]); break;
    case 64: statusValue = highByte(currentCanAuxStatus.values[11]); break;
    case 65: statusValue = lowByte(currentCanAuxStatus.values[12]); break;
    case 66: statusValue = highByte(currentCanAuxStatus.values[12]); break;
    case 67: statusValue = lowByte(currentCanAuxStatus.values[13]); break;
    case 68: statusValue = highByte(currentCanAuxStatus.values[13]); break;
    case 69: statusValue = lowByte(currentCanAuxStatus.values[14]); break;
    case 70: statusValue = highByte(currentCanAuxStatus.values[14]); break;
    case 71: statusValue = lowByte(currentCanAuxStatus.values[15]); break;
    case 72: statusValue = highByte(currentCanAuxStatus.values[15]); break;

    case 73: statusValue = currentStatus.tpsADC; break;
    case 74: statusValue = 0U /*getNextError()*/; break; // errorNum (0:1), currentError(2:7)

    case 75: statusValue = currentStatus.launchCorrection; break;
    case 76: statusValue = lowByte(currentStatus.PW2); break; //Pulsewidth 2 multiplied by 10 in ms. Have to convert from uS to mS.
    case 77: statusValue = highByte(currentStatus.PW2); break; //Pulsewidth 2 multiplied by 10 in ms. Have to convert from uS to mS.
    case 78: statusValue = lowByte(currentStatus.PW3); break; //Pulsewidth 3 multiplied by 10 in ms. Have to convert from uS to mS.
    case 79: statusValue = highByte(currentStatus.PW3); break; //Pulsewidth 3 multiplied by 10 in ms. Have to convert from uS to mS.
    case 80: statusValue = lowByte(currentStatus.PW4); break; //Pulsewidth 4 multiplied by 10 in ms. Have to convert from uS to mS.
    case 81: statusValue = highByte(currentStatus.PW4); break; //Pulsewidth 4 multiplied by 10 in ms. Have to convert from uS to mS.

    case 82: statusValue = buildStatus3(currentStatus); break;
    case 83: statusValue = buildEngineProtectStatus(currentStatus); break; //RPM(0), MAP(1), OIL(2), AFR(3), Unused(4:7)
    case 84: statusValue = lowByte(currentStatus.fuelLoad); break;
    case 85: statusValue = highByte(currentStatus.fuelLoad); break;
    case 86: statusValue = lowByte(currentStatus.ignLoad); break;
    case 87: statusValue = highByte(currentStatus.ignLoad); break;
    case 88: statusValue = lowByte(currentStatus.injAngle); break; 
    case 89: statusValue = highByte(currentStatus.injAngle); break; 
    case 90: statusValue = currentStatus.idleLoad; break;
    case 91: statusValue = currentStatus.CLIdleTarget; break; //closed loop idle target
    case 92: statusValue = currentStatus.mapDOT / 10; break; //rate of change of the map 
    case 93: statusValue = (int8_t)currentAdvancedEngineStatus.vvt1_angle; break;
    case 94: statusValue = currentAdvancedEngineStatus.vvt1_target_angle; break;
    case 95: statusValue = currentAdvancedEngineStatus.vvt1_duty; break;
    case 96: statusValue = lowByte(currentAdvancedEngineStatus.flex_boost_correction); break;
    case 97: statusValue = highByte(currentAdvancedEngineStatus.flex_boost_correction); break;
    case 98: statusValue = currentStatus.baroCorrection; break;
    case 99: statusValue = currentStatus.ASEValue; break; //Current ASE (%)
    case 100: statusValue = lowByte(currentStatus.vss); break; //speed reading from the speed sensor
    case 101: statusValue = highByte(currentStatus.vss); break;
    case 102: statusValue = currentStatus.gear; break; 
    case 103: statusValue = currentStatus.fuelPressure; break;
    case 104: statusValue = currentStatus.oilPressure; break;
    case 105: statusValue = currentAdvancedEngineStatus.wmi_pw; break;
    case 106: statusValue = buildStatus4(currentStatus); break;
    case 107: statusValue = (int8_t)currentAdvancedEngineStatus.vvt2_angle; break;
    case 108: statusValue = currentAdvancedEngineStatus.vvt2_target_angle; break;
    case 109: statusValue = currentAdvancedEngineStatus.vvt2_duty; break;
    case 110: statusValue = currentStatus.outputsStatus; break;
    case 111: statusValue = temperatureAddOffset(currentStatus.fuelTemp); break; //Fuel temperature from flex sensor
    case 112: statusValue = currentStatus.fuelTempCorrection; break; //Fuel temperature Correction (%)
    case 113: statusValue = currentStatus.VE1; break; //VE 1 (%)
    case 114: statusValue = currentStatus.VE2; break; //VE 2 (%)
    case 115: statusValue = currentStatus.advance1; break; //advance 1 
    case 116: statusValue = currentStatus.advance2; break; //advance 2 
    case 117: statusValue = currentAdvancedEngineStatus.nitrous_status; break;
    case 118: statusValue = buildSdCardStatus(currentStatus); break; //SD card status
    case 119: statusValue = lowByte(currentStatus.EMAP); break; //2 bytes for EMAP
    case 120: statusValue = highByte(currentStatus.EMAP); break;
    case 121: statusValue = currentAdvancedEngineStatus.fan_duty; break;
    case 122: statusValue = buildAirConStatus(currentStatus); break;
  }

  return statusValue;
}

/** 
 * Searches the log 2 byte array to determine whether a given index is a regular single byte or a 2 byte field
 * Uses a boundless binary search for improved performance, but requires the fsIntIndex to remain in order
 * 
 * @param key - Index in the log array to check
 * @return True if the index is a 2 byte log field. False if it is a single byte
 */
bool is2ByteEntry(uint8_t key)
{
  // This array indicates which index values from the log are 2 byte values
  // This array MUST remain in ascending order
  // !!!! WARNING: If any value above 255 is required in this array, changes MUST be made to is2ByteEntry() function !!!!
  static constexpr byte PROGMEM fsIntIndex[] = {4, 14, 17, 22, 26, 28, 33, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62, 64, 66, 68, 70, 72, 76, 78, 80, 82, 86, 88, 90, 93, 95, 99, 104, 111, 121, 125, 130, 132, 134, 136 };

  unsigned int bot = 0U;
  unsigned int mid = _countof(fsIntIndex);
  
  while (mid > 1U)
  {  
    if (key >= pgm_read_byte( &fsIntIndex[bot + mid / 2U]) )
    {
      bot += mid++ / 2U;
    }
    mid /= 2U;
  }

  return key == pgm_read_byte(&fsIntIndex[bot]);
}
