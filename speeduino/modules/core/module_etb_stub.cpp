#include "support/preprocessor.h"

#include "modules/etb/etb.h"

#if !FEATURE_MODULE_ETB

void module_etb_init_post_pin_mapping(void) {}
void module_etb_tick_200hz(void) {}
void module_etb_on_engine_stop(void) {}

module_page_maps_t getEtbPageMaps() { return { nullptr, 0U }; }
module_page_descriptors_t getEtbPageDescriptors() { return { nullptr, 0U }; }
module_storage_maps_t getEtbStorageMaps() { return { nullptr, 0U }; }

#endif
