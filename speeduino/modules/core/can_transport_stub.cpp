#include "modules/comms_extended/can_transport.h"
#include "preprocessor.h"

#if !FEATURE_MODULE_COMMS_EXTENDED

void sendCancommand(uint8_t, uint16_t, uint8_t, uint8_t, uint16_t) {}

#endif
