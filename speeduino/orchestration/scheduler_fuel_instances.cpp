#include "orchestration/scheduler.h"

FuelSchedule fuelSchedule1(FUEL1_COUNTER, FUEL1_COMPARE); //cppcheck-suppress misra-c2012-8.4
FuelSchedule fuelSchedule2(FUEL2_COUNTER, FUEL2_COMPARE); //cppcheck-suppress misra-c2012-8.4
FuelSchedule fuelSchedule3(FUEL3_COUNTER, FUEL3_COMPARE); //cppcheck-suppress misra-c2012-8.4
FuelSchedule fuelSchedule4(FUEL4_COUNTER, FUEL4_COMPARE); //cppcheck-suppress misra-c2012-8.4
#if (INJ_CHANNELS >= 5)
FuelSchedule fuelSchedule5(FUEL5_COUNTER, FUEL5_COMPARE); //cppcheck-suppress misra-c2012-8.4
#endif
#if (INJ_CHANNELS >= 6)
FuelSchedule fuelSchedule6(FUEL6_COUNTER, FUEL6_COMPARE); //cppcheck-suppress misra-c2012-8.4
#endif
#if (INJ_CHANNELS >= 7)
FuelSchedule fuelSchedule7(FUEL7_COUNTER, FUEL7_COMPARE); //cppcheck-suppress misra-c2012-8.4
#endif
#if (INJ_CHANNELS >= 8)
FuelSchedule fuelSchedule8(FUEL8_COUNTER, FUEL8_COMPARE); //cppcheck-suppress misra-c2012-8.4
#endif
