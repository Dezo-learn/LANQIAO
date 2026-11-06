#include "mymain.h"

void Led_Disp(uint8_t ucled)
{
    HAL_GPIO_WritePin(GPIOC, 0xFF00, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOC, ucled << 8, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET);
}

uint8_t Key_Read(void)
{
    uint8_t Val = 0;
    uint8_t b1 = (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_RESET);
    uint8_t b2 = (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_RESET);
    uint8_t b3 = (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2) == GPIO_PIN_RESET);
    uint8_t b4 = (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET);

    
    if(b1 && b2) 
    {
        Val = 0x10; 
    }
    else 
    {
        if(b1) Val |= 0x01;
        if(b2) Val |= 0x02;
    }
    if(b3) Val |= 0x04;
    if(b4) Val |= 0x08;

    return Val;
}

struct BKEY
{
    uint8_t state;
    uint16_t timer;
    
    uint8_t short_flag;
    uint8_t long_flag;
    uint8_t double_flag;
};


struct BKEY keys[6] = {0};
void Key_Serv(void)
{
    for(uint8_t i = 1;i <= 5;i++)
    {
        uint8_t key_value = Key_Read();
        uint8_t pressed = (key_value & (0x01 << (i - 1))) ? 1 : 0;
        
        switch(keys[i].state)
        {
            case 0:
            {
                if(pressed)
                {
                    keys[i].state = 1;
                    keys[i].timer= 0;
                }
                break;
            }
            
            case 1:
            {
                if(pressed)
                {
                    keys[i].timer++;
                    if(keys[i].timer >= 200)
                    {
                        keys[i].state = 3;
                        keys[i].timer= 0;	
                    }
                }
                else
                {
                    if(keys[i].timer >= 2)
                    {
                        keys[i].timer = 0;
                        keys[i].state = 2;
                    }
                    else
                    {
                        keys[i].state = 0;
                        keys[i].timer = 0;
                    }
                }
                break;
            }
            
            
            case 2:
            {
                if(pressed)
                {
                    keys[i].double_flag = 1;
                    keys[i].state = 0;
                    keys[i].timer= 0;
                }
                else
                {
                    keys[i].timer++;
                    if(keys[i].timer >= 25)
                    {
                        keys[i].short_flag = 1;
                        keys[i].state = 0;
                        keys[i].timer= 0;
                    }
                }
                
                break;
            }
            
            
            case 3:
            {
                if(!pressed)
                {
                    keys[i].long_flag = 1;
                    keys[i].state = 0;
                    keys[i].timer= 0;
                }
                break;
            }
        }
    }
}


void MCP_Wirte(uint8_t dat)
{
    I2CStart();
    I2CSendByte(0x5E);
    I2CWaitAck();
    I2CSendByte(dat);
    I2CWaitAck();
    I2CStop();
}

//-----------------主调变量--------------------//
uint8_t ucled = 0x00;
char lcd_buf[50];
uint8_t lcd_page = 0;
float R37_value;
float R38_value;
float mp4017_value;
uint32_t adc2[1];
uint32_t adc1[2];
uint32_t Freq = 1000;
uint16_t Duty = 50;
uint16_t R40_Value;
uint8_t R40_Duty;  
uint16_t R39_Value;
uint8_t R39_Duty;  
RTC_TimeTypeDef time;
RTC_DateTypeDef date;
char Rx_buf[50];
char Tx_buf[50];
uint32_t lcd_tick;
uint8_t uart_flag;


void Task_Hardware(void);
void Task_Key(void);
void Task_Lcd(void);
void Uart_Callback(void);

void Set_Up(void)
{
    HAL_TIM_Base_Start_IT(&htim6);                       
    HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED);
    HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
    HAL_ADC_Start_DMA(&hadc2, adc2, 1);                    
    HAL_ADC_Start_DMA(&hadc1, adc1, 2);                    
    HAL_TIM_PWM_Start(&htim15, TIM_CHANNEL_2);            
    HAL_TIM_IC_Start(&htim2, TIM_CHANNEL_1);             
    HAL_TIM_IC_Start(&htim2, TIM_CHANNEL_2);             
    HAL_TIM_IC_Start(&htim3, TIM_CHANNEL_1);             
    HAL_TIM_IC_Start(&htim3, TIM_CHANNEL_2);
    HAL_UARTEx_ReceiveToIdle_IT(&huart1, (uint8_t *)Rx_buf, sizeof(Rx_buf) - 1);
    
    I2CInit();                                           
    Led_Disp(0x00);
    LCD_Init();
    HAL_Delay(100);
    LCD_SetBackColor(Black);
    LCD_SetTextColor(White);
    LCD_Clear(Black);
}
void Loop(void)
{
    Task_Hardware();
    Task_Key();

    Uart_Callback();
    if(HAL_GetTick() - lcd_tick  >= 200)
    {
        Task_Lcd();
        lcd_tick = HAL_GetTick();
    }
    Led_Disp(ucled);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim == &htim6) Key_Serv();
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if(huart == &huart1)
    {
        Rx_buf[Size] = '\0';     
        uart_flag = 1;
    }
}

void Uart_Callback(void)
{
    if(uart_flag == 0) return;
    
    uint32_t temp_freq = 0;

    if(sscanf(Rx_buf, "%d", &temp_freq) == 1)  
    {
        if(temp_freq >= 10 && temp_freq <= 100000) 
        {
            Freq = temp_freq; 
            sprintf(Tx_buf, "\r\nFreq:%d", Freq); 
            HAL_UART_Transmit(&huart1, (uint8_t *)Tx_buf, strlen(Tx_buf), 50);
        }
    }
    uart_flag = 0;
    
    memset(Rx_buf, 0, sizeof(Rx_buf));          
    HAL_UARTEx_ReceiveToIdle_IT(&huart1, (uint8_t *)Rx_buf, sizeof(Rx_buf) - 1);
}

