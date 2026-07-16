#include "programmable_io.h"

#include "support/preprocessor.h"

#if FEATURE_MODULE_PROGRAMMABLE_IO

#include "boards/board_definition.h"
#include "data/config_pages.h"
#include "data/statuses.h"
#include "modules/logging/logger.h"
#include "support/bit_manip.h"
#include "support/utilities.h"

extern byte resetControl;
extern byte pinResetControl;
extern struct statuses currentStatus;
extern struct config13 configPage13;

static_assert(PROGRAMMABLE_IO_CHANNELS == sizeof(configPage13.outputPin), "PROGRAMMABLE_IO_CHANNELS must match configPage13.outputPin");

void module_programmable_io_init_post_pin_mapping(void)
{
  uint8_t outputPin;
  for (uint8_t y = 0; y < sizeof(configPage13.outputPin); y++)
  {
    ioDelay[y] = 0;
    ioOutDelay[y] = 0;
    outputPin = configPage13.outputPin[y];
    if (outputPin > 0)
    {
      if ( outputPin >= 128 )
      {
        BIT_WRITE(currentStatus.outputsStatus, y, BIT_CHECK(configPage13.outputInverted, y));
        BIT_SET(pinIsValid, y);
      }
      else
      {
        pinMode(outputPin, OUTPUT);
        digitalWrite(outputPin, BIT_CHECK(configPage13.outputInverted, y));
        BIT_WRITE(currentStatus.outputsStatus, y, BIT_CHECK(configPage13.outputInverted, y));
        BIT_SET(pinIsValid, y);
      }
    }
  }
}

void module_programmable_io_tick_10hz(void)
{
  int16_t data, data2;
  uint8_t dataRequested;
  bool firstCheck, secondCheck;

  for (uint8_t y = 0; y < sizeof(configPage13.outputPin); y++)
  {
    firstCheck = false;
    secondCheck = false;
    if ( BIT_CHECK(pinIsValid, y) )
    {
      dataRequested = configPage13.firstDataIn[y];
      if ( dataRequested > 239U )
      {
        dataRequested -= REUSE_RULES;
        if ( dataRequested <= sizeof(configPage13.outputPin) ) { data = BIT_CHECK(currentRuleStatus, dataRequested); }
        else { data = 0; }
      }
      else { data = ProgrammableIOGetData(dataRequested); }
      data2 = configPage13.firstTarget[y];

      if ( (configPage13.operation[y].firstCompType == COMPARATOR_EQUAL) && (data == data2) ) { firstCheck = true; }
      else if ( (configPage13.operation[y].firstCompType == COMPARATOR_NOT_EQUAL) && (data != data2) ) { firstCheck = true; }
      else if ( (configPage13.operation[y].firstCompType == COMPARATOR_GREATER) && (data > data2) ) { firstCheck = true; }
      else if ( (configPage13.operation[y].firstCompType == COMPARATOR_GREATER_EQUAL) && (data >= data2) ) { firstCheck = true; }
      else if ( (configPage13.operation[y].firstCompType == COMPARATOR_LESS) && (data < data2) ) { firstCheck = true; }
      else if ( (configPage13.operation[y].firstCompType == COMPARATOR_LESS_EQUAL) && (data <= data2) ) { firstCheck = true; }
      else if ( (configPage13.operation[y].firstCompType == COMPARATOR_AND) && ((data & data2) != 0) ) { firstCheck = true; }
      else if ( (configPage13.operation[y].firstCompType == COMPARATOR_XOR) && ((data ^ data2) != 0) ) { firstCheck = true; }
  else
  {
    ;
  }

      if (configPage13.operation[y].bitwise != BITWISE_DISABLED)
      {
        dataRequested = configPage13.secondDataIn[y];
        if ( dataRequested <= (REUSE_RULES + sizeof(configPage13.outputPin)) )
        {
          if ( dataRequested > 239U )
          {
            dataRequested -= REUSE_RULES;
            data = BIT_CHECK(currentRuleStatus, dataRequested);
          }
          else { data = ProgrammableIOGetData(dataRequested); }
          data2 = configPage13.secondTarget[y];

          if ( (configPage13.operation[y].secondCompType == COMPARATOR_EQUAL) && (data == data2) ) { secondCheck = true; }
          else if ( (configPage13.operation[y].secondCompType == COMPARATOR_NOT_EQUAL) && (data != data2) ) { secondCheck = true; }
          else if ( (configPage13.operation[y].secondCompType == COMPARATOR_GREATER) && (data > data2) ) { secondCheck = true; }
          else if ( (configPage13.operation[y].secondCompType == COMPARATOR_GREATER_EQUAL) && (data >= data2) ) { secondCheck = true; }
          else if ( (configPage13.operation[y].secondCompType == COMPARATOR_LESS) && (data < data2) ) { secondCheck = true; }
          else if ( (configPage13.operation[y].secondCompType == COMPARATOR_LESS_EQUAL) && (data <= data2) ) { secondCheck = true; }
          else if ( (configPage13.operation[y].secondCompType == COMPARATOR_AND) && ((data & data2) != 0) ) { secondCheck = true; }
          else if ( (configPage13.operation[y].secondCompType == COMPARATOR_XOR) && ((data ^ data2) != 0) ) { secondCheck = true; }
  else
  {
    ;
  }

          if (configPage13.operation[y].bitwise == BITWISE_AND) { firstCheck &= secondCheck; }
          if (configPage13.operation[y].bitwise == BITWISE_OR) { firstCheck |= secondCheck; }
          if (configPage13.operation[y].bitwise == BITWISE_XOR) { firstCheck ^= secondCheck; }
        }
      }

      if (BIT_CHECK(configPage13.kindOfLimiting, y))
      {
        if(firstCheck)
        {
          if ((configPage13.outputTimeLimit[y] != 0) && (ioOutDelay[y] >= configPage13.outputTimeLimit[y])) { firstCheck = false; }
        }
        else
        {
          if(BIT_CHECK(currentStatus.outputsStatus, y)) { ioOutDelay[y] = configPage13.outputTimeLimit[y]; }
          else { ioOutDelay[y] = 0; }
        }
      }

      if ( (firstCheck == true) && (configPage13.outputDelay[y] < UINT8_MAX) )
      {
        if (ioDelay[y] >= configPage13.outputDelay[y])
        {
          bool bitStatus = BIT_CHECK(configPage13.outputInverted, y) ^ firstCheck;
          if (BIT_CHECK(currentStatus.outputsStatus, y) && (ioOutDelay[y] < configPage13.outputTimeLimit[y])) { ioOutDelay[y]++; }
          if (configPage13.outputPin[y] < 128) { digitalWrite(configPage13.outputPin[y], bitStatus); }
          else { BIT_WRITE(currentRuleStatus, y, bitStatus); }
          BIT_WRITE(currentStatus.outputsStatus, y, bitStatus);
        }
        else { ioDelay[y]++; }
      }
      else
      {
        if (ioOutDelay[y] >= configPage13.outputTimeLimit[y])
        {
          bool bitStatus = BIT_CHECK(configPage13.outputInverted, y) ^ firstCheck;
          if (configPage13.outputPin[y] < 128) { digitalWrite(configPage13.outputPin[y], bitStatus); }
          else { BIT_WRITE(currentRuleStatus, y, bitStatus); }
          BIT_WRITE(currentStatus.outputsStatus, y, bitStatus);
          if(!BIT_CHECK(configPage13.kindOfLimiting, y)) { ioOutDelay[y] = 0; }
        }

        ioDelay[y] = 0;
      }
    }
  }
}

#endif
