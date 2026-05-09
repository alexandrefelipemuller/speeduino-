#include "orchestration/main_loop_timer_maintenance.h"

#include "data/runtime_constants.h"
#include "data/runtime_state.h"
#include "support/hw_test_bits.h"
#include "comms/comms.h"
#include "comms/comms_legacy.h"
#include "storage/storage.h"
#include "modules/core/module_runtime.h"

#ifndef UNIT_TEST

void runMainLoopTimerMaintenanceTasks(void)
{
    if(BIT_CHECK(LOOP_TIMER, BIT_TIMER_1KHZ))
    {
      BIT_CLEAR(TIMER_mask, BIT_TIMER_1KHZ);
    }
    if(BIT_CHECK(LOOP_TIMER, BIT_TIMER_200HZ))
    {
      BIT_CLEAR(TIMER_mask, BIT_TIMER_200HZ);
      core_modules_tick_200hz();
    }
    if(BIT_CHECK(LOOP_TIMER, BIT_TIMER_50HZ))
    {
      BIT_CLEAR(TIMER_mask, BIT_TIMER_50HZ);
      core_modules_tick_50hz();
    }
    if(BIT_CHECK(LOOP_TIMER, BIT_TIMER_30HZ))
    {
      BIT_CLEAR(TIMER_mask, BIT_TIMER_30HZ);
      core_modules_tick_30hz();
      if( (isEepromWritePending() == true) && (serialStatusFlag == SERIAL_INACTIVE) && storageWriteTimeoutExpired()) { saveAllPages(); }
    }
}

#endif // UNIT_TEST
