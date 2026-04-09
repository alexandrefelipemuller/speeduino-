#pragma once

#include "data/core_constants.h"
#include "data/runtime_state.h"

#define CRANK_ANGLE_MAX (max(CRANK_ANGLE_MAX_IGN, CRANK_ANGLE_MAX_INJ))

#define SERIAL_PORT_PRIMARY   0
#define SERIAL_PORT_SECONDARY 3

// note the sequence of these defines which reference the bits used in a byte has moved when the third trigger & engine cycle was incorporated
#define COMPOSITE_LOG_PRI   0
#define COMPOSITE_LOG_SEC   1
#define COMPOSITE_LOG_THIRD 2
#define COMPOSITE_LOG_TRIG 3
#define COMPOSITE_LOG_SYNC 4
#define COMPOSITE_ENGINE_CYCLE 5

#define OUTPUT_CONTROL_DIRECT   0
#define OUTPUT_CONTROL_MC33810  10

#define LOGGER_CSV_SEPARATOR_SEMICOLON  0
#define LOGGER_CSV_SEPARATOR_COMMA      1
#define LOGGER_CSV_SEPARATOR_TAB        2
#define LOGGER_CSV_SEPARATOR_SPACE      3

#define LOGGER_DISABLED                 0
#define LOGGER_CSV                      1
#define LOGGER_BINARY                   2

#define LOGGER_RATE_1HZ                 0
#define LOGGER_RATE_4HZ                 1
#define LOGGER_RATE_10HZ                2
#define LOGGER_RATE_30HZ                3

#define LOGGER_FILENAMING_OVERWRITE     0
#define LOGGER_FILENAMING_DATETIME      1
#define LOGGER_FILENAMING_SEQENTIAL     2
