#pragma once

#include <stdint.h>

#include "support/preprocessor.h"

#if FEATURE_MODULE_WMI
void module_wmi_save_pages(uint16_t &writes_remaining);
void module_wmi_load_pages(void);
void module_wmi_upgrade_v22(void);
#else
static inline void module_wmi_save_pages(uint16_t &) {}
static inline void module_wmi_load_pages(void) {}
static inline void module_wmi_upgrade_v22(void) {}
#endif
