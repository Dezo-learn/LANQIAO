#include "key.h"

struct Bkey bkey[5] = {0};


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
    uint8_t Key_val = Key_Read();
    if(Key_val != 0) bkey[Key_val].age++;
    else 
    {
        for(uint8_t i = 0 ; i <=4 ; i++)
        {   
            if(bkey[i].age >= 100) bkey[i].long_flag = 1;
            else if(bkey[i].age >= 3) bkey[i].short_flag = 1;
            bkey[i].age = 0;
        }
    }

}


