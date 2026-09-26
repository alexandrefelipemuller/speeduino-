#pragma once

#include "data/runtime_constants.h"
#include "data/runtime_state.h"
#include "orchestration/scheduler.h"
#include "engine/crankMaths.h"
#include "support/maths.h"
#include "orchestration/timers.h"

static inline void calculateIgnitionAngle(const uint16_t dwellAngle, const uint16_t channelIgnDegrees, int8_t advance, int *pEndAngle, int *pStartAngle)
{
  *pEndAngle = (int16_t)(channelIgnDegrees == 0U ? (uint16_t)CRANK_ANGLE_MAX_IGN : channelIgnDegrees) - (int16_t)advance;
  if (*pEndAngle > CRANK_ANGLE_MAX_IGN) { *pEndAngle -= CRANK_ANGLE_MAX_IGN; }
  *pStartAngle = *pEndAngle - dwellAngle;
  if (*pStartAngle < 0) { *pStartAngle += CRANK_ANGLE_MAX_IGN; }
}

static inline void calculateIgnitionTrailingRotary(uint16_t dwellAngle, int rotarySplitDegrees, int leadIgnitionAngle, int *pEndAngle, int *pStartAngle)
{
  *pEndAngle = leadIgnitionAngle + rotarySplitDegrees;
  *pStartAngle = *pEndAngle - dwellAngle;
  if (*pStartAngle > CRANK_ANGLE_MAX_IGN) { *pStartAngle -= CRANK_ANGLE_MAX_IGN; }
  if (*pStartAngle < 0) { *pStartAngle += CRANK_ANGLE_MAX_IGN; }
}

static inline __attribute__((always_inline)) uint32_t _calculateIgnitionTimeout(const IgnitionSchedule &schedule, int16_t startAngle, int16_t crankAngle)
{
  int16_t delta = startAngle - crankAngle;
  if (delta < 0)
  {
    if ((isRunning(schedule)) && (delta > -CRANK_ANGLE_MAX_IGN))
    {
      delta = delta + CRANK_ANGLE_MAX_IGN;
    }
    else
    {
      return 0U;
    }
  }
  return angleToTimeMicroSecPerDegree(delta);
}

static inline uint16_t _adjustToIgnChannel(int angle, int channelInjDegrees)
{
  angle = angle - channelInjDegrees;
  if (angle < 0) { return angle + CRANK_ANGLE_MAX_IGN; }
  return angle;
}

static inline bool ignitionStartIsDueNow(int startAngle, int crankAngle)
{
  return startAngle == crankAngle;
}

static inline uint32_t calculateIgnitionTimeout(const IgnitionSchedule &schedule, int startAngle, int channelIgnDegrees, int crankAngle)
{
  if (channelIgnDegrees == 0)
  {
    return _calculateIgnitionTimeout(schedule, startAngle, crankAngle);
  }
  return _calculateIgnitionTimeout(schedule, _adjustToIgnChannel(startAngle, channelIgnDegrees), _adjustToIgnChannel(crankAngle, channelIgnDegrees));
}

#define MIN_CYCLES_FOR_ENDCOMPARE 6U

static inline void adjustCrankAngle(IgnitionSchedule &schedule, int endAngle, int crankAngle)
{
  if (isRunning(schedule))
  {
    SET_COMPARE(schedule._compare, schedule._counter + uS_TO_TIMER_COMPARE(angleToTimeMicroSecPerDegree((uint16_t)ignitionLimits(endAngle - crankAngle))));
  }
  else if (currentStatus.startRevolutions > MIN_CYCLES_FOR_ENDCOMPARE)
  {
    schedule.endCompare = schedule._counter + uS_TO_TIMER_COMPARE(angleToTimeMicroSecPerDegree(ignitionLimits((endAngle - crankAngle))));
    schedule.endScheduleSetByDecoder = true;
  }
}
