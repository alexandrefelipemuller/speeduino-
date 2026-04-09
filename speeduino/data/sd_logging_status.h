#pragma once

#include <stdint.h>

struct sd_logging_status_t
{
  bool card_present = false;
  uint8_t card_type = 0;
  bool card_ready = false;
  bool card_logging = false;
  bool card_error = false;
  uint8_t card_fs = 0;
  bool card_unused = false;
};

extern sd_logging_status_t currentSdLoggingStatus;
