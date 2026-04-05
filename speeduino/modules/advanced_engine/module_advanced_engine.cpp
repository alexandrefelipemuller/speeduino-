#include "module_advanced_engine.h"

#include "preprocessor.h"

#if FEATURE_MODULE_ADVANCED_ENGINE

#include "advanced_engine_status.h"
#include "auxiliaries.h"
#include "board_definition.h"
#include "modules/advanced_engine/engineProtection.h"
#include "pin_registry.h"
#include "runtime_state.h"
#include "tune_registry.h"
#include "utilities.h"

extern byte pinIgnBypass;

static void checkLaunchAndFlatShift(void)
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

  if(configPage6.launchEnabled && currentStatus.clutchTrigger && (currentStatus.clutchEngagedRPM < ((unsigned int)(configPage6.flatSArm) * 100U)) && (currentStatus.TPS >= configPage10.lnchCtrlTPS))
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
  else if(configPage6.flatSEnable && currentStatus.clutchTrigger && (currentStatus.clutchEngagedRPM >= ((unsigned int)(configPage6.flatSArm * 100U))))
  {
    uint16_t flatRPMLimit = currentStatus.clutchEngagedRPM;
    if(configPage2.hardCutType == HARD_CUT_ROLLING) { flatRPMLimit += (configPage15.rollingProtRPMDelta[0] * 10); }

    if(currentStatus.RPM > flatRPMLimit)
    {
      currentAdvancedEngineStatus.flat_shifting_hard = true;
    }
  }
}

void module_advanced_engine_on_engine_stop(const config4 &page4)
{
  vvt1Off();
  vvt2Off();
  DISABLE_VVT_TIMER();
  boostDisable();
  if(page4.ignBypassEnabled > 0) { digitalWrite(pinIgnBypass, LOW); }
}

void module_advanced_engine_tick_30hz(void)
{
  // Most boost tends to run at about 30Hz, so placing it here ensures a new
  // target time is fetched frequently enough.
  boostControl();
  // VVT may eventually need to be synced with the cam readings, but for now it
  // still runs at 30Hz.
  vvtControl();
  wmiControl();
}

void module_advanced_engine_tick_15hz(void)
{
  checkLaunchAndFlatShift();
}

void module_advanced_engine_tick_10hz(void)
{
  checkProgrammableIO();
  airConControl();
}

void module_advanced_engine_tick_4hz(void)
{
  nitrousControl();
}

statuses::scheduler_cut_t module_advanced_engine_scheduler_cut(statuses &current, const config2 &page2, const config4 &page4, const config6 &page6, const config9 &page9, const config10 &page10)
{
  return calculateFuelIgnitionChannelCut(current, page2, page4, page6, page9, page10);
}

#endif
