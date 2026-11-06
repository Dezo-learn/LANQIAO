#include "mymain.h"


char text[30];
uint16_t i = 5;
uint8_t led_sta;

void setup(void){
    HAL_TIM_Base_Start_IT(&htim6);
    
    LED_Disp(0X00);
    LCD_Init();
    LCD_SetBackColor(Black);
    LCD_SetTextColor(White);
    LCD_Clear(Black);
    LED_Chg(4,1);

}

void loop(void){
    LCD_DisplayStringLine(Line4, (unsigned char *)"    Hello,world.   ");
    sprintf(text,"CNBR:%d     ",i);
    LCD_DisplayStringLine(Line9, (unsigned char *)text);
    
    if(bkey[1].short_flag == 1)
    {
      LCD_DisplayStringLine(Line5, (unsigned char *)" KEY1 DOWN     ");
      bkey[1].short_flag =0;
    }
    if(bkey[2].short_flag == 1)
    {
      LCD_DisplayStringLine(Line5, (unsigned char *)" KEY2 DOWN     ");
      bkey[2].short_flag =0;
    }
    if(bkey[3].long_flag == 1)
    {
      LCD_DisplayStringLine(Line5, (unsigned char *)" KEY3 HOLD");
      bkey[3].long_flag =0;
    }
    if(bkey[4].long_flag == 1)
    {
      LCD_DisplayStringLine(Line5, (unsigned char *)" KEY4 HOLD");
      bkey[4].long_flag =0;
    }
    if(bkey[1].double_flag == 1)
    {
      LCD_DisplayStringLine(Line5, (unsigned char *)" KEY1 Double");
      bkey[1].double_flag =0;
    }
    if(bkey[2].double_flag == 1)
    {
      LCD_DisplayStringLine(Line5, (unsigned char *)" KEY2 Double");
      bkey[2].double_flag =0;
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
//*********************×èÈû°´¼ü****************************
//  if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0) == GPIO_PIN_RESET)
//  {
//    HAL_Delay(10);
//    if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0) == GPIO_PIN_RESET)
//    {
//      led_sta = (led_sta & ~0x01) | 0x01;
//      led_sta = (led_sta & ~0x02) | 0x00;
//      LED_Disp(led_sta);
//    }
//  }

//  if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1) == GPIO_PIN_RESET)
//  {
//    HAL_Delay(10);
//    if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1) == GPIO_PIN_RESET)
//    {
//      led_sta = (led_sta & ~0x02) | 0x02;
//      led_sta = (led_sta & ~0x01) | 0x00;
//      LED_Disp(led_sta);
//    }
//  }
//*********************·Ç×èÈû°´¼ü***************************
  key_serv_double();
  
  HAL_Delay();
}

