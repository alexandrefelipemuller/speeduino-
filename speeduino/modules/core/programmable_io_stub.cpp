#include "modules/programmable_io/programmable_io.h"

#include "support/preprocessor.h"

#if !FEATURE_MODULE_PROGRAMMABLE_IO

void module_programmable_io_init_post_pin_mapping(void)
{
}

void module_programmable_io_tick_10hz(void)
{
}

#endif
