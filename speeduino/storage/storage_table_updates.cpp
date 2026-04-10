/** @file
 * Helpers for update-time table scaling.
 */

#include "storage/updates.h"
#include "storage/pages.h"

void multiplyTableLoad(table3d_t *pTable, TableType key, uint8_t multiplier)
{
  auto y_it = y_begin(pTable, key);
  while(!y_it.at_end())
  {
    *y_it = *y_it * multiplier;
    ++y_it;
  }
}

void divideTableLoad(table3d_t *pTable, TableType key, uint8_t divisor)
{
  auto y_it = y_begin(pTable, key);
  while(!y_it.at_end())
  {
    *y_it = *y_it / divisor; //Previous TS scale was 2.0, now is 0.5, 4x increase
    ++y_it;
  }
}

void multiplyTableValue(uint8_t pageNum, uint8_t multiplier)
{
  uint16_t count = getPageSize(pageNum);
  for (uint16_t i = 0; i < count; i++)
  {
    setPageValue(pageNum, i, (uint8_t)(getPageValue(pageNum, i) * multiplier));
  }
}

void divideTableValue(uint8_t pageNum, uint8_t divisor)
{
  uint16_t count = getPageSize(pageNum);
  for (uint16_t i = 0; i < count; i++)
  {
    setPageValue(pageNum, i, (uint8_t)(getPageValue(pageNum, i) / divisor));
  }
}
