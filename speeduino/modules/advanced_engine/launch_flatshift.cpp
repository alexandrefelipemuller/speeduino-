#include "modules/advanced_engine/launch_flatshift.h"

#include "advanced_engine_status.h"
#include "board_definition.h"
#include "pin_registry.h"
#include "runtime_state.h"
#include "tune_registry.h"

void advanced_engine_launch_flatshift_tick(void)
{
  currentStatus.previousClutchTrigger = currentStatus.clutchTrigger;

  if(configPage6.flatSEnable || configPage6.launchEnabled)
  {
    if(configPage6.launchHiLo > 0) { currentStatus.clutchTrigger = digitalRead(pinLaunch); }
    else { currentStatus.clutchTrigger = !digitalRead(pinLaunch); }

    currentStatus.clutchTriggerActive = currentStatus.clutchTrigger;
  }
  if(currentStatus.clutchTrigger && (currentStatus.previousClutchTrigger != currentStatus.clutchTrigger))
  {
    currentStatus.clutchEngagedRPM = currentStatus.RPM;
  }

  currentAdvancedEngineStatus.launching_hard = false;
  currentStatus.hardLaunchActive = false;
  currentAdvancedEngineStatus.flat_shifting_hard = false;

  if(configPage6.launchEnabled &&
     currentStatus.clutchTrigger &&
     (currentStatus.clutchEngagedRPM < ((unsigned int)(configPage6.flatSArm) * 100U)) &&
     (currentStatus.TPS >= configPage10.lnchCtrlTPS))
  {
    if((configPage2.vssMode == 0U) || ((configPage2.vssMode > 0U) && (currentStatus.vss < configPage10.lnchCtrlVss)))
    {
      uint16_t launchRPMLimit = (configPage6.lnchHardLim * 100U);
      if(configPage2.hardCutType == HARD_CUT_ROLLING) { launchRPMLimit += (configPage15.rollingProtRPMDelta[0] * 10); }

      if(currentStatus.RPM > launchRPMLimit)
      {
        currentAdvancedEngineStatus.launching_hard = true;
        currentStatus.hardLaunchActive = true;
      }
    }
  }
  else if(configPage6.flatSEnable &&
          currentStatus.clutchTrigger &&
          (currentStatus.clutchEngagedRPM >= ((unsigned int)(configPage6.flatSArm * 100U))))
  {
    uint16_t flatRPMLimit = currentStatus.clutchEngagedRPM;
    if(configPage2.hardCutType == HARD_CUT_ROLLING) { flatRPMLimit += (configPage15.rollingProtRPMDelta[0] * 10); }

    if(currentStatus.RPM > flatRPMLimit)
    {
      currentAdvancedEngineStatus.flat_shifting_hard = true;
    }
  }
}
