#ifndef __MYMAIN_H__
#define __MYMAIN_H__

#include "main.h"
#include "tim.h"
#include "gpio.h"
#include "stdio.h"

#include "key.h"
#include "led.h"
#include "lcd.h"

extern struct Bkey bkey[5];

void SetUp(void);
void Loop(void);


#endif
