#include "logger.h"

#include "data/sd_logging_status.h"
#include "logger_private.h"

byte buildSdCardStatus(const statuses &current)
{
  UNUSED(current);
  bool bits[] = {
    currentSdLoggingStatus.card_present,
    currentSdLoggingStatus.card_type == 1U,
    currentSdLoggingStatus.card_ready,
    currentSdLoggingStatus.card_logging,
    currentSdLoggingStatus.card_error,
    false, // Unused
    currentSdLoggingStatus.card_fs == 1U,
    currentSdLoggingStatus.card_unused,
  };
  return setStatusBits(0, bits);
}
