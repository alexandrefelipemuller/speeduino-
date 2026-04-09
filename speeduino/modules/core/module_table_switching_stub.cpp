#include "modules/table_switching/module_table_switching.h"
#include "support/preprocessor.h"

#if !FEATURE_MODULE_TABLE_SWITCHING

void module_table_switching_apply(const config2 &, const config10 &, const table3d16RpmLoad &, const table3d16RpmLoad &, statuses &current)
{
  current.secondFuelTableActive = false;
  current.secondSparkTableActive = false;
  current.VE2 = 0U;
  current.advance2 = 0;
}

#endif
