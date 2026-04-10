#include "orchestration/main_loop_engine_stopped.h"

#include "data/logger_status.h"
#include "data/runtime_constants.h"
#include "data/runtime_state.h"
#include "data/tune_registry.h"
#include "orchestration/loop_helpers.h"
#include "comms/comms.h"
#include "modules/core/module_runtime.h"
#include "engine/auxiliaries.h"
#include "engine/corrections.h"
#include "engine/decoder_init.h"
#include "engine/idle.h"
#include "engine/sensors.h"
#include "storage/storage.h"
#include "support/preprocessor.h"

#ifndef UNIT_TEST

void runMainLoopEngineStoppedTasks(void)
{
    setRpm(currentStatus, 0);
    currentStatus.PW1 = 0;
    currentStatus.VE = 0;
    currentStatus.VE2 = 0;
    currentStatus.decoder.reset();
    currentStatus.runSecs = 0;
    currentStatus.startRevolutions = 0;
    resetMAPcycleAndEvent();
    currentStatus.rpmDOT = 0;
    initialiseCorrections();
    ignitionCount = 0;
    if (currentStatus.fpPrimed == true) { fuelPumpOff(); }
    if (configPage6.iacPWMrun == false) { disableIdle(); }
    currentStatus.engineIsCranking = false;
    currentStatus.wueIsActive = false;
    currentStatus.engineIsRunning = false;
    currentStatus.aseIsActive = false;
    currentStatus.isAcceleratingTPS = false;
    currentStatus.isDeceleratingTPS = false;
    if( (currentLoggerStatus.tooth_log_enabled == false) && (currentLoggerStatus.composite_trigger_used == 0U) ) {
      currentStatus.decoder = buildDecoder(configPage4.TrigPattern);
    }

    core_modules_on_engine_stop();
}

#endif // UNIT_TEST
