#pragma once

#include "statuses.h"
#include "config_pages.h"
#include "table3d.h"

void module_table_switching_apply(const config2 &page2, const config10 &page10, const table3d16RpmLoad &fuel_table2, const table3d16RpmLoad &ignition_table2, statuses &current);
