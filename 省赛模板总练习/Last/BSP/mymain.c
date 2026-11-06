#include "mymain.h"

void Led_Disp(uint8_t ucled)
{
    HAL_GPIO_WritePin(GPIOC,0xFF00,GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOC,ucled << 8,GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
}
struct BKEY
{
    uint16_t press_time;
    uint16_t double_time;
    
    uint8_t short_flag;
    uint8_t long_flag;
    uint8_t double_flag;
};

struct BKEY bkey[5] = {0};

uint8_t Key_Read(void)
{
    if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0) == GPIO_PIN_RESET) return 1;
    else if (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1) == GPIO_PIN_RESET) return 2;
    else if (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2) == GPIO_PIN_RESET) return 3;
    else if (HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0) == GPIO_PIN_RESET) return 4;
    else return 0;
}

//void Key_Serv(void) //长按后操作
//{
//    uint8_t key_value = Key_Read();
//    for(uint8_t i = 1; i < 5;i++)
//    {
//        if(i == key_value)
//        {
//            bkey[i].press_time++;
//        }
//        else
//        {
//            if(bkey[i].press_time >= 100)
//            {
//                bkey[i].long_flag = 1;
//                bkey[i].press_time = 0;
//            }
//            else if(bkey[i].press_time >= 3)
//            {
//                if(bkey[i].double_time > 0)
//                {
//                    bkey[i].double_flag = 1;
//                    bkey[i].double_time = 0;
//                }
//                else
//                {
//                    bkey[i].double_time = 20;
//                }
//                bkey[i].press_time = 0;
//            }

//            if(bkey[i].double_time > 0)
//            {
//                bkey[i].double_time --;
//                if(bkey[i].double_time == 0)
//                {
//                    bkey[i].short_flag = 1;
//                }
//            }
//        }
//    }
//}

void Key_Serv(void) //长按时操作
{
    uint8_t key_value = Key_Read();
    for(uint8_t i = 1; i < 5;i++)
    {
        if(i == key_value)
        {
            bkey[i].press_time++;
            if(bkey[i].press_time >= 100)
            {
                bkey[i].long_flag = 1;
                bkey[i].press_time = 0;
            }
        }
        else
        {
            if(bkey[i].press_time >= 3 && bkey[i].press_time < 100)
            {
                if(bkey[i].double_time > 0)
                {
                    bkey[i].double_flag = 1;
                    bkey[i].double_time = 0;
                }
                else
                {
                    bkey[i].double_time = 20;
                }
                bkey[i].press_time = 0;
            }

            if(bkey[i].double_time > 0)
            {
                bkey[i].double_time --;
                if(bkey[i].double_time == 0)
                {
                    bkey[i].short_flag = 1;
                }
            }
        }
    }
}

void MCP_Write(uint8_t dat)
{
    I2CStart();
    I2CSendByte(0x5e);
    I2CWaitAck();
    I2CSendByte(dat);
    I2CWaitAck();
    I2CStop();
}

//*************主调*************
uint8_t count;
char lcd_char[50];
uint8_t lcd_page;
float R37_value;
float R38_value;
uint32_t adc1[2];
uint32_t adc2[1];
float freq;
float duty;
float MCP_value;
RTC_TimeTypeDef time;
RTC_DateTypeDef date;
uint8_t line_select;

void SetUp(void)
{
//**TIM IT
    HAL_TIM_Base_Start_IT(&htim6);
//**ADC
    HAL_ADCEx_Calibration_Start(&hadc1,ADC_SINGLE_ENDED);
    HAL_ADCEx_Calibration_Start(&hadc2,ADC_SINGLE_ENDED);
    HAL_ADC_Start_DMA(&hadc1,adc1,2);
    HAL_ADC_Start_DMA(&hadc2,adc2,1);

//**IC
    HAL_TIM_IC_Start(&htim2,TIM_CHANNEL_2);
    HAL_TIM_IC_Start(&htim2,TIM_CHANNEL_1);
//**PWM
    HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_2);
//**IIC
    I2CInit();


    Led_Disp(0x00);
    LCD_Init();
    HAL_Delay(100);;
    LCD_SetBackColor(Black);
    LCD_SetTextColor(White);
    LCD_Clear(Black);
    HAL_Delay(100);

    MCP_Write(100); //RW = 10 / 128 * 100   10k
}

