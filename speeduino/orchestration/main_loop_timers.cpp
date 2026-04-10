#include "orchestration/main_loop_timers.h"
#include "orchestration/main_loop_timer_maintenance.h"
#include "orchestration/main_loop_timer_engine.h"

#ifndef UNIT_TEST

void runMainLoopTimerTasks(void)
{
    runMainLoopTimerMaintenanceTasks();
    runMainLoopTimerEngineTasks();
}

#endif // UNIT_TEST
