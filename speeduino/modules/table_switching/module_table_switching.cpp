#include "module_table_switching.h"

#include "preprocessor.h"
#if FEATURE_MODULE_TABLE_SWITCHING

#include "secondaryTables.h"

void module_table_switching_apply(const config2 &page2, const config10 &page10, const table3d16RpmLoad &fuel_table2, const table3d16RpmLoad &ignition_table2, statuses &current)
{
  calculateSecondaryFuel(page10, fuel_table2, current);
  calculateSecondarySpark(page2, page10, ignition_table2, current);
}

#endif
