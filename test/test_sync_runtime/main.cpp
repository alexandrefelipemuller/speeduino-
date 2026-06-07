#include <unity.h>

#include "../test_harness_device.h"
#include "../test_harness_native.h"
#include "data/runtime_state.h"
#include "data/tune_registry.h"
#include "engine/decoder_builder.h"
#include "engine/decoder_init.h"
#include "orchestration/sync_runtime.h"

static uint8_t resetCalls;

static void countDecoderReset(void)
{
  resetCalls++;
}

static decoder_status_t getUnsyncedDecoderStatus(void)
{
  return decoder_status_t{false, false, SyncStatus::None};
}

static void test_unsynced_runtime_does_not_reset_or_rebuild_decoder(void)
{
  currentStatus = statuses{};
  configPage2.nCylinders = 4;
  configPage4.TrigPattern = DECODER_MISSING_TOOTH;
  resetCalls = 0;

  currentStatus.decoder = decoder_builder_t()
                            .setReset(countDecoderReset)
                            .setGetStatus(getUnsyncedDecoderStatus)
                            .build();
  auto originalReset = currentStatus.decoder.reset;

  TEST_ASSERT_FALSE(syncRuntimeUpdateEngineState());
  TEST_ASSERT_EQUAL_UINT8(0, resetCalls);
  TEST_ASSERT_EQUAL_PTR(originalReset, currentStatus.decoder.reset);
}

static void runSyncRuntimeTests(void)
{
  RUN_TEST(test_unsynced_runtime_does_not_reset_or_rebuild_decoder);
}

TEST_HARNESS(runSyncRuntimeTests)
