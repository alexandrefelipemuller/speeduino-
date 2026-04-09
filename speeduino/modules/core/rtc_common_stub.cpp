#include "modules/logging/rtc_common.h"

#include "support/preprocessor.h"

#if !FEATURE_MODULE_LOGGING

void initRTC() {}
uint8_t rtc_getSecond() { return 0U; }
uint8_t rtc_getMinute() { return 0U; }
uint8_t rtc_getHour() { return 0U; }
uint8_t rtc_getDay() { return 0U; }
uint8_t rtc_getDOW() { return 0U; }
uint8_t rtc_getMonth() { return 0U; }
uint16_t rtc_getYear() { return 0U; }
void rtc_setTime(byte, byte, byte, byte, byte, uint16_t) {}

#endif
