#ifndef __MYMAIN_H
#define __MYMAIN_H


#include "main.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "stdio.h"
#include "lcd.h"
#include "math.h"
#include "string.h"
#include "i2c_hal.h"

extern uint16_t LCDR1;
extern uint16_t LCDR96;

void Loop(void);
void SetUp(void);


#endif

