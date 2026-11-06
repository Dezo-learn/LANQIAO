#include "key_app.h"

struct BKEY bkey[5] = {0};

uint8_t Key_Read(void)
{
    if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0) == GPIO_PIN_RESET)return 1;
    else if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1) == GPIO_PIN_RESET)return 2;
    else if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2) == GPIO_PIN_RESET)return 3;
    else if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0) == GPIO_PIN_RESET)return 4;
    else return 0;
}


void key_serv(void)
{
    uint8_t key_value = Key_Read();
    if(key_value) bkey[key_value].age++;
    else
    {   
        for(uint8_t i = 0;i < 5;i++)
        {
            if(bkey[i].age >= 200) bkey[i].long_flag = 1;
            else if(bkey[i].age >= 3) bkey[i].short_flag = 1;
            bkey[i].age = 0;
        }

    }

}



