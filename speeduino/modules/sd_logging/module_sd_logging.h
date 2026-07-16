#pragma once

#include <stdint.h>

#include "modules/core/module_interfaces.h"

constexpr uint8_t progOutsPage = 13;

void module_sd_logging_init(const config13 &page13);
void module_sd_logging_tick_30hz(const config13 &page13);
void module_sd_logging_tick_10hz(const config13 &page13);
void module_sd_logging_tick_4hz(const config13 &page13);
void module_sd_logging_tick_1hz(const statuses &current, const config13 &page13);
