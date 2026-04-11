#pragma once

#include "data/config_pages.h"
#include "data/statuses.h"

bool checkEngineProtect(statuses &current, const config4 &page4, const config6 &page6, const config9 &page9, const config10 &page10);

uint8_t checkRevLimit(statuses &current, const config4 &page4, const config6 &page6, const config9 &page9);

statuses::scheduler_cut_t calculateFuelIgnitionChannelCut(statuses &current, const config2 &page2, const config4 &page4, const config6 &page6, const config9 &page9, const config10 &page10);
