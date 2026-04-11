#include "modules/advanced_engine/module_advanced_engine.h"
#include "support/preprocessor.h"

#if !FEATURE_MODULE_ADVANCED_ENGINE

void module_advanced_engine_on_engine_stop(const config4 &) {}
void module_advanced_engine_tick_30hz(void) {}
void module_advanced_engine_tick_10hz(void) {}

#endif
