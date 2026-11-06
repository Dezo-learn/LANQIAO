#include "led_app.h"

uint8_t ucled = 0x00;

void LED_Disp(uint8_t dsLED)
{
    HAL_GPIO_WritePin(GPIOC,GPIO_PIN_All,GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOC,dsLED<<8,GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
}

void LED_Chg(uint8_t num,uint8_t sta)
{
    uint8_t pos = 0x01;
    pos = pos << (num - 1);
    ucled =(ucled & (~pos)) | (pos * sta);
    LED_Disp(ucled);
}

