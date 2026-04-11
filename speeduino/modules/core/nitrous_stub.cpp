#include "modules/core/module_interfaces.h"

#include "support/preprocessor.h"

#if !FEATURE_MODULE_NITROUS

void module_nitrous_init_post_pin_mapping(void) {}
void module_nitrous_tick_4hz(void) {}

#endif
