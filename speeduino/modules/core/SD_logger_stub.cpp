#include "modules/sd_logging/SD_logger.h"

#include "support/preprocessor.h"

#if defined(SD_LOGGING) && !FEATURE_MODULE_LOGGING

uint8_t SD_status = SD_STATUS_OFF;
uint16_t currentLogFileNumber = 0U;
bool manualLogActive = false;
elapsedMillis msSinceLastSDSync;

void initSD() {}
void writeSDLogEntry() {}
void writetSDLogHeader() {}
void beginSDLogging() {}
void endSDLogging() {}
bool syncSDLog() { return false; }
void setTS_SD_status() {}
void formatExFat() {}
void deleteLogFile(char, char, char, char) {}
bool createLogFile() { return false; }
void dateTime(uint16_t*, uint16_t*, uint8_t*) {}
uint16_t getNextSDLogFileNumber() { return 0U; }
bool getSDLogFileDetails(uint8_t*, uint16_t) { return false; }
void readSDSectors(uint8_t*, uint32_t, uint16_t) {}
uint32_t sectorCount() { return 0UL; }

#endif
