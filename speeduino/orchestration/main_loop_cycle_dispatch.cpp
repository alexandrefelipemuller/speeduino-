#include "orchestration/main_loop_cycle_dispatch.h"

#include "data/pin_registry.h"
#include "data/runtime_state.h"
#include "data/tune_registry.h"
#include "orchestration/engine_runtime.h"
#include "orchestration/sync_runtime.h"
#include "support/preprocessor.h"

#ifndef UNIT_TEST

void runMainLoopCycleDispatchTasks(void)
{
    if (syncRuntimeUpdateEngineState())
    {
      engineRuntimeRunCycle();
    }
    else if ( (currentStatus.resetPreventActive) && (resetControl == RESET_CONTROL_PREVENT_WHEN_RUNNING) )
    {
      digitalWrite(pinResetControl, LOW);
      currentStatus.resetPreventActive = false;
    }
  else
  {
    ;
  }
}

#endif // UNIT_TEST
