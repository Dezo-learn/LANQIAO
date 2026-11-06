#include "mymain.h"

void Led_Disp(uint8_t ucled)
{
    HAL_GPIO_WritePin(GPIOC,0xFF00,GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOC,ucled << 8,GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
}

struct BKEY  //按键数组
{
    uint16_t press_time;
    uint16_t double_time;
    
    uint8_t short_flag;
    uint8_t long_flag;
    uint8_t double_flag;
    
};

struct BKEY bkey[5] = {0};

uint8_t Key_Read(void)              //按键读取
{
    if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0) == GPIO_PIN_RESET) return 1;
    else if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1) == GPIO_PIN_RESET) return 2;
    else if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2) == GPIO_PIN_RESET) return 3;
    else if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0) == GPIO_PIN_RESET) return 4;
    else return 0;
}


void Key_Serv(void)                 //按键: 含双击、单击、长按、长按后延迟触发
{
    uint8_t key_value;
    key_value = Key_Read();
    for(uint8_t i = 0;i < 5;i++)
    {
        if(key_value == i) bkey[i].press_time++;
        else
        {
            if(bkey[i].press_time >= 100)
            {
                bkey[i].long_flag = 1;
                bkey[i].press_time = 0;
            }
            else if(bkey[i].press_time >= 3)
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
                bkey[i].double_time -- ;
                if(bkey[i].double_time == 0)
                {
                    bkey[i].short_flag = 1;
                    bkey[i].press_time = 0;
                }
            
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

//-----------------主调函数--------------------//
uint8_t ucled = 0x00;
char lcd_buf[50];
uint8_t lcd_page = 0;
float R37_value;
float R38_value;
float mp4017_value;
uint32_t adc2[1];
uint32_t adc1[2];
uint32_t Freq = 1000;
uint16_t Duty = 50;//50%
uint16_t R40_Value;
uint8_t R40_Duty;  //R40对应的占空比 理论：50
uint16_t R39_Value;
uint8_t R39_Duty;  //R39对应的占空比 理论：50
RTC_TimeTypeDef time;
RTC_DateTypeDef date;
char Rx_buf[50];
char Tx_buf[50];
void Set_Up(void)
{
    HAL_TIM_Base_Start_IT(&htim6);                       //标准定时器6  用于KEY等计时函数
    HAL_ADCEx_Calibration_Start(&hadc2,ADC_SINGLE_ENDED);//ADC2校准
    HAL_ADCEx_Calibration_Start(&hadc1,ADC_SINGLE_ENDED);//ADC1校准
    HAL_ADC_Start_DMA(&hadc2,adc2,1);                    //ADC2 DMA ADC启用 -> adc2数组
    HAL_ADC_Start_DMA(&hadc1,adc1,2);                    //ADC1 DMA ADC启用 -> adc1数组
    HAL_TIM_PWM_Start(&htim15,TIM_CHANNEL_2);            //PWM 启用
    HAL_TIM_IC_Start(&htim2,TIM_CHANNEL_1);              //IC  启用 
    HAL_TIM_IC_Start(&htim2,TIM_CHANNEL_2);              //IC  启用 这里因为要检测下降沿所以这个也要开 别忘了
    HAL_TIM_IC_Start(&htim3,TIM_CHANNEL_1);              //IC  启用 
    HAL_TIM_IC_Start(&htim3,TIM_CHANNEL_2);              //IC  启用 这里因为要检测下降沿所以这个也要开 别忘了
    HAL_UARTEx_ReceiveToIdle_IT(&huart1,(uint8_t *)Rx_buf,sizeof(Rx_buf));
    
    I2CInit();                                           //IIC启用 别忘了
    Led_Disp(0x00);
    LCD_Init();
    HAL_Delay(100);
    LCD_SetBackColor(Black);
    LCD_SetTextColor(White);
    LCD_Clear(Black);
}


void Loop(void)
{
//********************任务********************//
    //ADC
    R37_value = adc2[0] * 3.3f / 65536.0f;                                              //超采样后转化完的值
    R38_value = adc1[0] * 3.3f / 65536.0f;                                              //超采样后转化完的值
    mp4017_value = adc1[1] * 3.3f / 65536.0f;   
    //PWM
    //函数写法
    __HAL_TIM_SET_AUTORELOAD(&htim15,1000000.0f / Freq - 1);                            //设置Freq = 1_000_000 / Freq - 1
    __HAL_TIM_SET_COMPARE(&htim15,TIM_CHANNEL_2,1000000.0f / Freq * Duty / 100.0f - 1); //设置DUTY = 1_000_000 / Freq * (Duty / 100) - 1
    //IC
    //函数写法
    R40_Value = 
    1000000.0f                                                                      //读取IC捕获FREQ值 = 1_000_000 / (捕获的上升沿数 + 1)
        /
        (HAL_TIM_ReadCapturedValue(&htim2,TIM_CHANNEL_1) +1);
    R40_Duty =                                                                      //读取IC捕获DUTY值 = (捕获的下降沿数 + 1) * 100.0f / (捕获的上升沿数 + 1) * 100.0f
        (HAL_TIM_ReadCapturedValue(&htim2,TIM_CHANNEL_2) + 1) * 100.0f
        /
        (HAL_TIM_ReadCapturedValue(&htim2,TIM_CHANNEL_1) + 1);
    
    R39_Value =
        1000000.0f
        /
        (HAL_TIM_ReadCapturedValue(&htim3,TIM_CHANNEL_1) +1);
        
    R39_Duty =
        (HAL_TIM_ReadCapturedValue(&htim3,TIM_CHANNEL_2) + 1) * 100.0f
        /
        (HAL_TIM_ReadCapturedValue(&htim3,TIM_CHANNEL_1) + 1);
    //数字电位器
    MCP_Wirte(56); // input输入:0 ~ 127    对应转化原理：  Rs / (Rs + 10k)  *  3.3V 而 Rs = Rab * input / 127  Rab = 100k (原理图+手册)
    //RTC
    HAL_RTC_GetTime(&hrtc,&time,RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc,&date,RTC_FORMAT_BIN);
    
//********************按键********************//
    if(bkey[1].short_flag)
    {
        ucled = (ucled & ~0x01) | 0x01;
        bkey[1].short_flag = 0;
    }
    if(bkey[2].short_flag)
    {
        time.Seconds = 0;
        time.Minutes = 0;
        time.Hours = 0;
        HAL_RTC_SetTime(&hrtc,&time,RTC_FORMAT_BIN);
        bkey[2].short_flag = 0;
    }
    if(bkey[3].short_flag)
    {
        lcd_page++;
        if(lcd_page == 2)lcd_page = 0;
        bkey[3].short_flag = 0;
    }
    
    if(bkey[1].double_flag)
    {
        ucled = (ucled & ~0x02) | 0x02;
        bkey[1].double_flag = 0;
    }
    
//********************LCD显示********************//
    switch(lcd_page)
    {
        case 0:
        {
            LCD_DisplayStringLine(Line1, (unsigned char *)"                   ");
            LCD_DisplayStringLine(Line2, (unsigned char *)"                   ");
            LCD_DisplayStringLine(Line3, (unsigned char *)"                   ");
            LCD_DisplayStringLine(Line4, (unsigned char *)"     Hello V1      ");
            LCD_DisplayStringLine(Line5, (unsigned char *)"                   ");
            LCD_DisplayStringLine(Line6, (unsigned char *)"                   ");
            LCD_DisplayStringLine(Line7, (unsigned char *)"                   ");
            LCD_DisplayStringLine(Line8, (unsigned char *)"                   ");
            LCD_DisplayStringLine(Line9, (unsigned char *)"                   ");
            break;
        }
        case 1:
        {
            sprintf(lcd_buf,"        PARA        ");
            LCD_DisplayStringLine(Line1,(unsigned char *)lcd_buf);
            sprintf(lcd_buf,"    R37:%.2f     ",R37_value);
            LCD_DisplayStringLine(Line2,(unsigned char *)lcd_buf);
            sprintf(lcd_buf,"    R38:%.2f     ",R38_value);
            LCD_DisplayStringLine(Line3,(unsigned char *)lcd_buf);
            sprintf(lcd_buf,"    R39:%d       ",R39_Value);
            LCD_DisplayStringLine(Line4,(unsigned char *)lcd_buf);
            sprintf(lcd_buf,"    R39_D:%d     ",R39_Duty);
            LCD_DisplayStringLine(Line5,(unsigned char *)lcd_buf);
            sprintf(lcd_buf,"    R40:%d       ",R40_Value);
            LCD_DisplayStringLine(Line6,(unsigned char *)lcd_buf);
            sprintf(lcd_buf,"    R40_D:%d     ",R40_Duty);
            LCD_DisplayStringLine(Line7,(unsigned char *)lcd_buf);
            sprintf(lcd_buf,"    MP4017:%.2f     ",mp4017_value);
            LCD_DisplayStringLine(Line8,(unsigned char *)lcd_buf);
            sprintf(lcd_buf,"  time:%dH:%dM:%dS     ",time.Hours,time.Minutes,time.Seconds);
            LCD_DisplayStringLine(Line9, (unsigned char *)lcd_buf);
            break;
        }
    }
    
    Led_Disp(ucled);
}

//********************中断********************//
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim == &htim6) Key_Serv();
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if(huart == &huart1)
    {
        Rx_buf[Size] = '\0';     //先加上'/0'用于sscanf识别文段末尾
        
        uint32_t temp_freq = 0;
        
        if(sscanf(Rx_buf, "%d", &temp_freq) == 1)  // sscanf返回本次成功提取并赋值的变量个数防止程序跑飞
        {
            if(temp_freq >= 10 && temp_freq <= 100000) 
            {
                Freq = temp_freq; 
                sprintf(Tx_buf,"\r\nFreq:%d",Freq); //这个是因为vofa不会自动换行所以\r\n在前面
                HAL_UART_Transmit(&huart1,(uint8_t *)Tx_buf,sizeof(Tx_buf),50);
            }
        }
        memset(Rx_buf, 0, sizeof(Rx_buf));          //清空Rx_buf缓存区
        HAL_UARTEx_ReceiveToIdle_IT(&huart1, (uint8_t *)Rx_buf, sizeof(Rx_buf));//重新打开中断
    }
}



