/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
*/
#include "auxiliaries.h"
#include "advanced_engine_status.h"
#include "maths.h"
#include "modules/advanced_engine/boost.h"
#include "modules/advanced_engine/fan_aircon.h"
#include "modules/advanced_engine/nitrous.h"
#include "modules/advanced_engine/vvt.h"
#include "src/PID_v1/PID_v1.h"
#include "decoders.h"
#include "timers.h"
#include "preprocessor.h"
#include "units.h"
#include "board_definition.h"
#include "atomic.h"
#include "pin_registry.h"
#include "port_pin.h"
#include "utilities.h"
#include "runtime_state.h"
#include "table_registry.h"
#include "tune_registry.h"

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
  initialiseBoost(pinBoost);
  initialiseVVT(pinVVT_1, pinVVT_2);
  initialiseNitrous(configPage10);
}

// Water methanol injection control
