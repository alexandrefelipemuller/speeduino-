#pragma once

#include <stdint.h>
#include "data/runtime_state.h"
#include "model/tables.h"

uint16_t applyFuelTrimToPW(trimTable3d *pTrimTable, uint16_t fuelLoad, int16_t RPM, uint16_t currentPW);
uint16_t getInjectorAngle(uint16_t rpm_div100);
uint16_t getIdleTarget(uint16_t coolant);
uint8_t getVE1(void);
void setFuelSchedules(const statuses &current, const uint16_t (&injectionStartAngles)[INJ_CHANNELS], uint16_t crankAngle, byte fuelChannelsOn);
