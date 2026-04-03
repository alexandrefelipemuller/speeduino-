#pragma once

#include <stdint.h>

#define MICROS_PER_SEC INT32_C(1000000)
#define MICROS_PER_MIN INT32_C(MICROS_PER_SEC*60U)
#define MICROS_PER_HOUR INT32_C(MICROS_PER_MIN*60U)
#define MILLI_PER_SEC INT32_C(MICROS_PER_SEC/1000)

#ifndef UNIT_TEST
  #define TOOTH_LOG_SIZE 127U
#else
  #define TOOTH_LOG_SIZE 1U
#endif
// Some code relies on TOOTH_LOG_SIZE being uint8_t.
static_assert(TOOTH_LOG_SIZE < UINT8_MAX, "Check all uses of TOOTH_LOG_SIZE");

#define BIT_TIMER_1HZ             0
#define BIT_TIMER_4HZ             1
#define BIT_TIMER_10HZ            2
#define BIT_TIMER_15HZ            3
#define BIT_TIMER_30HZ            4
#define BIT_TIMER_50HZ            5
#define BIT_TIMER_200HZ           6
#define BIT_TIMER_1KHZ            7

#define OFFSET_FUELTRIM 127U
#define OFFSET_IGNITION 40

#define CALIBRATION_TABLE_SIZE 512

#define UINT16_HALF_RANGE 0x8000

#define SERIAL_BUFFER_THRESHOLD 32
