#include "mymain.h"


//*************模板*************//

struct BKEY
{
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
    if (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0) == 0) return 1;
    else if (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1) == 0) return 2;
    else if (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2) == 0) return 3;
    else if (HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0) == 0) return 4;
    else return 0;
}

void Key_Serv(void)
{
    uint8_t key_value;
    key_value = Key_Read();
    for(uint8_t i = 1; i< 5;i++)
    {
        if(key_value == i)bkey[i].timer++;
        else
        {
            if(bkey[i].timer >= 100)bkey[i].long_flag = 1;
            else if (bkey[i].timer >= 3) bkey[i].short_flag = 1;
            bkey[i].timer = 0;
        }
    }
}




//*************主调*************//

char str[50];
uint8_t  lcd_page = 0;
float TEMP = 0;
uint16_t SPEED = 200;
uint8_t TH = 30;
uint8_t auto_set = 0;
uint32_t adc2_buf[1];
float R37_value;
float Freq = 1000.0f;
float Duty = 0.5f;
char uart_rx[50];
char uart_tx[50];
char uart_data[50];
uint8_t led8_sine;
uint8_t ucled;
void SetUp(void)
{
    HAL_TIM_Base_Start_IT(&htim6);
    HAL_ADCEx_Calibration_Start(&hadc2,ADC_SINGLE_ENDED);
    HAL_ADC_Start_DMA(&hadc2,adc2_buf,1);
    HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_2);
    HAL_UARTEx_ReceiveToIdle_IT(&huart1,(uint8_t *)uart_rx,50);
    
    Led_Disp(0x00);
    LCD_Init();
    HAL_Delay(100);
    LCD_SetBackColor(Black);
    LCD_SetTextColor(White);
    LCD_Clear(Black);
    HAL_Delay(100);
    
}



void Loop(void)
{
//功能
//（1）温度显示
    R37_value = adc2_buf[0] * 3.3f / 65535.0f;
    if(R37_value < 0.3f) TEMP = 0;
    else if(R37_value < 3.0f) TEMP = R37_value * 29.63f - 8.89f;
    else TEMP = 80.0f;

//（2）PWM输出
    Duty = (float)SPEED / 1000.0f;
    TIM3 -> ARR = 1000000.0f / Freq - 1;
    TIM3 -> CCR2 = 1000000.0f * Duty / Freq - 1;


//按键
    if(bkey[1].short_flag == 1)
    {
        auto_set = !auto_set;
        bkey[1].short_flag = 0;
    }

    if(bkey[2].short_flag == 1)
    {
        if(lcd_page == 0 && auto_set == 0)
        {
            if(SPEED <= 900)SPEED += 100;
        }
        if(lcd_page == 1)
        {
            if(TH <= 55) TH += 5;
        }
        bkey[2].short_flag = 0;
    }


    if(bkey[3].short_flag == 1)
    {
        if(lcd_page == 0 && auto_set == 0)
        {
            if(SPEED >= 200)SPEED -= 100;
        }
        if(lcd_page == 1)
        {
            if(TH >= 35)TH -= 5;
            
        }
        bkey[3].short_flag = 0;
    }



    if(bkey[4].short_flag == 1)
    {   
        lcd_page = !lcd_page;
        bkey[4].short_flag = 0;
    }

//（3）工作模式
    if(auto_set == 1)
    {
        if(TEMP > TH)
        {
            SPEED = (TEMP - TH) * 20;
            if(SPEED < 100)SPEED = 100; 
        }
    }
    switch(lcd_page) //0：工作页面 1：设置界面
    {
        case 0:
        {
            LCD_DisplayStringLine(Line1, (unsigned char *)"        DATA       ");
            sprintf(str,"      MODE:%s     ",auto_set? "AUTO":"MANU");
            LCD_DisplayStringLine(Line3, (unsigned char *)str);
            sprintf(str,"      TEMP:%.1f     ",TEMP);
            LCD_DisplayStringLine(Line4, (unsigned char *)str);
            sprintf(str,"      SPEED:%d     ",SPEED);
            LCD_DisplayStringLine(Line5, (unsigned char *)str);
        break;
        }
        case 1:
        {
            LCD_DisplayStringLine(Line1, (unsigned char *)"        SET       ");
            sprintf(str,"      TH:%d     ",TH);
            LCD_DisplayStringLine(Line3, (unsigned char *)str);
            LCD_DisplayStringLine(Line4, (unsigned char *)"                    ");
            LCD_DisplayStringLine(Line5, (unsigned char *)"                    ");
        break;
        }
    }

//LED
    if(lcd_page == 0) ucled = (ucled & ~0x01)|0x01;
    else ucled = (ucled & ~0x01)|0x00;

    if(lcd_page == 1) ucled = (ucled & ~0x02)|0x02;
    else ucled = (ucled & ~0x02)|0x00;

    if(auto_set == 1) ucled = (ucled & ~0x04)|0x04;
    else ucled = (ucled & ~0x04)|0x00;

    if(SPEED > 800)
    {
        if(led8_sine <= 10)
        ucled = (ucled & ~0x80)|0x80;
        else ucled = (ucled & ~0x80)|0x00;
    }
    else ucled = (ucled & ~0x80)|0x00;
    Led_Disp(ucled);
    
//    sprintf(str,"      RX:%s     ",uart_rx);
//    LCD_DisplayStringLine(Line9, (unsigned char *)str);
//    sprintf(str,"led8_sine:%d     ",led8_sine);
//    LCD_DisplayStringLine(Line8, (unsigned char *)str);

}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) // 10ms
{
    if(htim == &htim6) Key_Serv();
    led8_sine++;
    if(led8_sine == 21) led8_sine = 1;
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{   
    sprintf(uart_data,"%s",uart_rx);
    if(strcmp(uart_data,"M") == 0)
    {
        sprintf(uart_tx,"Mode:%s\r\n",auto_set ? "AUTO":"MANU");
        HAL_UART_Transmit(&huart1,(uint8_t *)uart_tx,strlen(uart_tx),50);
    }
    else if(strcmp(uart_data,"T") == 0)
    {
        sprintf(uart_tx,"Temp:%.1f\r\n",TEMP);
        HAL_UART_Transmit(&huart1,(uint8_t *)uart_tx,strlen(uart_tx),50);
    }
    else if(strcmp(uart_data,"S") == 0)
    {
        sprintf(uart_tx,"Speed:%drpm\r\n",SPEED);
        HAL_UART_Transmit(&huart1,(uint8_t *)uart_tx,strlen(uart_tx),50);
    }
    else
    {
        sprintf(uart_tx,"ERROR\r\n");
        HAL_UART_Transmit(&huart1,(uint8_t *)uart_tx,strlen(uart_tx),50);
    }
    memset(uart_rx,0,sizeof(uart_rx));
    HAL_UARTEx_ReceiveToIdle_IT(&huart1,(uint8_t *)uart_rx,50);
}



