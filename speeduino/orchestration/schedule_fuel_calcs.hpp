#pragma once

#include "data/runtime_constants.h"
#include "orchestration/scheduler.h"
#include "engine/crankMaths.h"
#include "support/maths.h"
#include "orchestration/timers.h"

static inline uint16_t calculateInjectorStartAngle(uint16_t pwDegrees, int16_t injChannelDegrees, uint16_t injAngle)
{
  uint16_t startAngle = (uint16_t)injAngle + (uint16_t)injChannelDegrees;
  while (startAngle < pwDegrees) { startAngle = startAngle + (uint16_t)CRANK_ANGLE_MAX_INJ; }
  startAngle = startAngle - pwDegrees;
  while (startAngle > (uint16_t)CRANK_ANGLE_MAX_INJ) { startAngle = startAngle - (uint16_t)CRANK_ANGLE_MAX_INJ; }
  return startAngle;
}

static inline bool injectorStartIsDueNow(int openAngle, int crankAngle)
{
  return openAngle == crankAngle;
}

static inline uint32_t calculateInjectorTimeout(const FuelSchedule &schedule, int openAngle, int crankAngle)
{
  int16_t delta = openAngle - crankAngle;
  if (delta < 0)
  {
    if (schedule.Status != PENDING)
    {
      while (delta < 0) { delta += CRANK_ANGLE_MAX_INJ; }
    }
    else
    {
      delta = 0;
    }
  }
  return angleToTimeMicroSecPerDegree((uint16_t)delta);
}
