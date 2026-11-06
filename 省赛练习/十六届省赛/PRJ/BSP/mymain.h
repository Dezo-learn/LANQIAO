#ifndef __MYMAIN__
#define __MYMAIN__


#include "main.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "gpio.h"
#include "lcd.h"
#include "stdio.h"
#include "rtc.h"

#include "led_app.h"
#include "key_app.h"

void Loop(void);
void SetUp(void);

extern struct BKEY bkey[];
#endif

