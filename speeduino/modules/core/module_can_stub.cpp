#include "support/preprocessor.h"

#include "modules/can/module_can.h"

#if !FEATURE_MODULE_CAN

void module_can_init(void) {}
void module_can_poll(uint8_t, uint8_t) {}
void module_can_tick_50hz(void) {}
void module_can_tick_30hz(void) {}
void module_can_tick_15hz(void) {}
void module_can_tick_10hz(void) {}
void module_can_tick_4hz(uint8_t, statuses &, const can_extended_config_t &) {}
void module_can_on_engine_stop(void) {}
module_page_maps_t getCanPageMaps() { return { nullptr, 0U }; }
module_page_descriptors_t getCanPageDescriptors() { return { nullptr, 0U }; }
module_storage_maps_t getCanStorageMaps() { return { nullptr, 0U }; }

#endif
