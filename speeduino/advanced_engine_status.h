#pragma once

#include <stdint.h>

struct advanced_engine_status_t
{
  bool nitrous_active = false;
  bool launching_soft = false;
  bool launching_hard = false;
  bool flat_shifting_hard = false;
  bool wmi_tank_empty = false;
  bool vvt1_angle_error = false;
  bool vvt2_angle_error = false;
  bool fan_on = false;
  bool aircon_requested = false;
  bool aircon_compressor_on = false;
  bool aircon_rpm_lockout = false;
  bool aircon_tps_lockout = false;
  bool aircon_turning_on = false;
  bool aircon_clt_lockout = false;
  bool aircon_fan_on = false;
  uint16_t boost_target = 0;
  uint16_t boost_duty = 0;
  int16_t flex_boost_correction = 0;
  int16_t vvt1_angle = 0;
  uint8_t vvt1_target_angle = 0;
  long vvt1_duty = 0;
  uint8_t wmi_pw = 0;
  int16_t vvt2_angle = 0;
  uint8_t vvt2_target_angle = 0;
  long vvt2_duty = 0;
  uint8_t nitrous_status = 0;
  uint8_t fan_duty = 0;
};

extern advanced_engine_status_t currentAdvancedEngineStatus;
