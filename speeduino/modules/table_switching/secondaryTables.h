#pragma once

#include <stdint.h>
#include "data/statuses.h"
#include "data/config_pages.h"
#include "support/table3d.h"

void calculateSecondaryFuel(const config10 &page10, const table3d16RpmLoad &veLookupTable, statuses &current);
void calculateSecondarySpark(const config2 &page2, const config10 &page10, const table3d16RpmLoad &sparkLookupTable, statuses &current);
