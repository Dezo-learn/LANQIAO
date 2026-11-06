#include "app.h"

struct BKEY
{
    uint16_t press_time;
    uint16_t double_timer;

    uint8_t short_flag;
    uint8_t long_flag;
    uint8_t double_flag;
};

struct BKEY bkey[5] = {0};

void LedDisp(uint8_t ucled)
{
    uint8_t odr = GPIOC -> ODR;
    GPIOC -> ODR = 0XFF00;
    GPIOC -> ODR = (~ucled << 8);
    HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
    GPIOC -> ODR = odr;
}

uint8_t KeyRead(void)
{
    uint8_t val = 0;
    uint8_t b1 = (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0) == GPIO_PIN_RESET);
    uint8_t b2 = (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1) == GPIO_PIN_RESET);
    uint8_t b3 = (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2) == GPIO_PIN_RESET);
    uint8_t b4 = (HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0) == GPIO_PIN_RESET);
    
    if(b1) val |= 0x01;
    if(b2) val |= 0x02;
    if(b3) val |= 0x04;
    if(b4) val |= 0x08;
    
    return val;
}

struct KEYS 
{
    uint8_t state;
    uint16_t timer;
    uint8_t click_cnt;
    
    uint8_t short_flag;
    uint8_t long_flag;
    uint8_t double_flag;
    uint8_t hold_flag;
}keys[5] = {0};

//长按 短按 双击 按住
void KeyServ(void)    //注意hold 与 short long double 等互斥 往下一个状态的时候清零timer 回去的时候 可以不要清零timer
{
    uint8_t key_val = KeyRead();
    for(uint8_t i = 1;i <= 4;i++)
    {
        uint8_t pressed = !!(key_val & (0x01 << (i - 1))); //归一化
        
        switch(keys[i].state)
        {
            case 0:
                if(pressed){keys[i].state = 1;keys[i].timer = 0;}
                break;
                
            case 1: //按下状态机
                if(pressed)

                {
                    keys[i].timer ++;
                    if(keys[i].timer >= 100) keys[i].hold_flag = 1;
                }
                else //松手
                {
                    if(keys[i].timer >= 200)
                    {
                        keys[i].long_flag = 1;
                        keys[i].hold_flag = 0;
                        keys[i].click_cnt = 0;
                        keys[i].timer = 0;
                    }
                    else if(keys[i].timer >= 3)
                    {
                        keys[i].click_cnt++;
                        keys[i].timer = 0;
                        keys[i].state = 2;
                    }
                    else //抖动
                    {
                        keys[i].state = 0;
                    }
                }
                break;
                
            case 2: //结算
                if(pressed) 
                {
                    keys[i].state = 1; 
                    keys[i].timer = 0;
                }
                else 
                {
                    keys[i].timer++;
                    if(keys[i].timer >= 25) // 超过250ms结算
                    {
                        if(keys[i].hold_flag == 1) 
                        {
                            keys[i].hold_flag = 0;
                        }
                        else if(keys[i].click_cnt == 1) //短按
                        {
                            keys[i].short_flag = 1;
                        }
                        else if(keys[i].click_cnt == 2) // 双击
                        {
                            keys[i].double_flag = 1;
                        }
                        
                        // 结算完毕
                        keys[i].click_cnt = 0;
                        keys[i].timer = 0;
                        keys[i].state = 0;
                    }
                }
                break;
        }
    }
}

void EEPROM_Write(uint8_t add,uint8_t dat)
{
    I2CStart();
    I2CSendByte(0xA0);
    I2CWaitAck();
    I2CSendByte(add);
    I2CWaitAck();
    I2CSendByte(dat);
    I2CWaitAck();
    I2CStop();
    HAL_Delay(10);
}

uint8_t EEPROM_Read(uint8_t add)
{
    uint8_t dat;
    I2CStart();
    I2CSendByte(0xA0);
    I2CWaitAck();
    I2CSendByte(add);
    I2CWaitAck();
    I2CStop();
    
    I2CStart();
    I2CSendByte(0xA1);
    I2CWaitAck();
    dat = I2CReceiveByte();
    I2CSendNotAck();
    I2CStop();
    return dat;
}

void MCP_Write(uint8_t dat)
{
    I2CStart();
    I2CSendByte(0x5E);
    I2CWaitAck();
    I2CSendByte(dat);
    I2CWaitAck();
    I2CStop();
    HAL_Delay(10);
}
//===========================================================================
//led
uint8_t ucled;
//lcd
char lcd_buf[50];

//prar
uint16_t pwm_freq = 1000;
uint8_t pwm_duty = 50;

uint16_t r39_freq;
uint8_t  r39_duty;
uint16_t r40_freq;
uint8_t  r40_duty;

float mcp_set = 2.3;
uint8_t mcp_set_u8;
float mcp_val;
float r37_val;
float r38_val;
uint16_t adc1[2];
uint16_t adc2[1];

RTC_TimeTypeDef rtc_time;
RTC_DateTypeDef rtc_date;

//uart
char rx_buf[50];
uint8_t uart_flag;

