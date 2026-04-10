#include "orchestration/scheduler.h"

#include "data/config_pages.h"
#include "data/statuses.h"
#include "orchestration/schedule_state_machine.h"
#include "support/preprocessor.h"

extern struct statuses currentStatus;
extern struct config2 configPage2;
extern struct config4 configPage4;
extern volatile uint16_t ignitionCount;

///@cond
// Dwell smoothing macros. They are split up like this for MISRA compliance.
#define DWELL_AVERAGE_ALPHA 30
#define DWELL_AVERAGE(input) LOW_PASS_FILTER((input), DWELL_AVERAGE_ALPHA, currentStatus.actualDwell)
///@endcond

/**
 * @brief Called when an ignition event ends. I.e. a spark fires
 */
static inline void onEndIgnitionEvent(IgnitionSchedule *pSchedule)
{
  pSchedule->endScheduleSetByDecoder = false;
  ignitionCount = ignitionCount + 1U;
  int32_t elapsed = (int32_t)(micros() - pSchedule->startTime);
  currentStatus.actualDwell = DWELL_AVERAGE(elapsed);
}

/** @brief Called when the supplied schedule transitions from a PENDING state to RUNNING */
BEGIN_LTO_ALWAYS_INLINE(void) static ignitionPendingToRunning(Schedule *pSchedule) {
  defaultPendingToRunning(pSchedule);
  IgnitionSchedule *pIgnition = (IgnitionSchedule *)pSchedule;
  pIgnition->startTime = micros();
  if(pIgnition->endScheduleSetByDecoder) { SET_COMPARE(pIgnition->_compare, pIgnition->endCompare); }
}
END_LTO_INLINE()

/** @brief Called when the supplied schedule transitions from a RUNNING state to OFF */
BEGIN_LTO_ALWAYS_INLINE(void) static ignitionRunningToOff(Schedule *pSchedule) {
  defaultRunningToOff(pSchedule);
  onEndIgnitionEvent((IgnitionSchedule *)pSchedule);
}
END_LTO_INLINE()

/** @brief Called when the supplied schedule transitions from a RUNNING state to PENDING */
BEGIN_LTO_ALWAYS_INLINE(void) static ignitionRunningToPending(Schedule *pSchedule) {
  defaultRunningToPending(pSchedule);
  onEndIgnitionEvent((IgnitionSchedule *)pSchedule);
}
END_LTO_INLINE()

void moveToNextState(IgnitionSchedule &schedule)
{
  movetoNextState(schedule, ignitionPendingToRunning, ignitionRunningToOff, ignitionRunningToPending);
}
