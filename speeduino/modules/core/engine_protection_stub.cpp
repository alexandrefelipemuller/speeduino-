#include "modules/core/module_interfaces.h"

#include "data/config_pages.h"
#include "data/statuses.h"
#include "support/preprocessor.h"

#if !FEATURE_MODULE_ENGINE_PROTECTION
statuses::scheduler_cut_t module_engine_protection_scheduler_cut(statuses &current, const config2 &, const config4 &, const config6 &, const config9 &, const config10 &)
{
  return current.schedulerCutState;
}
#endif
