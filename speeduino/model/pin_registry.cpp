#include "data/pin_registry.h"
#include "data/runtime_constants.h"
#include "data/runtime_state.h"
#include "data/tune_registry.h"

byte pinInjector1;
byte pinInjector2;
byte pinInjector3;
byte pinInjector4;
byte pinInjector5;
byte pinInjector6;
byte pinInjector7;
byte pinInjector8;
byte injectorOutputControl = OUTPUT_CONTROL_DIRECT;
byte pinCoil1;
byte pinCoil2;
byte pinCoil3;
byte pinCoil4;
byte pinCoil5;
byte pinCoil6;
byte pinCoil7;
byte pinCoil8;
byte ignitionOutputControl = OUTPUT_CONTROL_DIRECT;
byte pinTrigger;
byte pinTrigger2;
byte pinTrigger3;
byte pinTPS;
byte pinMAP;
byte pinEMAP;
byte pinMAP2;
byte pinIAT;
byte pinCLT;
byte pinO2;
byte pinO2_2;
byte pinBat;
byte pinDisplayReset;
byte pinTachOut;
byte pinFuelPump;
byte pinIdle1;
byte pinIdle2;
byte pinIdleUp;
byte pinIdleUpOutput;
byte pinCTPS;
byte pinFuel2Input;
byte pinSpark2Input;
byte pinSpareTemp1;
byte pinSpareTemp2;
byte pinSpareOut1;
byte pinSpareOut2;
byte pinSpareOut3;
byte pinSpareOut4;
byte pinSpareOut5;
byte pinSpareOut6;
byte pinSpareHOut1;
byte pinSpareHOut2;
byte pinSpareLOut1;
byte pinSpareLOut2;
byte pinSpareLOut3;
byte pinSpareLOut4;
byte pinSpareLOut5;
byte pinBoost;
byte pinVVT_1;
byte pinVVT_2;
byte pinFan;
byte pinStepperDir;
byte pinStepperStep;
byte pinStepperEnable;
byte pinLaunch;
byte pinIgnBypass;
byte pinFlex;
byte pinVSS;
byte pinBaro;
byte pinResetControl;
byte pinFuelPressure;
byte pinOilPressure;
byte pinWMIEmpty;
byte pinWMIIndicator;
byte pinWMIEnabled;
byte pinMC33810_1_CS;
byte pinMC33810_2_CS;
byte pinSDEnable;
#ifdef USE_SPI_EEPROM
byte pinSPIFlash_CS;
#endif
byte pinAirConComp;
byte pinAirConFan;
byte pinAirConRequest;

bool pinIsOutput(byte pin)
{
  bool used = false;
  bool isIdlePWM = (configPage6.iacAlgorithm > 0) && ((configPage6.iacAlgorithm <= 3) || (configPage6.iacAlgorithm == 6));
  bool isIdleSteper = (configPage6.iacAlgorithm > 3) && (configPage6.iacAlgorithm != 6);
  if ((pin == pinInjector1)
  || ((pin == pinInjector2) && (configPage2.nInjectors > 1))
  || ((pin == pinInjector3) && (configPage2.nInjectors > 2))
  || ((pin == pinInjector4) && (configPage2.nInjectors > 3))
  || ((pin == pinInjector5) && (configPage2.nInjectors > 4))
  || ((pin == pinInjector6) && (configPage2.nInjectors > 5))
  || ((pin == pinInjector7) && (configPage2.nInjectors > 6))
  || ((pin == pinInjector8) && (configPage2.nInjectors > 7)))
  {
    used = true;
  }
  if ((pin == pinCoil1)
  || ((pin == pinCoil2) && (currentStatus.maxIgnOutputs > 1))
  || ((pin == pinCoil3) && (currentStatus.maxIgnOutputs > 2))
  || ((pin == pinCoil4) && (currentStatus.maxIgnOutputs > 3))
  || ((pin == pinCoil5) && (currentStatus.maxIgnOutputs > 4))
  || ((pin == pinCoil6) && (currentStatus.maxIgnOutputs > 5))
  || ((pin == pinCoil7) && (currentStatus.maxIgnOutputs > 6))
  || ((pin == pinCoil8) && (currentStatus.maxIgnOutputs > 7)))
  {
    used = true;
  }
  if ((pin == pinFuelPump)
  || ((pin == pinFan) && (configPage2.fanEnable == 1))
  || ((pin == pinVVT_1) && (configPage6.vvtEnabled > 0))
  || ((pin == pinVVT_2) && (configPage10.wmiEnabled > 0))
  || ((pin == pinVVT_2) && (configPage10.vvt2Enabled > 0))
  || ((pin == pinBoost) && (configPage6.boostEnabled == 1))
  || ((pin == pinIdle1) && isIdlePWM)
  || ((pin == pinIdle2) && isIdlePWM && (configPage6.iacChannels == 1))
  || ((pin == pinStepperEnable) && isIdleSteper)
  || ((pin == pinStepperStep) && isIdleSteper)
  || ((pin == pinStepperDir) && isIdleSteper)
  || (pin == pinTachOut)
  || ((pin == pinAirConComp) && (configPage15.airConEnable > 0))
  || ((pin == pinAirConFan) && (configPage15.airConEnable > 0) && (configPage15.airConFanEnabled > 0)) )
  {
    used = true;
  }
  if ( pinIsReserved(pin) ) { used = true; }

  return used;
}

#define pinIsSensor(pin)    ( ((pin) == pinCLT) || ((pin) == pinIAT) || ((pin) == pinMAP) || ((pin) == pinTPS) || ((pin) == pinO2) || ((pin) == pinBat) || (((pin) == pinFlex) && (configPage2.flexEnabled != 0)) )

bool pinIsUsed(byte pin)
{
  bool used = false;
  if ( pinIsSensor(pin) )
  {
    used = true;
  }
  if ( pinIsOutput(pin) )
  {
    used = true;
  }
  return used;
}
