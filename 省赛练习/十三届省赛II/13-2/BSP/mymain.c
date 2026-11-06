#include "mymain.h"

struct BKEY
{
    uint16_t press_time;
    uint8_t short_flag;
    uint8_t long_flag;
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
    if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0) == GPIO_PIN_RESET) return 1;
    else if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1) == GPIO_PIN_RESET) return 2;
    else if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2) == GPIO_PIN_RESET) return 3;
    else if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0) == GPIO_PIN_RESET) return 4;
    else return 0;
}

void Key_Serv(void)
{
    uint8_t key_value = Key_Read();
    for(uint8_t i = 1 ;i < 5;i++)
    {
        if(key_value == i) bkey[i].press_time++;
        else
        {
            if(bkey[i].press_time >= 100) bkey[i].long_flag = 1;
            else if(bkey[i].press_time >= 3) bkey[i].short_flag = 1;
            bkey[i].press_time = 0;
        }
    }
}
void EEPROM_Write(uint8_t addr,uint8_t data)
{
    I2CStart();
    I2CSendByte(0XA0);
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
    uint8_t temp;
    I2CStart();
    I2CSendByte(0XA0);
    I2CWaitAck();
    I2CSendByte(addr);
    I2CWaitAck();
    I2CStop();

    I2CStart();
    I2CSendByte(0XA1);
    I2CWaitAck();
    temp = I2CReceiveByte();
    I2CSendNotAck();
    I2CStop();
    return temp;
}
//************主调**************

char lcd_str[50];
uint8_t Lcd_Page = 0;
uint8_t X_num = 0;
uint8_t Y_num = 0;
float X_price = 1.0f;
float Y_price = 1.0f;
uint8_t X_rep = 10;
uint8_t Y_rep = 10;

float X_price_old = 1.0f;
float Y_price_old = 1.0f;
uint8_t X_rep_old = 10;
uint8_t Y_rep_old = 10;

char rx_buf[50];
char rx_data[50];
char tx_buf[50];

uint16_t pwm_timer = 0;
uint8_t ucled = 0x00;
uint8_t led2_blink = 0;
uint8_t led2_timer = 0;

void SetUp(void)
{
    HAL_TIM_Base_Start_IT(&htim6);
    HAL_UARTEx_ReceiveToIdle_IT(&huart1,(uint8_t *)rx_buf,50);
    HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_2);

    Led_Disp(0x00);
    LCD_Init();
    HAL_Delay(100);;
    LCD_SetBackColor(Black);
    LCD_SetTextColor(White);
    LCD_Clear(Black);
    HAL_Delay(100);
    I2CInit();


    if(EEPROM_Read(0) == 255 || EEPROM_Read(1) == 255 || EEPROM_Read(2) == 255 || EEPROM_Read(3) == 255)
    {
        EEPROM_Write(0,10);
        EEPROM_Write(1,10);
        EEPROM_Write(2,10);
        EEPROM_Write(3,10);
    }
    X_rep = EEPROM_Read(0);
    Y_rep = EEPROM_Read(1);
    X_price = (float)EEPROM_Read(2) / 10.0f;
    Y_price = (float)EEPROM_Read(3) / 10.0f;
}


