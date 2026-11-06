#include "mymain_new.h"

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

void Key_Serv(void)
{
    uint8_t key_pin[5] = {0};
    key_pin[1] = (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_RESET);
    key_pin[2] = (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_RESET);
    key_pin[3] = (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2) == GPIO_PIN_RESET);
    key_pin[4] = (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET);

    for(uint8_t i = 1; i < 5; i++)
    {
        if(key_pin[i])
        {
            if(bkey[i].press_time < 65535)
                bkey[i].press_time++;
            if(bkey[i].press_time == 100)
                bkey[i].long_flag = 1;
        }
        else
        {
            if(bkey[i].press_time >= 3 && bkey[i].press_time < 100)
                bkey[i].short_flag = 1;
            bkey[i].press_time = 0;
        }
    }
}

void EEPROM_Write(uint8_t addr,uint8_t data)
{
    I2CStart();
    I2CSendByte(0xA0);
    I2CWaitAck();
    I2CSendByte(addr);
    I2CWaitAck();
    I2CSendByte(data);
    I2CWaitAck();
    I2CStop();
    HAL_Delay(10);
}

uint8_t EEPROM_Read(uint8_t addr)
{
    uint8_t dat;
    I2CStart();
    I2CSendByte(0xA0);
    I2CWaitAck();
    I2CSendByte(addr);
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

//*****************MAIN*****************//

uint8_t Lcd_Page = 0;
char lcd_str[50];
uint8_t ucled;

float PA4_adc = 0.00f;
float PA5_adc = 0.00f;
float PA5_adc_View = 0.00f;
float PA4_adc_View = 0.00f;
float PA5_adc_MIN_view;
float PA4_adc_MIN_view;
float PA1_freq = 0;

uint8_t X = 1;
uint8_t Y = 1;
uint8_t N = 23;

uint8_t rec_select = 0;
uint8_t PA5_N = 0;
float PA5_adc_MAX = 0.0f;
float PA5_adc_MIN = 4.0f;
float PA5_adc_AVE = 0.0f;

uint8_t PA4_N = 0;
float PA4_adc_MAX = 0.0f;
float PA4_adc_MIN = 4.0f;
float PA4_adc_AVE = 0.0f;

uint8_t  PSC_enable = 0;
uint32_t adc_buf[2];

char rx_buf[50];
char tx_buf[50];
char rx_data[50];
uint8_t lcd_swtich;
uint8_t lcd_swtich_old;
uint8_t led3_timer = 0xff;
uint8_t led3_blink;
uint16_t LCDR1 = 0x0000;
uint16_t LCDR96 = 0x2700;

void SetUp(void)
{
    HAL_TIM_Base_Start_IT(&htim6);
    HAL_TIM_IC_Start(&htim2,TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_2);
    HAL_ADCEx_Calibration_Start(&hadc2,ADC_SINGLE_ENDED);
    HAL_ADC_Start_DMA(&hadc2,adc_buf,2);
    HAL_UARTEx_ReceiveToIdle_IT(&huart1,(uint8_t *)rx_buf,50);

    I2CInit();
    Led_Disp(0x00);
    LCD_Init();
    HAL_Delay(100);;
    LCD_SetBackColor(Black);
    LCD_SetTextColor(White);
    LCD_Clear(Black);
    HAL_Delay(100);
    if(EEPROM_Read(0) > 4 || EEPROM_Read(1) > 4)
    {
        EEPROM_Write(0,1);
        EEPROM_Write(1,1);
    }

    X = EEPROM_Read(1);
    Y = EEPROM_Read(0);

}


void Loop(void)
{
//*****************TASK*****************//
    PA4_adc = adc_buf[0] * 3.3f / 65535.0f;
    PA5_adc = adc_buf[1] * 3.3f / 65535.0f;

//*****************KEY*****************//
    if(bkey[1].short_flag == 1)
    {
        Lcd_Page++;
        if(Lcd_Page == 3)Lcd_Page = 0;
        if(Lcd_Page == 2)rec_select = 0;
        bkey[1].short_flag = 0;
    }

    if(bkey[2].short_flag == 1)
    {
        if(Lcd_Page == 1)
        {
            X++;
            if(X == 5) X = 1;
            EEPROM_Write(1,X);
        }
        bkey[2].short_flag = 0;
    }

    if(bkey[3].short_flag == 1)
    {
        if(Lcd_Page == 1)
        {
            Y++;
            if(Y == 5) Y = 1;
            EEPROM_Write(0,Y);
        }
        bkey[3].short_flag = 0;
    }

    if(bkey[4].short_flag == 1)
    {
        if(Lcd_Page == 0)
        {
            PA4_adc_View = PA4_adc;
            PA5_adc_View = PA5_adc;

                PA5_N ++;
                if(PA5_N == 1) PA5_adc_AVE = PA5_adc;
                else PA5_adc_AVE = ((PA5_adc_AVE * (PA5_N - 1)) + PA5_adc) / PA5_N;
                if(PA5_adc > PA5_adc_MAX) PA5_adc_MAX = PA5_adc;
                if(PA5_adc < PA5_adc_MIN) PA5_adc_MIN = PA5_adc;

                PA4_N ++;
                if(PA4_N == 1) PA4_adc_AVE = PA4_adc;
                else PA4_adc_AVE = ((PA4_adc_AVE * (PA4_N - 1)) + PA4_adc) / PA4_N;
                if(PA4_adc > PA4_adc_MAX) PA4_adc_MAX = PA4_adc;
                if(PA4_adc < PA4_adc_MIN) PA4_adc_MIN = PA4_adc;
        }
        if(Lcd_Page == 1)
        {
            PSC_enable = !PSC_enable;
        }
        if(Lcd_Page == 2)
        {
            rec_select = !rec_select;
        }
        bkey[4].short_flag = 0;
    }

    if(bkey[4].long_flag == 1)
    {
        if(Lcd_Page == 2)
        {

                PA5_N = 0;
                PA5_adc_MAX = 0;
                PA5_adc_MIN = 0;
                PA5_adc_AVE = 0;

                PA4_N = 0;
                PA4_adc_MAX = 0;
                PA4_adc_MIN = 0;
                PA4_adc_AVE = 0;
        }
        bkey[4].long_flag = 0;
    }
//*****************TASK*****************//
    PA1_freq = 1000000.0f / (HAL_TIM_ReadCapturedValue(&htim2,TIM_CHANNEL_2) - 1);

    if(PSC_enable)
    {
        __HAL_TIM_SetAutoreload(&htim3,(uint16_t)(1000000.0f / (PA1_freq / X) - 1));
        __HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_2, (uint16_t)(1000000.0f / (PA1_freq / X) / 2 - 1));
    }
    else
    {
        __HAL_TIM_SetAutoreload(&htim3,(uint16_t)(1000000.0f / (PA1_freq * X) - 1));
        __HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_2,(uint16_t)(1000000.0f / (PA1_freq * X) / 2 - 1));
    }

    if(PA5_adc_MIN == 4.0f)PA5_adc_MIN_view = 0.0f;
    else PA5_adc_MIN_view = PA5_adc_MIN;

    if(PA4_adc_MIN == 4.0f)PA4_adc_MIN_view = 0.0f;
    else PA4_adc_MIN_view = PA4_adc_MIN;
