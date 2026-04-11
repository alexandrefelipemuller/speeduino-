/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,la
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
/** @file
 * Speeduino initialisation and main loop.
 */
#include <stdint.h> //developer.mbed.org/handbook/C-Data-Types
//************************************************
#include "data/advanced_engine_status.h"
#include "data/runtime_constants.h"
#include "data/runtime_state.h"
#include "data/tune_registry.h"
#include "data/pin_registry.h"
#include "support/hw_test_bits.h"
#include "data/logger_status.h"
#include "orchestration/scheduler.h"
#include "comms/comms.h"
#include "comms/comms_legacy.h"
#include "modules/secondary_serial/secondary_serial.h"
#include "support/maths.h"
#include "engine/corrections.h"
#include "orchestration/timers.h"
#include "engine/decoders.h"
#include "engine/idle.h"
#include "engine/auxiliaries.h"
#include "engine/sensors.h"
#include "storage/storage.h"
#include "engine/crankMaths.h"
#include "orchestration/init.h"
#include "support/utilities.h"
#include "modules/engine_protection/engine_protection.h"
#include "orchestration/schedule_calcs.h"
#include "engine/auxiliaries.h"
#include "engine/load_source.h"
#include "boards/board_definition.h"
#include "support/unit_testing.h"
#include RTC_LIB_H //Defined in each boards .h file
#include "support/units.h"
#include "engine/fuel_calcs.h"
#include "support/preprocessor.h"
#include "engine/dwell.h"
#include "engine/decoder_init.h"
#include "orchestration/loop_helpers.h"
#include "orchestration/sync_runtime.h"
#include "orchestration/engine_runtime.h"
#include "orchestration/main_loop.h"
#include "modules/core/module_interfaces.h"
#include "modules/core/module_runtime.h"

#ifndef UNIT_TEST // Scope guard for unit testing

void setup(void)
{
  currentStatus.initialisationComplete = false; //Tracks whether the initialiseAll() function has run completely
  initialiseAll();
}

BEGIN_LTO_ALWAYS_INLINE(void) loop(void)
{
  runMainLoopIteration();
} //loop()
END_LTO_INLINE()

#endif //Unit test guard
