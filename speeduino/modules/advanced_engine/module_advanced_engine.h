#pragma once

#include "statuses.h"
#include "config_pages.h"

void module_advanced_engine_on_engine_stop(const config4 &page4);
void module_advanced_engine_tick_30hz(void);
void module_advanced_engine_tick_15hz(void);
void module_advanced_engine_tick_10hz(void);
void module_advanced_engine_tick_4hz(void);
statuses::scheduler_cut_t module_advanced_engine_scheduler_cut(statuses &current, const config2 &page2, const config4 &page4, const config6 &page6, const config9 &page9, const config10 &page10);
