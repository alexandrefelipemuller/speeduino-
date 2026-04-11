#include "support/preprocessor.h"

#if !FEATURE_MODULE_BOOST

#include <stdint.h>

uint16_t boost_pwm_max_count = 0U;

void initialiseBoost(uint8_t)
{
}

void boostControl(void)
{
}

void boostDisable(void)
{
}

void boostInterrupt(void)
{
}

#endif