void Loop(void)
{
    //ADC
    R37_value = adc2[0] * 3.3f / 65535.0f;
    R38_value = adc1[0] * 3.3f / 65535.0f;

    //IC捕获
    freq = 1000000.0f / (TIM2->CCR2 + 1);
    duty = (TIM2->CCR1 + 1) * 100.0f / (TIM2->CCR2 + 1);

    //PWM
    __HAL_TIM_SET_AUTORELOAD(&htim3,1000000.0f / freq - 1);
    __HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_2,1000000.0f / freq / 2 - 1);

    //MCP
    MCP_value = adc1[1] * 3.3f / 65535.0f;

    //RTC
    HAL_RTC_GetTime(&hrtc,&time,RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc,&date,RTC_FORMAT_BIN);

    if(bkey[1].long_flag == 1)
    {
        line_select ++;
        if(line_select > 8) line_select = 1;
        count++;
        bkey[1].long_flag = 0;
    }

    if(bkey[2].short_flag == 1)
    {
        line_select --;
        if(line_select < 0) line_select = 8;
        count--;
        bkey[2].short_flag = 0;
    }
    if(bkey[3].double_flag == 1)
    {
        count -= 2;
        bkey[3].double_flag = 0;
    }
    if(bkey[4].short_flag == 1)
    {
        lcd_page ++;
        if(lcd_page == 2)lcd_page = 0;
        bkey[4].short_flag = 0;
        time.Seconds = 0;
        time.Minutes = 0;
        time.Hours = 0;
        HAL_RTC_SetTime(&hrtc,&time,RTC_FORMAT_BIN);
    }

    
    switch(lcd_page)
    {
        case 0:
        {
            LCD_DisplayStringLine(Line0, (unsigned char *)"                  ");
            LCD_DisplayStringLine(Line1, (unsigned char *)"                  ");
            LCD_DisplayStringLine(Line2, (unsigned char *)"                  ");
            LCD_DisplayStringLine(Line3, (unsigned char *)"                  ");
            LCD_DisplayStringLine(Line4, (unsigned char *)"    Hello,world   ");
            LCD_DisplayStringLine(Line5, (unsigned char *)"                  ");
            LCD_DisplayStringLine(Line6, (unsigned char *)"                  ");
            LCD_DisplayStringLine(Line7, (unsigned char *)"                  ");
            LCD_DisplayStringLine(Line8, (unsigned char *)"                  ");
            LCD_DisplayStringLine(Line9, (unsigned char *)"                  ");
            break;
        }
        case 1:
        {
            if(line_select)LCD_SetBackColor(Blue); else LCD_SetBackColor(Black);
            LCD_DisplayStringLine(Line1, (unsigned char *)"        INPUT       "); 

            if(line_select == 1)LCD_SetBackColor(Blue); else LCD_SetBackColor(Black);
            sprintf(lcd_char,"     N:%d        ",count);
            LCD_DisplayStringLine(Line3, (unsigned char *)lcd_char);

            if(line_select == 2)LCD_SetBackColor(Blue); else LCD_SetBackColor(Black);
            sprintf(lcd_char,"     R37:%.2fV        ",R37_value);
            LCD_DisplayStringLine(Line4, (unsigned char *)lcd_char);
            
            if(line_select == 3)LCD_SetBackColor(Blue); else LCD_SetBackColor(Black);
            sprintf(lcd_char,"     R38:%.2fV        ",R38_value);
            LCD_DisplayStringLine(Line5, (unsigned char *)lcd_char);

            if(line_select == 4)LCD_SetBackColor(Blue); else LCD_SetBackColor(Black);
            sprintf(lcd_char,"     FREQ:%.0fHZ  DUTY:%.1f%%     ",freq,duty);
            LCD_DisplayStringLine(Line6, (unsigned char *)lcd_char);

            if(line_select == 5)LCD_SetBackColor(Blue); else LCD_SetBackColor(Black);
            sprintf(lcd_char,"     MCP:%.2fV          ",MCP_value);
            LCD_DisplayStringLine(Line7, (unsigned char *)lcd_char);

            if(line_select == 6)LCD_SetBackColor(Blue); else LCD_SetBackColor(Black);
            sprintf(lcd_char,"    time:%02dH:%02dM:%02dS     ",time.Hours,time.Minutes,time.Seconds);
            LCD_DisplayStringLine(Line8, (unsigned char *)lcd_char);

            if(line_select == 7)LCD_SetBackColor(Blue); else LCD_SetBackColor(Black);
            sprintf(lcd_char,"    date:%02dY:%02dM:%02dD     ",date.Year,date.Month,date.Date);
            LCD_DisplayStringLine(Line9, (unsigned char *)lcd_char);

            LCD_SetBackColor(Black);
            break;
        }
    }
}
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim == &htim6)Key_Serv();

}
