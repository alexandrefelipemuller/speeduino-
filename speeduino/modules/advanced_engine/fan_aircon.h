#pragma once

#include <stdint.h>

void initialiseFan(uint8_t fan_pin);
void initialiseAirCon(void);
void fanControl(void);
void airConControl(void);
void fanOn(void);
void fanOff(void);

void advanced_engine_fan_aircon_init(uint8_t fan_pin);
void advanced_engine_fan_aircon_tick_10hz(void);
void advanced_engine_fan_aircon_tick_1hz(void);

#if defined(PWM_FAN_AVAILABLE)
extern uint16_t fan_pwm_max_count;
void fanInterrupt(void);
#endif
