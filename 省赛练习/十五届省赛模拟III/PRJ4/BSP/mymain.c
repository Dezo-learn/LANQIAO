#include "mymain.h"

uint16_t adc1_value[1];  //必须是16位的
uint16_t adc2_value[1];  //必须是16位的
float R38_value;
float R37_value;
char Lcd_temp[50];

uint8_t Lcd_Page = 0;

float SR37_Min = 1.2;
float SR37_Max = 2.2;
float SR38_Min = 1.4;
float SR38_Max = 3.0;


float PR37 = 0;
float PR38 = 0;

float R37_PASS = 0;
float R37_Counter = 0;
float R38_PASS = 0;
float R38_Counter = 0;
uint8_t Select = 0;
uint8_t Clean_Flag= 0;
uint8_t STD_Change = 0;

char Rx_Buf[50];
char Tx_Buf[100]; 

char STD_string37[50];
char STD_string38[50];

uint8_t R37_PASS_F;
uint8_t R38_PASS_F;
uint8_t timer_38 = 0;
uint8_t timer_37 = 0;

uint8_t ucled;

void SetUp(void)
{
    HAL_TIM_Base_Start_IT(&htim6);

    HAL_ADCEx_Calibration_Start(&hadc1,ADC_SINGLE_ENDED);
    HAL_ADCEx_Calibration_Start(&hadc2,ADC_SINGLE_ENDED);

    HAL_ADC_Start_DMA(&hadc1,(uint32_t *)adc1_value,1);
    HAL_ADC_Start_DMA(&hadc2,(uint32_t *)adc2_value,1);  //记得加强转

    HAL_UARTEx_ReceiveToIdle_IT(&huart1,(unsigned char *)Rx_Buf,50);
    LCD_Init();
    LCD_SetBackColor(Black);
    LCD_SetTextColor(White);
    LCD_Clear(Black);
    Led_Disp(0x00);
    
    sprintf(STD_string37,"R37");
    sprintf(STD_string38,"R38");
//    LCD_DisplayStringLine(Line1, (unsigned char *)"    Hello,world.   ");
}

