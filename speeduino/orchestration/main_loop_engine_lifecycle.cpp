#include "orchestration/main_loop_engine_lifecycle.h"

#include "orchestration/main_loop_engine_running.h"
#include "orchestration/main_loop_engine_stopped.h"
#include "comms/comms.h"
#include "data/runtime_state.h"
#include "storage/storage.h"
#include "support/unit_testing.h"

static constexpr uint32_t DECODER_STOPPED_RESET_DELAY_US = 100000UL;

TESTABLE_STATIC uint32_t stoppedDecoderResetCandidateTime = 0U;
TESTABLE_STATIC bool stoppedDecoderResetCandidateActive = false;
TESTABLE_STATIC bool stoppedDecoderResetCandidateWasActive = false;
TESTABLE_STATIC bool stoppedDecoderResetComplete = false;

TESTABLE_STATIC void resetStoppedDecoderResetState(void)
{
    stoppedDecoderResetCandidateTime = 0U;
    stoppedDecoderResetCandidateActive = false;
    stoppedDecoderResetCandidateWasActive = false;
    stoppedDecoderResetComplete = false;
}

TESTABLE_STATIC bool shouldResetStoppedDecoder(bool decoderReportsRunning, bool engineWasActive, uint32_t loopTime)
{
    if (decoderReportsRunning)
    {
      resetStoppedDecoderResetState();
      return false;
    }

    if (stoppedDecoderResetComplete)
    {
      return false;
    }

    if (!stoppedDecoderResetCandidateActive)
    {
      stoppedDecoderResetCandidateTime = loopTime;
      stoppedDecoderResetCandidateActive = true;
      stoppedDecoderResetCandidateWasActive = engineWasActive;
    }
    else if (engineWasActive)
    {
      stoppedDecoderResetCandidateWasActive = true;
    }

    if (!stoppedDecoderResetCandidateWasActive)
    {
      return false;
    }

    if ((uint32_t)(loopTime - stoppedDecoderResetCandidateTime) >= DECODER_STOPPED_RESET_DELAY_US)
    {
      stoppedDecoderResetComplete = true;
      return true;
    }

    return false;
}

#ifndef UNIT_TEST

void runMainLoopEngineLifecycleTasks(void)
{
    if(currentLoopTime > micros())
    {
      setStorageWriteTimeout(0);
    }

    currentLoopTime = micros();
    const bool decoderReportsRunning = currentStatus.decoder.isEngineRunning(currentLoopTime);
    const bool engineWasActive = currentStatus.engineIsRunning || currentStatus.engineIsCranking || (currentStatus.RPM > 0U);

    if (decoderReportsRunning)
    {
      resetStoppedDecoderResetState();
      runMainLoopEngineRunningTasks();
    }
    else
    {
      runMainLoopEngineStoppedTasks(shouldResetStoppedDecoder(decoderReportsRunning, engineWasActive, currentLoopTime), engineWasActive);
    }
}

#endif // UNIT_TEST
