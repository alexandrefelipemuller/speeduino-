#include "support/preprocessor.h"

#include "modules/launch_flatshift/launch_flatshift.h"

#if FEATURE_MODULE_LAUNCH_FLATSHIFT

void module_launch_flatshift_tick_15hz(void)
{
  advanced_engine_launch_flatshift_tick();
}

#else

void module_launch_flatshift_tick_15hz(void) {}

#endif
