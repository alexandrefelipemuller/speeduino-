#include "modules/comms_extended/module_comms_extended.h"
#include "support/preprocessor.h"

#if !FEATURE_MODULE_COMMS_EXTENDED

void module_comms_extended_init(void) {}
void module_comms_extended_poll(uint8_t, uint8_t) {}
void module_comms_extended_tick_50hz(void) {}
void module_comms_extended_tick_30hz(void) {}
void module_comms_extended_tick_15hz(void) {}
void module_comms_extended_tick_10hz(void) {}
void module_comms_extended_tick_4hz(uint8_t, statuses &, const can_extended_config_t &) {}

#endif
