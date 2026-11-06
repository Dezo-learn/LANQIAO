#include "mymain.h"

void Led_Disp(uint8_t ucled)
{
    HAL_GPIO_WritePin(GPIOC,GPIO_PIN_All,GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOC,ucled << 8,GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
}

struct BKEY
{
    uint16_t press_time;
    uint8_t short_flag;
    uint8_t long_flag;
};

struct BKEY bkey[5] = {0};

uint8_t Key_Read(void)
{
    if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0) == GPIO_PIN_RESET) return 1;
    else if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1) == GPIO_PIN_RESET) return 2;
    else if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2) == GPIO_PIN_RESET) return 3;
    else if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0) == GPIO_PIN_RESET) return 4;
    else return 0;
}




void Key_Serv(void)
{
    uint8_t key_value = Key_Read();
    for(uint8_t i = 1; i < 5;i++)
    {
        if(key_value == i) bkey[i].press_time ++;
        else
        {
            if(bkey[i].press_time >= 150) bkey[i].long_flag = 1;
            else if(bkey[i].press_time >= 3)bkey[i].short_flag = 1;
            bkey[i].press_time = 0;
        }
    }


}

void SetUp(void)
{
    HAL_TIM_Base_Start_IT(&htim6);

    Led_Disp(0x00);
    LCD_Init();
    HAL_Delay(100);;
    LCD_SetBackColor(Black);
    LCD_SetTextColor(White);
    LCD_Clear(Black);
    HAL_Delay(100);

    
}

void Loop(void)
{
    if(bkey[1].short_flag == 1)
    {
        Led_Disp(0x01);
        bkey[1].short_flag = 0;
    }

}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim == &htim6) Key_Serv();
}

