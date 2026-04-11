#include "modules/knock/module_knock.h"

#include "support/preprocessor.h"

#if !FEATURE_MODULE_KNOCK

void module_knock_init_post_pin_mapping(void) {}
void module_knock_tick_10hz(void) {}
void module_knock_on_engine_stop(void) {}

module_page_maps_t getKnockPageMaps()
{
  return { nullptr, 0U };
}

module_page_descriptors_t getKnockPageDescriptors()
{
  return { nullptr, 0U };
}

module_storage_maps_t getKnockStorageMaps()
{
  return { nullptr, 0U };
}

#endif
