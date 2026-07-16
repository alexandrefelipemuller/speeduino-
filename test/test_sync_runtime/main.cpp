#include <unity.h>

#include "../test_harness_device.h"
#include "../test_harness_native.h"
#include "data/runtime_state.h"
#include "data/tune_registry.h"
#include "engine/decoder_builder.h"
#include "engine/decoder_init.h"
#include "orchestration/sync_runtime.h"

void resetStoppedDecoderResetState(void);
bool shouldResetStoppedDecoder(bool decoderReportsRunning, bool engineWasActive, uint32_t loopTime);

static uint8_t resetCalls;

static void countDecoderReset(void)
{
  resetCalls++;
}

static decoder_status_t getUnsyncedDecoderStatus(void)
{
  return decoder_status_t{false, false, SyncStatus::None};
}

static decoder_status_t getSyncedDecoderStatus(void)
{
  return decoder_status_t{false, false, SyncStatus::Partial};
}

static uint16_t getDecoderRpmDuringSync(void)
{
  return 150U;
}

static void test_synced_runtime_uses_decoder_rpm_before_status_rpm_is_published(void)
{
  currentStatus = statuses{};
  configPage2.fanWhenCranking = 1U;
  configPage4.ignBypassEnabled = 0U;
  currentStatus.crankRPM = 300U;
  currentStatus.RPM = 0U;
  currentStatus.engineIsRunning = false;
  currentStatus.engineIsCranking = false;

  currentStatus.decoder = decoder_builder_t()
                            .setGetStatus(getSyncedDecoderStatus)
                            .setGetRPM(getDecoderRpmDuringSync)
                            .build();

  TEST_ASSERT_TRUE(syncRuntimeUpdateEngineState());
  TEST_ASSERT_TRUE(currentStatus.engineIsCranking);
  TEST_ASSERT_FALSE(currentStatus.engineIsRunning);
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


static void test_stopped_lifecycle_does_not_reset_decoder_before_engine_was_active(void)
{
  resetStoppedDecoderResetState();

  TEST_ASSERT_FALSE(shouldResetStoppedDecoder(false, false, 1000UL));
  TEST_ASSERT_FALSE(shouldResetStoppedDecoder(false, false, 200000UL));
}

static void test_stopped_lifecycle_debounces_transient_stall_before_decoder_reset(void)
{
  resetStoppedDecoderResetState();

  TEST_ASSERT_FALSE(shouldResetStoppedDecoder(false, true, 1000UL));
  TEST_ASSERT_FALSE(shouldResetStoppedDecoder(false, false, 99999UL));
  TEST_ASSERT_FALSE(shouldResetStoppedDecoder(true, false, 100000UL));

  TEST_ASSERT_FALSE(shouldResetStoppedDecoder(false, true, 500000UL));
  TEST_ASSERT_FALSE(shouldResetStoppedDecoder(false, false, 599999UL));
}

static void test_stopped_lifecycle_resets_decoder_once_after_confirmed_stop(void)
{
  resetStoppedDecoderResetState();

  TEST_ASSERT_FALSE(shouldResetStoppedDecoder(false, true, 1000UL));
  TEST_ASSERT_FALSE(shouldResetStoppedDecoder(false, false, 100999UL));
  TEST_ASSERT_TRUE(shouldResetStoppedDecoder(false, false, 101000UL));
  TEST_ASSERT_FALSE(shouldResetStoppedDecoder(false, false, 250000UL));
}

static void runSyncRuntimeTests(void)
{
  RUN_TEST(test_unsynced_runtime_does_not_reset_or_rebuild_decoder);
  RUN_TEST(test_synced_runtime_uses_decoder_rpm_before_status_rpm_is_published);
  RUN_TEST(test_stopped_lifecycle_does_not_reset_decoder_before_engine_was_active);
  RUN_TEST(test_stopped_lifecycle_debounces_transient_stall_before_decoder_reset);
  RUN_TEST(test_stopped_lifecycle_resets_decoder_once_after_confirmed_stop);
}

TEST_HARNESS(runSyncRuntimeTests)
