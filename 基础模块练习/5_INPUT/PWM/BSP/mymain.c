#include "mymain.h"


char text[30];
uint16_t i = 5;
uint8_t led_sta;
uint32_t time500ms;




void setup(void){
    HAL_TIM_Base_Start_IT(&htim6);
    HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_2);
    HAL_TIM_IC_Start(&htim15,TIM_CHANNEL_1);
    HAL_TIM_IC_Start(&htim15,TIM_CHANNEL_2);
    LED_Disp(0X00);
    LCD_Init();
    LCD_SetBackColor(Black);
    LCD_SetTextColor(White);
    LCD_Clear(Black);
    LED_Chg(4,1);
    
    time500ms = uwTick;
}

void loop(void){
    float frq1 = 0;
    float duty1 = 0;
    frq1 = 1000000.0f/ (HAL_TIM_ReadCapturedValue(&htim15,TIM_CHANNEL_1) + 1);
    duty1 = (HAL_TIM_ReadCapturedValue(&htim15,TIM_CHANNEL_2) + 1) * 100.0f / (HAL_TIM_ReadCapturedValue(&htim15,TIM_CHANNEL_1) + 1);

    
    sprintf(text,"FREQ:%.2f",frq1);
    LCD_DisplayStringLine(Line4, (unsigned char *)"    Hello,world.   ");
//    sprintf(text,"CNBR:%d     ",i);
    LCD_DisplayStringLine(Line1, (unsigned char *)text);
    sprintf(text,"DUTY:%.2f%%",duty1);
    LCD_DisplayStringLine(Line2, (unsigned char *)text);
    
    if(bkey[1].short_flag == 1)
    {
      LCD_DisplayStringLine(Line5, (unsigned char *)" KEY1 DOWN     ");
      bkey[1].short_flag =0;
      __HAL_TIM_SET_AUTORELOAD(&htim2,250-1);
      __HAL_TIM_SetCompare(&htim2,TIM_CHANNEL_2,125);
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
    if(uwTick - time500ms > 500)
    {
     static uint8_t LED_sta = 0;
     LED_Chg(2,LED_sta);
     LED_sta = !LED_sta;
     time500ms = uwTick;
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
  
}