void Loop(void)
{
    R38_value = adc1_value[0] * 3.3f / 65536.0f;
    R37_value = adc2_value[0] * 3.3f / 65536.0f;



/******按键控制模块*****/

//按键1
    if(Bkey[1].short_flag)
    {
        Lcd_Page++;
        if(Lcd_Page == 3) Lcd_Page = 0;

        Bkey[1].short_flag = 0;
    }

//按键2
    if(Bkey[2].short_flag)
    {
        if(Lcd_Page == 0)
        {
            if(R37_value <= SR37_Max && R37_value >= SR37_Min)
            {
                R37_PASS ++;
                R37_PASS_F = 1;
            }
            R37_Counter++;
        }

        if(Lcd_Page == 1)
        {
            Select++;
            if(Select == 4) Select = 0;
        }
        Bkey[2].short_flag = 0;
    }

//按键3
    if(Bkey[3].short_flag)
    {
        if(Lcd_Page == 0)
        {
            if(R38_value <= SR38_Max && R38_value >= SR38_Min)
            {
                R38_PASS ++;
                R38_PASS_F = 1;
            }
            R38_Counter++;
        }

        if(Lcd_Page == 1)
        {
            switch(Select)
            {
                case 0:
                {
                    SR37_Max += 0.2f;
                    if(SR37_Max >= 3.2f) SR37_Max = 2.2f;
                break;
                }
                case 1:
                {
                    SR37_Min += 0.2f;
                    if(SR37_Min >= 2.2f) SR37_Min = 1.2f;
                break;
                }
                case 2:
                {
                    SR38_Max += 0.2f;
                    if(SR38_Max >= 3.2f) SR38_Max = 2.2f;
                break;
                }
                case 3:
                {
                    SR38_Min += 0.2f;
                    if(SR38_Min >= 2.2f) SR38_Min = 1.2f;
                break;
                }
            }
            STD_Change = 1;
        }
        Bkey[3].short_flag = 0;
    }

//按键4
    if(Bkey[4].short_flag)
    {
        if(Lcd_Page == 2)
        {
            Clean_Flag = 1;
        }

        if(Lcd_Page == 1) //   这里的判断需要都以<=的形式来写   如果正常写 < 某个值往往到不了这个值    这是由于float的精度导致的
        {
            switch(Select)
            {
                case 0:
                {
                    SR37_Max -= 0.2f;
                    if(SR37_Max <= 2.0f) SR37_Max = 3.0f;
                break;
                }
                case 1:
                {
                    SR37_Min -= 0.2f;
                    if(SR37_Min <= 1.0f) SR37_Min = 2.0f;
                break;
                }
                case 2:
                {
                    SR38_Max -= 0.2f;
                    if(SR38_Max <= 2.0f) SR38_Max = 3.0f;
                break;
                }
                case 3:
                {
                    SR38_Min -= 0.2f;
                    if(SR38_Min <= 1.0f) SR38_Min = 2.0f;
                break;
                }
            }
            STD_Change = 1;
        }
        Bkey[4].short_flag = 0;
    }



/******题目要求的相应公式******/
    if(R37_Counter != 0.0f) PR37 = R37_PASS / R37_Counter * 100.0f;
    if(R38_Counter != 0.0f) PR38 = R38_PASS / R38_Counter * 100.0f;

    if(Clean_Flag) //B4的清零
    {
        PR37 = 0.0f;
        PR38 = 0.0f;
        R37_PASS = 0.0f;
        R37_Counter = 0.0f;
        R38_PASS = 0.0f;
        R38_Counter = 0.0f;
        Clean_Flag = 0;
    }
    
    if(STD_Change)//标准改变的清零
    {
        PR37 = 0.0f;
        PR38 = 0.0f;
        R37_PASS = 0.0f;
        R37_Counter = 0.0f;
        R38_PASS = 0.0f;
        R38_Counter = 0.0f;
        STD_Change = 0;
    }

//测试
//    sprintf(Lcd_temp,"     R37_P:%.2f    ",R37_PASS);
//    LCD_DisplayStringLine(Line8,(unsigned char *)Lcd_temp);
//    sprintf(Lcd_temp,"     R37_C:%.2f    ",R37_Counter);
//    LCD_DisplayStringLine(Line9,(unsigned char *)Lcd_temp);


/******LCD显示模块*****/
    switch(Lcd_Page) // 0：产品参数界面 1：标准设置界面 2：合格率界面
    {
        case 0:
        {
            LCD_DisplayStringLine(Line1, (unsigned char *)"       GOODS      ");
            sprintf(Lcd_temp,"     R37:%.2fV    ",R37_value);
            LCD_DisplayStringLine(Line3,(unsigned char *)Lcd_temp);
            sprintf(Lcd_temp,"     R38:%.2fV    ",R38_value);
            LCD_DisplayStringLine(Line4,(unsigned char *)Lcd_temp);
        break;
        }
        case 1:
        {
            LCD_DisplayStringLine(Line1, (unsigned char *)"       STANDARD      ");
            sprintf(Lcd_temp,"     SR37:%.1f-%.1f    ",SR37_Min,SR37_Max);
            LCD_DisplayStringLine(Line3,(unsigned char *)Lcd_temp);
            sprintf(Lcd_temp,"     SR38:%.1f-%.1f    ",SR38_Min,SR38_Max);
            LCD_DisplayStringLine(Line4,(unsigned char *)Lcd_temp);
        break;
        }
        case 2:
        {
            LCD_DisplayStringLine(Line1, (unsigned char *)"       PASS      ");
            sprintf(Lcd_temp,"     PR37:%.1f%%      ",PR37);
            LCD_DisplayStringLine(Line3,(unsigned char *)Lcd_temp);
            sprintf(Lcd_temp,"     PR38:%.1f%%      ",PR38);
            LCD_DisplayStringLine(Line4,(unsigned char *)Lcd_temp);
        break;
        }
    }

/****** LED ******/

    if(R37_PASS_F)
    {
        timer_37 = 100;
        R37_PASS_F = 0;
    }
    if(R38_PASS_F)
    {
        timer_38 = 100;
        R38_PASS_F = 0;
    }


    if(timer_37 > 0)ucled = (ucled & ~0x01) | 0x01 ;
    else ucled = (ucled & ~0x01) | 0x00 ;

    if(timer_38 > 0) ucled = (ucled & ~0x02) | 0x02 ;
    else ucled = (ucled & ~0x02) | 0x00 ;

    if(Lcd_Page == 0) ucled = (ucled & ~0x04) | 0x04 ;
    else ucled = (ucled & ~0x04) | 0x00 ;

    if(Lcd_Page == 1) ucled = (ucled & ~0x08) | 0x08 ;
    else ucled = (ucled & ~0x08) | 0x00 ;

    if(Lcd_Page == 2) ucled = (ucled & ~0x10) | 0x10 ;
    else ucled = (ucled & ~0x10) | 0x00 ;
    Led_Disp(ucled);

//    sprintf(Lcd_temp,"R37_Timer:%d",timer_37);
//    LCD_DisplayStringLine(Line7,(unsigned char *)Lcd_temp);
/******串口******/

//    sprintf(Tx_Buf,"R37:%.0f,%.0f,%.2f",R37_PASS,R37_Counter,PR37);
//    LCD_DisplayStringLine(Line7,(unsigned char *)Tx_Buf);

//    LCD_DisplayStringLine(Line8,(unsigned char *)STD_string38);
//    LCD_DisplayStringLine(Line9,(unsigned char *)Rx_Buf);
}






void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    Key_Porc();

    if(timer_37 > 0) timer_37--;
    if(timer_38 > 0) timer_38--;
}

/******串口******/
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    char string[50];
    sscanf(Rx_Buf,"%s",string);

    if(strcmp(Rx_Buf,STD_string37) == 0)
    {
        sprintf(Tx_Buf,"R37:%.0f,%.0f,%.1f%%\r\n",R37_Counter,R37_PASS,PR37);
        HAL_UART_Transmit(&huart1,(uint8_t * )Tx_Buf,strlen(Tx_Buf),100);
    }
    if(strcmp(Rx_Buf,STD_string38) == 0)
    {
        sprintf(Tx_Buf,"R38:%.0f,%.0f,%.1f%%\r\n",R38_Counter,R38_PASS,PR38);
        HAL_UART_Transmit(&huart1,(uint8_t * )Tx_Buf,strlen(Tx_Buf),100);
    }
    memset(Rx_Buf,0,sizeof(Rx_Buf)); // 清空接收数组
    HAL_UARTEx_ReceiveToIdle_IT(&huart1,(unsigned char *)Rx_Buf,50);
}

