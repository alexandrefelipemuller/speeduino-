#ifndef SECONDARY_SERIAL_H
#define SECONDARY_SERIAL_H

#include <stdint.h>
#include "board_definition.h"

#define NEW_CAN_PACKET_SIZE   123
#define CAN_PACKET_SIZE   75

#define SECONDARY_SERIAL_PROTO_GENERIC_FIXED  0
#define SECONDARY_SERIAL_PROTO_GENERIC_INI    1
#define SECONDARY_SERIAL_PROTO_CAN            2
#define SECONDARY_SERIAL_PROTO_MSDROID        3
#define SECONDARY_SERIAL_PROTO_REALDASH       4
#define SECONDARY_SERIAL_PROTO_TUNERSTUDIO    5

extern SECONDARY_SERIAL_T *pSecondarySerial;
#define secondarySerial (*pSecondarySerial)

void secondserial_Command(void);

#endif // SECONDARY_SERIAL_H
