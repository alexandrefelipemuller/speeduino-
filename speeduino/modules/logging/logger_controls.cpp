#include "logger_controls.h"
#include "data/logger_status.h"
#include "data/config_pages.h"
#include "engine/decoders.h"
#include "orchestration/init.h"
#include "data/statuses.h"
#include "engine/decoder_t.h"
#include "boards/board_definition.h"

extern struct statuses currentStatus;
extern struct config2 configPage2;
extern volatile unsigned int toothHistoryIndex;
extern byte pinTrigger;
extern byte pinTrigger2;
extern byte pinTrigger3;
extern byte pinVSS;
extern byte pinFlex;

static inline void attachLoggerInterrupt(uint8_t pin, void (*loggerISR)(void))
{
  detachInterrupt(digitalPinToInterrupt(pin));
  attachInterrupt(digitalPinToInterrupt(pin), loggerISR, CHANGE);
}

static inline void detachLoggerInterrupt(uint8_t pin, const interrupt_t &decoderInterrupt)
{
  detachInterrupt(digitalPinToInterrupt(pin));
  decoderInterrupt.attach(pin);
}

void startToothLogger(void)
{
  currentLoggerStatus.tooth_log_enabled = true;
  currentLoggerStatus.composite_trigger_used = 0U;
  currentLoggerStatus.is_tooth_log_1_full = false;
  toothHistoryIndex = 0U;

  attachLoggerInterrupt(pinTrigger, loggerPrimaryISR);

  if(VSS_USES_RPM2() != true)
  {
    attachLoggerInterrupt(pinTrigger2, loggerSecondaryISR);
  }
}

void stopToothLogger(void)
{
  currentLoggerStatus.tooth_log_enabled = false;

  detachLoggerInterrupt(pinTrigger, currentStatus.decoder.primary);

  if(VSS_USES_RPM2() != true)
  {
    detachLoggerInterrupt(pinTrigger2, currentStatus.decoder.secondary);
  }
}

void startCompositeLogger(void)
{
  currentLoggerStatus.composite_trigger_used = 2U;
  currentLoggerStatus.tooth_log_enabled = false;
  currentLoggerStatus.is_tooth_log_1_full = false;
  toothHistoryIndex = 0U;

  attachLoggerInterrupt(pinTrigger, loggerPrimaryISR);

  if((VSS_USES_RPM2() != true) && (FLEX_USES_RPM2() != true))
  {
    attachLoggerInterrupt(pinTrigger2, loggerSecondaryISR);
  }
}

void stopCompositeLogger(void)
{
  currentLoggerStatus.composite_trigger_used = 0U;

  detachLoggerInterrupt(pinTrigger, currentStatus.decoder.primary);

  if((VSS_USES_RPM2() != true) && (FLEX_USES_RPM2() != true))
  {
    detachLoggerInterrupt(pinTrigger2, currentStatus.decoder.secondary);
  }
}

void startCompositeLoggerTertiary(void)
{
  currentLoggerStatus.composite_trigger_used = 3U;
  currentLoggerStatus.tooth_log_enabled = false;
  currentLoggerStatus.is_tooth_log_1_full = false;
  toothHistoryIndex = 0U;

  attachLoggerInterrupt(pinTrigger, loggerPrimaryISR);
  attachLoggerInterrupt(pinTrigger3, loggerTertiaryISR);
}

void stopCompositeLoggerTertiary(void)
{
  currentLoggerStatus.composite_trigger_used = 0U;

  detachLoggerInterrupt(pinTrigger, currentStatus.decoder.primary);
  detachLoggerInterrupt(pinTrigger3, currentStatus.decoder.tertiary);
}

void startCompositeLoggerCams(void)
{
  currentLoggerStatus.composite_trigger_used = 4U;
  currentLoggerStatus.tooth_log_enabled = false;
  currentLoggerStatus.is_tooth_log_1_full = false;
  toothHistoryIndex = 0U;

  if((VSS_USES_RPM2() != true) && (FLEX_USES_RPM2() != true))
  {
    attachLoggerInterrupt(pinTrigger2, loggerSecondaryISR);
  }

  attachLoggerInterrupt(pinTrigger3, loggerTertiaryISR);
}

void stopCompositeLoggerCams(void)
{
  currentLoggerStatus.composite_trigger_used = 0U;

  if((VSS_USES_RPM2() != true) && (FLEX_USES_RPM2() != true))
  {
    detachLoggerInterrupt(pinTrigger2, currentStatus.decoder.secondary);
  }

  detachLoggerInterrupt(pinTrigger3, currentStatus.decoder.tertiary);
}
