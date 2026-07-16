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
  const decoder_status_t decoderStatus = currentStatus.decoder.getStatus();
  const uint16_t decoderRpm = currentStatus.decoder.getRPM();

  if ((decoderStatus.syncStatus != SyncStatus::None) && (decoderRpm > 0U))
  {
    if (decoderRpm > currentStatus.crankRPM)
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
      if (!currentStatus.engineIsRunning || (decoderRpm < (currentStatus.crankRPM - CRANK_RUN_HYSTER)))
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

  return false;
}
