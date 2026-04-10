#include "orchestration/fuel_runtime.h"

#include "data/runtime_constants.h"
#include "data/tune_registry.h"
#include "support/hw_test_bits.h"
#include "support/maths.h"
#include "support/units.h"
#include "support/table2d.h"
#include "engine/corrections.h"
#include "orchestration/schedule_fuel_calcs.hpp"

constexpr table2D_u8_u16_4 injectorAngleTable(&configPage2.injAngRPM, &configPage2.injAng);
constexpr table2D_u8_u8_10 idleTargetTable(&configPage6.iacBins, &configPage6.iacCLValues);

namespace {

uint16_t set_fuel_trim_to_pw(trimTable3d *pTrimTable, uint16_t fuelLoad, int16_t RPM, uint16_t currentPW)
{
    uint8_t pw1percent = 100U + get3DTableValue(pTrimTable, fuelLoad, RPM) - OFFSET_FUELTRIM;
    return percentageApprox(pw1percent, currentPW);
}

void set_fuel_schedule(FuelSchedule &schedule, uint8_t channel, uint16_t pw, uint16_t startAngle, uint16_t crankAngle, byte fuelChannelsOn)
{
  if( (pw != 0U) && (BIT_CHECK(fuelChannelsOn, INJ1_CMD_BIT+channel-1U)) )
  {
    uint32_t timeOut = calculateInjectorTimeout(schedule, startAngle, crankAngle);
    if (timeOut>0U)
    {
      setFuelSchedule(schedule, timeOut, pw);
    }
  }
}

} // namespace

uint16_t applyFuelTrimToPW(trimTable3d *pTrimTable, uint16_t fuelLoad, int16_t RPM, uint16_t currentPW)
{
    return set_fuel_trim_to_pw(pTrimTable, fuelLoad, RPM, currentPW);
}

uint16_t getInjectorAngle(uint16_t rpm_div100)
{
  return table2D_getValue(&injectorAngleTable, (uint8_t)rpm_div100);
}

uint16_t getIdleTarget(uint16_t coolant)
{
  return table2D_getValue(&idleTargetTable, temperatureAddOffset(coolant));
}

uint8_t getVE1(void)
{
  currentStatus.fuelLoad = getLoad(configPage2.fuelAlgorithm, currentStatus);
  return get3DTableValue(&fuelTable, currentStatus.fuelLoad, currentStatus.RPM);
}

void setFuelSchedules(const statuses &current, const uint16_t (&injectionStartAngles)[INJ_CHANNELS], uint16_t crankAngle, byte fuelChannelsOn)
{
#define SET_FUEL_CHANNEL(channel) \
  set_fuel_schedule(fuelSchedule ##channel, UINT8_C(channel), current.PW ##channel, injectionStartAngles[(channel)-1U], crankAngle, fuelChannelsOn);

#if INJ_CHANNELS >= 1
  SET_FUEL_CHANNEL(1)
#endif
#if INJ_CHANNELS >= 2
  SET_FUEL_CHANNEL(2)
#endif
#if INJ_CHANNELS >= 3
  SET_FUEL_CHANNEL(3)
#endif
#if INJ_CHANNELS >= 4
  SET_FUEL_CHANNEL(4)
#endif
#if INJ_CHANNELS >= 5
  SET_FUEL_CHANNEL(5)
#endif
#if INJ_CHANNELS >= 6
  SET_FUEL_CHANNEL(6)
#endif
#if INJ_CHANNELS >= 7
  SET_FUEL_CHANNEL(7)
#endif
#if INJ_CHANNELS >= 8
  SET_FUEL_CHANNEL(8)
#endif

#undef SET_FUEL_CHANNEL
}
