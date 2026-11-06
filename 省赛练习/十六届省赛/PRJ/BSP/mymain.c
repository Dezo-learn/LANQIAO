#include "mymain.h"

uint16_t adc2_buf[1] = {0};
uint16_t adc1_buf[1] = {0};
float adc1;
float adc2;
char text[50];
float PA15_Value;


uint8_t Hour = 0; //时
uint8_t Min= 0;   //分
uint8_t Sec= 0;   //秒

uint8_t Hour_save = 0; //时
uint8_t Min_save= 0;   //分
uint8_t Sec_save= 0;   //秒

uint8_t Time_counter = 0;
uint8_t Time_counter_EN = 1;

uint8_t Page = 0;
uint32_t CF_view = 8800;
uint8_t CD_view = 30;
uint32_t DF_view = 8831;
uint8_t LOCKED = 0;

uint32_t CF_save = 0;
uint8_t CD_save = 0;
uint32_t DF_save = 0;
uint32_t XF_save = 0;

uint8_t DS_view = 1;   //占空比步长 
uint8_t DR_view = 80;  //占空比范围 10% - DR
uint32_t FS_view = 100;
uint32_t FR_view = 2000;
uint8_t UNREAL = 0; 
uint8_t UNREAL_last = 0;
uint8_t PARA_select = 0;

uint8_t ucled = 0x00;
void SetUp(void)
{
    HAL_TIM_Base_Start_IT(&htim6);
//    HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim17,TIM_CHANNEL_1);
    HAL_TIM_IC_Start(&htim2,TIM_CHANNEL_1);

    HAL_ADCEx_Calibration_Start(&hadc1,ADC_SINGLE_ENDED);
    HAL_ADCEx_Calibration_Start(&hadc2,ADC_SINGLE_ENDED);
    HAL_ADC_Start_DMA(&hadc1,(uint32_t *)adc2_buf,1);
    HAL_ADC_Start_DMA(&hadc2,(uint32_t *)adc1_buf,1);

    Led_Disp(0x00);//LED复位
    LCD_Init();
    LCD_SetBackColor(Black);
    LCD_SetTextColor(White);
    LCD_Clear(Black);
    
    
//    LCD_DisplayStringLine(Line2, (uint8_t *)"     Hello World");//测试LCD
}

