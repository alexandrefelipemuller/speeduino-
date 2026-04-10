#include "orchestration/scheduler.h"

#include "data/core_constants.h"
#include "support/preprocessor.h"
#include "support/units.h"

// Event duration cannot be longer than the maximum timer period.
static inline uint16_t clipDuration(uint16_t duration)
{
  if (MAX_TIMER_PERIOD < (uint32_t)UINT16_MAX)
  {
    return min((uint16_t)(MAX_TIMER_PERIOD - 1U), duration);
  }
  return duration;
}

void setCallbacks(Schedule &schedule, Schedule::callback pStartCallback, Schedule::callback pEndCallback)
{
  schedule.pStartCallback = pStartCallback;
  schedule.pEndCallback = pEndCallback;
}

static inline void setScheduleNext(Schedule &schedule, uint32_t delay, uint16_t duration)
{
  // The duration of the pulsewidth cannot be longer than the maximum timer period.
  schedule.duration = uS_TO_TIMER_COMPARE(clipDuration(duration));
  schedule.nextStartCompare = schedule._counter + uS_TO_TIMER_COMPARE(delay);
  schedule.Status = RUNNING_WITHNEXT;
}

static inline void setScheduleRunning(Schedule &schedule, uint32_t delay, uint16_t duration)
{
  // The following must be enclosed in the noInterrupts block to avoid contention.
  schedule.duration = uS_TO_TIMER_COMPARE(clipDuration(duration));
  SET_COMPARE(schedule._compare, schedule._counter + uS_TO_TIMER_COMPARE(delay));
  schedule.Status = PENDING;
}

void setSchedule(Schedule &schedule, uint32_t delay, uint16_t duration, bool allowQueuedSchedule)
{
  if((delay>0U) && (delay < MAX_TIMER_PERIOD) && (duration > 0U))
  {
    noInterrupts();
    if(!isRunning(schedule))
    {
      setScheduleRunning(schedule, delay, duration);
    }
    else if(allowQueuedSchedule)
    {
      setScheduleNext(schedule, delay, duration);
    }
    interrupts();
  }
}
