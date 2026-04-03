#include "modules/logging/logger_controls.h"
#include "preprocessor.h"

#if !FEATURE_MODULE_LOGGING
void startToothLogger(void) {}
void stopToothLogger(void) {}

void startCompositeLogger(void) {}
void stopCompositeLogger(void) {}

void startCompositeLoggerTertiary(void) {}
void stopCompositeLoggerTertiary(void) {}

void startCompositeLoggerCams(void) {}
void stopCompositeLoggerCams(void) {}
#endif
