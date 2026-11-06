#ifndef __MYMAIN_H
#define __MYMAIN_H


#include "main.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "gpio.h"
#include "lcd.h"
#include "stdio.h"
#include "usart.h"
#include "string.h"

#include "led_app.h"
#include "key_app.h"

void Loop(void);
void SetUp(void);

extern struct BKEY Bkey[];

#endif




