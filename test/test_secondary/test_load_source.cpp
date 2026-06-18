#include <unity.h>
#include "../test_utils.h"
#include "engine/load_source.h"

static void test_getLoad_map(void)
{
  statuses current = {};
  current.MAP = 87U;

  TEST_ASSERT_EQUAL_UINT16(87U, getLoad(LOAD_SOURCE_MAP, current));
}

static void test_getLoad_tps(void)
{
  statuses current = {};
  current.TPS = 42U;

  TEST_ASSERT_EQUAL_UINT16(84U, getLoad(LOAD_SOURCE_TPS, current));
}

static void test_getLoad_imap_emap(void)
{
  statuses current = {};
  current.MAP = 200U;
  current.EMAP = 250U;

  TEST_ASSERT_EQUAL_UINT16(80U, getLoad(LOAD_SOURCE_IMAPEMAP, current));
}

static void test_getLoad_imap_emap_zero_emap_falls_back_to_map(void)
{
  statuses current = {};
  current.MAP = 145U;
  current.EMAP = 0U;

  TEST_ASSERT_EQUAL_UINT16(current.MAP, getLoad(LOAD_SOURCE_IMAPEMAP, current));
}

void test_load_source(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_getLoad_map);
    RUN_TEST_P(test_getLoad_tps);
    RUN_TEST_P(test_getLoad_imap_emap);
    RUN_TEST_P(test_getLoad_imap_emap_zero_emap_falls_back_to_map);
  }
}
