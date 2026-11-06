#include "led.h"


uint8_t ucled = 0x00;

void Led_Disp(uint8_t ddsled)
{
  HAL_GPIO_WritePin(GPIOC,GPIO_PIN_All,GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOC,ddsled<<8,GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
}

void Led_serv(uint8_t num,uint8_t sta)
{
  uint8_t pos = 0x01;
  pos = pos << (num -1);
  ucled = (ucled &( ~pos)) | (pos * sta);
  Led_Disp(ucled);
}





