#pragma once

#include <stdint.h>
#include "data/runtime_state.h"
#include "orchestration/schedule_calcs.h"
#include "model/tables.h"

constexpr uint16_t CRANK_RUN_HYSTER = 15;

uint16_t applyFuelTrimToPW(trimTable3d *pTrimTable, uint16_t fuelLoad, int16_t RPM, uint16_t currentPW);
uint16_t getInjectorAngle(uint16_t rpm_div100);
uint16_t getIdleTarget(uint16_t coolant);
uint8_t getRotarySplit(uint8_t ignLoad);
uint8_t getVE1(void);
int8_t getAdvance1(void);
void setFuelSchedules(const statuses &current, const uint16_t (&injectionStartAngles)[INJ_CHANNELS], uint16_t crankAngle, byte fuelChannelsOn);
void setIgnitionChannels(uint16_t crankAngle, uint16_t dwell, byte ignitionChannelsOn);
void calculateIgnitionAngles(uint16_t dwellAngle);
