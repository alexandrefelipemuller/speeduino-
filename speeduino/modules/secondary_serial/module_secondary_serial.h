#pragma once

#include "config9_domains.h"

void module_secondary_serial_init(const secondary_serial_config_t &config);
void module_secondary_serial_poll(const secondary_serial_config_t &config);
void module_secondary_serial_tick_30hz(const secondary_serial_config_t &config);
