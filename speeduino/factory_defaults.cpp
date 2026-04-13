#include "globals.h"
#include "factory_defaults.h"
#include "page_crc.h"
#include "pages.h"
#include "storage.h"
#include EEPROM_LIB_H

#include "generated/factory_defaults_data.h"

namespace {

constexpr uint8_t FACTORY_DATA_VERSION = 24U;

struct FactoryPageBlob
{
  eeprom_address_t eepromAddress;
  const uint8_t *data;
  uint16_t size;
};

static byte readFactoryByte(const uint8_t *data, uint16_t offset)
{
#if defined(CORE_AVR)
  return pgm_read_byte(data + offset);
#else
  return data[offset];
#endif
}

static uint16_t readFactoryWord(const uint16_t *data, uint16_t offset)
{
#if defined(CORE_AVR)
  return pgm_read_word(data + offset);
#else
  return data[offset];
#endif
}

template <typename T>
static void copyFactoryArray(T *destination, const T *source, uint16_t length);

template <>
void copyFactoryArray<uint16_t>(uint16_t *destination, const uint16_t *source, uint16_t length)
{
  for (uint16_t index = 0U; index < length; ++index)
  {
    destination[index] = readFactoryWord(source, index);
  }
}

template <>
void copyFactoryArray<uint8_t>(uint8_t *destination, const uint8_t *source, uint16_t length)
{
  for (uint16_t index = 0U; index < length; ++index)
  {
    destination[index] = readFactoryByte(source, index);
  }
}

static void writeFactoryBlob(eeprom_address_t address, const uint8_t *data, uint16_t size)
{
  for (uint16_t offset = 0U; offset < size; ++offset)
  {
    EEPROM.update(address + offset, readFactoryByte(data, offset));
  }
}

static uint32_t computeCalibrationCrc(const void *data, uint16_t size)
{
  FastCRC32 crcCalc;
  return crcCalc.crc32(static_cast<const uint8_t *>(data), size, false);
}

static uint32_t computeFactoryTemperatureCalibrationCrc(const uint16_t *values)
{
  uint8_t payload[64];

  for (uint8_t index = 0U; index < 32U; ++index)
  {
    const int16_t temperatureC = static_cast<int16_t>(values[index]) - CALIBRATION_TEMPERATURE_OFFSET;
    const int16_t temperatureF10 = static_cast<int16_t>(((temperatureC * 9) / 5 + 32) * 10);
    payload[(index * 2U)] = static_cast<uint8_t>(temperatureF10 & 0xFF);
    payload[(index * 2U) + 1U] = static_cast<uint8_t>((temperatureF10 >> 8) & 0xFF);
  }

  return computeCalibrationCrc(payload, sizeof(payload));
}

static uint32_t computeFactoryO2CalibrationCrc(void)
{
  uint8_t payload[1024];

  for (uint16_t adc = 0U; adc < 1024U; ++adc)
  {
    const float voltage = static_cast<float>(adc) * 5.0F / 1023.0F;
    const float afr = 9.7F + (voltage - 1.0F) * ((18.7F - 9.7F) / (4.0F - 1.0F));
    int16_t afr10 = static_cast<int16_t>(afr * 10.0F + 0.5F);
    if (afr10 < 0)
    {
      afr10 = 0;
    }
    else if (afr10 > 255)
    {
      afr10 = 255;
    }
    payload[adc] = static_cast<uint8_t>(afr10);
  }

  return computeCalibrationCrc(payload, sizeof(payload));
}

static void seedFactoryCalibrationDefaults(void)
{
  copyFactoryArray(cltCalibration_bins, factory_clt_bins, sizeof(cltCalibration_bins) / sizeof(cltCalibration_bins[0]));
  copyFactoryArray(cltCalibration_values, factory_clt_values, sizeof(cltCalibration_values) / sizeof(cltCalibration_values[0]));
  copyFactoryArray(iatCalibration_bins, factory_iat_bins, sizeof(iatCalibration_bins) / sizeof(iatCalibration_bins[0]));
  copyFactoryArray(iatCalibration_values, factory_iat_values, sizeof(iatCalibration_values) / sizeof(iatCalibration_values[0]));
  copyFactoryArray(o2Calibration_bins, factory_o2_bins, sizeof(o2Calibration_bins) / sizeof(o2Calibration_bins[0]));
  copyFactoryArray(o2Calibration_values, factory_o2_values, sizeof(o2Calibration_values) / sizeof(o2Calibration_values[0]));

  writeCalibration();

  storeCalibrationCRC32(CLT_CALIBRATION_PAGE, computeFactoryTemperatureCalibrationCrc(cltCalibration_values));
  storeCalibrationCRC32(IAT_CALIBRATION_PAGE, computeFactoryTemperatureCalibrationCrc(iatCalibration_values));
  storeCalibrationCRC32(O2_CALIBRATION_PAGE, computeFactoryO2CalibrationCrc());
}

constexpr FactoryPageBlob FACTORY_PAGE_BLOBS[] = {
  {0U, nullptr, 0U},
  {EEPROM_CONFIG2_START, factory_page_1, sizeof(factory_page_1)},
  {EEPROM_CONFIG1_MAP, factory_page_2, sizeof(factory_page_2)},
  {EEPROM_CONFIG3_MAP, factory_page_3, sizeof(factory_page_3)},
  {EEPROM_CONFIG4_START, factory_page_4, sizeof(factory_page_4)},
  {EEPROM_CONFIG5_MAP, factory_page_5, sizeof(factory_page_5)},
  {EEPROM_CONFIG6_START, factory_page_6, sizeof(factory_page_6)},
  {EEPROM_CONFIG7_MAP1, factory_page_7, sizeof(factory_page_7)},
  {EEPROM_CONFIG8_MAP1, factory_page_8, sizeof(factory_page_8)},
  {EEPROM_CONFIG9_START, factory_page_9, sizeof(factory_page_9)},
  {EEPROM_CONFIG10_START, factory_page_10, sizeof(factory_page_10)},
  {EEPROM_CONFIG11_MAP, factory_page_11, sizeof(factory_page_11)},
  {EEPROM_CONFIG12_MAP, factory_page_12, sizeof(factory_page_12)},
  {EEPROM_CONFIG13_START, factory_page_13, sizeof(factory_page_13)},
  {EEPROM_CONFIG14_MAP, factory_page_14, sizeof(factory_page_14)},
  {EEPROM_CONFIG15_MAP, factory_page_15, sizeof(factory_page_15)},
};

static_assert((sizeof(FACTORY_PAGE_BLOBS) / sizeof(FACTORY_PAGE_BLOBS[0])) == 16U, "Factory page mapping must cover all logical pages");

static bool validateFactoryPageBlobSizes(void)
{
  for (uint8_t page = 1U; page < getPageCount(); ++page)
  {
    if (FACTORY_PAGE_BLOBS[page].size != getPageSize(page))
    {
      return false;
    }
  }

  return true;
}

} // namespace

bool seedFactoryDefaultsIfBlank(void)
{
  const uint8_t version = readEEPROMVersion();
  if ((version != 0U) && (version != 255U))
  {
    return false;
  }

  if (!validateFactoryPageBlobSizes())
  {
    return false;
  }

  for (uint8_t page = 1U; page < getPageCount(); ++page)
  {
    const FactoryPageBlob &blob = FACTORY_PAGE_BLOBS[page];
    writeFactoryBlob(blob.eepromAddress, blob.data, blob.size);
  }

  seedFactoryCalibrationDefaults();

  storeEEPROMVersion(FACTORY_DATA_VERSION);
  return true;
}

void storeFactoryPageCRCs(void)
{
  for (uint8_t page = 1U; page < getPageCount(); ++page)
  {
    storePageCRC32(page, calculatePageCRC32(page));
  }
}
