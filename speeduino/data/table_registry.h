#pragma once

#include "table2d.h"
#include "table3d.h"

using trimTable3d = table3d6RpmLoad;

extern struct table3d16RpmLoad fuelTable;
extern struct table3d16RpmLoad fuelTable2;
extern struct table3d16RpmLoad ignitionTable;
extern struct table3d16RpmLoad ignitionTable2;
extern struct table3d16RpmLoad afrTable;
extern struct table3d8RpmLoad stagingTable;
extern struct table3d8RpmLoad boostTable;
extern struct table3d8RpmLoad boostTableLookupDuty;
extern struct table3d8RpmLoad vvtTable;
extern struct table3d8RpmLoad vvt2Table;
extern struct table3d8RpmLoad wmiTable;
extern trimTable3d trim1Table;
extern trimTable3d trim2Table;
extern trimTable3d trim3Table;
extern trimTable3d trim4Table;
extern trimTable3d trim5Table;
extern trimTable3d trim6Table;
extern trimTable3d trim7Table;
extern trimTable3d trim8Table;
extern struct table3d4RpmLoad dwellTable;
