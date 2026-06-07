#include "modules/launch_control/launch_control.h"

#include "support/preprocessor.h"

#include "data/advanced_engine_status.h"
#include "data/core_constants.h"
#include "data/runtime_state.h"
#include "data/tune_registry.h"
#include "support/units.h"

#if FEATURE_MODULE_LAUNCH_CONTROL
static constexpr uint8_t NO_FUEL_CORRECTION = 100U;

uint8_t launch_control_fuel_correction(void)
{
  int8_t correction = (int8_t)NO_FUEL_CORRECTION;
  bool launchingHard = currentAdvancedEngineStatus.launching_hard;
  bool launchingSoft = currentAdvancedEngineStatus.launching_soft;
#ifdef UNIT_TEST
  launchingHard = launchingHard || currentStatus.launchingHard;
  launchingSoft = launchingSoft || currentStatus.launchingSoft;
#endif
  if (launchingHard || launchingSoft) {
    correction = correction + configPage6.lnchFuelAdd;
  }
  return (uint8_t)correction;
}

int8_t launch_control_soft_ignition_correction(int8_t advance)
{
  // Keep the launch decision in one place so launch_flatshift can reuse it.
  if(  configPage6.launchEnabled 
    && currentStatus.clutchTrigger 
    && (currentStatus.clutchEngagedRPM < RPM_COARSE.toUser( configPage6.flatSArm))
    && (currentStatus.RPM > RPM_COARSE.toUser( configPage6.lnchSoftLim))
    && (currentStatus.TPS >= configPage10.lnchCtrlTPS) 
    && ( (configPage2.vssMode == VSS_MODE_OFF) || (currentStatus.vss <= configPage10.lnchCtrlVss) )
    )
  {
    currentAdvancedEngineStatus.launching_soft = true;
#ifdef UNIT_TEST
    currentStatus.launchingSoft = true;
#endif
    currentStatus.softLaunchActive = true;
    advance = configPage6.lnchRetard;
  }
  else
  {
    currentAdvancedEngineStatus.launching_soft = false;
#ifdef UNIT_TEST
    currentStatus.launchingSoft = false;
#endif
    currentStatus.softLaunchActive = false;
  }

  return advance;
}

#endif
