#include "data/runtime_state.h"
#include "data/config_pages.h"

byte fpPrimeTime = 0; ///< The time (in seconds, based on @ref statuses.secl) that the fuel pump started priming
uint8_t softLimitTime = 0; //The time (in 0.1 seconds, based on seclx10) that the soft limiter started
volatile uint16_t mainLoopCount; //Main loop counter (incremented at each main loop rev., used for maintaining currentStatus.loopsPerSecond)
volatile unsigned long timer5_overflow_count = 0; //Increments every time counter 5 overflows. Used for the fast version of micros()
volatile unsigned long ms_counter = 0; //A counter that increments once per ms
uint16_t fixedCrankingOverride = 0;
bool clutchTrigger;
bool previousClutchTrigger;
volatile uint32_t toothHistory[TOOTH_LOG_SIZE]; ///< Tooth trigger history - delta time (in uS) from last tooth (Indexed by @ref toothHistoryIndex)
volatile uint8_t compositeLogHistory[TOOTH_LOG_SIZE];
volatile unsigned int toothHistoryIndex = 0; ///< Current index to @ref toothHistory array
unsigned long currentLoopTime; /**< The time (in uS) that the current mainloop started */
volatile uint16_t ignitionCount; /**< The count of ignition events that have taken place since the engine started */
int CRANK_ANGLE_MAX_IGN = 360;
int CRANK_ANGLE_MAX_INJ = 360; ///< The number of crank degrees that the system track over. Typically 720 divided by the number of squirts per cycle (Eg 360 for wasted 2 squirt and 720 for sequential single squirt)
volatile uint32_t runSecsX10;
volatile uint32_t seclx10;
volatile byte HWTest_INJ = 0; /**< Each bit in this variable represents one of the injector channels and it's HW test status */
volatile byte HWTest_INJ_Pulsed = 0; /**< Each bit in this variable represents one of the injector channels and it's pulsed HW test status */
volatile byte HWTest_IGN = 0; /**< Each bit in this variable represents one of the ignition channels and it's HW test status */
volatile byte HWTest_IGN_Pulsed = 0;
byte resetControl = RESET_CONTROL_DISABLED;
volatile byte TIMER_mask;
volatile byte LOOP_TIMER;
struct statuses currentStatus;
