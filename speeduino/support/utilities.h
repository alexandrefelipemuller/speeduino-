/*
These are some utility functions and variables used through the main code
*/
#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

#define COMPARATOR_EQUAL 0
#define COMPARATOR_NOT_EQUAL 1
#define COMPARATOR_GREATER 2
#define COMPARATOR_GREATER_EQUAL 3
#define COMPARATOR_LESS 4
#define COMPARATOR_LESS_EQUAL 5
#define COMPARATOR_AND 6
#define COMPARATOR_XOR 7

#define BITWISE_DISABLED 0
#define BITWISE_AND 1
#define BITWISE_OR 2
#define BITWISE_XOR 3

#define REUSE_RULES 240
#define PROGRAMMABLE_IO_CHANNELS 8U

extern uint8_t ioOutDelay[PROGRAMMABLE_IO_CHANNELS];
extern uint8_t ioDelay[PROGRAMMABLE_IO_CHANNELS];
extern uint8_t pinIsValid;
extern uint8_t currentRuleStatus;
//uint8_t outputPin[PROGRAMMABLE_IO_CHANNELS];

void setResetControlPinState(void);
byte pinTranslate(byte rawPin);
byte pinTranslateAnalog(byte rawPin);
int16_t ProgrammableIOGetData(uint16_t index);

#endif // UTILS_H
