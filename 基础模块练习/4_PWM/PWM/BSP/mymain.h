#ifndef _MYMAIN_
#define _MYMAIN_

#include "main.h"
#include "gpio.h"
#include "tim.h"
#include "lcd.h"
#include "stdio.h"


#include "led_app.h"
#include "key_app.h"


extern uint8_t ucled;
extern struct Bkeys bkey[];



void setup(void);
void loop(void);

#endif

