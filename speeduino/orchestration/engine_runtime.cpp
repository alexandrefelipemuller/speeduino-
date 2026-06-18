#include "orchestration/engine_runtime.h"

#include "data/config_pages.h"
#include "data/pin_registry.h"
#include "data/runtime_state.h"
#include "data/tune_registry.h"
#include "engine/corrections.h"
#include "engine/dwell.h"
#include "engine/fuel_calcs.h"
#include "engine/idle.h"
#include "engine/sensors.h"
#include "modules/core/module_runtime.h"
#include "orchestration/fuel_runtime.h"
#include "orchestration/ignition_runtime.h"
#include "orchestration/init.h"
#include "orchestration/schedule_angles.h"
#include "orchestration/schedule_fuel_calcs.hpp"
#include "support/maths.h"

void engineRuntimeRunCycle(void)
{
  const decoder_status_t decoderStatus = currentStatus.decoder.getStatus();
  const decoder_features_t decoderFeatures = currentStatus.decoder.getFeatures();
  const int16_t crankAngleSnapshot = currentStatus.decoder.getCrankAngle();
  unsigned int PWdivTimerPerDegree = timeToAngleDegPerMicroSec(currentStatus.PW1);
  uint16_t injectionStartAngles[INJ_CHANNELS] = {0U};
  pulseWidths pulse_widths = computePulseWidths(
                                configPage2,
                                configPage6,
                                configPage10, 
                                decoderStatus,
                                currentStatus);
  currentStatus.stagingActive = pulse_widths.secondary!=0U;
  applyPwToInjectorChannels(pulse_widths, configPage2, currentStatus);

  //***********************************************************************************************
  //BEGIN INJECTION TIMING
  currentStatus.injAngle = getInjectorAngle(currentStatus.RPMdiv100);
  if(currentStatus.injAngle > uint16_t(CRANK_ANGLE_MAX_INJ)) { currentStatus.injAngle = uint16_t(CRANK_ANGLE_MAX_INJ); }

  PWdivTimerPerDegree = timeToAngleDegPerMicroSec(currentStatus.PW1);
  injectionStartAngles[0] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel1InjDegrees, currentStatus.injAngle);

  switch (configPage2.nCylinders)
  {
    case 1:
      if( (configPage10.stagingEnabled == true) && (currentStatus.stagingActive == true) )
      {
        PWdivTimerPerDegree = timeToAngleDegPerMicroSec(currentStatus.PW2);
        injectionStartAngles[1] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel1InjDegrees, currentStatus.injAngle);
      }
      break;
    case 2:
      injectionStartAngles[1] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel2InjDegrees, currentStatus.injAngle);

      if ( (configPage2.injLayout == INJ_SEQUENTIAL) && (configPage6.fuelTrimEnabled > 0) )
      {
        currentStatus.PW1 = applyFuelTrimToPW(&trim1Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW1);
        currentStatus.PW2 = applyFuelTrimToPW(&trim2Table, currentStatus.fuelLoad, currentStatus.RPM, currentStatus.PW2);
      }
      else if( (configPage10.stagingEnabled == true) && (currentStatus.stagingActive == true) )
      {
        PWdivTimerPerDegree = timeToAngleDegPerMicroSec(currentStatus.PW3);
        injectionStartAngles[2] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel1InjDegrees, currentStatus.injAngle);
        injectionStartAngles[3] = injectionStartAngles[2] + (CRANK_ANGLE_MAX_INJ / 2);
        if(injectionStartAngles[3] > (uint16_t)CRANK_ANGLE_MAX_INJ) { injectionStartAngles[3] -= CRANK_ANGLE_MAX_INJ; }
      }
      break;
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
          PWdivTimerPerDegree = timeToAngleDegPerMicroSec(currentStatus.PW4);
          injectionStartAngles[3] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel1InjDegrees, currentStatus.injAngle);
          injectionStartAngles[4] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel2InjDegrees, currentStatus.injAngle);
          injectionStartAngles[5] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel3InjDegrees, currentStatus.injAngle);
        }
#endif
      }
      else if( (configPage10.stagingEnabled == true) && (currentStatus.stagingActive == true) )
      {
        PWdivTimerPerDegree = timeToAngleDegPerMicroSec(currentStatus.PW4);
        injectionStartAngles[3] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel1InjDegrees, currentStatus.injAngle);
#if INJ_CHANNELS >= 6
        injectionStartAngles[4] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel2InjDegrees, currentStatus.injAngle);
        injectionStartAngles[5] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel3InjDegrees, currentStatus.injAngle);
#endif
      }
      break;
    case 4:
      injectionStartAngles[1] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel2InjDegrees, currentStatus.injAngle);
      if((configPage2.injLayout == INJ_SEQUENTIAL) && decoderStatus.syncStatus==SyncStatus::Full)
      {
        if( CRANK_ANGLE_MAX_INJ != 720 ) { changeHalfToFullSync(configPage2, configPage4, currentStatus); }
        injectionStartAngles[2] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel3InjDegrees, currentStatus.injAngle);
        injectionStartAngles[3] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel4InjDegrees, currentStatus.injAngle);
