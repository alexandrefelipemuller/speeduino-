#include "modules/launch_control/launch_control.h"

#include "support/preprocessor.h"

#if FEATURE_MODULE_LAUNCH_CONTROL
void module_launch_control_init_post_pin_mapping(void)
{
}

void module_launch_control_tick_10hz(void)
{
  // launch control is evaluated from correction paths, not on a fixed tick
}

void module_launch_control_on_engine_stop(void)
{
}

#endif
