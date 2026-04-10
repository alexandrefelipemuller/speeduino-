#include "orchestration/main_loop_cycle_inputs.h"

#include "data/runtime_state.h"
#include "orchestration/loop_helpers.h"
#include "engine/sensors.h"
#include "modules/core/module_runtime.h"

#ifndef UNIT_TEST

void runMainLoopCycleInputTasks(void)
{
    readPolledSensors(LOOP_TIMER);

    currentStatus.VE1 = getVE1();
    currentStatus.VE = currentStatus.VE1;
    currentStatus.advance1 = getAdvance1();
    currentStatus.advance = currentStatus.advance1;

    core_modules_apply_table_switching(currentStatus);
}

#endif // UNIT_TEST
