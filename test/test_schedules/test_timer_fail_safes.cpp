
#include <Arduino.h>
#include <unity.h>
#include "../test_utils.h"
#include "data/runtime_state.h"

extern bool hasDwellExceededLimit(uint32_t nowUs, uint32_t startUs, uint32_t dwellLimitUs);
extern bool engineAllowsHardwareTestOutputs(void);
extern bool mainLoopStallRequiresSafeStop(uint16_t loopsLastSecond);

static void test_overdwell_limit_before_timeout(void)
{
  TEST_ASSERT_FALSE(hasDwellExceededLimit(10000UL, 9000UL, 2000UL));
}

static void test_overdwell_limit_after_timeout(void)
{
  TEST_ASSERT_TRUE(hasDwellExceededLimit(12001UL, 10000UL, 2000UL));
}

static void test_overdwell_limit_handles_micros_rollover(void)
{
  const uint32_t startUs = UINT32_MAX - 500UL;
  const uint32_t nowUs = 1500UL;

  TEST_ASSERT_TRUE(hasDwellExceededLimit(nowUs, startUs, 2000UL));
}

static void test_overdwell_limit_rollover_before_timeout(void)
{
  const uint32_t startUs = UINT32_MAX - 500UL;
  const uint32_t nowUs = 1000UL;

  TEST_ASSERT_FALSE(hasDwellExceededLimit(nowUs, startUs, 2000UL));
}

static void test_hardware_test_outputs_require_inactive_engine(void)
{
  currentStatus.RPM = 0U;
  currentStatus.engineIsRunning = false;
  currentStatus.engineIsCranking = false;
  TEST_ASSERT_TRUE(engineAllowsHardwareTestOutputs());

  currentStatus.engineIsRunning = true;
  TEST_ASSERT_FALSE(engineAllowsHardwareTestOutputs());

  currentStatus.engineIsRunning = false;
  currentStatus.engineIsCranking = true;
  TEST_ASSERT_FALSE(engineAllowsHardwareTestOutputs());

  currentStatus.engineIsCranking = false;
  currentStatus.RPM = 1U;
  TEST_ASSERT_FALSE(engineAllowsHardwareTestOutputs());
}

static void test_main_loop_stall_fail_safe_requires_active_engine(void)
{
  currentStatus.RPM = 0U;
  currentStatus.engineIsRunning = false;
  currentStatus.engineIsCranking = false;
  TEST_ASSERT_FALSE(mainLoopStallRequiresSafeStop(0U));
}

static void test_main_loop_stall_fail_safe_detects_running_engine(void)
{
  currentStatus.RPM = 900U;
  currentStatus.engineIsRunning = true;
  currentStatus.engineIsCranking = false;
  TEST_ASSERT_TRUE(mainLoopStallRequiresSafeStop(0U));
  TEST_ASSERT_FALSE(mainLoopStallRequiresSafeStop(1U));
}

static void test_main_loop_stall_fail_safe_detects_cranking_engine(void)
{
  currentStatus.RPM = 0U;
  currentStatus.engineIsRunning = false;
  currentStatus.engineIsCranking = true;
  TEST_ASSERT_TRUE(mainLoopStallRequiresSafeStop(0U));
}

void test_timer_fail_safes(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_overdwell_limit_before_timeout);
    RUN_TEST_P(test_overdwell_limit_after_timeout);
    RUN_TEST_P(test_overdwell_limit_handles_micros_rollover);
    RUN_TEST_P(test_overdwell_limit_rollover_before_timeout);
    RUN_TEST_P(test_hardware_test_outputs_require_inactive_engine);
    RUN_TEST_P(test_main_loop_stall_fail_safe_requires_active_engine);
    RUN_TEST_P(test_main_loop_stall_fail_safe_detects_running_engine);
    RUN_TEST_P(test_main_loop_stall_fail_safe_detects_cranking_engine);
  }
}
