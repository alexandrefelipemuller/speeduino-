#pragma once

#include "data/core_constants.h"
#include "data/statuses.h"

extern byte fpPrimeTime;
extern uint8_t softLimitTime;
extern volatile uint16_t mainLoopCount;
extern volatile unsigned long timer5_overflow_count;
extern volatile unsigned long ms_counter;
extern uint16_t fixedCrankingOverride;
extern volatile uint32_t toothHistory[TOOTH_LOG_SIZE];
extern volatile uint8_t compositeLogHistory[TOOTH_LOG_SIZE];
extern volatile unsigned int toothHistoryIndex;
extern unsigned long currentLoopTime;
extern volatile uint16_t ignitionCount;
extern int CRANK_ANGLE_MAX_IGN;
extern int CRANK_ANGLE_MAX_INJ;
extern volatile uint32_t runSecsX10;
extern volatile uint32_t seclx10;
extern volatile byte HWTest_INJ;
extern volatile byte HWTest_INJ_Pulsed;
extern volatile byte HWTest_IGN;
extern volatile byte HWTest_IGN_Pulsed;
extern byte resetControl;
extern volatile byte TIMER_mask;
extern volatile byte LOOP_TIMER;
extern struct statuses currentStatus;
