#include "orchestration/main_loop_engine_running.h"

#include "engine/auxiliaries.h"
#include "data/runtime_state.h"

#ifndef UNIT_TEST

void runMainLoopEngineRunningTasks(void)
{
    setRpm(currentStatus, currentStatus.decoder.getRPM());
    if( (currentStatus.RPM > 0) && (currentStatus.fuelPumpOn == false) )
    {
      fuelPumpOn();
    }
}

#endif // UNIT_TEST
