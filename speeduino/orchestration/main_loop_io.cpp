#include "orchestration/main_loop_io.h"

#include "data/runtime_state.h"
#include "boards/board_definition.h"
#include "support/hw_test_bits.h"
#include "support/atomic.h"
#include "comms/comms.h"
#include "comms/comms_legacy.h"
#include "modules/core/module_runtime.h"

#ifndef UNIT_TEST

void runMainLoopIoTasks(void)
{
      if(mainLoopCount < UINT16_MAX) { mainLoopCount++; }
      serviceWatchdog();
      ATOMIC()
      {
        LOOP_TIMER = TIMER_mask;
        TIMER_mask = 0U;
      }

      if (serialTransmitInProgress())
      {
        serialTransmit();
      }

      if( (Serial.available() > 0) || serialRecieveInProgress() )
      {
        serialReceive();
      }
      
      core_modules_poll();
}

#endif // UNIT_TEST
