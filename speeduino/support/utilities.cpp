/*
  Speeduino - Simple engine management for the Arduino Mega 2560 platform
  Copyright (C) Josh Stewart
  A full copy of the license may be found in the projects root directory
*/
/** @file
 * General utility functions shared across the firmware.
 */
#include <avr/pgmspace.h>
#include "support/utilities.h"
#include "data/config_pages.h"
#include "data/statuses.h"
#include "engine/decoders.h"
#include "comms/comms.h"
#include "modules/logging/logger.h"
#include "support/units.h"

extern byte resetControl;
extern byte pinResetControl;
extern struct statuses currentStatus;
extern volatile uint32_t runSecsX10;
bool pinIsUsed(byte pin);

uint8_t ioDelay[PROGRAMMABLE_IO_CHANNELS];
uint8_t ioOutDelay[PROGRAMMABLE_IO_CHANNELS];
uint8_t pinIsValid = 0;
uint8_t currentRuleStatus = 0;

/** Translate between the pin list that appears in TS and the actual pin numbers.
For the **digital IO**, this will simply return the same number as the rawPin value as those are mapped directly.
For **analog pins**, it will translate them into the correct internal pin number.
* @param rawPin - High level pin number
* @return Translated / usable pin number
*/
byte pinTranslate(byte rawPin)
{
  byte outputPin = rawPin;
  if(rawPin > BOARD_MAX_DIGITAL_PINS) { outputPin = A8 + (outputPin - BOARD_MAX_DIGITAL_PINS - 1); }

  return outputPin;
}
/** Translate a pin number (0 - 22) to the relevant Ax (analog) pin reference.
* This is required as some ARM chips do not have all analog pins in order (EG pin A15 != A14 + 1).
* */
byte pinTranslateAnalog(byte rawPin)
{
  byte outputPin = rawPin;
  switch(rawPin)
  {
    case 0: outputPin = A0; break;
    case 1: outputPin = A1; break;
    case 2: outputPin = A2; break;
    case 3: outputPin = A3; break;
    case 4: outputPin = A4; break;
    case 5: outputPin = A5; break;
    case 6: outputPin = A6; break;
    case 7: outputPin = A7; break;
    case 8: outputPin = A8; break;
    case 9: outputPin = A9; break;
    case 10: outputPin = A10; break;
    case 11: outputPin = A11; break;
    case 12: outputPin = A12; break;
    case 13: outputPin = A13; break;
  #if BOARD_MAX_ADC_PINS >= 14
      case 14: outputPin = A14; break;
    #endif
    #if BOARD_MAX_ADC_PINS >= 15
      case 15: outputPin = A15; break;
    #endif
    #if BOARD_MAX_ADC_PINS >= 16
      case 16: outputPin = A16; break;
    #endif
    #if BOARD_MAX_ADC_PINS >= 17
      case 17: outputPin = A17; break;
    #endif
    #if BOARD_MAX_ADC_PINS >= 18
      case 18: outputPin = A18; break;
    #endif
    #if BOARD_MAX_ADC_PINS >= 19
      case 19: outputPin = A19; break;
    #endif
    #if BOARD_MAX_ADC_PINS >= 20
      case 20: outputPin = A20; break;
    #endif
    #if BOARD_MAX_ADC_PINS >= 21
      case 21: outputPin = A21; break;
    #endif
    #if BOARD_MAX_ADC_PINS >= 22
      case 22: outputPin = A22; break;
    #endif
    default:
      // Keep MISRA happy when a non-mapped value is passed.
      outputPin = rawPin;
      break;
  }

  return outputPin;
}

void setResetControlPinState(void)
{
  currentStatus.resetPreventActive = false;

  /* Setup reset control initial state */
  switch (resetControl)
  {
    case RESET_CONTROL_PREVENT_WHEN_RUNNING:
      /* Set the reset control pin LOW and change it to HIGH later when we get sync. */
      digitalWrite(pinResetControl, LOW);
      currentStatus.resetPreventActive = false;
      break;
    case RESET_CONTROL_PREVENT_ALWAYS:
      /* Set the reset control pin HIGH and never touch it again. */
      digitalWrite(pinResetControl, HIGH);
      currentStatus.resetPreventActive = true;
      break;
    case RESET_CONTROL_SERIAL_COMMAND:
      /* Set the reset control pin HIGH. There currently isn't any practical difference
         between this and PREVENT_ALWAYS but it doesn't hurt anything to have them separate. */
      digitalWrite(pinResetControl, HIGH);
      currentStatus.resetPreventActive = false;
      break;
    default:
      // Do nothing - keep MISRA happy
      break;
  }
}

/** Get single I/O data var (from currentStatus) for comparison.
 * @param index - Field index/number (?)
 * @return 16 bit (int) result
 */
int16_t ProgrammableIOGetData(uint16_t index)
{
  int16_t result;
  if ( index < LOG_ENTRY_SIZE )
  {
    if(is2ByteEntry(index)) { result = word(getTSLogEntry(index+1), getTSLogEntry(index)); }
    else { result = getTSLogEntry(index); }

    //Special cases for temperatures
    if( (index == 6) || (index == 7) ) { result = temperatureRemoveOffset(result); }
  }
  else if ( index == 239U ) { result = (int16_t)max((uint32_t)runSecsX10, (uint32_t)32768); } //STM32 used std lib
  else { result = -1; } //Index is bigger than fullStatus array
  return result;
}
