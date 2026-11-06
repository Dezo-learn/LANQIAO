#include "mymain.h"

/**************模块**************/
struct BKEY{
    uint16_t timer;
    uint16_t double_timer;

    uint8_t short_flag;
    uint8_t long_flag;
    uint8_t double_flag;
};

struct BKEY bkey[5] = {0};

void Led_Disp(uint8_t ucled)
{
    HAL_GPIO_WritePin(GPIOC,GPIO_PIN_All,GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOC,ucled << 8,GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
}

uint8_t Key_Read(void)
{
    if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0) == GPIO_PIN_RESET)return 1;
    else if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1) == GPIO_PIN_RESET)return 2;
    else if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2) == GPIO_PIN_RESET)return 3;
    else if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0) == GPIO_PIN_RESET)return 4;
    else return 0;
}

void Key_Serv(void)
{
    uint8_t key_value = Key_Read();
    for(uint8_t i = 1 ; i < 5 ; i++)
    {
        if(key_value == i) bkey[i].timer++;
        else
        {
            if(bkey[i].timer > 100) bkey[i].long_flag = 1;
            else if(bkey[i].timer >= 3) bkey[i].short_flag = 1;
            bkey[i].timer = 0;
        }
    }
}
/**************主调**************/

char lcd_temp[50];
char uart_rx[50];
char uart_tx[50];
uint32_t adc2_value[1];
float R37_Value;
uint8_t lcd_page = 0;
float D = 0;
uint8_t NAV = 0; //0:直行 1:左转 2:右转
uint8_t NAV_Old = 0; //0:直行 1:左转 2:右转
uint32_t nav_timer = 0;
uint8_t nav_c; // 完成操作标志
uint8_t nav_error; //错误标志
uint8_t ucled;
uint16_t Led_timer = 1;

void SetUp(void)
{
    HAL_TIM_Base_Start_IT(&htim6);
    HAL_ADCEx_Calibration_Start(&hadc2,ADC_SINGLE_ENDED);
    HAL_ADC_Start_DMA(&hadc2,adc2_value,1);
    HAL_UARTEx_ReceiveToIdle_IT(&huart1,(uint8_t *)uart_rx,50);
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
//模拟功能
    R37_Value = adc2_value[0] * 3.3f / 65535.0f;
    if(R37_Value < 3.0f) D = R37_Value * 100.0f;
    else D = 300.0f;

//按键功能
    if(bkey[1].short_flag == 1) //调头
    {
        if(lcd_page == 1)
        {
            lcd_page = 0;
            NAV = 0;
            sprintf(uart_tx , "Success\r\n");
            HAL_UART_Transmit(&huart1,(uint8_t *)uart_tx,sizeof(uart_tx),50);
        }
        bkey[1].short_flag = 0;
    }

    if(bkey[3].short_flag == 1)
    {
        if(lcd_page == 0)
        {
            if(NAV == 1 && nav_timer > 0)//如果左转
            {
                nav_c = 1;
                nav_timer = 0;
                NAV = 0;
            }
            else nav_error = 1;
        }

        bkey[3].short_flag = 0;
    }

    if(bkey[4].short_flag == 1)
    {
        if(lcd_page == 0)
        {
            if(NAV == 2 && nav_timer > 0)//如果左转
            {
                nav_c = 1;
                nav_timer = 0;
                NAV = 0;
            }
            else nav_error = 1;
        }
        bkey[4].short_flag = 0;
    }


//标志位处理
    if(nav_error)
    {
        sprintf(uart_tx , "Warn\r\n");
        HAL_UART_Transmit(&huart1,(uint8_t *)uart_tx,sizeof(uart_tx),50);
        lcd_page = 1;
        nav_error = 0;
        nav_timer = 0;
    }
    if(nav_c)
    {
        sprintf(uart_tx , "Success\r\n");
        HAL_UART_Transmit(&huart1,(uint8_t *)uart_tx,sizeof(uart_tx),50);
        nav_c = 0;
    }

//Lcd显示 0车辆行驶数据界面 1偏航
    switch(lcd_page)
    {
        case 0:
        {
            LCD_DisplayStringLine(Line1,(unsigned char *)"        DATA        ");
            if(NAV == 0)
            LCD_DisplayStringLine(Line3,(unsigned char *)"       N:S         ");
            if(NAV == 1)
            LCD_DisplayStringLine(Line3,(unsigned char *)"       N:L         ");
            if(NAV == 2)
            LCD_DisplayStringLine(Line3,(unsigned char *)"       N:R         ");     
            sprintf(lcd_temp,"       D:%.2f         ",D);
            LCD_DisplayStringLine(Line4,(unsigned char *)lcd_temp);
        break;
        }
        case 1:
        {
            LCD_DisplayStringLine(Line1,(unsigned char *)"                    ");
            LCD_DisplayStringLine(Line3,(unsigned char *)"                    ");
            LCD_DisplayStringLine(Line4,(unsigned char *)"        WARN        ");

        break;
        }
    }


//LED
    if(NAV == 0)
    {
        ucled = (ucled & ~0x01) | 0x00; 
        ucled = (ucled & ~0x02) | 0x00;
    }
    if(NAV == 1)
    {
        if(Led_timer <= 10)  //timer <= 10 时LED1点亮
        ucled = (ucled & ~0x01) | 0x01;
        else
        ucled = (ucled & ~0x01) | 0x00;
    }
    if(NAV == 2)
    {
        if(Led_timer <= 10)
        ucled = (ucled & ~0x02) | 0x02;
        else
        ucled = (ucled & ~0x02) | 0x00;
    }
    if(lcd_page == 1)
    {
        ucled = (ucled & ~0x80) | 0x80;
    }
    else
    {
        ucled = (ucled & ~0x80) | 0x00;
    }



    Led_Disp(ucled);

//测试


//    sprintf(lcd_temp," NAV:%d",NAV);
//    LCD_DisplayStringLine(Line9,(unsigned char *)lcd_temp);

//    
//    sprintf(lcd_temp," timer:%d  ",nav_timer);
//    LCD_DisplayStringLine(Line8,(unsigned char *)lcd_temp);
}



//TIM中断服务函数
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim == &htim6)  Key_Serv();
    if(nav_timer > 0)
    {
        nav_timer --;
        if(nav_timer == 0) nav_error = 1;
    }
    Led_timer++;
    if(Led_timer == 21) Led_timer = 1;

}

//串口中断服务函数
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if(lcd_page == 0)
    {
        if (strcmp(uart_rx,"L") == 0 && nav_timer == 0)
        {
            NAV = 1;
            nav_timer = 500;
        }
        else if (strcmp(uart_rx,"R") == 0 && nav_timer == 0)
        {
            NAV = 2;
            nav_timer = 500;
        }
        else if(strcmp(uart_rx,"R") == 0 || strcmp(uart_rx,"L") == 0)
        {
            //无回应
        }
        else 
        {
            sprintf(uart_tx , "ERROR\r\n");
            HAL_UART_Transmit(&huart1,(uint8_t *)uart_tx,sizeof(uart_tx),50);
        }
    }
    else
    {
        sprintf(uart_tx , "WAIT\r\n");
        HAL_UART_Transmit(&huart1,(uint8_t *)uart_tx,sizeof(uart_tx),50);
    }

    memset(uart_rx,0,sizeof(uart_rx));
    HAL_UARTEx_ReceiveToIdle_IT(&huart1,(uint8_t *)uart_rx,50);
}
