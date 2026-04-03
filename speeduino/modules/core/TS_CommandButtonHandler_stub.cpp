#include "modules/logging/TS_CommandButtonHandler.h"

#include "preprocessor.h"

#if !FEATURE_MODULE_LOGGING

bool TS_CommandButtonsHandler(uint16_t)
{
  return false;
}

#endif