void Loop(void)
{

//************按键**************
    if(bkey[1].short_flag == 1)
    {
        Lcd_Page ++;
        if(Lcd_Page == 3)Lcd_Page= 0;

        bkey[1].short_flag = 0;
    }

    if(bkey[2].short_flag == 1)
    {
        if(Lcd_Page == 0)
        {
            X_num++;
            if(X_num > X_rep) X_num = 0;
        }
        else if(Lcd_Page == 1)
        {
            X_price += 0.1f;
            if(X_price >= 2.1f) X_price = 1.0f;
        }
        else if(Lcd_Page == 2)
        {
            X_rep++;
        }
        bkey[2].short_flag = 0;
    }


    if(bkey[3].short_flag == 1)
    {
        if(Lcd_Page == 0)
        {
            Y_num++;
            if(Y_num > Y_rep) Y_num = 0;
        }
        else if(Lcd_Page == 1)
        {
            Y_price += 0.1f;
            if(Y_price >= 2.1f) Y_price = 1.0f;
        }
        else if(Lcd_Page == 2)
        {
            Y_rep++;
        }
        bkey[3].short_flag = 0;
    }


    if(bkey[4].short_flag == 1)
    {
        if(Lcd_Page == 0)
        {
            X_rep = X_rep - X_num;
            Y_rep = Y_rep - Y_num;
            sprintf(tx_buf,"X:%d,Y:%d,Z:%.1f\r\n",X_num,Y_num,X_num * X_price + Y_num * Y_price);
            HAL_UART_Transmit(&huart1,(uint8_t *)tx_buf,strlen(tx_buf),50);
            X_num = 0;
            Y_num = 0;
            pwm_timer = 500;
        }
        bkey[4].short_flag = 0;
    }
//************PWM && 标志位处理**************
    if(pwm_timer > 0)
    {
        TIM2 -> ARR = 500 - 1;
        TIM2 -> CCR2 = 150 - 1;
    }
    else
    {
        TIM2 -> ARR = 500 - 1;
        TIM2 -> CCR2 = 25 - 1;
    }
    if(X_rep == 0 && Y_rep == 0) led2_blink = 1;
    else led2_blink = 0;
//************EEPROM**************
    if(X_rep_old != X_rep)
    {
        EEPROM_Write(0,X_rep);
        X_rep_old = X_rep;
    }
    if(Y_rep_old != Y_rep)
    {
        EEPROM_Write(1,Y_rep);
        Y_rep_old = Y_rep;
    }
    if(X_price_old != X_price)
    {
        EEPROM_Write(2,(uint8_t)(X_price * 10.0f));
        X_price_old = X_price;
    }
    if(Y_price_old != Y_price)
    {
        EEPROM_Write(3,(uint8_t)(Y_price * 10.0f));
        Y_price_old = Y_price;
    }
//************擦除动作**************
//        EEPROM_Write(0,0xff);
//        EEPROM_Write(1,0xff);
//        EEPROM_Write(2,0xff);
//        EEPROM_Write(3,0xff);
//************LCD显示**************
    switch(Lcd_Page)// 0:购买界面 1:价格界面 2：库存界面
    {
        case 0:
        {
            LCD_DisplayStringLine(Line1, (unsigned char *)"        SHOP        ");
            sprintf(lcd_str,"     X:%d       ",X_num);
            LCD_DisplayStringLine(Line3, (unsigned char *)lcd_str);
            sprintf(lcd_str,"     Y:%d       ",Y_num);
            LCD_DisplayStringLine(Line4, (unsigned char *)lcd_str);
        break;
        }
        case 1:
        {
            LCD_DisplayStringLine(Line1, (unsigned char *)"        PRICE        ");
            sprintf(lcd_str,"     X:%.1f       ",X_price);
            LCD_DisplayStringLine(Line3, (unsigned char *)lcd_str);
            sprintf(lcd_str,"     Y:%.1f       ",Y_price);
            LCD_DisplayStringLine(Line4, (unsigned char *)lcd_str);
        break;
        }
        case 2:
        {
            LCD_DisplayStringLine(Line1, (unsigned char *)"        REP        ");
            sprintf(lcd_str,"     X:%d       ",X_rep);
            LCD_DisplayStringLine(Line3, (unsigned char *)lcd_str);
            sprintf(lcd_str,"     Y:%d       ",Y_rep);
            LCD_DisplayStringLine(Line4, (unsigned char *)lcd_str);
        break;
        }
    }

//***********LED显示**************
    if(pwm_timer > 0) ucled = (ucled & ~0x01) | 0x01;
    else ucled = (ucled & ~0x01) | 0x00;

    if(led2_blink)
    {
        if(led2_timer >= 10) ucled = (ucled & ~0x02) | 0x02;
        else ucled = (ucled & ~0x02) | 0x00;
    }
    else ucled = (ucled & ~0x02) | 0x00;
    Led_Disp(ucled);



}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim == &htim6) Key_Serv();
    if(pwm_timer > 0) pwm_timer --;
    led2_timer ++;
    if(led2_timer == 20) led2_timer = 0;
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    sprintf(rx_data,"%s",rx_buf);
    
    if(strcmp(rx_data,"?") == 0)
    {
            sprintf(tx_buf,"X:%.1f,Y:%.1f\r\n",X_price,Y_price);
            HAL_UART_Transmit(&huart1,(uint8_t *)tx_buf,strlen(tx_buf),50);
    }
    memset(rx_buf,0,50);
    HAL_UARTEx_ReceiveToIdle_IT(&huart1,(uint8_t *)rx_buf,50);
}

