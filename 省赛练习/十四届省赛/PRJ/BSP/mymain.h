#ifndef __MY_MAIN_H__
#define __MY_MAIN_H__

#include "main.h"
#include "adc.h"
#include "tim.h"
#include "gpio.h"
#include "lcd.h"
#include "stdio.h"
#include "string.h"


#include "led.h" 
#include "key.h"

extern struct BKEY bkey[];

void setup(void);
void loop(void);
#endif



