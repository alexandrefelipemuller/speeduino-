#include "orchestration/loop_helpers.h"

#include "data/runtime_constants.h"
#include "data/tune_registry.h"
#include "orchestration/init.h"
#include "engine/corrections.h"
#include "support/maths.h"
#include "support/units.h"
#include "support/hw_test_bits.h"
#include "support/table2d.h"

constexpr table2D_u8_u16_4 injectorAngleTable(&configPage2.injAngRPM, &configPage2.injAng);
constexpr table2D_u8_u8_8 rotarySplitTable(&configPage10.rotarySplitBins, &configPage10.rotarySplitValues);
constexpr table2D_u8_u8_10 idleTargetTable(&configPage6.iacBins, &configPage6.iacCLValues);

namespace {

uint16_t set_fuel_trim_to_pw(trimTable3d *pTrimTable, uint16_t fuelLoad, int16_t RPM, uint16_t currentPW)
{
    uint8_t pw1percent = 100U + get3DTableValue(pTrimTable, fuelLoad, RPM) - OFFSET_FUELTRIM;
    return percentageApprox(pw1percent, currentPW);
}

void set_fuel_schedule(FuelSchedule &schedule, uint8_t channel, uint16_t pw, uint16_t startAngle, uint16_t crankAngle, byte fuelChannelsOn)
{
  if( (pw != 0U) && (BIT_CHECK(fuelChannelsOn, INJ1_CMD_BIT+channel-1U)) )
  {
    uint32_t timeOut = calculateInjectorTimeout(schedule, startAngle, crankAngle);
    if (timeOut>0U)
    {
      setFuelSchedule(schedule, timeOut, pw);
    }
  }
}

void set_ignition_channel(IgnitionSchedule &schedule, uint8_t channel, uint16_t channelDegrees, uint16_t startAngle, uint16_t crankAngle, uint16_t dwell, byte ignitionChannelsOn)
{
  if ((currentStatus.maxIgnOutputs >= channel) && BIT_CHECK(ignitionChannelsOn, channel-1U))
  {
    uint32_t timeOut = calculateIgnitionTimeout(schedule, startAngle, channelDegrees, crankAngle);
    if (timeOut > 0U)
    {
      setIgnitionSchedule(schedule, timeOut, dwell);
    }
  }
}

} // namespace

uint16_t applyFuelTrimToPW(trimTable3d *pTrimTable, uint16_t fuelLoad, int16_t RPM, uint16_t currentPW)
{
    return set_fuel_trim_to_pw(pTrimTable, fuelLoad, RPM, currentPW);
}

uint16_t getInjectorAngle(uint16_t rpm_div100)
{
  return table2D_getValue(&injectorAngleTable, (uint8_t)rpm_div100);
}

uint16_t getIdleTarget(uint16_t coolant)
{
  return table2D_getValue(&idleTargetTable, temperatureAddOffset(coolant));
}

uint8_t getRotarySplit(uint8_t ignLoad)
{
  return table2D_getValue(&rotarySplitTable, ignLoad);
}

uint8_t getVE1(void)
{
  currentStatus.fuelLoad = getLoad(configPage2.fuelAlgorithm, currentStatus);
  return get3DTableValue(&fuelTable, currentStatus.fuelLoad, currentStatus.RPM);
}

int8_t getAdvance1(void)
{
  currentStatus.ignLoad = getLoad(configPage2.ignAlgorithm, currentStatus);
  return correctionsIgn((int16_t)get3DTableValue(&ignitionTable, currentStatus.ignLoad, currentStatus.RPM) - INT16_C(OFFSET_IGNITION));
}

