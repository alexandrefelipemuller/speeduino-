#include "modules/advanced_engine/module_advanced_engine.h"
#include "support/preprocessor.h"

#if !FEATURE_MODULE_ADVANCED_ENGINE

void module_advanced_engine_on_engine_stop(const config4 &) {}
void module_advanced_engine_tick_30hz(void) {}
void module_advanced_engine_tick_15hz(void) {}
void module_advanced_engine_tick_10hz(void) {}
void module_advanced_engine_tick_4hz(void) {}

statuses::scheduler_cut_t module_advanced_engine_scheduler_cut(statuses &, const config2 &, const config4 &, const config6 &, const config9 &, const config10 &)
{
  return statuses::scheduler_cut_t();
}

#endif
