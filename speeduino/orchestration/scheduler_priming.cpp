#include "orchestration/scheduler_priming.h"

#include "data/config_pages.h"
#include "data/tune_registry.h"
#include "data/runtime_state.h"
#include "support/table2d.h"
#include "support/units.h"
#include "engine/sensors.h"
#include "orchestration/scheduler.h"

constexpr table2D_u8_u8_4 PrimingPulseTable(&configPage2.primeBins, &configPage2.primePulse);

/** Perform the injector priming pulses.
 * Set these to run at an arbitrary time in the future (100us).
 * The prime pulse value is in ms*10, so need to multiple by 100 to get to uS
 */
void beginInjectorPriming(void)
{
  uint16_t primingValue = (uint16_t)table2D_getValue(&PrimingPulseTable, temperatureAddOffset(currentStatus.coolant));
  if( (primingValue > 0U) && (currentStatus.TPS <= configPage4.floodClear) )
  {
    constexpr uint32_t PRIMING_DELAY = 100U; // 100us
    // The prime pulse value is in ms*2, so need to multiply by 500 to get to µS
    constexpr uint16_t PULSE_TS_SCALE_FACTOR = 100U * 5U;

    const uint32_t scaledPrimingValue = (uint32_t)primingValue * (uint32_t)PULSE_TS_SCALE_FACTOR;
    primingValue = (scaledPrimingValue > UINT16_MAX) ? UINT16_MAX : (uint16_t)scaledPrimingValue;
    if ( currentStatus.maxInjOutputs >= 1U ) { setFuelSchedule(fuelSchedule1, PRIMING_DELAY, primingValue); }
#if (INJ_CHANNELS >= 2)
    if ( currentStatus.maxInjOutputs >= 2U ) { setFuelSchedule(fuelSchedule2, PRIMING_DELAY, primingValue); }
#endif
#if (INJ_CHANNELS >= 3)
    if ( currentStatus.maxInjOutputs >= 3U ) { setFuelSchedule(fuelSchedule3, PRIMING_DELAY, primingValue); }
#endif
#if (INJ_CHANNELS >= 4)
    if ( currentStatus.maxInjOutputs >= 4U ) { setFuelSchedule(fuelSchedule4, PRIMING_DELAY, primingValue); }
#endif
#if (INJ_CHANNELS >= 5)
    if ( currentStatus.maxInjOutputs >= 5U ) { setFuelSchedule(fuelSchedule5, PRIMING_DELAY, primingValue); }
#endif
#if (INJ_CHANNELS >= 6)
    if ( currentStatus.maxInjOutputs >= 6U ) { setFuelSchedule(fuelSchedule6, PRIMING_DELAY, primingValue); }
#endif
#if (INJ_CHANNELS >= 7)
    if ( currentStatus.maxInjOutputs >= 7U) { setFuelSchedule(fuelSchedule7, PRIMING_DELAY, primingValue); }
#endif
#if (INJ_CHANNELS >= 8)
    if ( currentStatus.maxInjOutputs >= 8U ) { setFuelSchedule(fuelSchedule8, PRIMING_DELAY, primingValue); }
#endif
  }
}
