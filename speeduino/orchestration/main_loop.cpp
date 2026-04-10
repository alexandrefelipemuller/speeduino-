#include "orchestration/main_loop.h"

#include "support/preprocessor.h"
#include "orchestration/main_loop_io.h"
#include "orchestration/main_loop_timers.h"
#include "orchestration/main_loop_cycle.h"

#ifndef UNIT_TEST

BEGIN_LTO_ALWAYS_INLINE(void) runMainLoopIteration(void)
{
    runMainLoopIoTasks();
    runMainLoopCycleTasks();
    runMainLoopTimerTasks();
}

#endif // UNIT_TEST
