#include "data/runtime_state.h"

byte fpPrimeTime = 0; ///< The time (in seconds, based on @ref statuses.secl) that the fuel pump started priming
volatile uint16_t mainLoopCount = 0; //Main loop counter (incremented at each main loop rev., used for maintaining currentStatus.loopsPerSecond)
volatile unsigned long timer5_overflow_count = 0; //Increments every time counter 5 overflows. Used for the fast version of micros()
volatile unsigned long ms_counter = 0; //A counter that increments once per ms
volatile uint32_t toothHistory[TOOTH_LOG_SIZE] = {0}; ///< Tooth trigger history - delta time (in uS) from last tooth (Indexed by @ref toothHistoryIndex)
volatile uint8_t compositeLogHistory[TOOTH_LOG_SIZE] = {0};
volatile unsigned int toothHistoryIndex = 0; ///< Current index to @ref toothHistory array
unsigned long currentLoopTime = 0; /**< The time (in uS) that the current mainloop started */
volatile uint16_t ignitionCount = 0; /**< The count of ignition events that have taken place since the engine started */
volatile uint32_t runSecsX10 = 0;
volatile uint32_t seclx10 = 0;
volatile byte TIMER_mask = 0;
volatile byte LOOP_TIMER = 0;
