#include "modules/nitrous/module_nitrous.h"

#include "support/preprocessor.h"

#if FEATURE_MODULE_NITROUS

#include "data/tune_registry.h"

void module_nitrous_init_post_pin_mapping(void)
{
  initialiseNitrous(configPage10);
}

void module_nitrous_tick_4hz(void)
{
  nitrousControl();
}
#endif