void setFuelSchedules(const statuses &current, const uint16_t (&injectionStartAngles)[INJ_CHANNELS], uint16_t crankAngle, byte fuelChannelsOn)
{
#define SET_FUEL_CHANNEL(channel) \
  set_fuel_schedule(fuelSchedule ##channel, UINT8_C(channel), current.PW ##channel, injectionStartAngles[(channel)-1U], crankAngle, fuelChannelsOn);

#if INJ_CHANNELS >= 1
  SET_FUEL_CHANNEL(1)
#endif
#if INJ_CHANNELS >= 2
  SET_FUEL_CHANNEL(2)
#endif
#if INJ_CHANNELS >= 3
  SET_FUEL_CHANNEL(3)
#endif
#if INJ_CHANNELS >= 4
  SET_FUEL_CHANNEL(4)
#endif
#if INJ_CHANNELS >= 5
  SET_FUEL_CHANNEL(5)
#endif
#if INJ_CHANNELS >= 6
  SET_FUEL_CHANNEL(6)
#endif
#if INJ_CHANNELS >= 7
  SET_FUEL_CHANNEL(7)
#endif
#if INJ_CHANNELS >= 8
  SET_FUEL_CHANNEL(8)
#endif

#undef SET_FUEL_CHANNEL
}

void setIgnitionChannels(uint16_t crankAngle, uint16_t dwell, byte ignitionChannelsOn)
{
#define SET_IGNITION_CHANNEL(channelIdx) \
  set_ignition_channel(ignitionSchedule ##channelIdx, UINT8_C((channelIdx)), channel ##channelIdx ##IgnDegrees, ignition ##channelIdx ##StartAngle, crankAngle, dwell, ignitionChannelsOn);

#if IGN_CHANNELS >= 1
  SET_IGNITION_CHANNEL(1)
#endif

#if defined(USE_IGN_REFRESH)
  if( (isRunning(ignitionSchedule1)) && (ignition1EndAngle > (int)crankAngle) && (configPage4.StgCycles == 0) && (configPage2.perToothIgn != true) )
  {
    unsigned long uSToEnd = 0;
    crankAngle = ignitionLimits(currentStatus.decoder.getCrankAngle());
    if(ignition1EndAngle > (int)crankAngle) { uSToEnd = angleToTimeMicroSecPerDegree( (ignition1EndAngle - crankAngle) ); }
    else { uSToEnd = angleToTimeMicroSecPerDegree( (360 + ignition1EndAngle - crankAngle) ); }
    refreshIgnitionSchedule1( uSToEnd + fixedCrankingOverride );
  }
#endif

#if IGN_CHANNELS >= 2
  SET_IGNITION_CHANNEL(2)
#endif
#if IGN_CHANNELS >= 3
  SET_IGNITION_CHANNEL(3)
#endif
#if IGN_CHANNELS >= 4
  SET_IGNITION_CHANNEL(4)
#endif
#if IGN_CHANNELS >= 5
  SET_IGNITION_CHANNEL(5)
#endif
#if IGN_CHANNELS >= 6
  SET_IGNITION_CHANNEL(6)
#endif
#if IGN_CHANNELS >= 7
  SET_IGNITION_CHANNEL(7)
#endif
#if IGN_CHANNELS >= 8
  SET_IGNITION_CHANNEL(8)
#endif

#undef SET_IGNITION_CHANNEL
}

void calculateIgnitionAngles(uint16_t dwellAngle)
{
  switch (configPage2.nCylinders)
  {
    case 1:
      calculateIgnitionAngle(dwellAngle, channel1IgnDegrees, currentStatus.advance, &ignition1EndAngle, &ignition1StartAngle);
      break;
    case 2:
      calculateIgnitionAngle(dwellAngle, channel1IgnDegrees, currentStatus.advance, &ignition1EndAngle, &ignition1StartAngle);
      calculateIgnitionAngle(dwellAngle, channel2IgnDegrees, currentStatus.advance, &ignition2EndAngle, &ignition2StartAngle);
      break;
    case 3:
      calculateIgnitionAngle(dwellAngle, channel1IgnDegrees, currentStatus.advance, &ignition1EndAngle, &ignition1StartAngle);
      calculateIgnitionAngle(dwellAngle, channel2IgnDegrees, currentStatus.advance, &ignition2EndAngle, &ignition2StartAngle);
      calculateIgnitionAngle(dwellAngle, channel3IgnDegrees, currentStatus.advance, &ignition3EndAngle, &ignition3StartAngle);
      break;
    case 4:
      calculateIgnitionAngle(dwellAngle, channel1IgnDegrees, currentStatus.advance, &ignition1EndAngle, &ignition1StartAngle);
      calculateIgnitionAngle(dwellAngle, channel2IgnDegrees, currentStatus.advance, &ignition2EndAngle, &ignition2StartAngle);

#if IGN_CHANNELS >= 4
      if((configPage4.sparkMode == IGN_MODE_SEQUENTIAL) && currentStatus.decoder.getStatus().syncStatus==SyncStatus::Full)
      {
        if( CRANK_ANGLE_MAX_IGN != 720 ) { changeHalfToFullSync(configPage2, configPage4, currentStatus); }

        calculateIgnitionAngle(dwellAngle, channel3IgnDegrees, currentStatus.advance, &ignition3EndAngle, &ignition3StartAngle);
        calculateIgnitionAngle(dwellAngle, channel4IgnDegrees, currentStatus.advance, &ignition4EndAngle, &ignition4StartAngle);
      }
      else if(configPage4.sparkMode == IGN_MODE_ROTARY)
      {
        byte splitDegrees = getRotarySplit((uint8_t)currentStatus.ignLoad);
        calculateIgnitionTrailingRotary(dwellAngle, splitDegrees, ignition1EndAngle, &ignition3EndAngle, &ignition3StartAngle);
        calculateIgnitionTrailingRotary(dwellAngle, splitDegrees, ignition2EndAngle, &ignition4EndAngle, &ignition4StartAngle);
      }
      else
      {
        if( currentStatus.decoder.getStatus().syncStatus==SyncStatus::Partial && (CRANK_ANGLE_MAX_IGN != 360) ) { changeFullToHalfSync(configPage2, configPage4, currentStatus); }
      }
#endif
      break;
    case 5:
      calculateIgnitionAngle(dwellAngle, channel1IgnDegrees, currentStatus.advance, &ignition1EndAngle, &ignition1StartAngle);
      calculateIgnitionAngle(dwellAngle, channel2IgnDegrees, currentStatus.advance, &ignition2EndAngle, &ignition2StartAngle);
      calculateIgnitionAngle(dwellAngle, channel3IgnDegrees, currentStatus.advance, &ignition3EndAngle, &ignition3StartAngle);
      calculateIgnitionAngle(dwellAngle, channel4IgnDegrees, currentStatus.advance, &ignition4EndAngle, &ignition4StartAngle);
#if (IGN_CHANNELS >= 5)
      calculateIgnitionAngle(dwellAngle, channel5IgnDegrees, currentStatus.advance, &ignition5EndAngle, &ignition5StartAngle);
#endif
      break;
    case 6:
      calculateIgnitionAngle(dwellAngle, channel1IgnDegrees, currentStatus.advance, &ignition1EndAngle, &ignition1StartAngle);
      calculateIgnitionAngle(dwellAngle, channel2IgnDegrees, currentStatus.advance, &ignition2EndAngle, &ignition2StartAngle);
      calculateIgnitionAngle(dwellAngle, channel3IgnDegrees, currentStatus.advance, &ignition3EndAngle, &ignition3StartAngle);

#if IGN_CHANNELS >= 6
      if((configPage4.sparkMode == IGN_MODE_SEQUENTIAL) && currentStatus.decoder.getStatus().syncStatus==SyncStatus::Full)
      {
        if( CRANK_ANGLE_MAX_IGN != 720 ) { changeHalfToFullSync(configPage2, configPage4, currentStatus); }

        calculateIgnitionAngle(dwellAngle, channel4IgnDegrees, currentStatus.advance, &ignition4EndAngle, &ignition4StartAngle);
        calculateIgnitionAngle(dwellAngle, channel5IgnDegrees, currentStatus.advance, &ignition5EndAngle, &ignition5StartAngle);
        calculateIgnitionAngle(dwellAngle, channel6IgnDegrees, currentStatus.advance, &ignition6EndAngle, &ignition6StartAngle);
      }
      else
      {
        if( currentStatus.decoder.getStatus().syncStatus==SyncStatus::Partial && (CRANK_ANGLE_MAX_IGN != 360) ) { changeFullToHalfSync(configPage2, configPage4, currentStatus); }
      }
#endif
      break;
    case 7:
      calculateIgnitionAngle(dwellAngle, channel1IgnDegrees, currentStatus.advance, &ignition1EndAngle, &ignition1StartAngle);
      calculateIgnitionAngle(dwellAngle, channel2IgnDegrees, currentStatus.advance, &ignition2EndAngle, &ignition2StartAngle);
      calculateIgnitionAngle(dwellAngle, channel3IgnDegrees, currentStatus.advance, &ignition3EndAngle, &ignition3StartAngle);
      calculateIgnitionAngle(dwellAngle, channel4IgnDegrees, currentStatus.advance, &ignition4EndAngle, &ignition4StartAngle);
#if (IGN_CHANNELS >= 7)
      calculateIgnitionAngle(dwellAngle, channel5IgnDegrees, currentStatus.advance, &ignition5EndAngle, &ignition5StartAngle);
      calculateIgnitionAngle(dwellAngle, channel6IgnDegrees, currentStatus.advance, &ignition6EndAngle, &ignition6StartAngle);
      calculateIgnitionAngle(dwellAngle, channel7IgnDegrees, currentStatus.advance, &ignition7EndAngle, &ignition7StartAngle);
#endif
      break;
    case 8:
      calculateIgnitionAngle(dwellAngle, channel1IgnDegrees, currentStatus.advance, &ignition1EndAngle, &ignition1StartAngle);
      calculateIgnitionAngle(dwellAngle, channel2IgnDegrees, currentStatus.advance, &ignition2EndAngle, &ignition2StartAngle);
      calculateIgnitionAngle(dwellAngle, channel3IgnDegrees, currentStatus.advance, &ignition3EndAngle, &ignition3StartAngle);
      calculateIgnitionAngle(dwellAngle, channel4IgnDegrees, currentStatus.advance, &ignition4EndAngle, &ignition4StartAngle);

#if IGN_CHANNELS >= 8
      if((configPage4.sparkMode == IGN_MODE_SEQUENTIAL) && currentStatus.decoder.getStatus().syncStatus==SyncStatus::Full)
      {
        if( CRANK_ANGLE_MAX_IGN != 720 ) { changeHalfToFullSync(configPage2, configPage4, currentStatus); }

        calculateIgnitionAngle(dwellAngle, channel5IgnDegrees, currentStatus.advance, &ignition5EndAngle, &ignition5StartAngle);
        calculateIgnitionAngle(dwellAngle, channel6IgnDegrees, currentStatus.advance, &ignition6EndAngle, &ignition6StartAngle);
        calculateIgnitionAngle(dwellAngle, channel7IgnDegrees, currentStatus.advance, &ignition7EndAngle, &ignition7StartAngle);
        calculateIgnitionAngle(dwellAngle, channel8IgnDegrees, currentStatus.advance, &ignition8EndAngle, &ignition8StartAngle);
      }
      else
      {
        if( currentStatus.decoder.getStatus().syncStatus==SyncStatus::Partial && (CRANK_ANGLE_MAX_IGN != 360) ) { changeFullToHalfSync(configPage2, configPage4, currentStatus); }
      }
#endif
      break;
    default:
      break;
  }
}
