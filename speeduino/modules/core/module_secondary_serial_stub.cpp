#include "modules/secondary_serial/module_secondary_serial.h"
#include "support/preprocessor.h"

#if !FEATURE_MODULE_SECONDARY_SERIAL

void module_secondary_serial_init(const secondary_serial_config_t &) {}
void module_secondary_serial_poll(const secondary_serial_config_t &) {}
void module_secondary_serial_tick_30hz(const secondary_serial_config_t &) {}

#endif
