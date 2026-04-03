#include "can_transport.h"

#include "comms_CAN.h"
#include "maths.h"
#include "preprocessor.h"
#include "modules/secondary_serial/secondary_serial.h"

#if FEATURE_MODULE_COMMS_EXTENDED

void sendCancommand(uint8_t cmdtype, uint16_t canaddress, uint8_t candata1, uint8_t candata2, uint16_t sourcecanAddress)
{
  switch (cmdtype)
  {
    case 0:
      #if FEATURE_MODULE_SECONDARY_SERIAL
        secondarySerial.print("G");
        secondarySerial.write(canaddress);
        secondarySerial.write(candata1);
        secondarySerial.write(candata2);
      #else
        UNUSED(canaddress);
        UNUSED(candata1);
        UNUSED(candata2);
      #endif
      break;

    case 1:
      #if FEATURE_MODULE_SECONDARY_SERIAL
        secondarySerial.print("L");
        secondarySerial.write(canaddress);
      #else
        UNUSED(canaddress);
      #endif
      UNUSED(candata1);
      UNUSED(candata2);
      UNUSED(sourcecanAddress);
      break;

    case 2:
      #if FEATURE_MODULE_SECONDARY_SERIAL
        secondarySerial.print("R");
        secondarySerial.write(candata1);
        secondarySerial.write(lowByte(sourcecanAddress));
        secondarySerial.write(highByte(sourcecanAddress));
      #else
        UNUSED(candata1);
        UNUSED(sourcecanAddress);
      #endif
      UNUSED(canaddress);
      UNUSED(candata2);
      break;

    case 3:
      #if defined(NATIVE_CAN_AVAILABLE)
        outMsg.id = canaddress;
        outMsg.len = 8;
        outMsg.buf[0] = 0x0B;
        outMsg.buf[1] = 0x15;
        outMsg.buf[2] = candata1;
        outMsg.buf[3] = 0x24;
        outMsg.buf[4] = 0x7F;
        outMsg.buf[5] = 0x70;
        outMsg.buf[6] = 0x9E;
        outMsg.buf[7] = 0x4D;
        CAN_write();
      #else
        UNUSED(canaddress);
        UNUSED(candata1);
      #endif
      UNUSED(candata2);
      UNUSED(sourcecanAddress);
      break;

    default:
      break;
  }
}

#endif
