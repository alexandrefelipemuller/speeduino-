#include "orchestration/main_loop_timer_engine.h"

#include "orchestration/main_loop_timer_engine_idle.h"
#include "orchestration/main_loop_timer_engine_monitoring.h"

#ifndef UNIT_TEST

void runMainLoopTimerEngineTasks(void)
{
    runMainLoopTimerEngineMonitoringTasks();
    runMainLoopTimerEngineIdleTasks();
}

#endif // UNIT_TEST
