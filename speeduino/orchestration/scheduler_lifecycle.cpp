#include "orchestration/scheduler_lifecycle.h"

#include "orchestration/schedule_angles.h"
#include "orchestration/schedule_calcs.h"
#include "support/preprocessor.h"

static void resetSchedule(Schedule &schedule)
{
    schedule.Status = OFF;
    setCallbacks(schedule, nullCallback, nullCallback);
}

void initialiseFuelSchedulers(void)
{
    resetSchedule(fuelSchedule1);
    resetSchedule(fuelSchedule2);
    resetSchedule(fuelSchedule3);
    resetSchedule(fuelSchedule4);
#if INJ_CHANNELS >= 5
    resetSchedule(fuelSchedule5);
#endif
#if INJ_CHANNELS >= 6
    resetSchedule(fuelSchedule6);
#endif
#if INJ_CHANNELS >= 7
    resetSchedule(fuelSchedule7);
#endif
#if INJ_CHANNELS >= 8
    resetSchedule(fuelSchedule8);
#endif

	channel1InjDegrees = 0;
	channel2InjDegrees = 0;
	channel3InjDegrees = 0;
	channel4InjDegrees = 0;
#if (INJ_CHANNELS >= 5)
	channel5InjDegrees = 0;
#endif
#if (INJ_CHANNELS >= 6)
	channel6InjDegrees = 0;
#endif
#if (INJ_CHANNELS >= 7)
	channel7InjDegrees = 0;
#endif
#if (INJ_CHANNELS >= 8)
	channel8InjDegrees = 0;
#endif
}

void initialiseIgnitionSchedulers(void)
{
    resetSchedule(ignitionSchedule1);
    resetSchedule(ignitionSchedule2);
    resetSchedule(ignitionSchedule3);
    resetSchedule(ignitionSchedule4);
#if (IGN_CHANNELS >= 5)
    resetSchedule(ignitionSchedule5);
#endif
#if IGN_CHANNELS >= 6
    resetSchedule(ignitionSchedule6);
#endif
#if IGN_CHANNELS >= 7
    resetSchedule(ignitionSchedule7);
#endif
#if IGN_CHANNELS >= 8
    resetSchedule(ignitionSchedule8);
#endif

  ignition1StartAngle=0;
  ignition1EndAngle=0;
  channel1IgnDegrees=0;
  ignition2StartAngle=0;
  ignition2EndAngle=0;
  channel2IgnDegrees=0;
  ignition3StartAngle=0;
  ignition3EndAngle=0;
  channel3IgnDegrees=0;
  ignition4StartAngle=0;
  ignition4EndAngle=0;
  channel4IgnDegrees=0;

#if (IGN_CHANNELS >= 5)
  ignition5StartAngle=0;
  ignition5EndAngle=0;
  channel5IgnDegrees=0;
#endif
#if (IGN_CHANNELS >= 6)
  ignition6StartAngle=0;
  ignition6EndAngle=0;
  channel6IgnDegrees=0;
#endif
#if (IGN_CHANNELS >= 7)
  ignition7StartAngle=0;
  ignition7EndAngle=0;
  channel7IgnDegrees=0;
#endif
#if (IGN_CHANNELS >= 8)
  ignition8StartAngle=0;
  ignition8EndAngle=0;
  channel8IgnDegrees=0;
#endif
}

void startIgnitionSchedulers(void)
{
  IGN1_TIMER_ENABLE();
#if IGN_CHANNELS >= 2
  IGN2_TIMER_ENABLE();
#endif
#if IGN_CHANNELS >= 3
  IGN3_TIMER_ENABLE();
#endif
#if IGN_CHANNELS >= 4
  IGN4_TIMER_ENABLE();
#endif
#if IGN_CHANNELS >= 5
  IGN5_TIMER_ENABLE();
#endif
#if IGN_CHANNELS >= 6
  IGN6_TIMER_ENABLE();
#endif
#if IGN_CHANNELS >= 7
  IGN7_TIMER_ENABLE();
#endif
#if IGN_CHANNELS >= 8
  IGN8_TIMER_ENABLE();
#endif
}

void stopIgnitionSchedulers(void)
{
  IGN1_TIMER_DISABLE();
#if IGN_CHANNELS >= 2
  IGN2_TIMER_DISABLE();
#endif
#if IGN_CHANNELS >= 3
  IGN3_TIMER_DISABLE();
#endif
#if IGN_CHANNELS >= 4
  IGN4_TIMER_DISABLE();
#endif
#if IGN_CHANNELS >= 5
  IGN5_TIMER_DISABLE();
#endif
#if IGN_CHANNELS >= 6
  IGN6_TIMER_DISABLE();
#endif
#if IGN_CHANNELS >= 7
  IGN7_TIMER_DISABLE();
#endif
#if IGN_CHANNELS >= 8
  IGN8_TIMER_DISABLE();
#endif
}

void startFuelSchedulers(void)
{
  FUEL1_TIMER_ENABLE();
  FUEL2_TIMER_ENABLE();
  FUEL3_TIMER_ENABLE();
  FUEL4_TIMER_ENABLE();
#if INJ_CHANNELS >= 5
  FUEL5_TIMER_ENABLE();
#endif
#if INJ_CHANNELS >= 6
  FUEL6_TIMER_ENABLE();
#endif
#if INJ_CHANNELS >= 7
  FUEL7_TIMER_ENABLE();
#endif
#if INJ_CHANNELS >= 8
  FUEL8_TIMER_ENABLE();
#endif
}

void stopFuelSchedulers(void)
{
  FUEL1_TIMER_DISABLE();
  FUEL2_TIMER_DISABLE();
  FUEL3_TIMER_DISABLE();
  FUEL4_TIMER_DISABLE();
#if INJ_CHANNELS >= 5
  FUEL5_TIMER_DISABLE();
#endif
#if INJ_CHANNELS >= 6
  FUEL6_TIMER_DISABLE();
#endif
#if INJ_CHANNELS >= 7
  FUEL7_TIMER_DISABLE();
#endif
#if INJ_CHANNELS >= 8
  FUEL8_TIMER_DISABLE();
#endif
}

void refreshIgnitionSchedule1(unsigned long timeToEnd)
{
  if( (isRunning(ignitionSchedule1)) && (uS_TO_TIMER_COMPARE(timeToEnd) < ignitionSchedule1.duration) )
  {
    ATOMIC() {
      ignitionSchedule1.endCompare = IGN1_COUNTER + uS_TO_TIMER_COMPARE(timeToEnd);
      SET_COMPARE(IGN1_COMPARE, ignitionSchedule1.endCompare);
    }
  }
}
