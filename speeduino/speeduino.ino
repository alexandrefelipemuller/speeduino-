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

#if 0
/** Calculate the Ignition angles for all cylinders (based on @ref config2.nCylinders).
 * both start and end angles are calculated for each channel.
 * Also the mode of ignition firing - wasted spark vs. dedicated spark per cyl. - is considered here.
 */
void calculateIgnitionAngles(uint16_t dwellAngle)
{
  //This test for more cylinders and do the same thing
  switch (configPage2.nCylinders)
  {
    //1 cylinder
    case 1:
      calculateIgnitionAngle(dwellAngle, channel1IgnDegrees, currentStatus.advance, &ignition1EndAngle, &ignition1StartAngle);
      break;
    //2 cylinders
    case 2:
      calculateIgnitionAngle(dwellAngle, channel1IgnDegrees, currentStatus.advance, &ignition1EndAngle, &ignition1StartAngle);
      calculateIgnitionAngle(dwellAngle, channel2IgnDegrees, currentStatus.advance, &ignition2EndAngle, &ignition2StartAngle);
      break;
    //3 cylinders
    case 3:
      calculateIgnitionAngle(dwellAngle, channel1IgnDegrees, currentStatus.advance, &ignition1EndAngle, &ignition1StartAngle);
      calculateIgnitionAngle(dwellAngle, channel2IgnDegrees, currentStatus.advance, &ignition2EndAngle, &ignition2StartAngle);
      calculateIgnitionAngle(dwellAngle, channel3IgnDegrees, currentStatus.advance, &ignition3EndAngle, &ignition3StartAngle);
      break;
    //4 cylinders
    case 4:
      calculateIgnitionAngle(dwellAngle, channel1IgnDegrees, currentStatus.advance, &ignition1EndAngle, &ignition1StartAngle);
      calculateIgnitionAngle(dwellAngle, channel2IgnDegrees, currentStatus.advance, &ignition2EndAngle, &ignition2StartAngle);

      #if IGN_CHANNELS >= 4
      if((configPage4.sparkMode == IGN_MODE_SEQUENTIAL) && currentStatus.decoder.getStatus().syncStatus==SyncStatus::Full)
      {
        if( CRANK_ANGLE_MAX_IGN != 720 ) { changeHalfToFullSync(configPage2, configPage4, currentStatus); }

        calculateIgnitionAngle(dwellAngle, channel3IgnDegrees, currentStatus.advance, &ignition3EndAngle, &ignition3StartAngle);
        calculateIgnitionAngle(dwellAngle, channel4IgnDegrees, currentStatus.advance, &ignition4EndAngle, &ignition4StartAngle);
      }
      else if(configPage4.sparkMode == IGN_MODE_ROTARY)
      {
        byte splitDegrees = 0;
        splitDegrees = table2D_getValue(&rotarySplitTable, (uint8_t)currentStatus.ignLoad);

        //The trailing angles are set relative to the leading ones
        calculateIgnitionTrailingRotary(dwellAngle, splitDegrees, ignition1EndAngle, &ignition3EndAngle, &ignition3StartAngle);
        calculateIgnitionTrailingRotary(dwellAngle, splitDegrees, ignition2EndAngle, &ignition4EndAngle, &ignition4StartAngle);
      }
      else
      {
        if( currentStatus.decoder.getStatus().syncStatus==SyncStatus::Partial && (CRANK_ANGLE_MAX_IGN != 360) ) { changeFullToHalfSync(configPage2, configPage4, currentStatus); }
      }
      #endif
      break;
    //5 cylinders
    case 5:
      calculateIgnitionAngle(dwellAngle, channel1IgnDegrees, currentStatus.advance, &ignition1EndAngle, &ignition1StartAngle);
      calculateIgnitionAngle(dwellAngle, channel2IgnDegrees, currentStatus.advance, &ignition2EndAngle, &ignition2StartAngle);
      calculateIgnitionAngle(dwellAngle, channel3IgnDegrees, currentStatus.advance, &ignition3EndAngle, &ignition3StartAngle);
      calculateIgnitionAngle(dwellAngle, channel4IgnDegrees, currentStatus.advance, &ignition4EndAngle, &ignition4StartAngle);
      #if (IGN_CHANNELS >= 5)
      calculateIgnitionAngle(dwellAngle, channel5IgnDegrees, currentStatus.advance, &ignition5EndAngle, &ignition5StartAngle);
      #endif
      break;
    //6 cylinders
    case 6:
      calculateIgnitionAngle(dwellAngle, channel1IgnDegrees, currentStatus.advance, &ignition1EndAngle, &ignition1StartAngle);
      calculateIgnitionAngle(dwellAngle, channel2IgnDegrees, currentStatus.advance, &ignition2EndAngle, &ignition2StartAngle);
      calculateIgnitionAngle(dwellAngle, channel3IgnDegrees, currentStatus.advance, &ignition3EndAngle, &ignition3StartAngle);

      #if IGN_CHANNELS >= 6
      if((configPage4.sparkMode == IGN_MODE_SEQUENTIAL) && currentStatus.decoder.getStatus().syncStatus==SyncStatus::Full)
      {
        if( CRANK_ANGLE_MAX_IGN != 720 ) { changeHalfToFullSync(configPage2, configPage4, currentStatus); }

        calculateIgnitionAngle(dwellAngle, channel4IgnDegrees, currentStatus.advance, &ignition4EndAngle, &ignition4StartAngle);
        calculateIgnitionAngle(dwellAngle, channel5IgnDegrees, currentStatus.advance, &ignition5EndAngle, &ignition5StartAngle);
        calculateIgnitionAngle(dwellAngle, channel6IgnDegrees, currentStatus.advance, &ignition6EndAngle, &ignition6StartAngle);
      }
      else
      {
        if( currentStatus.decoder.getStatus().syncStatus==SyncStatus::Partial && (CRANK_ANGLE_MAX_IGN != 360) ) { changeFullToHalfSync(configPage2, configPage4, currentStatus); }
      }
      #endif
      break;
    //8 cylinders
    case 8:
      calculateIgnitionAngle(dwellAngle, channel1IgnDegrees, currentStatus.advance, &ignition1EndAngle, &ignition1StartAngle);
      calculateIgnitionAngle(dwellAngle, channel2IgnDegrees, currentStatus.advance, &ignition2EndAngle, &ignition2StartAngle);
      calculateIgnitionAngle(dwellAngle, channel3IgnDegrees, currentStatus.advance, &ignition3EndAngle, &ignition3StartAngle);
      calculateIgnitionAngle(dwellAngle, channel4IgnDegrees, currentStatus.advance, &ignition4EndAngle, &ignition4StartAngle);

      #if IGN_CHANNELS >= 8
      if((configPage4.sparkMode == IGN_MODE_SEQUENTIAL) && currentStatus.decoder.getStatus().syncStatus==SyncStatus::Full)
      {
        if( CRANK_ANGLE_MAX_IGN != 720 ) { changeHalfToFullSync(configPage2, configPage4, currentStatus); }

        calculateIgnitionAngle(dwellAngle, channel5IgnDegrees, currentStatus.advance, &ignition5EndAngle, &ignition5StartAngle);
        calculateIgnitionAngle(dwellAngle, channel6IgnDegrees, currentStatus.advance, &ignition6EndAngle, &ignition6StartAngle);
        calculateIgnitionAngle(dwellAngle, channel7IgnDegrees, currentStatus.advance, &ignition7EndAngle, &ignition7StartAngle);
        calculateIgnitionAngle(dwellAngle, channel8IgnDegrees, currentStatus.advance, &ignition8EndAngle, &ignition8StartAngle);
      }
      else
      {
        if( currentStatus.decoder.getStatus().syncStatus==SyncStatus::Partial && (CRANK_ANGLE_MAX_IGN != 360) ) { changeFullToHalfSync(configPage2, configPage4, currentStatus); }
      }
      #endif
      break;

    //Will hit the default case on >8 cylinders. Do nothing in these cases
    default:
      break;
  }
}
#endif
