#include "modules/logging/module_logging.h"
#include "support/preprocessor.h"

#if !FEATURE_MODULE_LOGGING

void module_logging_init(const config13 &) {}
void module_logging_tick_30hz(const config13 &) {}
void module_logging_tick_10hz(const config13 &) {}
void module_logging_tick_4hz(const config13 &) {}
void module_logging_tick_1hz(const statuses &, const config13 &) {}

#endif
