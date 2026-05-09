#pragma once

#include <stdint.h>

struct etb_status_t
{
  bool enabled = false;
  bool fault = false;
  uint8_t fault_code = 0U;
  uint8_t pedal_percent = 0U;
  uint8_t throttle_percent = 0U;
  uint8_t target_percent = 0U;
  uint8_t open_duty = 0U;
  uint8_t close_duty = 0U;
  int16_t error = 0;
  uint16_t pedal1_adc = 0U;
  uint16_t pedal2_adc = 0U;
  uint16_t throttle2_adc = 0U;
};

extern etb_status_t currentEtbStatus;
