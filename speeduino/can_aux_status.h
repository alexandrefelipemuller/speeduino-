#pragma once

#include <stdint.h>

struct can_aux_status_t
{
  uint16_t values[16];
  uint8_t current_channel = 0;
};

extern can_aux_status_t currentCanAuxStatus;
