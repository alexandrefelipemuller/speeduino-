#include "orchestration/ignition_runtime.h"

#include "data/runtime_constants.h"
#include "data/tune_registry.h"
#include "model/tables.h"
#include "orchestration/init.h"
#include "support/hw_test_bits.h"
#include "support/maths.h"
#include "support/table2d.h"
#include "engine/corrections.h"
#include "orchestration/schedule_angles.h"
#include "orchestration/schedule_ignition_calcs.hpp"

constexpr table2D_u8_u8_8 rotarySplitTable(&configPage10.rotarySplitBins, &configPage10.rotarySplitValues);

namespace {

static constexpr uint32_t MINIMUM_SCHEDULE_DELAY_US = ticksToMicros((COMPARE_TYPE)1U);

void set_ignition_channel(IgnitionSchedule &schedule, uint8_t channel, uint16_t channelDegrees, uint16_t startAngle, uint16_t crankAngle, uint16_t dwell, byte ignitionChannelsOn)
{
  if ((currentStatus.maxIgnOutputs >= channel) && BIT_CHECK(ignitionChannelsOn, channel-1U))
  {
    uint32_t timeOut = calculateIgnitionTimeout(schedule, startAngle, channelDegrees, crankAngle);
    if ((timeOut > 0U) || ignitionStartIsDueNow(startAngle, crankAngle))
    {
      if (timeOut == 0U) { timeOut = MINIMUM_SCHEDULE_DELAY_US; }
      setIgnitionSchedule(schedule, timeOut, dwell);
    }
  }
}

} // namespace

uint8_t getRotarySplit(uint8_t ignLoad)
{
  return table2D_getValue(&rotarySplitTable, ignLoad);
}

int8_t getAdvance1(void)
{
  currentStatus.ignLoad = getLoad(configPage2.ignAlgorithm, currentStatus);
  return correctionsIgn((int16_t)get3DTableValue(&ignitionTable, currentStatus.ignLoad, currentStatus.RPM) - INT16_C(OFFSET_IGNITION));
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

void calculateIgnitionAngles(uint16_t dwellAngle, SyncStatus syncStatus)
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
      if((configPage4.sparkMode == IGN_MODE_SEQUENTIAL) && syncStatus==SyncStatus::Full)
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
        if( syncStatus==SyncStatus::Partial && (CRANK_ANGLE_MAX_IGN != 360) ) { changeFullToHalfSync(configPage2, configPage4, currentStatus); }
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
      if((configPage4.sparkMode == IGN_MODE_SEQUENTIAL) && syncStatus==SyncStatus::Full)
      {
        if( CRANK_ANGLE_MAX_IGN != 720 ) { changeHalfToFullSync(configPage2, configPage4, currentStatus); }

        calculateIgnitionAngle(dwellAngle, channel4IgnDegrees, currentStatus.advance, &ignition4EndAngle, &ignition4StartAngle);
        calculateIgnitionAngle(dwellAngle, channel5IgnDegrees, currentStatus.advance, &ignition5EndAngle, &ignition5StartAngle);
        calculateIgnitionAngle(dwellAngle, channel6IgnDegrees, currentStatus.advance, &ignition6EndAngle, &ignition6StartAngle);
      }
      else
      {
        if( syncStatus==SyncStatus::Partial && (CRANK_ANGLE_MAX_IGN != 360) ) { changeFullToHalfSync(configPage2, configPage4, currentStatus); }
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
      if((configPage4.sparkMode == IGN_MODE_SEQUENTIAL) && syncStatus==SyncStatus::Full)
      {
        if( CRANK_ANGLE_MAX_IGN != 720 ) { changeHalfToFullSync(configPage2, configPage4, currentStatus); }

        calculateIgnitionAngle(dwellAngle, channel5IgnDegrees, currentStatus.advance, &ignition5EndAngle, &ignition5StartAngle);
        calculateIgnitionAngle(dwellAngle, channel6IgnDegrees, currentStatus.advance, &ignition6EndAngle, &ignition6StartAngle);
        calculateIgnitionAngle(dwellAngle, channel7IgnDegrees, currentStatus.advance, &ignition7EndAngle, &ignition7StartAngle);
        calculateIgnitionAngle(dwellAngle, channel8IgnDegrees, currentStatus.advance, &ignition8EndAngle, &ignition8StartAngle);
      }
      else
      {
        if( syncStatus==SyncStatus::Partial && (CRANK_ANGLE_MAX_IGN != 360) ) { changeFullToHalfSync(configPage2, configPage4, currentStatus); }
      }
#endif
      break;
    default:
      break;
  }
}
