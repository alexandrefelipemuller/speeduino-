#include "logger_controls.h"
#include "config_pages.h"
#include "decoders.h"
#include "init.h"
#include "statuses.h"
#include "decoder_t.h"
#include "board_definition.h"

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
  currentStatus.toothLogEnabled = true;
  currentStatus.compositeTriggerUsed = 0U;
  currentStatus.isToothLog1Full = false;
  toothHistoryIndex = 0U;

  attachLoggerInterrupt(pinTrigger, loggerPrimaryISR);

  if(VSS_USES_RPM2() != true)
  {
    attachLoggerInterrupt(pinTrigger2, loggerSecondaryISR);
  }
}

void stopToothLogger(void)
{
  currentStatus.toothLogEnabled = false;

  detachLoggerInterrupt(pinTrigger, currentStatus.decoder.primary);

  if(VSS_USES_RPM2() != true)
  {
    detachLoggerInterrupt(pinTrigger2, currentStatus.decoder.secondary);
  }
}

void startCompositeLogger(void)
{
  currentStatus.compositeTriggerUsed = 2U;
  currentStatus.toothLogEnabled = false;
  currentStatus.isToothLog1Full = false;
  toothHistoryIndex = 0U;

  attachLoggerInterrupt(pinTrigger, loggerPrimaryISR);

  if((VSS_USES_RPM2() != true) && (FLEX_USES_RPM2() != true))
  {
    attachLoggerInterrupt(pinTrigger2, loggerSecondaryISR);
  }
}

void stopCompositeLogger(void)
{
  currentStatus.compositeTriggerUsed = 0U;

  detachLoggerInterrupt(pinTrigger, currentStatus.decoder.primary);

  if((VSS_USES_RPM2() != true) && (FLEX_USES_RPM2() != true))
  {
    detachLoggerInterrupt(pinTrigger2, currentStatus.decoder.secondary);
  }
}

void startCompositeLoggerTertiary(void)
{
  currentStatus.compositeTriggerUsed = 3U;
  currentStatus.toothLogEnabled = false;
  currentStatus.isToothLog1Full = false;
  toothHistoryIndex = 0U;

  attachLoggerInterrupt(pinTrigger, loggerPrimaryISR);
  attachLoggerInterrupt(pinTrigger3, loggerTertiaryISR);
}

void stopCompositeLoggerTertiary(void)
{
  currentStatus.compositeTriggerUsed = 0U;

  detachLoggerInterrupt(pinTrigger, currentStatus.decoder.primary);
  detachLoggerInterrupt(pinTrigger3, currentStatus.decoder.tertiary);
}

void startCompositeLoggerCams(void)
{
  currentStatus.compositeTriggerUsed = 4U;
  currentStatus.toothLogEnabled = false;
  currentStatus.isToothLog1Full = false;
  toothHistoryIndex = 0U;

  if((VSS_USES_RPM2() != true) && (FLEX_USES_RPM2() != true))
  {
    attachLoggerInterrupt(pinTrigger2, loggerSecondaryISR);
  }

  attachLoggerInterrupt(pinTrigger3, loggerTertiaryISR);
}

void stopCompositeLoggerCams(void)
{
  currentStatus.compositeTriggerUsed = 0U;

  if((VSS_USES_RPM2() != true) && (FLEX_USES_RPM2() != true))
  {
    detachLoggerInterrupt(pinTrigger2, currentStatus.decoder.secondary);
  }

  detachLoggerInterrupt(pinTrigger3, currentStatus.decoder.tertiary);
}