void Loop(void)
{
    adc1 = adc1_buf[0] * 3.3f / 65536.0f;
    adc2 = adc2_buf[0] * 3.3f / 65536.0f;

    PA15_Value = 1000000.0f / HAL_TIM_ReadCapturedValue(&htim2,TIM_CHANNEL_1); //PA15捕获值
    DF_view = PA15_Value;


//************主要逻辑************//
    //1）占空比调节策略
    float s_P = 3.3f / ((DR_view - 10) / DS_view);
    uint8_t stand_P = adc1 / s_P;//处于哪级
    uint8_t real_P = (stand_P)* DS_view + 10;

    //2)频率调节策略
    float s_F = 3.3f / ((FR_view - 1000.0f) / FS_view); // 3.3 / ((5000 - 1000) / 7)
    uint32_t stand_F = adc2 / s_F; //处于哪级
    uint32_t real_F = (stand_F)*FS_view + 1000; 
    

    sprintf(text,"     s_F %.4f     ",s_F);
    LCD_DisplayStringLine(Line8, (uint8_t *)text);
    sprintf(text,"     adc1 %.4f     ",adc2);
    LCD_DisplayStringLine(Line9, (uint8_t *)text);
    //3)输出锁定功能
    if(LOCKED == 0)
    {
        CF_view = real_F;
        CD_view = real_P;
        __HAL_TIM_SetAutoreload(&htim17,(1000000.0f / real_F) - 1);
        __HAL_TIM_SET_COMPARE(&htim17,TIM_CHANNEL_1,(1000000.0f / real_F * real_P * 0.01f) - 1);
    }
    else
    {

    }
    
    //4)异常判定
    if ((DF_view - real_F)> 1000 ||(real_F - DF_view)> 1000) UNREAL = 1;
    else 
    {
        UNREAL = 0;
        UNREAL_last = UNREAL; // 此时再次复位UNREAL_last
    }


//时钟逻辑
    if(Sec == 60)
    {
        Min++;
        Sec = 0;
    }
    if(Min == 24)
    {
        Hour++;
        Min = 0;
    }

    
//    sprintf(text,"     adc1 %.4f     ",adc1);
//    LCD_DisplayStringLine(Line9, (uint8_t *)text);

//    sprintf(text,"     adc2 %.4f     ",adc2);
//    LCD_DisplayStringLine(Line8, (uint8_t *)text);





//************按键************//
    if(bkey[1].short_flag)
    {   
        Page++;
        if(Page == 3) Page = 0;

        if(Page == 2) PARA_select = 0;
        bkey[1].short_flag = 0;
    }
    if(bkey[2].short_flag)
    {
        if(Page == 2)
        {
            PARA_select ++;
            if(PARA_select == 4) PARA_select = 0;
        }
        if(Page == 0)LOCKED = !LOCKED;
        bkey[2].short_flag = 0;
    }

    if(bkey[2].long_flag)
    {
        Sec = 0;
        Min = 0;
        Hour = 0;

        bkey[2].long_flag = 0;
    }

    if(bkey[3].short_flag)
    {
        if(Page == 2)
        {
            switch(PARA_select)
            {
                case 0:
                {
                    DS_view++;
                    break;
                }
                case 1:
                {
                    DR_view += 10;
                    break;
                }
                case 2:
                {
                    FS_view += 100;
                    break;
                }
                case 3:
                {
                    FR_view += 1000;
                    break;
                }
            }
        }
        if(DS_view == 101)DS_view = 0;
        if(DR_view == 110)DR_view = 0;

        bkey[3].short_flag = 0;
    }

    if(bkey[4].short_flag)
    {
        if(Page == 2)
        {
            switch(PARA_select)
            {
                case 0:
                {
                    DS_view--;
                    break;
                }
                case 1:
                {
                    DR_view -= 10;
                    break;
                }
                case 2:
                {
                    FS_view -= 100;
                    break;
                }
                case 3:
                {
                    FR_view -= 1000;
                    break;
                }
            }
        }

        if(DS_view == 255) DS_view = 100;
        if(DR_view == 246)DR_view = 100;
        if(FS_view == -100)FS_view = 0; //会有警告—但可以无视
        if(FR_view == -1000)FR_view = 0;
        bkey[4].short_flag = 0;
    }




//异常判定    
    if(UNREAL == 1 && UNREAL_last == 0)
    {
        Sec_save = Sec;
        Min_save = Min;
        Hour_save = Hour;
        
        CF_save = CF_view;
        CD_save = CD_view;
        DF_save = DF_view;
        if((DF_view - real_F) > 0) XF_save = DF_view - real_F;
        else XF_save = real_F - DF_view;
        UNREAL_last = UNREAL; //让UNREAL_last变量置1,防止连续计数
        UNREAL = 0;
    }

//************LCD************//  0：监控  1：统计  2：参数
    
    switch(Page)
    {
        case 0:
        {
            LCD_DisplayStringLine(Line1, (uint8_t *)"       PWM      ");
            
            sprintf(text,"   CF=%dHz     ",CF_view);
            LCD_DisplayStringLine(Line3, (uint8_t *)text);
            sprintf(text,"   CD=%d%%     ",CD_view);
            LCD_DisplayStringLine(Line4, (uint8_t *)text);
            sprintf(text,"   DF=%dHz     ",DF_view);
            LCD_DisplayStringLine(Line5, (uint8_t *)text);
            if(LOCKED)LCD_DisplayStringLine(Line6, (uint8_t *)"   ST=LOCK       ");
            else LCD_DisplayStringLine(Line6, (uint8_t *)"   ST=UNLOCK       ");

            sprintf(text,"   %d%dH%d%dM%d%dS  ",Hour / 10, Hour % 10 ,Min / 10,Min % 10,Sec / 10,Sec % 10);
            LCD_DisplayStringLine(Line7, (uint8_t *)text);
            break;
        }
        case 1:
        {
            LCD_DisplayStringLine(Line1, (uint8_t *)"       RECD      ");
            
            sprintf(text,"   CF=%dHz     ",CF_save);
            LCD_DisplayStringLine(Line3, (uint8_t *)text);
            sprintf(text,"   CD=%d%%     ",CD_save);
            LCD_DisplayStringLine(Line4, (uint8_t *)text);
            sprintf(text,"   DF=%dHz     ",DF_save);
            LCD_DisplayStringLine(Line5, (uint8_t *)text);
            sprintf(text,"   XF=%dHz     ",XF_save);
            LCD_DisplayStringLine(Line6, (uint8_t *)text);


            sprintf(text,"   %d%dH%d%dM%d%dS  ",Hour_save / 10, Hour_save % 10 ,Min_save / 10,Min_save % 10,Sec_save / 10,Sec_save % 10);
            LCD_DisplayStringLine(Line7, (uint8_t *)text);
            break;
        }
        case 2:
        {
            LCD_DisplayStringLine(Line1, (uint8_t *)"       PARA      ");
            
            sprintf(text,"   DS=%d%%     ",DS_view);
            LCD_DisplayStringLine(Line3, (uint8_t *)text);
            sprintf(text,"   DR=%d%%     ",DR_view);
            LCD_DisplayStringLine(Line4, (uint8_t *)text);
            sprintf(text,"   FS=%uHz     ",FS_view);
            LCD_DisplayStringLine(Line5, (uint8_t *)text);
            sprintf(text,"   FR=%uHz     ",FR_view);
            LCD_DisplayStringLine(Line6, (uint8_t *)text);
            LCD_DisplayStringLine(Line7, (uint8_t *)"                ");
            break;
        }
    
    }

//************LED************// 
    if(Page == 0)
    {
        ucled = (ucled & ~ 0x01)| 0x01;
        Led_Disp(ucled);
    }
    else 
    {
        ucled = (ucled & ~ 0x01)| 0x00;
        Led_Disp(ucled);
    }

    if(LOCKED)
    {
        ucled = (ucled & ~ 0x02) | 0x02;
        Led_Disp(ucled);
    }
    else 
    {
        ucled = (ucled & ~ 0x02) | 0x00;
        Led_Disp(ucled);
    }

    if(UNREAL)
    {
        ucled = (ucled & ~ 0x04) | 0x04;
        Led_Disp(ucled);
    }
    else 
    {
        ucled = (ucled & ~ 0x04) | 0x00;
        Led_Disp(ucled);
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) //10ms定时器
{
    key_serv();

    if(Time_counter_EN) Time_counter++;
    if(Time_counter == 100)
    {
        Sec++;
        Time_counter = 0;
    }
}

