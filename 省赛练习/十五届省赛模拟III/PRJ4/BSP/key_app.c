#include "key_app.h"

struct BKEY Bkey[5] = {0};

uint8_t Key_Read(void)
{
    if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0) == GPIO_PIN_RESET) return 1;
    else if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1) == GPIO_PIN_RESET) return 2;
    else if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2) == GPIO_PIN_RESET) return 3;
    else if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0) == GPIO_PIN_RESET) return 4;
    else return 0;
}

void Key_Porc(void)
{
    uint8_t key_val = Key_Read();
    if(key_val) Bkey[key_val].age++;
    else
    {
        for(uint8_t i = 0;i < 5;i++)
        {
            if(Bkey[i].age >= 300) Bkey[i].long_flag = 1;
            else if(Bkey[i].age >= 3)Bkey[i].short_flag = 1;
            Bkey[i].age = 0;
        }
    }
}


