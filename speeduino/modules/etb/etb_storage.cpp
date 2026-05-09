/** @file
 * ETB EEPROM storage helpers.
 */

#include "modules/etb/etb_storage.h"

#if FEATURE_MODULE_ETB
#include "data/tune_registry.h"
#include "storage/storage.h"

namespace {
static inline uint16_t write_range(const byte *pStart, const byte *pEnd, uint16_t address, uint16_t writesRemaining)
{
  return updateBlockLimitWriteOps(getStorageAPI(), address, pStart, pEnd, writesRemaining);
}

static inline uint16_t load_range(uint16_t address, byte *pFirst, const byte *pLast)
{
  return loadBlock(getStorageAPI(), address, pFirst, pLast);
}
} // namespace

void module_etb_save_pages(uint16_t &writesRemaining)
{
  writesRemaining = write_range((byte *)&configPage16, (byte *)&configPage16 + sizeof(configPage16), EEPROM_CONFIG16_START, writesRemaining);
}

void module_etb_load_pages(void)
{
  (void)load_range(EEPROM_CONFIG16_START, (byte *)&configPage16, (byte *)&configPage16 + sizeof(configPage16));
}
#endif