//*****************LCD*****************//

    if(lcd_swtich == 1)
    {
        LCDR1 = 0x0100;
        LCDR96 =  0xA700;
    }
    else
    {
        LCDR1 = 0x0000;
        LCDR96 =  0x2700;
    }
    if(lcd_swtich_old != lcd_swtich)
    {
        LCD_Init();
        lcd_swtich_old = lcd_swtich;
    }

    switch(Lcd_Page)
    {
        case 0:
        {
            LCD_DisplayStringLine(Line1, (unsigned char *)"        DATA       ");
            sprintf(lcd_str,"     PA4=%.2f      ",PA4_adc_View);
            LCD_DisplayStringLine(Line3, (unsigned char *)lcd_str);
            sprintf(lcd_str,"     PA5=%.2f      ",PA5_adc_View);
            LCD_DisplayStringLine(Line4, (unsigned char *)lcd_str);
            sprintf(lcd_str,"     PA1=%.0f      ",PA1_freq);
            LCD_DisplayStringLine(Line5, (unsigned char *)lcd_str);
            sprintf(lcd_str,"                 ");
            LCD_DisplayStringLine(Line6, (unsigned char *)lcd_str);
            break;
        }

        case 1:
        {
            LCD_DisplayStringLine(Line1, (unsigned char *)"        PARA       ");
            sprintf(lcd_str,"     X=%d      ",X);
            LCD_DisplayStringLine(Line3, (unsigned char *)lcd_str);
            sprintf(lcd_str,"     Y=%d      ",Y);
            LCD_DisplayStringLine(Line4, (unsigned char *)lcd_str);
            sprintf(lcd_str,"                 ");
            LCD_DisplayStringLine(Line5, (unsigned char *)lcd_str);
            sprintf(lcd_str,"                 ");
            LCD_DisplayStringLine(Line6, (unsigned char *)lcd_str);
            break;
        }

        case 2:
        {
            sprintf(lcd_str,"        REC-%s      ",rec_select? "PA5":"PA4");
            LCD_DisplayStringLine(Line1, (unsigned char *)lcd_str);
            sprintf(lcd_str,"     N=%d      ",rec_select? PA5_N:PA4_N);
            LCD_DisplayStringLine(Line3, (unsigned char *)lcd_str);
            sprintf(lcd_str,"     A=%.2f      ",rec_select? PA5_adc_MAX:PA4_adc_MAX);
            LCD_DisplayStringLine(Line4, (unsigned char *)lcd_str);
            sprintf(lcd_str,"     T=%.2f      ",rec_select? PA5_adc_MIN_view:PA4_adc_MIN_view);
            LCD_DisplayStringLine(Line5, (unsigned char *)lcd_str);
            sprintf(lcd_str,"     H=%.2f      ",rec_select? PA5_adc_AVE:PA4_adc_AVE);
            LCD_DisplayStringLine(Line6, (unsigned char *)lcd_str);
            break;
        }
    }

    sprintf(lcd_str,"                 ");
    LCD_DisplayStringLine(Line7, (unsigned char *)lcd_str);
    sprintf(lcd_str,"                 ");
    LCD_DisplayStringLine(Line8, (unsigned char *)lcd_str);
    sprintf(lcd_str,"                 ");
    LCD_DisplayStringLine(Line0, (unsigned char *)lcd_str);
    sprintf(lcd_str,"                 ");
    LCD_DisplayStringLine(Line9, (unsigned char *)lcd_str);

