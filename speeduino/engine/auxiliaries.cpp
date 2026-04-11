/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
*/
#include "engine/auxiliaries.h"
#include "data/advanced_engine_status.h"
#include "support/maths.h"
#include "modules/vvt/vvt.h"
#include "modules/advanced_engine/nitrous.h"
#include "src/PID_v1/PID_v1.h"
#include "engine/decoders.h"
#include "orchestration/timers.h"
#include "support/preprocessor.h"
#include "support/units.h"
#include "boards/board_definition.h"
#include "support/atomic.h"
#include "data/pin_registry.h"
#include "support/port_pin.h"
#include "support/utilities.h"
#include "data/runtime_state.h"
#include "data/table_registry.h"
#include "data/tune_registry.h"

bool pinIsOutput(byte pin);

/*
 * Fuel pump control
 */

#if(defined(CORE_TEENSY) || defined(CORE_STM32))
#define FUEL_PUMP_PIN_HIGH()  digitalWrite(pinFuelPump, HIGH)
#define FUEL_PUMP_PIN_LOW()   digitalWrite(pinFuelPump, LOW) 
static inline void initialisePumpPin(uint8_t pin) 
{ 
  pinMode(pin, OUTPUT);
}
#else

static port_register_t pump_pin_port;
static pin_mask_t pump_pin_mask;

static inline void initialisePumpPin(uint8_t pin) 
{ 
  pinMode(pin, OUTPUT);

  pump_pin_port = portOutputRegister(digitalPinToPort(pin));
  pump_pin_mask = digitalPinToBitMask(pin);
}

#define FUEL_PUMP_PIN_HIGH() *pump_pin_port |= (pump_pin_mask)
#define FUEL_PUMP_PIN_LOW()  *pump_pin_port &= ~(pump_pin_mask)

#endif

void fuelPumpOn(void)
{
  ATOMIC() { 
    FUEL_PUMP_PIN_HIGH();
    currentStatus.fuelPumpOn = true;
  }
}
void fuelPumpOff(void)
{
  ATOMIC() { 
    FUEL_PUMP_PIN_LOW();
    currentStatus.fuelPumpOn = false;
  }
}

bool initialiseFuelPump(const config2 &page2, uint8_t pumpPin)
{
  initialisePumpPin(pumpPin);
  fuelPumpOff();  //Initialise program with the fuel pump in the off state

  //Begin priming the fuel pump. This is turned off in the low resolution, 1s interrupt in timers.ino
  //First check that the priming time is not 0
  if(page2.fpPrime>0U)
  {
    fuelPumpOn();
    return false; // Priming not complete
  }
  return true; //If the user has set 0 for the pump priming, immediately mark the priming as being completed
}


void initialiseAuxPWM(void)
{
  initialiseVVT(pinVVT_1, pinVVT_2);
}

// Water methanol injection control
