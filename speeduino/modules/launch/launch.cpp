#include "modules/launch/launch.h"

#include "boards/board_definition.h"
#include "data/pin_registry.h"
#include "data/runtime_state.h"
#include "data/tune_registry.h"

void launch_update_clutch_state(void)
{
  currentStatus.previousClutchTrigger = currentStatus.clutchTrigger;

  if (configPage6.flatSEnable || configPage6.launchEnabled)
  {
    if (configPage6.launchHiLo > 0) { currentStatus.clutchTrigger = digitalRead(pinLaunch); }
    else { currentStatus.clutchTrigger = !digitalRead(pinLaunch); }

    currentStatus.clutchTriggerActive = currentStatus.clutchTrigger;
  }

  if (currentStatus.clutchTrigger && (currentStatus.previousClutchTrigger != currentStatus.clutchTrigger))
  {
    currentStatus.clutchEngagedRPM = currentStatus.RPM;
  }
}