//*****************LED*****************//
    if(PSC_enable)
    {
        ucled = (ucled & ~0x02) | 0x02;
        ucled = (ucled & ~0x01) | 0x00;
    }
    else
    {
        ucled = (ucled & ~0x01) | 0x01;
        ucled = (ucled & ~0x02) | 0x00;
    }

    if(PA4_adc_View > (PA5_adc_View * Y)) led3_blink = 1;
    else led3_blink = 0;
    if(led3_timer < 10) ucled = (ucled & ~0x04) | 0x04;
    else ucled = (ucled & ~0x04) | 0x00;

    if(lcd_swtich == 0)ucled = (ucled & ~0x08) | 0x08;
    else ucled = (ucled & ~0x08) | 0x00;

    Led_Disp(ucled);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim == &htim6) Key_Serv();
    if(led3_blink)
    {
        led3_timer++;
        if(led3_timer >= 20) led3_timer = 0;
    }
    else
    {
        led3_timer = 0xff;
    }
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    sprintf(rx_data,"%s",rx_buf);

    if(strcmp(rx_data,"X")==0)
    {
        sprintf(tx_buf,"X:%d\r\n",X);
        HAL_UART_Transmit(&huart1,(uint8_t *)tx_buf,strlen(tx_buf),50);
    }
    else if(strcmp(rx_data,"Y")==0)
    {
        sprintf(tx_buf,"Y:%d\r\n",Y);
        HAL_UART_Transmit(&huart1,(uint8_t *)tx_buf,strlen(tx_buf),50);
    }
    else if(strcmp(rx_data,"PA1")==0)
    {
        sprintf(tx_buf,"PA1:%.1f\r\n",PA1_freq);
        HAL_UART_Transmit(&huart1,(uint8_t *)tx_buf,strlen(tx_buf),50);
    }
    else if(strcmp(rx_data,"PA4")==0)
    {
        sprintf(tx_buf,"PA4:%.2f\r\n",PA4_adc_View);
        HAL_UART_Transmit(&huart1,(uint8_t *)tx_buf,strlen(tx_buf),50);
    }
    else if(strcmp(rx_data,"PA5")==0)
    {
        sprintf(tx_buf,"PA5:%.2f\r\n",PA5_adc_View);
        HAL_UART_Transmit(&huart1,(uint8_t *)tx_buf,strlen(tx_buf),50);
    }
    else if(strcmp(rx_data,"#")==0)
    {
        lcd_swtich = !lcd_swtich;
    }
    memset(rx_buf,0,50);
    HAL_UARTEx_ReceiveToIdle_IT(&huart1,(uint8_t *)rx_buf,50);
}
