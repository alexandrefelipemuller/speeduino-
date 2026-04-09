#include "module_logging.h"

#include "support/preprocessor.h"

#if FEATURE_MODULE_LOGGING && defined(SD_LOGGING)

#include "SD_logger.h"
#include "rtc_common.h"

void module_logging_init(const config13 &page13)
{
  initRTC();
  if(page13.onboard_log_file_style) { initSD(); }
}

void module_logging_tick_30hz(const config13 &page13)
{
  if(page13.onboard_log_file_rate == LOGGER_RATE_30HZ) { writeSDLogEntry(); }
}

void module_logging_tick_10hz(const config13 &page13)
{
  if(page13.onboard_log_file_rate == LOGGER_RATE_10HZ) { writeSDLogEntry(); }
}

void module_logging_tick_4hz(const config13 &page13)
{
  if(page13.onboard_log_file_rate == LOGGER_RATE_4HZ) { writeSDLogEntry(); }
}

void module_logging_tick_1hz(const statuses &current, const config13 &page13)
{
  if(page13.onboard_log_file_rate == LOGGER_RATE_1HZ) { writeSDLogEntry(); }

  if( (current.RPM < SD_SYNC_RPM_THRESHOLD) || (msSinceLastSDSync > SD_SYNC_MAX_TIME_PERIOD) )
  {
    if(syncSDLog()) { msSinceLastSDSync = 0; }
  }
}

#else

void module_logging_init(const config13 &) {}
void module_logging_tick_30hz(const config13 &) {}
void module_logging_tick_10hz(const config13 &) {}
void module_logging_tick_4hz(const config13 &) {}
void module_logging_tick_1hz(const statuses &, const config13 &) {}

#endif
