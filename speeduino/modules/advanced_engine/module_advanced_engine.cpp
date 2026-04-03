#include "module_advanced_engine.h"

#include "preprocessor.h"

#if FEATURE_MODULE_ADVANCED_ENGINE

#include "auxiliaries.h"
#include "board_definition.h"
#include "modules/advanced_engine/engineProtection.h"
#include "utilities.h"

extern byte pinIgnBypass;

void checkLaunchAndFlatShift(void);

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
