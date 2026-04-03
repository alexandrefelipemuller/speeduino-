#pragma once

#include "statuses.h"
#include "config_pages.h"

void module_logging_init(const config13 &page13);
void module_logging_tick_30hz(const config13 &page13);
void module_logging_tick_10hz(const config13 &page13);
void module_logging_tick_4hz(const config13 &page13);
void module_logging_tick_1hz(const statuses &current, const config13 &page13);
