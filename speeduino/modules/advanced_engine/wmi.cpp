#include "modules/advanced_engine/wmi.h"

#include "advanced_engine_status.h"
#include "board_definition.h"
#include "maths.h"
#include "modules/services/aux_pwm.h"
#include "pin_registry.h"
#include "runtime_state.h"
#include "table_registry.h"
#include "tune_registry.h"
#include "units.h"

#define WMI_TANK_IS_EMPTY() ((configPage10.wmiEmptyEnabled) ? ((configPage10.wmiEmptyPolarity) ? digitalRead(pinWMIEmpty) : !digitalRead(pinWMIEmpty)) : 1)

void wmiControl(void)
{
  int wmiPW = 0;

  if((configPage10.vvt2Enabled == 0) && (configPage10.wmiEnabled >= 1))
  {
    if(WMI_TANK_IS_EMPTY())
    {
      currentAdvancedEngineStatus.wmi_tank_empty = false;
      if((currentStatus.TPS >= configPage10.wmiTPS) &&
         (currentStatus.RPMdiv100 >= configPage10.wmiRPM) &&
         ((currentStatus.MAP / 2) >= configPage10.wmiMAP) &&
         (temperatureAddOffset(currentStatus.IAT) >= configPage10.wmiIAT))
      {
        switch(configPage10.wmiMode)
        {
          case WMI_MODE_SIMPLE:
            wmiPW = 200;
            break;
          case WMI_MODE_PROPORTIONAL:
            wmiPW = map(currentStatus.MAP / 2, configPage10.wmiMAP, configPage10.wmiMAP2, 0, 200);
            break;
          case WMI_MODE_OPENLOOP:
            wmiPW = get3DTableValue(&wmiTable, (uint16_t)currentStatus.MAP, currentStatus.RPM);
            break;
          case WMI_MODE_CLOSEDLOOP:
            wmiPW = max(0, ((int)currentStatus.PW1 + configPage10.wmiOffset)) * get3DTableValue(&wmiTable, (uint16_t)currentStatus.MAP, currentStatus.RPM) / 200;
            break;
          default:
            wmiPW = 0;
            break;
        }
        if(wmiPW > 200) { wmiPW = 200; }
      }
    }
    else
    {
      currentAdvancedEngineStatus.wmi_tank_empty = true;
    }

    currentAdvancedEngineStatus.wmi_pw = wmiPW;
    auxPwmSetSecondaryPwmValue(halfPercentage(currentAdvancedEngineStatus.wmi_pw, auxPwmGetMaxCount()));

    if(wmiPW == 0)
    {
      auxPwmSecondaryOff();
      auxPwmSetSecondaryPwmState(false);
      auxPwmSetSecondaryMaxPwm(false);
      if(configPage6.vvtEnabled == 0) { auxPwmDisableTimer(); }
      digitalWrite(pinWMIEnabled, LOW);
    }
    else
    {
      digitalWrite(pinWMIEnabled, HIGH);
      if(wmiPW >= 200)
      {
        auxPwmSecondaryOn();
        auxPwmSetSecondaryPwmState(true);
        auxPwmSetSecondaryMaxPwm(true);
        if(configPage6.vvtEnabled == 0) { auxPwmDisableTimer(); }
      }
      else
      {
        auxPwmSetSecondaryMaxPwm(false);
        auxPwmEnableTimer();
      }
    }
  }
}
