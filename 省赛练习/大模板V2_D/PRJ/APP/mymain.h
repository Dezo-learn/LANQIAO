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
#include "i2c_hal.h"

#include "Led_App.h"
#include "Key_App.h"


void Setup(void);
void Loop(void);

extern struct BKEY bkey[];


#endif

