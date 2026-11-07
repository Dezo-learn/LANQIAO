#include "mymain.h"
//2026/4/13 finished
//****************ģ��****************
struct BKEY
{
    uint16_t press_time;
    uint16_t double_timer;

    uint8_t short_flag;
    uint8_t long_flag;
    uint8_t double_flag;
};

struct BKEY bkey[5] = {0};

void Led_Disp(uint8_t dsled)
{
    HAL_GPIO_WritePin(GPIOC,GPIO_PIN_All,GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOC,dsled << 8,GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
}

uint8_t Key_Read(void)
{
    if (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0) == GPIO_PIN_RESET)return 1;
    else if (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1) == GPIO_PIN_RESET)return 2;  
    else if (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2) == GPIO_PIN_RESET)return 3;  
    else if (HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0) == GPIO_PIN_RESET)return 4;  
    else return 0;
}

void Key_Serv(void)
{
    uint8_t Key_val;
    Key_val = Key_Read();
    for(uint8_t i = 1; i < 5;i++)
    {
        if(Key_val == i) //����
        {
            bkey[i].press_time++;
        }
        else             //����
        {
            if(bkey[i].press_time >= 100)
            {
                bkey[i].long_flag = 1;
                bkey[i].press_time = 0;
            }
            else if(bkey[i].press_time >= 3)
            {
                if(bkey[i].double_timer > 0)
                {
                    bkey[i].double_flag = 1;
                    bkey[i].double_timer = 0;
                }
                else
                {
                    bkey[i].double_timer = 20;
                }
                bkey[i].press_time = 0;
            }
            
            if(bkey[i].double_timer > 0)
            {
                bkey[i].double_timer--;
                if(bkey[i].double_timer == 0)
                {
                    bkey[i].short_flag = 1;
                }
            }

        }
    }
}



//****************����****************

char str[50];
uint8_t ucled = 0x00;
uint8_t enable = 0;
uint16_t led2_timer = 0;
uint16_t led3_tick = 0;
uint8_t LED3_ENABLE = 0; //0:��Ϊ�ر�LED
uint32_t adc1[2];
uint32_t adc2[1];   //ADC������32λ�� ��DMA��һ��һ��Ҫ��WORLD������
float R37_Value = 0.0f;
float R38_Value = 0.0f;
uint8_t Page = 0;
float R39_Value = 0.0f;
float R40_Value = 0.0f;
float R39_DUTY;
float R40_DUTY;
float Real_Freq = 1000.0f;
float Real_Duty = 0.60f;
char rx_buf[50];
char tx_buf[50];
uint8_t eeprom_num;
float MCP_Value;

//****************����****************
void Set_up(void)
{
    HAL_TIM_Base_Start_IT(&htim6);
    HAL_ADCEx_Calibration_Start(&hadc1,ADC_SINGLE_ENDED);
    HAL_ADCEx_Calibration_Start(&hadc2,ADC_SINGLE_ENDED);
    HAL_ADC_Start_DMA(&hadc1,adc1,2);
    HAL_ADC_Start_DMA(&hadc2,adc2,1);
    HAL_TIM_IC_Start(&htim2,TIM_CHANNEL_1);
    HAL_TIM_IC_Start(&htim2,TIM_CHANNEL_2);
    HAL_TIM_IC_Start(&htim3,TIM_CHANNEL_1);
    HAL_TIM_IC_Start(&htim3,TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim17,TIM_CHANNEL_1);
    HAL_UARTEx_ReceiveToIdle_IT(&huart1,(uint8_t *)rx_buf,sizeof(rx_buf));

    Led_Disp(0x00);

    LCD_Init();
    HAL_Delay(100);;
    LCD_SetBackColor(Black);
    LCD_SetTextColor(White);
    LCD_Clear(Black);
    HAL_Delay(100);
    I2CInit();
}

