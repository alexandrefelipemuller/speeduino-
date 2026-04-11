#pragma once

#include <stdint.h>

void initialiseVVT(uint8_t pin1, uint8_t pin2);
void vvtControl(void);
void vvtInterrupt(void);
void vvt1On(void);
void vvt1Off(void);
void vvt2On(void);
void vvt2Off(void);

uint16_t vvtGetPwmMaxCount(void);
void vvtSetVvt2PwmValue(long pwm_value);
void vvtSetVvt2PwmState(bool pwm_state);
void vvtSetVvt2MaxPwm(bool max_pwm);

extern uint16_t vvt_pwm_max_count;
