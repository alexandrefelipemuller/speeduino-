#include "data/config_pages.h"
#include "data/runtime_state.h"

uint8_t softLimitTime = 0; //The time (in 0.1 seconds, based on seclx10) that the soft limiter started
uint16_t fixedCrankingOverride = 0;
volatile byte HWTest_INJ = 0; /**< Each bit in this variable represents one of the injector channels and it's HW test status */
volatile byte HWTest_INJ_Pulsed = 0; /**< Each bit in this variable represents one of the injector channels and it's pulsed HW test status */
volatile byte HWTest_IGN = 0; /**< Each bit in this variable represents one of the ignition channels and it's HW test status */
volatile byte HWTest_IGN_Pulsed = 0;
byte resetControl = RESET_CONTROL_DISABLED;
bool clutchTrigger = false;
bool previousClutchTrigger = false;
int CRANK_ANGLE_MAX_IGN = 360;
int CRANK_ANGLE_MAX_INJ = 360; ///< The number of crank degrees that the system track over. Typically 720 divided by the number of squirts per cycle (Eg 360 for wasted 2 squirt and 720 for sequential single squirt)
