/** @file
 * EEPROM Storage updates.
 */
/** Store and load various configs to/from EEPROM considering the the data format versions of various SW generations.
 * This routine is used for doing any data conversions that are required during firmware changes.
 * This prevents users getting difference reports in TS when such a data change occurs.
 * It also can be used for setting good values when there are variables that move locations in the ini.
 * When a user skips multiple firmware versions at a time, this will roll through the updates 1 at a time.
 * The doUpdates() uses may lower level routines from Arduino EEPROM library and storage.ino to carry out EEPROM storage tasks.
 */
#include "storage/storage.h"
#include "data/table_registry.h"
#include "data/tune_registry.h"
#include "engine/sensors.h"
#include "storage/updates.h"
#include "storage/pages.h"
#include "comms/comms_CAN.h"
#include "support/units.h"
#include "support/unit_testing.h"

// Minimize flash usage of the non-performance critical code in this file.
#pragma GCC optimize ("Os") 

static constexpr uint8_t CURRENT_DATA_VERSION = 27U;
void runLegacyStorageUpdates(void);
void runRecentStorageUpdates(void);

void doUpdates(void)
{
  runLegacyStorageUpdates();
  runRecentStorageUpdates();
}
