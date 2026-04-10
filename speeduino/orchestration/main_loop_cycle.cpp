#include "orchestration/main_loop_cycle.h"

#include "orchestration/main_loop_cycle_dispatch.h"
#include "orchestration/main_loop_cycle_inputs.h"

#ifndef UNIT_TEST

void runMainLoopCycleTasks(void)
{
    runMainLoopEngineLifecycleTasks();
    runMainLoopCycleInputTasks();
    runMainLoopCycleDispatchTasks();
}

#endif // UNIT_TEST
