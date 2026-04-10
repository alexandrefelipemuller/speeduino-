#include "orchestration/main_loop_timer_engine_idle.h"

#include "data/advanced_engine_status.h"
#include "data/runtime_constants.h"
#include "data/runtime_state.h"
#include "data/tune_registry.h"
#include "engine/idle.h"
#include "engine/sensors.h"
#include "modules/core/module_runtime.h"
#include "orchestration/loop_helpers.h"
#include "support/hw_test_bits.h"
#include "support/preprocessor.h"

#ifndef UNIT_TEST

void runMainLoopTimerEngineIdleTasks(void)
{
    if(BIT_CHECK(LOOP_TIMER, BIT_TIMER_10HZ))
    {
      BIT_CLEAR(TIMER_mask, BIT_TIMER_10HZ);
      idleControl();
      core_modules_tick_10hz();
    }
    if (BIT_CHECK(LOOP_TIMER, BIT_TIMER_4HZ))
    {
      BIT_CLEAR(TIMER_mask, BIT_TIMER_4HZ);
      if( (configPage2.idleAdvEnabled != IDLEADVANCE_MODE_OFF) || (configPage6.iacAlgorithm != IAC_ALGORITHM_NONE) )
      {
        currentStatus.CLIdleTarget = getIdleTarget(currentStatus.coolant);
        if(currentAdvancedEngineStatus.aircon_turning_on) { currentStatus.CLIdleTarget += configPage15.airConIdleUpRPMAdder;  }
      }

      core_modules_tick_4hz(statusSensors, currentStatus);
    }

    if( (configPage6.iacAlgorithm == IAC_ALGORITHM_STEP_OL)
    || (configPage6.iacAlgorithm == IAC_ALGORITHM_STEP_CL)
    || (configPage6.iacAlgorithm == IAC_ALGORITHM_STEP_OLCL) )
    {
      idleControl();
    }
}

#endif // UNIT_TEST
