#include <FastCRC.h>
#include "storage/page_crc.h"
#include "storage/pages.h"

uint32_t __attribute__((optimize("Os"))) calculatePageCRC32(uint8_t pageNum)
{
    const uint16_t pageSize = getPageSize(pageNum);
    if (pageSize == 0U)
    {
        return 0U;
    }

    FastCRC32 crcCalc;

    byte buffer = getPageValue(pageNum, 0);
    uint32_t crc = crcCalc.crc32(&buffer, 1U);

    for (uint16_t offset=1; offset<pageSize; ++offset)
    {
        buffer = getPageValue(pageNum, offset);
        crc = crcCalc.crc32_upd(&buffer, 1U);
    }

    return crc;
}