#if INJ_CHANNELS >= 8
        if( (configPage10.stagingEnabled == true) && (currentStatus.stagingActive == true) )
        {
          PWdivTimerPerDegree = timeToAngleDegPerMicroSec(currentStatus.PW5);
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
        PWdivTimerPerDegree = timeToAngleDegPerMicroSec(currentStatus.PW3);
        injectionStartAngles[2] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel1InjDegrees, currentStatus.injAngle);
        injectionStartAngles[3] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel2InjDegrees, currentStatus.injAngle);
      }
      else
      {
        if( decoderStatus.syncStatus==SyncStatus::Partial && (CRANK_ANGLE_MAX_INJ != 360) ) { changeFullToHalfSync(configPage2, configPage4, currentStatus); }
      }
      break;
    case 5:
      injectionStartAngles[1] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel2InjDegrees, currentStatus.injAngle);
      injectionStartAngles[2] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel3InjDegrees, currentStatus.injAngle);
      injectionStartAngles[3] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel4InjDegrees, currentStatus.injAngle);
#if INJ_CHANNELS >= 5
      injectionStartAngles[4] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel5InjDegrees, currentStatus.injAngle);
#endif
#if INJ_CHANNELS >= 6
      if( (configPage10.stagingEnabled == true) && (currentStatus.stagingActive == true) )
      {
        PWdivTimerPerDegree = timeToAngleDegPerMicroSec(currentStatus.PW6);
        injectionStartAngles[5] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel6InjDegrees, currentStatus.injAngle);
      }
#endif
      break;
    case 6:
      injectionStartAngles[1] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel2InjDegrees, currentStatus.injAngle);
      injectionStartAngles[2] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel3InjDegrees, currentStatus.injAngle);
#if INJ_CHANNELS >= 6
      if((configPage2.injLayout == INJ_SEQUENTIAL) && decoderStatus.syncStatus==SyncStatus::Full)
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
#if INJ_CHANNELS >= 8
        if( (configPage10.stagingEnabled == true) && (currentStatus.stagingActive == true) )
        {
          PWdivTimerPerDegree = timeToAngleDegPerMicroSec(currentStatus.PW4);
          injectionStartAngles[3] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel1InjDegrees, currentStatus.injAngle);
          injectionStartAngles[4] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel2InjDegrees, currentStatus.injAngle);
          injectionStartAngles[5] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel3InjDegrees, currentStatus.injAngle);
        }
#endif
      }
      else
      {
        if( decoderStatus.syncStatus==SyncStatus::Partial && (CRANK_ANGLE_MAX_INJ != 360) ) { changeFullToHalfSync(configPage2, configPage4, currentStatus); }
        if( (configPage10.stagingEnabled == true) && (currentStatus.stagingActive == true) )
        {
          PWdivTimerPerDegree = timeToAngleDegPerMicroSec(currentStatus.PW4);
          injectionStartAngles[3] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel1InjDegrees, currentStatus.injAngle);
          injectionStartAngles[4] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel2InjDegrees, currentStatus.injAngle);
          injectionStartAngles[5] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel3InjDegrees, currentStatus.injAngle); 
        }
      }
#endif
      break;
    case 8:
      injectionStartAngles[1] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel2InjDegrees, currentStatus.injAngle);
      injectionStartAngles[2] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel3InjDegrees, currentStatus.injAngle);
      injectionStartAngles[3] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel4InjDegrees, currentStatus.injAngle);
#if INJ_CHANNELS >= 8
      if((configPage2.injLayout == INJ_SEQUENTIAL) && decoderStatus.syncStatus==SyncStatus::Full)
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
        if( decoderStatus.syncStatus==SyncStatus::Partial && (CRANK_ANGLE_MAX_INJ != 360) ) { changeFullToHalfSync(configPage2, configPage4, currentStatus); }
        if( (configPage10.stagingEnabled == true) && (currentStatus.stagingActive == true) )
        {
          PWdivTimerPerDegree = timeToAngleDegPerMicroSec(currentStatus.PW5);
          injectionStartAngles[4] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel1InjDegrees, currentStatus.injAngle);
          injectionStartAngles[5] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel2InjDegrees, currentStatus.injAngle);
          injectionStartAngles[6] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel3InjDegrees, currentStatus.injAngle);
          injectionStartAngles[7] = calculateInjectorStartAngle(PWdivTimerPerDegree, channel4InjDegrees, currentStatus.injAngle);
        }
      }
#endif
      break;
    default:
      break;
  }

  //***********************************************************************************************
  //| BEGIN IGNITION CALCULATIONS
  currentStatus.dwell = correctionsDwell(computeDwell(currentStatus, configPage2, configPage4, dwellTable));
  calculateIgnitionAngles(timeToAngleDegPerMicroSec(currentStatus.dwell), decoderStatus.syncStatus);
  if( (configPage2.perToothIgn == true) ) { currentStatus.decoder.setEndTeeth(); }

  currentStatus.schedulerCutState = core_modules_get_scheduler_cut(currentStatus);
  setFuelSchedules(currentStatus, injectionStartAngles, injectorLimits(crankAngleSnapshot), currentStatus.schedulerCutState.fuelChannels);

  if ( configPage4.ignCranklock && currentStatus.engineIsCranking && (decoderFeatures.hasFixedCrankingTiming) )
  {
    fixedCrankingOverride = currentStatus.dwell * 3;
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

  setIgnitionChannels(ignitionLimits(crankAngleSnapshot), currentStatus.dwell + fixedCrankingOverride, currentStatus.schedulerCutState.ignitionChannels);

  if ( (!currentStatus.resetPreventActive) && (resetControl == RESET_CONTROL_PREVENT_WHEN_RUNNING) ) 
  {
    digitalWrite(pinResetControl, HIGH);
    currentStatus.resetPreventActive = true;
  }
}
