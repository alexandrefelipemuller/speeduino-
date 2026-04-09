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
#include "modules/advanced_engine/engineProtection.h"
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
#include "modules/core/module_interfaces.h"
#include "modules/core/module_runtime.h"

#ifndef UNIT_TEST // Scope guard for unit testing

void setup(void)
{
  currentStatus.initialisationComplete = false; //Tracks whether the initialiseAll() function has run completely
  initialiseAll();
}

/** Speeduino main loop.
 * 
 * Main loop chores (roughly in the order that they are performed):
 * - Check if serial comms or tooth logging are in progress (send or receive, prioritise communication)
 * - Record loop timing vars
 * - Check tooth time, update @ref statuses (currentStatus) variables
 * - Read sensors
 * - get VE for fuel calcs and spark advance for ignition
 * - Check crank/cam/tooth/timing sync (skip remaining ops if out-of-sync)
 * - execute doCrankSpeedCalcs()
 * 
 * single byte variable @ref LOOP_TIMER plays a big part here as:
 * - it contains expire-bits for interval based frequency driven events (e.g. 15Hz, 4Hz, 1Hz)
 * - Can be tested for certain frequency interval being expired by (eg) BIT_CHECK(LOOP_TIMER, BIT_TIMER_15HZ)
 * 
 * Sometimes loop() is inlined by LTO & sometimes not
 * When not inlined, there is a huge difference in stack usage: 60+ bytes
 * That eats into available RAM.
 * Adding __attribute__((always_inline)) forces the LTO process to inline.
 *
 * Since the function is declared in an Arduino header, we can't change
 * it to inline, so we need to suppress the resulting warning.
 */
