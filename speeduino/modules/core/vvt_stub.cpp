#include "support/preprocessor.h"

#if !FEATURE_MODULE_VVT

#include <stdint.h>

uint16_t vvt_pwm_max_count = 0U;

void initialiseVVT(uint8_t, uint8_t)
{
}

void vvtControl(void)
{
}

void vvtInterrupt(void)
{
}

void vvt1On(void)
{
}

void vvt1Off(void)
{
}

void vvt2On(void)
{
}

void vvt2Off(void)
{
}

uint16_t vvtGetPwmMaxCount(void)
{
  return 0U;
}

void vvtSetVvt2PwmValue(long)
{
}

void vvtSetVvt2PwmState(bool)
{
}

void vvtSetVvt2MaxPwm(bool)
{
}

#endif
