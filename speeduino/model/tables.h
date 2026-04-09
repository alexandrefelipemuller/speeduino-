#ifndef TABLES_H
#define TABLES_H

#include "support/table3d.h"

extern struct table3d16RpmLoad fuelTable; //16x16 fuel map
extern struct table3d16RpmLoad fuelTable2; //16x16 fuel map
extern struct table3d16RpmLoad ignitionTable; //16x16 ignition map
extern struct table3d16RpmLoad ignitionTable2; //16x16 ignition map
extern struct table3d16RpmLoad afrTable; //16x16 afr target map
extern struct table3d8RpmLoad stagingTable; //8x8 fuel staging table
extern struct table3d8RpmLoad boostTable; //8x8 boost map
extern struct table3d8RpmLoad boostTableLookupDuty; //8x8 boost map
extern struct table3d8RpmLoad vvtTable; //8x8 vvt map
extern struct table3d8RpmLoad vvt2Table; //8x8 vvt map
extern struct table3d8RpmLoad wmiTable; //8x8 wmi map

using trimTable3d = table3d6RpmLoad;

extern trimTable3d trim1Table; //6x6 Fuel trim 1 map
extern trimTable3d trim2Table; //6x6 Fuel trim 2 map
extern trimTable3d trim3Table; //6x6 Fuel trim 3 map
extern trimTable3d trim4Table; //6x6 Fuel trim 4 map
extern trimTable3d trim5Table; //6x6 Fuel trim 5 map
extern trimTable3d trim6Table; //6x6 Fuel trim 6 map
extern trimTable3d trim7Table; //6x6 Fuel trim 7 map
extern trimTable3d trim8Table; //6x6 Fuel trim 8 map

extern struct table3d4RpmLoad dwellTable; //4x4 Dwell map

#endif // TABLES_H
