#pragma once

#include <stdint.h>

constexpr uint8_t wmiMapPage = 12;

void module_wmi_tick_30hz(void);
void wmiControl(void);
