/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
can_comms was originally contributed by Darren Siepka
*/

#include "comms.h"
#include "secondary_serial.h"
#include "config9_domains.h"
#include "config_pages.h"
#include "statuses.h"
#include "bit_manip.h"
#include "preprocessor.h"
#include "comms_legacy.h"
#include "modules/logging/logger.h"
#include "page_crc.h"
#include "board_definition.h"

extern struct statuses currentStatus;
extern struct config9 configPage9;

uint8_t currentSecondaryCommand;
SECONDARY_SERIAL_T* pSecondarySerial;

#if defined(CORE_AVR)
#pragma GCC push_options
// This minimizes RAM usage at no performance cost
#pragma GCC optimize ("Os") 
#endif

void secondserial_Command(void)
{
  const secondary_serial_config_t serial_config = get_secondary_serial_config(configPage9);

  // Primary tuning comms must remain on the primary serial port.
  if (serial_config.protocol() == SECONDARY_SERIAL_PROTO_TUNERSTUDIO)
  {
    if (serialSecondaryStatusFlag == SERIAL_INACTIVE)
    {
      (void)secondarySerial.read();
    }
    return;
  }

  if ( serialSecondaryStatusFlag == SERIAL_INACTIVE )  { currentSecondaryCommand = secondarySerial.read(); }

  switch (currentSecondaryCommand)
  {
    case 'A': 
      // sends a fixed 75 bytes of data. Used by Real Dash (Among others)
      if(serial_config.is_legacy_fixed_protocol()) { sendValues(0, CAN_PACKET_SIZE, 0x31, secondarySerial, serialSecondaryStatusFlag, &getLegacySecondarySerialLogEntry); } // Send values using the legacy fixed byte order
      else { sendValues(0, CAN_PACKET_SIZE, 0x31, secondarySerial, serialSecondaryStatusFlag); } //send values to serial3 using the order in the ini file
      break;

    case 'b': // New EEPROM burn command to only burn a single page at a time
      legacySerialHandler(currentSecondaryCommand, secondarySerial, serialSecondaryStatusFlag);
      break;

    case 'B': // AS above but for the serial compatibility mode. 
      currentStatus.commCompat = true; //Force the compat mode
      legacySerialHandler(currentSecondaryCommand, secondarySerial, serialSecondaryStatusFlag);
      break;

    case 'd': // Send a CRC32 hash of a given page
      legacySerialHandler(currentSecondaryCommand, secondarySerial, serialSecondaryStatusFlag);
      break;

    case 'G': // this is the reply command sent by the Can interface
      serialSecondaryStatusFlag = SERIAL_COMMAND_INPROGRESS_LEGACY;
      byte destcaninchannel;
      if (secondarySerial.available() >= 9)
      {
        serialSecondaryStatusFlag = SERIAL_INACTIVE;
        uint8_t cmdSuccessful = secondarySerial.read();        //0 == fail,  1 == good.
        destcaninchannel = secondarySerial.read();  // the input channel that requested the data value
        if (cmdSuccessful != 0)
        {                                 // read all 8 bytes of data.
          uint8_t Gdata[9];
          uint8_t Glow, Ghigh;

          for (byte Gx = 0; Gx < 8; Gx++) // first two are the can address the data is from. next two are the can address the data is for.then next 1 or two bytes of data
          {
            Gdata[Gx] = secondarySerial.read();
          }
          Glow = Gdata[(serial_config.caninput_start_byte(destcaninchannel) & 7U)];
          if (serial_config.caninput_is_two_bytes(destcaninchannel))
          {
            if ((serial_config.caninput_start_byte(destcaninchannel) & 7U) < 8U)
            {
              Ghigh = Gdata[((serial_config.caninput_start_byte(destcaninchannel) & 7U) + 1U)];
            }
            else { Ghigh = 0; }
          }
        else
        {
          Ghigh = 0;
        }

        currentStatus.canin[destcaninchannel] = (Ghigh<<8) | Glow;
      }

        else{}  //continue as command request failed and/or data/device was not available

      }
      break;

    case 'k':   //placeholder for new can interface (toucan etc) commands

        break;

    case 'M':
      legacySerialHandler(currentSecondaryCommand, secondarySerial, serialSecondaryStatusFlag);
      break;
      
    case 'n': // sends the bytes of realtime values from the NEW CAN list
      //sendValues(0, NEW_CAN_PACKET_SIZE, 0x32, secondarySerial, serialSecondaryStatusFlag); //send values to serial3
      if(serial_config.is_legacy_fixed_protocol()) { sendValues(0, NEW_CAN_PACKET_SIZE, 0x32, secondarySerial, serialSecondaryStatusFlag, &getLegacySecondarySerialLogEntry); } // Send values using the legacy fixed byte order
      else { sendValues(0, NEW_CAN_PACKET_SIZE, 0x32, secondarySerial, serialSecondaryStatusFlag); } //send values to serial3 using the order in the ini file
      break;

    case 'p':
      legacySerialHandler(currentSecondaryCommand, secondarySerial, serialSecondaryStatusFlag);
      break;

    case 'Q': // send code version
      legacySerialHandler(currentSecondaryCommand, secondarySerial, serialSecondaryStatusFlag);
       break;

    case 'r': //New format for the optimised OutputChannels over CAN
      legacySerialHandler(currentSecondaryCommand, secondarySerial, serialSecondaryStatusFlag);
      break;

    case 's': // send the "a" stream code version
      secondarySerial.print(F("Speeduino csx02019.8"));
      break;

    case 'S': // send code version
      if(serial_config.is_msdroid_protocol()) { legacySerialHandler('Q', secondarySerial, serialSecondaryStatusFlag); } //Note 'Q', this is a workaround for msDroid
      else { legacySerialHandler(currentSecondaryCommand, secondarySerial, serialSecondaryStatusFlag); }
      
      break;

    case 'Z': //dev use
       break;

    default:
       break;
  }
}

#if defined(CORE_AVR)
#pragma GCC pop_options
#endif
