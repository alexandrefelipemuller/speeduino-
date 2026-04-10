#include "orchestration/scheduler_fuel_state.h"

#include "orchestration/schedule_state_machine.h"

void moveToNextState(FuelSchedule &schedule)
{
  movetoNextState(schedule, defaultPendingToRunning, defaultRunningToOff, defaultRunningToPending);
}
