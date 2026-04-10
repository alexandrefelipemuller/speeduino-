#include "orchestration/sync_runtime.h"

#include "data/config_pages.h"
#include "data/logger_status.h"
#include "data/pin_registry.h"
#include "data/runtime_state.h"
#include "data/tune_registry.h"
#include "engine/auxiliaries.h"
#include "engine/corrections.h"
#include "engine/decoder_init.h"
#include "engine/idle.h"
#include "engine/sensors.h"
#include "modules/core/module_runtime.h"
#include "orchestration/loop_helpers.h"

bool syncRuntimeUpdateEngineState(void)
{
  if ((currentStatus.decoder.getStatus().syncStatus != SyncStatus::None) && (currentStatus.RPM > 0))
  {
    if (currentStatus.RPM > currentStatus.crankRPM)
    {
      currentStatus.engineIsRunning = true;
      if (currentStatus.engineIsCranking)
      {
        currentStatus.engineIsCranking = false;
        if (configPage4.ignBypassEnabled > 0) { digitalWrite(pinIgnBypass, HIGH); }
      }
    }
    else
    {
      if (!currentStatus.engineIsRunning || (currentStatus.RPM < (currentStatus.crankRPM - CRANK_RUN_HYSTER)))
      {
        currentStatus.engineIsCranking = true;
        currentStatus.engineIsRunning = false;
        currentStatus.runSecs = 0;
        if (configPage4.ignBypassEnabled > 0) { digitalWrite(pinIgnBypass, LOW); }
        if (configPage2.fanWhenCranking == 0) { fanOff(); }
      }
    }

    return true;
  }

  setRpm(currentStatus, 0U);
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
  if ((currentLoggerStatus.tooth_log_enabled == false) && (currentLoggerStatus.composite_trigger_used == 0U))
  {
    currentStatus.decoder = buildDecoder(configPage4.TrigPattern);
  }

  core_modules_on_engine_stop();
  return false;
}
