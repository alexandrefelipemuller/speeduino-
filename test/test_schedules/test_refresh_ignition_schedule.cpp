#include <unity.h>
#include "../test_utils.h"
#include "boards/board_native.h"
#include "orchestration/scheduler.h"
#include "orchestration/scheduler_lifecycle.h"

static void reset_schedule_state(void)
{
  ignitionSchedule1.Status = OFF;
  ignitionSchedule1.duration = 0U;
  ignitionSchedule1.nextStartCompare = 0U;
  ignitionSchedule1.endCompare = 0U;
  ignitionSchedule1.endScheduleSetByDecoder = false;
  IGN1_COUNTER = 100U;
  IGN1_COMPARE = 150U;
}

static void test_refresh_ignition_schedule_ignores_short_refresh_window(void)
{
  reset_schedule_state();
  ignitionSchedule1.Status = RUNNING;
  ignitionSchedule1.duration = uS_TO_TIMER_COMPARE(2000U);

  refreshIgnitionSchedule1(IGNITION_REFRESH_THRESHOLD);

  TEST_ASSERT_EQUAL_UINT32(0U, ignitionSchedule1.endCompare);
  TEST_ASSERT_EQUAL_UINT32(150U, IGN1_COMPARE);
}

static void test_refresh_ignition_schedule_updates_when_window_is_safe(void)
{
  reset_schedule_state();
  ignitionSchedule1.Status = RUNNING;
  ignitionSchedule1.duration = uS_TO_TIMER_COMPARE(2000U);

  refreshIgnitionSchedule1(IGNITION_REFRESH_THRESHOLD + 1U);

  const auto expected = (uint32_t)IGN1_COUNTER + uS_TO_TIMER_COMPARE(IGNITION_REFRESH_THRESHOLD + 1U);
  TEST_ASSERT_EQUAL_UINT32(expected, ignitionSchedule1.endCompare);
  TEST_ASSERT_EQUAL_UINT32(expected, IGN1_COMPARE);
}

static void test_refresh_ignition_schedule_ignores_longer_than_duration(void)
{
  reset_schedule_state();
  ignitionSchedule1.Status = RUNNING;
  ignitionSchedule1.duration = uS_TO_TIMER_COMPARE(200U);

  refreshIgnitionSchedule1(500U);

  TEST_ASSERT_EQUAL_UINT32(0U, ignitionSchedule1.endCompare);
  TEST_ASSERT_EQUAL_UINT32(150U, IGN1_COMPARE);
}

void test_refresh_ignition_schedule(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_refresh_ignition_schedule_ignores_short_refresh_window);
    RUN_TEST_P(test_refresh_ignition_schedule_updates_when_window_is_safe);
    RUN_TEST_P(test_refresh_ignition_schedule_ignores_longer_than_duration);
  }
}
