#pragma once

#include <stdint.h>

struct logger_status_t
{
  bool is_tooth_log_1_full = false;
  bool tooth_log_enabled = false;
  uint8_t composite_trigger_used = 0;
  uint16_t free_ram = 0;
};

extern logger_status_t currentLoggerStatus;
