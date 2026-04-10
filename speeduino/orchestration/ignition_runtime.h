#pragma once

#include <stdint.h>
#include "data/runtime_state.h"

uint8_t getRotarySplit(uint8_t ignLoad);
int8_t getAdvance1(void);
void setIgnitionChannels(uint16_t crankAngle, uint16_t dwell, byte ignitionChannelsOn);
void calculateIgnitionAngles(uint16_t dwellAngle);

