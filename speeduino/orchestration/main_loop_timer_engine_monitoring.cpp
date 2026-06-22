#include "orchestration/main_loop_timer_engine_monitoring.h"

#include "data/advanced_engine_status.h"
#include "data/logger_status.h"
#include "data/pin_registry.h"
#include "data/runtime_constants.h"
#include "data/runtime_state.h"
#include "data/tune_registry.h"
#include "support/hw_test_bits.h"
#include "orchestration/loop_helpers.h"
#include "modules/core/module_runtime.h"
#include "support/preprocessor.h"

#ifndef UNIT_TEST

void runMainLoopTimerEngineMonitoringTasks(void)
{
    if (BIT_CHECK(LOOP_TIMER, BIT_TIMER_15HZ))
    {
#if  defined(CORE_TEENSY35)
      if (configPage9.enable_intcan == 1)
      {
      }
#endif
      core_modules_tick_15hz();
      if(toothHistoryIndex >= TOOTH_LOG_SIZE) { currentLoggerStatus.is_tooth_log_1_full = true; }
    }
    if (BIT_CHECK(LOOP_TIMER, BIT_TIMER_1HZ))
    {
      currentStatus.systemTemp = getSystemTemp();

      if ( (configPage10.wmiEnabled > 0) && (configPage10.wmiIndicatorEnabled > 0) )
      {
        if (currentAdvancedEngineStatus.wmi_tank_empty)
        {
          digitalWrite(pinWMIIndicator, !digitalRead(pinWMIIndicator));
        }
        else
        {
          digitalWrite(pinWMIIndicator, configPage10.wmiIndicatorPolarity ? HIGH : LOW);
        }
      }

      core_modules_tick_1hz(currentStatus);
    }
}

#endif // UNIT_TEST