void app_proc(void)
{
    r38_val = 3.3f * adc1[0] / 65535;
    mcp_val = 3.3f * adc1[1] / 65535;
    r37_val = 3.3f * adc2[0] / 65535;
    
    r39_freq = 1000000.0f / (TIM2->CCR1 + 1);
    r39_duty = (TIM2->CCR2 + 1) * 100.0f / (TIM2->CCR1 + 1) ;
    
    r40_freq = 1000000.0f / (TIM3->CCR1 + 1);
    r40_duty = (TIM3->CCR2 + 1)  * 100.0f / (TIM3->CCR1 + 1);
    
    
    TIM16->ARR =  1000000.0f / pwm_freq - 1;
    TIM16->CCR1 = (TIM16->ARR + 1) * (r39_duty / 100.0f);
    
    TIM17->ARR =  1000000.0f / pwm_freq - 1;
    TIM17->CCR1 = (TIM16->ARR + 1) * (r39_duty / 100.0f);
    
//    mcp_set_u8 = (uint8_t)(12.7f * mcp_set / (3.3f - mcp_set));
}


void lcd_proc(void)
{
    LCD_DisplayStringLine(Line1, (unsigned char *)"         PRAR       ");
    
    sprintf(lcd_buf,"PWM:Freq:%d,Duty:%d%% ",pwm_freq,pwm_duty);
    LCD_DisplayStringLine(Line3, (unsigned char *)lcd_buf);

    sprintf(lcd_buf,"R39:Freq:%d,Duty:%d%% ",r39_freq,r39_duty);
    LCD_DisplayStringLine(Line4, (unsigned char *)lcd_buf);
    
    sprintf(lcd_buf,"R40:Freq:%d,Duty:%d%% ",r40_freq,r40_duty);
    LCD_DisplayStringLine(Line5, (unsigned char *)lcd_buf);
    
    sprintf(lcd_buf,"R37:%.1f,R38:%.1f     ",r37_val,r38_val);
    LCD_DisplayStringLine(Line6, (unsigned char *)lcd_buf);
    
    sprintf(lcd_buf,"u8:%d,MCP:%.1f        ",mcp_set_u8,mcp_val);
    LCD_DisplayStringLine(Line7, (unsigned char *)lcd_buf);
    
    sprintf(lcd_buf,"DATE:%02d-%02d-%02d      ",rtc_date.Year,rtc_date.Month,rtc_date.Date);
    LCD_DisplayStringLine(Line8, (unsigned char *)lcd_buf);
    
    sprintf(lcd_buf,"TIME:%02d:%02d:%02d      ",rtc_time.Hours,rtc_time.Minutes,rtc_time.Seconds);
    LCD_DisplayStringLine(Line9, (unsigned char *)lcd_buf);
}


void key_proc(void)
{
    if(keys[1].short_flag == 1)
    {
        keys[1].short_flag = 0;
        MCP_Write(mcp_set_u8);
        ucled = (ucled & ~0x01) | 0x01;
    }
    if(keys[1].long_flag == 1)
    {
        keys[1].long_flag = 0;
        ucled = (ucled & ~0x02) | 0x02;
    }
    
    if(keys[1].double_flag == 1)
    {

        keys[1].double_flag = 0;
        EEPROM_Write(0,mcp_set_u8);
        
        ucled = (ucled & ~0x04) | 0x04;
    }
    if(keys[1].hold_flag == 1)
    {
        mcp_set_u8 ++;
        ucled = (ucled & ~0x08) | 0x08;
    }
    else
    {
        ucled = (ucled & ~0x08) | 0x00;
    }
    LedDisp(ucled);
}



void setup(void)
{
    HAL_TIM_Base_Start_IT(&htim6);
    HAL_TIM_IC_Start(&htim2,TIM_CHANNEL_1);
    HAL_TIM_IC_Start(&htim3,TIM_CHANNEL_1);
    HAL_TIM_IC_Start(&htim2,TIM_CHANNEL_2);
    HAL_TIM_IC_Start(&htim3,TIM_CHANNEL_2);
    
    HAL_TIM_PWM_Start(&htim16,TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim17,TIM_CHANNEL_1);
    
    HAL_ADCEx_Calibration_Start(&hadc1,ADC_SINGLE_ENDED);
    HAL_ADCEx_Calibration_Start(&hadc2,ADC_SINGLE_ENDED);
    HAL_ADC_Start_DMA(&hadc1,(uint32_t *)adc1,2);
    HAL_ADC_Start_DMA(&hadc2,(uint32_t *)adc2,1);
    
    rtc_date.Year = 26;rtc_date.Month = 6;rtc_date.Date = 6;
    rtc_time.Hours = 9;rtc_time.Minutes = 0;rtc_time.Seconds = 0;
    
    HAL_RTC_SetTime(&hrtc,&rtc_time,RTC_FORMAT_BIN);
    HAL_RTC_SetDate(&hrtc,&rtc_date,RTC_FORMAT_BIN);

    
    LCD_Init();
    HAL_Delay(100);;
    LCD_SetBackColor(Black);
    LCD_SetTextColor(White);
    LCD_Clear(Black);
    HAL_Delay(100);
    
    LedDisp(0x00);
    I2CInit();
    mcp_set_u8 = EEPROM_Read(0);
}

void loop(void)
{
    HAL_RTC_GetTime(&hrtc,&rtc_time,RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc,&rtc_date,RTC_FORMAT_BIN);
    key_proc();
    app_proc();
    lcd_proc();
    
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim == &htim6) //10ms
    {
        KeyServ();
    }

}
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if(huart == &huart1)
    {
        rx_buf[Size] = '\0';
        uart_flag =1;
    }
}


int fputc (int ch ,FILE* file)
{
    HAL_UART_Transmit(&huart1,(uint8_t *)&ch,1,50);
    return ch;
}
