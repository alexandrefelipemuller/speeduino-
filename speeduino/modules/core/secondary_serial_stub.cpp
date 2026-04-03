#include "modules/secondary_serial/secondary_serial.h"
#include "preprocessor.h"

#if !FEATURE_MODULE_SECONDARY_SERIAL

SECONDARY_SERIAL_T *pSecondarySerial = &Serial;

void secondserial_Command(void) {}

#endif
