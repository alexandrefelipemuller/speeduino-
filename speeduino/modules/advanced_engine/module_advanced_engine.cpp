#include "module_advanced_engine.h"

#include "support/preprocessor.h"

#if FEATURE_MODULE_ADVANCED_ENGINE

#include "data/advanced_engine_status.h"
#include "engine/auxiliaries.h"
#include "boards/board_definition.h"
#include "data/pin_registry.h"
#include "data/runtime_state.h"
#include "data/tune_registry.h"

extern byte pinIgnBypass;

void module_advanced_engine_on_engine_stop(const config4 &page4)
{
  if(page4.ignBypassEnabled > 0) { digitalWrite(pinIgnBypass, LOW); }
}

void module_advanced_engine_tick_10hz(void)
{
}

#endif
