#ifndef _MYMAIN_
#define _MYMAIN_

#include "main.h"
#include "gpio.h"
#include "adc.h"
#include "tim.h"
#include "lcd.h"
#include "stdio.h"
#include "string.h"
#include "usart.h"
#include "i2c_hal.h"

#include "led_app.h"
#include "key_app.h"
#include "adc_app.h"

extern uint8_t ucled;
extern struct Bkeys bkey[];



void setup(void);
void loop(void);
uint8_t key_read(void);
#endif

