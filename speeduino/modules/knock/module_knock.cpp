#include "modules/knock/module_knock.h"

#include "support/preprocessor.h"

#if FEATURE_MODULE_KNOCK

void module_knock_init_post_pin_mapping(void)
{
}

void module_knock_tick_10hz(void)
{
}

void module_knock_on_engine_stop(void)
{
}

#else
void module_knock_init_post_pin_mapping(void) {}
void module_knock_tick_10hz(void) {}
void module_knock_on_engine_stop(void) {}
#endif