void Task_Key(void)
{
    if(keys[1].short_flag)
    {
        ucled = (ucled & ~0x01) | 0x01;
        keys[1].short_flag = 0;
    }
    if(keys[1].double_flag)
    {
        ucled = (ucled & ~0x02) | 0x02;
        keys[1].double_flag = 0;
    }
    if(keys[2].short_flag)
    {
        time.Seconds = 0; time.Minutes = 0; time.Hours = 0;
        HAL_RTC_SetTime(&hrtc, &time, RTC_FORMAT_BIN);
        keys[2].short_flag = 0;
    }
    if(keys[3].short_flag)
    {
        lcd_page = (lcd_page + 1) % 2;
        keys[3].short_flag = 0;
    }
    if(keys[4].short_flag)
    {
        ucled = 0xff;
        keys[4].short_flag = 0;
    }
    if(keys[5].short_flag)
    {
        ucled ^= 0x01;  
        keys[5].short_flag = 0;
    }
    if(keys[5].long_flag)
    {
        ucled ^= 0xFF;  
        keys[5].long_flag = 0;
    }
}

void Task_Lcd(void)
{
    switch(lcd_page)
    {
        case 0:
        {
            LCD_DisplayStringLine(Line1, (unsigned char *)"                    ");
            LCD_DisplayStringLine(Line2, (unsigned char *)"                    ");
            LCD_DisplayStringLine(Line3, (unsigned char *)"                    ");
            LCD_DisplayStringLine(Line4, (unsigned char *)"      Hello V1      ");
            LCD_DisplayStringLine(Line5, (unsigned char *)"                    ");
            LCD_DisplayStringLine(Line6, (unsigned char *)"                    ");
            LCD_DisplayStringLine(Line7, (unsigned char *)"                    ");
            LCD_DisplayStringLine(Line8, (unsigned char *)"                    ");
            LCD_DisplayStringLine(Line9, (unsigned char *)"                    ");
//            sprintf(lcd_buf, "  time:%d     ",keys[i].timer);
//            LCD_DisplayStringLine(Line9, (unsigned char *)lcd_buf);
            break;
        }
        case 1:
            {
                sprintf(lcd_buf, "        PARA        ");
            LCD_DisplayStringLine(Line1, (unsigned char *)lcd_buf);
            sprintf(lcd_buf, "    R37:%.2f     ", R37_value);
            LCD_DisplayStringLine(Line2, (unsigned char *)lcd_buf);
            sprintf(lcd_buf, "    R38:%.2f     ", R38_value);
            LCD_DisplayStringLine(Line3, (unsigned char *)lcd_buf);
            sprintf(lcd_buf, "    R39:%d       ", R39_Value);
            LCD_DisplayStringLine(Line4, (unsigned char *)lcd_buf);
            sprintf(lcd_buf, "    R39_D:%d     ", R39_Duty);
            LCD_DisplayStringLine(Line5, (unsigned char *)lcd_buf);
            sprintf(lcd_buf, "    R40:%d       ", R40_Value);
            LCD_DisplayStringLine(Line6, (unsigned char *)lcd_buf);
            sprintf(lcd_buf, "    R40_D:%d     ", Freq);
            LCD_DisplayStringLine(Line7, (unsigned char *)lcd_buf);
            sprintf(lcd_buf, "    MP4017:%.2f     ", mp4017_value);
            LCD_DisplayStringLine(Line8, (unsigned char *)lcd_buf);
            sprintf(lcd_buf, "  time:%dH:%dM:%dS     ", time.Hours, time.Minutes, time.Seconds);
            LCD_DisplayStringLine(Line9, (unsigned char *)lcd_buf);
            break;
        }
    }
}

void Task_Hardware(void)
{
    // ADC 变换
    R37_value = adc2[0] * 3.3f / 65536.0f;                                                              
    R38_value = adc1[0] * 3.3f / 65536.0f;                                                              
    mp4017_value = adc1[1] * 3.3f / 65536.0f;   
    
    // TIM PWM 设置
    __HAL_TIM_SET_AUTORELOAD(&htim15, 1000000.0f / Freq - 1);                                           
    __HAL_TIM_SET_COMPARE(&htim15, TIM_CHANNEL_2, 1000000.0f / Freq * Duty / 100.0f - 1); 
    
    // 输入捕获测量
    R40_Value = 1000000.0f / (HAL_TIM_ReadCapturedValue(&htim2, TIM_CHANNEL_1) + 1);
    R40_Duty = (HAL_TIM_ReadCapturedValue(&htim2, TIM_CHANNEL_2) + 1) * 100.0f / (HAL_TIM_ReadCapturedValue(&htim2, TIM_CHANNEL_1) + 1);
    
    R39_Value = 1000000.0f / (HAL_TIM_ReadCapturedValue(&htim3, TIM_CHANNEL_1) + 1);
    R39_Duty = (HAL_TIM_ReadCapturedValue(&htim3, TIM_CHANNEL_2) + 1) * 100.0f / (HAL_TIM_ReadCapturedValue(&htim3, TIM_CHANNEL_1) + 1);
    
    MCP_Wirte(56); 
    
    HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);
}