BEGIN_LTO_ALWAYS_INLINE(void) loop(void)
{
      if(mainLoopCount < UINT16_MAX) { mainLoopCount++; }
      LOOP_TIMER = TIMER_mask;

      //SERIAL Comms
      //Initially check that the last serial send values request is not still outstanding
      if (serialTransmitInProgress())
      {
        serialTransmit();
      }

      //Check for any new or in-progress requests from serial.
      if( (Serial.available() > 0) || serialRecieveInProgress() )
      {
        serialReceive();
      }
      
      // Check for any secondary comms requiring action.
      core_modules_poll();
          
    if(currentLoopTime > micros())
    {
      //Occurs when micros() has overflowed
      setStorageWriteTimeout(0); //Required to ensure that EEPROM writes are not deferred indefinitely
    }

    currentLoopTime = micros();
    if ( currentStatus.decoder.isEngineRunning(currentLoopTime) )
    {
      setRpm(currentStatus, currentStatus.decoder.getRPM());
      if( (currentStatus.RPM > 0) && (currentStatus.fuelPumpOn == false) )
      {
        fuelPumpOn();
      }
    }
    else
    {
      //We reach here if the time between teeth is too great. This VERY likely means the engine has stopped
      setRpm(currentStatus, 0);
      currentStatus.PW1 = 0;
      currentStatus.VE = 0;
      currentStatus.VE2 = 0;
      currentStatus.decoder.reset();
      currentStatus.runSecs = 0; //Reset the counter for number of seconds running.
      currentStatus.startRevolutions = 0;
      resetMAPcycleAndEvent();
      currentStatus.rpmDOT = 0;
      initialiseCorrections();
      ignitionCount = 0;
      if (currentStatus.fpPrimed == true) { fuelPumpOff(); } //Turn off the fuel pump, but only if the priming is complete
      if (configPage6.iacPWMrun == false) { disableIdle(); } //Turn off the idle PWM
      currentStatus.engineIsCranking = false; //Clear cranking bit (Can otherwise get stuck 'on' even with 0 rpm)
      currentStatus.wueIsActive = false; //Same as above except for WUE
      currentStatus.engineIsRunning = false; //Same as above except for RUNNING status
      currentStatus.aseIsActive = false; //Same as above except for ASE status
      currentStatus.isAcceleratingTPS = false; //Same as above but the accel enrich (If using MAP accel enrich a stall will cause this to trigger)
      currentStatus.isDeceleratingTPS = false; //Same as above but the decel enleanment
      //This is a safety check. If for some reason the interrupts have got screwed up (Leading to 0rpm), this resets them.
      //It can possibly be run much less frequently.
      //This should only be run if the high speed logger are off because it will change the trigger interrupts back to defaults rather than the logger versions
      if( (currentLoggerStatus.tooth_log_enabled == false) && (currentLoggerStatus.composite_trigger_used == 0U) ) {
        currentStatus.decoder = buildDecoder(configPage4.TrigPattern);
      }

      core_modules_on_engine_stop();
    }
    //***Perform sensor reads***
    //-----------------------------------------------------------------------------------------------------
    readPolledSensors(LOOP_TIMER);

    if (BIT_CHECK(LOOP_TIMER, BIT_TIMER_1KHZ)) //Every 1ms. NOTE: This is NOT guaranteed to run at 1kHz on AVR systems. It will run at 1kHz if possible or as fast as loops/s allows if not. 
    {
      BIT_CLEAR(TIMER_mask, BIT_TIMER_1KHZ);
    }
    if(BIT_CHECK(LOOP_TIMER, BIT_TIMER_200HZ))
    {
      BIT_CLEAR(TIMER_mask, BIT_TIMER_200HZ);
    }
    if(BIT_CHECK(LOOP_TIMER, BIT_TIMER_50HZ)) //50 hertz
    {
      BIT_CLEAR(TIMER_mask, BIT_TIMER_50HZ);

      core_modules_tick_50hz();
    }
    if(BIT_CHECK(LOOP_TIMER, BIT_TIMER_30HZ)) //30 hertz
    {
      BIT_CLEAR(TIMER_mask, BIT_TIMER_30HZ);
      core_modules_tick_30hz();

      //Check for any outstanding EEPROM writes.
      if( (isEepromWritePending() == true) && (serialStatusFlag == SERIAL_INACTIVE) && storageWriteTimeoutExpired()) { saveAllPages(); } 
    }
    if (BIT_CHECK(LOOP_TIMER, BIT_TIMER_15HZ)) //Every 32 loops
    {
      BIT_CLEAR(TIMER_mask, BIT_TIMER_15HZ);
      #if  defined(CORE_TEENSY35)       
          if (configPage9.enable_intcan == 1) // use internal can module
          {
           // this is just to test the interface is sending
           //sendCancommand(3,((configPage9.realtime_base_address & 0x3FF)+ 0x100),currentStatus.TPS,0,0x200);
          }
      #endif     

      core_modules_tick_15hz();

      //And check whether the tooth log buffer is ready
      if(toothHistoryIndex > TOOTH_LOG_SIZE) { currentLoggerStatus.is_tooth_log_1_full = true; }
    }
    if(BIT_CHECK(LOOP_TIMER, BIT_TIMER_10HZ)) //10 hertz
    {
      BIT_CLEAR(TIMER_mask, BIT_TIMER_10HZ);
      //updateFullStatus();
      idleControl(); //Perform any idle related actions. This needs to be run at 10Hz to align with the idle taper resolution of 0.1s
      core_modules_tick_10hz();
    }
    if (BIT_CHECK(LOOP_TIMER, BIT_TIMER_4HZ))
    {
      BIT_CLEAR(TIMER_mask, BIT_TIMER_4HZ);
      //Lookup the current target idle RPM. This is aligned with coolant and so needs to be calculated at the same rate CLT is read
      if( (configPage2.idleAdvEnabled != IDLEADVANCE_MODE_OFF) || (configPage6.iacAlgorithm != IAC_ALGORITHM_NONE) )
      {
        currentStatus.CLIdleTarget = getIdleTarget(currentStatus.coolant); //All temps are offset by 40 degrees
        if(currentAdvancedEngineStatus.aircon_turning_on) { currentStatus.CLIdleTarget += configPage15.airConIdleUpRPMAdder;  } //Adds Idle Up RPM amount if active
      }

      core_modules_tick_4hz(statusSensors, currentStatus);
    } //4Hz timer
    if (BIT_CHECK(LOOP_TIMER, BIT_TIMER_1HZ)) //Once per second)
    {
      BIT_CLEAR(TIMER_mask, BIT_TIMER_1HZ);
      currentStatus.systemTemp = getSystemTemp();

      if ( (configPage10.wmiEnabled > 0) && (configPage10.wmiIndicatorEnabled > 0) )
      {
        // water tank empty
        if (currentAdvancedEngineStatus.wmi_tank_empty)
        {
          // flash with 1sec interval
          digitalWrite(pinWMIIndicator, !digitalRead(pinWMIIndicator));
        }
        else
        {
          digitalWrite(pinWMIIndicator, configPage10.wmiIndicatorPolarity ? HIGH : LOW);
        } 
      }

      core_modules_tick_1hz(currentStatus);

    } //1Hz timer

    if( (configPage6.iacAlgorithm == IAC_ALGORITHM_STEP_OL)
    || (configPage6.iacAlgorithm == IAC_ALGORITHM_STEP_CL)
    || (configPage6.iacAlgorithm == IAC_ALGORITHM_STEP_OLCL) )
    {
      idleControl(); //Run idlecontrol every loop for stepper idle.
    }

    //VE and advance calculation were moved outside the sync/RPM check so that the fuel and ignition load value will be accurately shown when RPM=0
    currentStatus.VE1 = getVE1();
    currentStatus.VE = currentStatus.VE1; //Set the final VE value to be VE 1 as a default. This may be changed in the section below

    currentStatus.advance1 = getAdvance1();
    currentStatus.advance = currentStatus.advance1; //Set the final advance value to be advance 1 as a default. This may be changed in the section below

    core_modules_apply_table_switching(currentStatus);

    //Always check for sync
    //Main loop runs within this clause
    if ((currentStatus.decoder.getStatus().syncStatus!=SyncStatus::None) && (currentStatus.RPM > 0))
    {
        //Check whether running or cranking
        if(currentStatus.RPM > currentStatus.crankRPM) //Crank RPM in the config is stored as a x10. currentStatus.crankRPM is set in timers.ino and represents the true value
        {
          currentStatus.engineIsRunning = true; //Sets the engine running bit
          //Only need to do anything if we're transitioning from cranking to running
          if( currentStatus.engineIsCranking )
          {
            currentStatus.engineIsCranking = false;
            if(configPage4.ignBypassEnabled > 0) { digitalWrite(pinIgnBypass, HIGH); }
          }
        }
        else
        {  
          if( !currentStatus.engineIsRunning || (currentStatus.RPM < (currentStatus.crankRPM - CRANK_RUN_HYSTER)) )
          {
            //Sets the engine cranking bit, clears the engine running bit
            currentStatus.engineIsCranking = true;
            currentStatus.engineIsRunning = false;
            currentStatus.runSecs = 0; //We're cranking (hopefully), so reset the engine run time to prompt ASE.
            if(configPage4.ignBypassEnabled > 0) { digitalWrite(pinIgnBypass, LOW); }

            //Check whether the user has selected to disable to the fan during cranking
            if(configPage2.fanWhenCranking == 0) { fanOff(); }
          }
        }
      //END SETTING ENGINE STATUSES
      //-----------------------------------------------------------------------------------------------------

      //Begin the fuel calculation
      
      //Check that the duty cycle of the chosen pulsewidth isn't too high.
      //Calculate an injector pulsewidth from the VE
      currentStatus.afrTarget = calculateAfrTarget(afrTable, currentStatus, configPage2, configPage6);
      currentStatus.corrections = correctionsFuel();

      pulseWidths pulse_widths = computePulseWidths(
                                    configPage2,
                                    configPage6,
                                    configPage10, 
                                    currentStatus.decoder.getStatus(),
                                    currentStatus);
      currentStatus.stagingActive = pulse_widths.secondary!=0U;
      applyPwToInjectorChannels(pulse_widths, configPage2, currentStatus);

      //***********************************************************************************************
      //BEGIN INJECTION TIMING
      currentStatus.injAngle = getInjectorAngle(currentStatus.RPMdiv100);
      if(currentStatus.injAngle > uint16_t(CRANK_ANGLE_MAX_INJ)) { currentStatus.injAngle = uint16_t(CRANK_ANGLE_MAX_INJ); }

      unsigned int PWdivTimerPerDegree = timeToAngleDegPerMicroSec(currentStatus.PW1); //How many crank degrees the calculated PW will take at the current speed

      uint16_t injectionStartAngles[INJ_CHANNELS];
      injectionStartAngles[0] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel1InjDegrees, currentStatus.injAngle);

      //Repeat the above for each cylinder
      switch (configPage2.nCylinders)
      {
        //Single cylinder
        case 1:
          //The only thing that needs to be done for single cylinder is to check for staging. 
          if( (configPage10.stagingEnabled == true) && (currentStatus.stagingActive == true) )
          {
            PWdivTimerPerDegree = timeToAngleDegPerMicroSec(currentStatus.PW2); //Need to redo this for PW2 as it will be dramatically different to PW1 when staging
            injectionStartAngles[1] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel1InjDegrees, currentStatus.injAngle);
          }
          break;
        //2 cylinders
        case 2:
          injectionStartAngles[1] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel2InjDegrees, currentStatus.injAngle);
          
          if ( (configPage2.injLayout == INJ_SEQUENTIAL) && (configPage6.fuelTrimEnabled > 0) )
          {
            currentStatus.PW1 = applyFuelTrimToPW(&trim1Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW1);
            currentStatus.PW2 = applyFuelTrimToPW(&trim2Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW2);
          }
          else if( (configPage10.stagingEnabled == true) && (currentStatus.stagingActive == true) )
          {
            PWdivTimerPerDegree = timeToAngleDegPerMicroSec(currentStatus.PW3); //Need to redo this for PW3 as it will be dramatically different to PW1 when staging
            injectionStartAngles[2] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel1InjDegrees, currentStatus.injAngle);
            // injectionStartAngles[3] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel2InjDegrees, currentStatus.injAngle);

            injectionStartAngles[3] = injectionStartAngles[2] + (CRANK_ANGLE_MAX_INJ / 2); //Phase this either 180 or 360 degrees out from inj3 (In reality this will always be 180 as you can't have sequential and staged currently)
            if(injectionStartAngles[3] > (uint16_t)CRANK_ANGLE_MAX_INJ) { injectionStartAngles[3] -= CRANK_ANGLE_MAX_INJ; }
          }
          break;
        //3 cylinders
        case 3:
          injectionStartAngles[1] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel2InjDegrees, currentStatus.injAngle);
          injectionStartAngles[2] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel3InjDegrees, currentStatus.injAngle);
          
          if ( (configPage2.injLayout == INJ_SEQUENTIAL) && (configPage6.fuelTrimEnabled > 0) )
          {
            currentStatus.PW1 = applyFuelTrimToPW(&trim1Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW1);
            currentStatus.PW2 = applyFuelTrimToPW(&trim2Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW2);
            currentStatus.PW3 = applyFuelTrimToPW(&trim3Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW3);

            #if INJ_CHANNELS >= 6
              if( (configPage10.stagingEnabled == true) && (currentStatus.stagingActive == true) )
              {
                PWdivTimerPerDegree = timeToAngleDegPerMicroSec(currentStatus.PW4); //Need to redo this for PW4 as it will be dramatically different to PW1 when staging
                injectionStartAngles[3] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel1InjDegrees, currentStatus.injAngle);
                injectionStartAngles[4] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel2InjDegrees, currentStatus.injAngle);
                injectionStartAngles[5] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel3InjDegrees, currentStatus.injAngle);
              }
            #endif
          }
          else if( (configPage10.stagingEnabled == true) && (currentStatus.stagingActive == true) )
          {
            PWdivTimerPerDegree = timeToAngleDegPerMicroSec(currentStatus.PW4); //Need to redo this for PW3 as it will be dramatically different to PW1 when staging
            injectionStartAngles[3] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel1InjDegrees, currentStatus.injAngle);
            #if INJ_CHANNELS >= 6
              injectionStartAngles[4] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel2InjDegrees, currentStatus.injAngle);
              injectionStartAngles[5] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel3InjDegrees, currentStatus.injAngle);
            #endif
          }
          break;
        //4 cylinders
        case 4:
          injectionStartAngles[1] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel2InjDegrees, currentStatus.injAngle);

          if((configPage2.injLayout == INJ_SEQUENTIAL) && currentStatus.decoder.getStatus().syncStatus==SyncStatus::Full)
          {
            if( CRANK_ANGLE_MAX_INJ != 720 ) { changeHalfToFullSync(configPage2, configPage4, currentStatus); }

            injectionStartAngles[2] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel3InjDegrees, currentStatus.injAngle);
            injectionStartAngles[3] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel4InjDegrees, currentStatus.injAngle);
            #if INJ_CHANNELS >= 8
              if( (configPage10.stagingEnabled == true) && (currentStatus.stagingActive == true) )
              {
                PWdivTimerPerDegree = timeToAngleDegPerMicroSec(currentStatus.PW5); //Need to redo this for PW5 as it will be dramatically different to PW1 when staging
                injectionStartAngles[4] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel1InjDegrees, currentStatus.injAngle);
                injectionStartAngles[5] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel2InjDegrees, currentStatus.injAngle);
                injectionStartAngles[6] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel3InjDegrees, currentStatus.injAngle);
                injectionStartAngles[7] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel4InjDegrees, currentStatus.injAngle);
              }
            #endif

            if(configPage6.fuelTrimEnabled > 0)
            {
              currentStatus.PW1 = applyFuelTrimToPW(&trim1Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW1);
              currentStatus.PW2 = applyFuelTrimToPW(&trim2Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW2);
              currentStatus.PW3 = applyFuelTrimToPW(&trim3Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW3);
              currentStatus.PW4 = applyFuelTrimToPW(&trim4Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW4);
            }
          }
          else if( (configPage10.stagingEnabled == true) && (currentStatus.stagingActive == true) )
          {
            PWdivTimerPerDegree = timeToAngleDegPerMicroSec(currentStatus.PW3); //Need to redo this for PW3 as it will be dramatically different to PW1 when staging
            injectionStartAngles[2] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel1InjDegrees, currentStatus.injAngle);
            injectionStartAngles[3] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel2InjDegrees, currentStatus.injAngle);
          }
          else
          {
            if( currentStatus.decoder.getStatus().syncStatus==SyncStatus::Partial && (CRANK_ANGLE_MAX_INJ != 360) ) { changeFullToHalfSync(configPage2, configPage4, currentStatus); }
          }
          break;
        //5 cylinders
        case 5:
          injectionStartAngles[1] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel2InjDegrees, currentStatus.injAngle);
          injectionStartAngles[2] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel3InjDegrees, currentStatus.injAngle);
          injectionStartAngles[3] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel4InjDegrees, currentStatus.injAngle);
          #if INJ_CHANNELS >= 5
            injectionStartAngles[4] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel5InjDegrees, currentStatus.injAngle);
          #endif

          //Staging is possible by using the 6th channel if available
          #if INJ_CHANNELS >= 6
            if( (configPage10.stagingEnabled == true) && (currentStatus.stagingActive == true) )
            {
              PWdivTimerPerDegree = timeToAngleDegPerMicroSec(currentStatus.PW6);
              injectionStartAngles[5] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel6InjDegrees, currentStatus.injAngle);
            }
          #endif

          break;
        //6 cylinders
        case 6:
          injectionStartAngles[1] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel2InjDegrees, currentStatus.injAngle);
          injectionStartAngles[2] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel3InjDegrees, currentStatus.injAngle);
          
          #if INJ_CHANNELS >= 6
            if((configPage2.injLayout == INJ_SEQUENTIAL) && currentStatus.decoder.getStatus().syncStatus==SyncStatus::Full)
            {
              if( CRANK_ANGLE_MAX_INJ != 720 ) { changeHalfToFullSync(configPage2, configPage4, currentStatus); }

              injectionStartAngles[3] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel4InjDegrees, currentStatus.injAngle);
              injectionStartAngles[4] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel5InjDegrees, currentStatus.injAngle);
              injectionStartAngles[5] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel6InjDegrees, currentStatus.injAngle);

              if(configPage6.fuelTrimEnabled > 0)
              {
                currentStatus.PW1 = applyFuelTrimToPW(&trim1Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW1);
                currentStatus.PW2 = applyFuelTrimToPW(&trim2Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW2);
                currentStatus.PW3 = applyFuelTrimToPW(&trim3Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW3);
                currentStatus.PW4 = applyFuelTrimToPW(&trim4Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW4);
                currentStatus.PW5 = applyFuelTrimToPW(&trim5Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW5);
                currentStatus.PW6 = applyFuelTrimToPW(&trim6Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW6);
              }

              //Staging is possible with sequential on 8 channel boards by using outputs 7 + 8 for the staged injectors
              #if INJ_CHANNELS >= 8
                if( (configPage10.stagingEnabled == true) && (currentStatus.stagingActive == true) )
                {
                  PWdivTimerPerDegree = timeToAngleDegPerMicroSec(currentStatus.PW4); //Need to redo this for staging PW as it will be dramatically different to PW1 when staging
                  injectionStartAngles[3] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel1InjDegrees, currentStatus.injAngle);
                  injectionStartAngles[4] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel2InjDegrees, currentStatus.injAngle);
                  injectionStartAngles[5] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel3InjDegrees, currentStatus.injAngle);
                }
              #endif
            }
            else
            {
              if( currentStatus.decoder.getStatus().syncStatus==SyncStatus::Partial && (CRANK_ANGLE_MAX_INJ != 360) ) { changeFullToHalfSync(configPage2, configPage4, currentStatus); }

              if( (configPage10.stagingEnabled == true) && (currentStatus.stagingActive == true) )
              {
                PWdivTimerPerDegree = timeToAngleDegPerMicroSec(currentStatus.PW4); //Need to redo this for staging PW as it will be dramatically different to PW1 when staging
                injectionStartAngles[3] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel1InjDegrees, currentStatus.injAngle);
                injectionStartAngles[4] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel2InjDegrees, currentStatus.injAngle);
                injectionStartAngles[5] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel3InjDegrees, currentStatus.injAngle); 
              }
            }
          #endif
          break;
        //8 cylinders
        case 8:
          injectionStartAngles[1] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel2InjDegrees, currentStatus.injAngle);
          injectionStartAngles[2] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel3InjDegrees, currentStatus.injAngle);
          injectionStartAngles[3] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel4InjDegrees, currentStatus.injAngle);

          #if INJ_CHANNELS >= 8
            if((configPage2.injLayout == INJ_SEQUENTIAL) && currentStatus.decoder.getStatus().syncStatus==SyncStatus::Full)
            {
              if( CRANK_ANGLE_MAX_INJ != 720 ) { changeHalfToFullSync(configPage2, configPage4, currentStatus); }

              injectionStartAngles[4] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel5InjDegrees, currentStatus.injAngle);
              injectionStartAngles[5] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel6InjDegrees, currentStatus.injAngle);
              injectionStartAngles[6] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel7InjDegrees, currentStatus.injAngle);
              injectionStartAngles[7] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel8InjDegrees, currentStatus.injAngle);

              if(configPage6.fuelTrimEnabled > 0)
              {
                currentStatus.PW1 = applyFuelTrimToPW(&trim1Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW1);
                currentStatus.PW2 = applyFuelTrimToPW(&trim2Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW2);
                currentStatus.PW3 = applyFuelTrimToPW(&trim3Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW3);
                currentStatus.PW4 = applyFuelTrimToPW(&trim4Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW4);
                currentStatus.PW5 = applyFuelTrimToPW(&trim5Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW5);
                currentStatus.PW6 = applyFuelTrimToPW(&trim6Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW6);
                currentStatus.PW7 = applyFuelTrimToPW(&trim7Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW7);
                currentStatus.PW8 = applyFuelTrimToPW(&trim8Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW8);
              }
            }
            else
            {
              if( currentStatus.decoder.getStatus().syncStatus==SyncStatus::Partial && (CRANK_ANGLE_MAX_INJ != 360) ) { changeFullToHalfSync(configPage2, configPage4, currentStatus); }

              if( (configPage10.stagingEnabled == true) && (currentStatus.stagingActive == true) )
              {
                PWdivTimerPerDegree = timeToAngleDegPerMicroSec(currentStatus.PW5); //Need to redo this for PW3 as it will be dramatically different to PW1 when staging
                injectionStartAngles[4] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel1InjDegrees, currentStatus.injAngle);
                injectionStartAngles[5] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel2InjDegrees, currentStatus.injAngle);
                injectionStartAngles[6] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel3InjDegrees, currentStatus.injAngle);
                injectionStartAngles[7] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel4InjDegrees, currentStatus.injAngle);
              }
            }

          #endif
          break;

        //Will hit the default case on 1 cylinder or >8 cylinders. Do nothing in these cases
        default:
          break;
      }

      //***********************************************************************************************
      //| BEGIN IGNITION CALCULATIONS

      //Set dwell
      currentStatus.dwell = correctionsDwell(computeDwell(currentStatus, configPage2, configPage4, dwellTable));

      // Convert the dwell time to dwell angle based on the current engine speed
      calculateIgnitionAngles(timeToAngleDegPerMicroSec(currentStatus.dwell));

      //If ignition timing is being tracked per tooth, perform the calcs to get the end teeth
      //This only needs to be run if the advance figure has changed, otherwise the end teeth will still be the same
      //if( (configPage2.perToothIgn == true) && (lastToothCalcAdvance != currentStatus.advance) ) { triggerSetEndTeeth(); }
      if( (configPage2.perToothIgn == true) ) { currentStatus.decoder.setEndTeeth(); }

      //***********************************************************************************************
      //| BEGIN FUEL SCHEDULES
      //Finally calculate the time (uS) until we reach the firing angles and set the schedules
      //We only need to set the schedule if we're BEFORE the open angle
      //This may potentially be called a number of times as we get closer and closer to the opening time

      // if(Serial && false)
      // {
      //   if(ignition1StartAngle > crankAngle)
      //   {
      //     noInterrupts();
      //     Serial.print("Time2LastTooth:"); Serial.println(micros()-toothLastToothTime);
      //     Serial.print("elapsedTime:"); Serial.println(elapsedTime);
      //     Serial.print("CurAngle:"); Serial.println(crankAngle);
      //     Serial.print("RPM:"); Serial.println(currentStatus.RPM);
      //     Serial.print("Tooth:"); Serial.println(toothCurrentCount);
      //     Serial.print("timePerDegree:"); Serial.println(timePerDegree);
      //     Serial.print("IGN1Angle:"); Serial.println(ignition1StartAngle);
      //     Serial.print("TimeToIGN1:"); Serial.println(angleToTime((ignition1StartAngle - crankAngle), CRANKMATH_METHOD_INTERVAL_REV));
      //     interrupts();
      //   }
      // }
      
      currentStatus.schedulerCutState = core_modules_get_scheduler_cut(currentStatus);
      
      setFuelSchedules(currentStatus, injectionStartAngles, injectorLimits(currentStatus.decoder.getCrankAngle()), currentStatus.schedulerCutState.fuelChannels);
    
      //***********************************************************************************************
      //| BEGIN IGNITION SCHEDULES
      //Same as above, except for ignition

      //fixedCrankingOverride is used to extend the dwell during cranking so that the decoder can trigger the spark upon seeing a certain tooth. Currently only available on the basic distributor and 4g63 decoders.
      if ( configPage4.ignCranklock && currentStatus.engineIsCranking && (currentStatus.decoder.getFeatures().hasFixedCrankingTiming) )
      {
        fixedCrankingOverride = currentStatus.dwell * 3;
        //This is a safety step to prevent the ignition start time occurring AFTER the target tooth pulse has already occurred. It simply moves the start time forward a little, which is compensated for by the increase in the dwell time
        if(currentStatus.RPM < 250)
        {
          ignition1StartAngle -= 5;
          ignition2StartAngle -= 5;
          ignition3StartAngle -= 5;
          ignition4StartAngle -= 5;
#if IGN_CHANNELS >= 5
          ignition5StartAngle -= 5;
#endif
#if IGN_CHANNELS >= 6          
          ignition6StartAngle -= 5;
#endif
#if IGN_CHANNELS >= 7
          ignition7StartAngle -= 5;
#endif
#if IGN_CHANNELS >= 8
          ignition8StartAngle -= 5;
#endif
        }
      }
      else { fixedCrankingOverride = 0; }

      setIgnitionChannels(ignitionLimits(currentStatus.decoder.getCrankAngle()), currentStatus.dwell + fixedCrankingOverride, currentStatus.schedulerCutState.ignitionChannels);

      if ( (!currentStatus.resetPreventActive) && (resetControl == RESET_CONTROL_PREVENT_WHEN_RUNNING) ) 
      {
        //Reset prevention is supposed to be on while the engine is running but isn't. Fix that.
        digitalWrite(pinResetControl, HIGH);
        currentStatus.resetPreventActive = true;
      }
    } //Has sync and RPM
    else if ( (currentStatus.resetPreventActive) && (resetControl == RESET_CONTROL_PREVENT_WHEN_RUNNING) )
    {
      digitalWrite(pinResetControl, LOW);
      currentStatus.resetPreventActive = false;
    }
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
