#include "orchestration/main_loop_engine_lifecycle.h"

#include "orchestration/main_loop_engine_running.h"
#include "orchestration/main_loop_engine_stopped.h"
#include "comms/comms.h"
#include "data/runtime_state.h"
#include "storage/storage.h"

#ifndef UNIT_TEST

void runMainLoopEngineLifecycleTasks(void)
{
    if(currentLoopTime > micros())
    {
      setStorageWriteTimeout(0);
    }

    currentLoopTime = micros();
    if ( currentStatus.decoder.isEngineRunning(currentLoopTime) )
    {
      runMainLoopEngineRunningTasks();
    }
    else
    {
      runMainLoopEngineStoppedTasks();
    }
}

#endif // UNIT_TEST