void Loop(void)
{
//****************ADC****************
    R37_Value = adc2[0] * 3.3f / 65535.0f;
    R38_Value = adc1[0] * 3.3f / 65535.0f;
    MCP_Value = adc1[1] * 3.3f / 65535.0f;

//****************IC****************
    R39_Value = 1000000.0f / (TIM2 -> CCR1 + 1);
    R40_Value = 1000000.0f / (TIM3 -> CCR1 + 1); // ��ʵ - 1 = �Ĵ������ֵ
    R39_DUTY  = (float)(TIM2 -> CCR2 + 1) * 100.0f  / (float)(TIM2 -> CCR1 + 1);
    R40_DUTY  = (TIM3 -> CCR2 + 1) * 100.0f  / (TIM3 -> CCR1 + 1);

//****************PWM****************
    TIM17 -> ARR = 1000000.0f / Real_Freq - 1;
    TIM17 -> CCR1 = 1000000.0f / Real_Freq * Real_Duty - 1;

//****************����****************
    if(bkey[1].short_flag == 1) //����1�̰� + LCD��ʾ + LED������ת
    {
        sprintf(str,"    KEY1_Down!    ");
        LCD_DisplayStringLine(Line0, (unsigned char *)str);
        
        enable = !enable;
        eeprom_num++;
        
        bkey[1].short_flag = 0;
    }
    if(bkey[2].long_flag == 1) //����2���� + LCD��ʾ + LED2���μ���
    {
        sprintf(str,"    KEY2_HODE!    ");
        LCD_DisplayStringLine(Line0, (unsigned char *)str);
        led2_timer = 100; //����һ��
        bkey[2].long_flag = 0;
    }


    if(bkey[3].double_flag == 1) //����3˫�� + LCD��ʾ + LED2ѭ������
    {
        sprintf(str,"    KEY2_DOUBLE!    ");
        LCD_DisplayStringLine(Line0, (unsigned char *)str);
        LED3_ENABLE = !LED3_ENABLE;
        led3_tick = 0;
        bkey[3].double_flag = 0;
    }


    if(bkey[4].double_flag == 1)
    {
        sprintf(str,"    SAVE!    ");
        LCD_DisplayStringLine(Line0, (unsigned char *)str);
        EEPROM_Write(8,eeprom_num);
        
        bkey[4].double_flag = 0;
    }
    if(bkey[4].short_flag == 1)
    {
        eeprom_num = EEPROM_Read(8);
        bkey[4].short_flag = 0;
    }


//****************MCP��λ��****************
    MCP_Write(80); //  RW = 80 / 128 * 100



//****************LCD****************
    switch(Page)
    {
        case 0:
        {   
            sprintf(str,"R37:%.2f",R37_Value);
            LCD_DisplayStringLine(Line1, (unsigned char *)str);
            sprintf(str,"R38:%.2f",R38_Value);
            LCD_DisplayStringLine(Line2, (unsigned char *)str);
            sprintf(str,"R39:%.2f",R39_Value);
            LCD_DisplayStringLine(Line3, (unsigned char *)str);
            sprintf(str,"R40:%.2f",R40_Value);
            LCD_DisplayStringLine(Line4, (unsigned char *)str);
            sprintf(str,"R39_D:%.2f",R39_DUTY);
            LCD_DisplayStringLine(Line5, (unsigned char *)str);
            sprintf(str,"R40_D:%.2f",R40_DUTY);
            LCD_DisplayStringLine(Line6, (unsigned char *)str);
            sprintf(str,"EEPROM:%d",eeprom_num);
            LCD_DisplayStringLine(Line7, (unsigned char *)str);
            sprintf(str,"MCP:%.2f",MCP_Value);
            LCD_DisplayStringLine(Line8, (unsigned char *)str);

            break;  
        }
        case 1:
        {

            break;
        }
    }


//****************LED****************
    ucled = (ucled & ~0x01) | (0x01 * enable);
    if(led2_timer > 0 )ucled = (ucled & ~0x02) | 0x02;
    else ucled = (ucled & ~0x02) | 0x00;
    if(led3_tick <= 100 && led3_tick > 0) ucled = (ucled & ~0x04) | 0x04;
    else ucled = (ucled & ~0x04) | 0x00;
    Led_Disp(ucled);
}




//****************�ж�****************
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim == &htim6)
    {
        Key_Serv();
    }

    if(led2_timer > 0) led2_timer--;

    if(LED3_ENABLE)
    {
        led3_tick++;
        if(led3_tick > 200) led3_tick = 1;
    }
    
    
}
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    
    memcpy(tx_buf, rx_buf, sizeof(rx_buf));
    HAL_UART_Transmit(&huart1,(uint8_t *)tx_buf,sizeof(tx_buf),50);


    
    memset(rx_buf,0,sizeof(rx_buf));
    HAL_UARTEx_ReceiveToIdle_IT(&huart1,(uint8_t *)rx_buf,sizeof(rx_buf));
}


/* led��ʾ����· �����μ�����
    // �ж��� (10msһ��)
    if(timer > 0) timer--;

    // Loop��
    if(����) timer = 100; // ����1��ĵ�
    if(timer > 0) LED_ON; else LED_OFF; //���Զ���0
*/ 

/* led��ʾ����· ��ѭ��������
// �ж��� (10msһ��)
blink_tick++;
if(blink_tick > 100) blink_tick = 1; // 1��һ������ ����0  *��Ȼ�ϵ����LED���Լ���(ֻ�������Ҫ����0)

// Loop�� (0.5s��, 0.5s��)
if(blink_tick <= 50 && blink_tick > 50) LED_ON; 
else LED_OFF;
*/ 


