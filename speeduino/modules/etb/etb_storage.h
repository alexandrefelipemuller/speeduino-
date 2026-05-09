#pragma once

#include <stdint.h>

#include "support/preprocessor.h"

static constexpr uint16_t EEPROM_CONFIG16_START = 3537U;

#if FEATURE_MODULE_ETB
void module_etb_save_pages(uint16_t &writes_remaining);
void module_etb_load_pages(void);
#else
inline void module_etb_save_pages(uint16_t &) {}
inline void module_etb_load_pages(void) {}
#endif
