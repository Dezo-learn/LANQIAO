#include "mymain.h"

uint8_t dsled = 0x00;






float A_view_f = 0;//LCD 显示A变量
float B_view_f = 0;//LCD 显示B变量
float A_f = 0;//LCD A变量原
float B_f = 0;//LCD B变量原
float A_view_f_old = 0;//LCD A变量跟踪
float B_view_f_old = 0;//LCD B变量跟踪

float A_MAX = 0;//LCD A变量最大值
float A_MIN = 99999;
float B_MAX = 0;//LCD B变量最大值
float B_MIN = 99999;
uint8_t B_MAX_F = 0;
uint8_t B_MIN_F = 0;
uint8_t A_MAX_F = 0;
uint8_t A_MIN_F = 0;
uint8_t BTimer3S_EN = 0;
uint16_t BTimer3S = 0;
uint8_t ATimer3S_EN = 0;
uint16_t ATimer3S = 0;

uint8_t Page = 0; //LCD显示页面
uint8_t Page_mode = 0; //LCD显示子页面
char text[30]; //LCD缓存区

uint8_t NDA_view = 0; 
uint8_t NDB_view = 0;
uint8_t NHA_view = 0;
uint8_t NHB_view = 0;
int PD_view = 1000;
int PH_view = 5000;
int PX_view = 0;
uint8_t mode1_select = 0;


uint8_t ucled = 0x00;//LED缓存区

void SetUp(void)
{
    HAL_TIM_Base_Start_IT(&htim6);
    HAL_TIM_IC_Start(&htim2,TIM_CHANNEL_1);
    HAL_TIM_IC_Start(&htim3,TIM_CHANNEL_1);
    
    Led_Disp(0x00);
    LCD_Init();
    LCD_SetBackColor(Black);
    LCD_SetTextColor(White);
    LCD_Clear(Black);

}

void Loop(void)
{   
    //1）频率测量
    A_f = 1000000.0f / (HAL_TIM_ReadCapturedValue(&htim2,TIM_CHANNEL_1)); //A捕获值
    B_f = 1000000.0f / (HAL_TIM_ReadCapturedValue(&htim3,TIM_CHANNEL_1)); //B捕获值


    //2）频率校准
    A_view_f = A_f + PX_view;
    B_view_f = B_f + PX_view;


    //3）频率超限
    if(A_view_f_old <= PH_view && A_view_f >= PH_view)
    {
        NHA_view++;
        A_view_f_old = A_view_f;
    }
    else A_view_f_old = A_view_f;

    if(B_view_f_old <= PH_view && B_view_f >= PH_view)
    {
        NHB_view++;
        B_view_f_old = B_view_f;
    }
    else B_view_f_old = B_view_f;

//    sprintf(text,"     B=%.0fHz     ",B_view_f);
//    LCD_DisplayStringLine(Line9, (uint8_t *)text);
//    sprintf(text,"     B=%.0fHz     ",B_view_f_old);
//    LCD_DisplayStringLine(Line8, (uint8_t *)text);

    //4）频率突变
    if(BTimer3S_EN == 0)
    {
        BTimer3S_EN = 1;
        B_MAX = B_view_f;
        B_MIN = B_view_f;
    }

    if(B_view_f > B_MAX) B_MAX = B_view_f;
    if(B_view_f < B_MIN) B_MIN = B_view_f;
    
    if(ATimer3S_EN == 0)
    {
        ATimer3S_EN = 1;
        A_MAX = A_view_f;
        A_MIN = A_view_f;
    }

    if(A_view_f > A_MAX) A_MAX = A_view_f;
    if(A_view_f < A_MIN) A_MIN = A_view_f;
//    sprintf(text,"     ATimer3S_EN %d     ",ATimer3S_EN);
//    LCD_DisplayStringLine(Line9, (uint8_t *)text);

//    sprintf(text,"     ATimer3S %d     ",ATimer3S);
//    LCD_DisplayStringLine(Line8, (uint8_t *)text);
/**********按键设置*************/
    if(bkey[1].short_flag == 1)
    {
        if(Page == 1)
        {
            if(mode1_select == 0) PD_view += 100;
            if(mode1_select == 1) PH_view += 100;
            if(mode1_select == 2) PX_view += 100;
            
            if(PX_view >= 1000)PX_view = 1000;
            if(PD_view >= 1000)PD_view = 1000;
            if(PH_view >= 10000)PH_view = 10000;
        }
        bkey[1].short_flag = 0;
    }


    if(bkey[2].short_flag == 1)
    {
        if(Page == 1)
        {
            if(mode1_select == 0) PD_view -= 100;
            if(mode1_select == 1) PH_view -= 100;
            if(mode1_select == 2) PX_view -= 100;

            if(PD_view <= 0)PD_view = 100;
            if(PH_view <= 1000)PH_view = 1000;
            if(PX_view <= -1000)PX_view = -1000;
        }

        bkey[2].short_flag = 0;
    }



    if(bkey[3].short_flag == 1)
    {
        if(Page == 1)
        {
            mode1_select++;
            if(mode1_select == 3)mode1_select=0; //0:PD 1:PH 2:PX
        }
        if(Page == 0)
        {
            Page_mode++;
            if(Page_mode == 2)Page_mode=0;
        }
        
        bkey[3].short_flag = 0;
    }


    if(bkey[4].short_flag == 1)
    {
        Page++;
        if(Page == 3)Page = 0;
        
        if(Page == 1)mode1_select = 0;
        if(Page == 0)Page_mode = 0;
        bkey[4].short_flag = 0;
    }


    if(bkey[3].long_flag == 1)
    {
        if(Page == 2)
        {   
            NDA_view = 0;
            NDB_view = 0;
            NHA_view = 0;
            NHB_view = 0;
        }
        bkey[3].long_flag = 0;
    }

    //LCD显示
    switch(Page){  //0：数据页面 1：参数页面 2：统计页面
        case 0:
        {
            if(Page_mode == 0) //子页面0 F
            {
                LCD_DisplayStringLine(Line1, (uint8_t *)"        DATA        ");
                if(A_view_f <= 1000.0f && A_view_f >= 0.0f)
                {
                    sprintf(text,"     A=%.0fHz     ",A_view_f);
                    LCD_DisplayStringLine(Line3, (uint8_t *)text);
                }
                else if(A_view_f > 1000.0f)
                {
                    sprintf(text,"     A=%.2fKHz     ",A_view_f / 1000.0f);
                    LCD_DisplayStringLine(Line3, (uint8_t *)text);
                }
                else
                {
                    LCD_DisplayStringLine(Line3, (uint8_t *)"     A=NULL     ");
                }


                if(B_view_f <= 1000.0f && B_view_f >= 0.0f)
                {
                    sprintf(text,"     B=%.0fHz     ",B_view_f);
                    LCD_DisplayStringLine(Line4, (uint8_t *)text);
                }
                else if(B_view_f > 1000.0f)
                {
                    sprintf(text,"     B=%.2fKHz     ",B_view_f / 1000.0f);
                    LCD_DisplayStringLine(Line4, (uint8_t *)text);
                }
                else
                {
                    LCD_DisplayStringLine(Line4, (uint8_t *)"     B=NULL     ");
                }
            }

            else //子页面1 ms/usw   
            {
                LCD_DisplayStringLine(Line1, (uint8_t *)"        DATA        ");
                if(A_view_f <= 1000.0f && A_view_f >= 0.0f)
                {
                    sprintf(text,"     A=%.2fmS     ",(1.0f / A_view_f) * 1000.0f);
                    LCD_DisplayStringLine(Line3, (uint8_t *)text);
                }
                else if(A_view_f > 1000.0f)
                {
                    sprintf(text,"     A=%.0fuS     ",(1.0f / A_view_f) * 1000000.0f);
                    LCD_DisplayStringLine(Line3, (uint8_t *)text);
                }
                else
                {
                    LCD_DisplayStringLine(Line3, (uint8_t *)"     A=NULL     ");
                }


                if(B_view_f <= 1000.0f && B_view_f >= 0.0f)
                {
                    sprintf(text,"     B=%.2fmS     ",(1.0f / B_view_f) * 1000.0f);
                    LCD_DisplayStringLine(Line4, (uint8_t *)text);
                }
                else if(B_view_f > 1000.0f)
                {
                    sprintf(text,"     B=%.0fuS     ",(1.0f / B_view_f) * 1000000.0f);
                    LCD_DisplayStringLine(Line4, (uint8_t *)text);
                }
                else
                {
                    LCD_DisplayStringLine(Line4, (uint8_t *)"     B=NULL     ");
                }
            }
            LCD_DisplayStringLine(Line5, (uint8_t *)"                     ");
            LCD_DisplayStringLine(Line6, (uint8_t *)"                     ");
        break;
        }
        case 1:
        {
            LCD_DisplayStringLine(Line1, (uint8_t *)"        PARA        ");
            sprintf(text,"     PD=%dHz     ",PD_view);
            LCD_DisplayStringLine(Line3, (uint8_t *)text);
            sprintf(text,"     PH=%dHz     ",PH_view);
            LCD_DisplayStringLine(Line4, (uint8_t *)text);
            if(PX_view >= 0)
                sprintf(text,"     PX=%dHz     ",PX_view);
            else
                sprintf(text,"     PX=-%dHz     ",PX_view * -1);
            LCD_DisplayStringLine(Line5, (uint8_t *)text);
            LCD_DisplayStringLine(Line6, (uint8_t *)"                     ");
        break;
        }
        case 2:
        {
            LCD_DisplayStringLine(Line1, (uint8_t *)"        RECD        ");
            sprintf(text,"     NDA=%d     ",NDA_view);
            LCD_DisplayStringLine(Line3, (uint8_t *)text);
            sprintf(text,"     NDB=%d     ",NDB_view);
            LCD_DisplayStringLine(Line4, (uint8_t *)text);
            sprintf(text,"     NHA=%d     ",NHA_view);
            LCD_DisplayStringLine(Line5, (uint8_t *)text);
            sprintf(text,"     NHB=%d     ",NHB_view);
            LCD_DisplayStringLine(Line6, (uint8_t *)text);
        break;
        }
    }



//*************LED************
    if(Page == 0)
    {
        ucled = (ucled & ~0x01) | 0x01;
        Led_Disp(ucled);
    }
    else
    {
        ucled = (ucled & ~0x01) | 0x00;
        Led_Disp(ucled);
    }

    if(A_view_f > PH_view)
    {
        ucled = (ucled & ~0x02) | 0x02;
        Led_Disp(ucled);
    }
    else
    {
        ucled = (ucled & ~0x02) | 0x00;
        Led_Disp(ucled);
    }

    if(B_view_f > PH_view)
    {
        ucled = (ucled & ~0x04) | 0x04;
        Led_Disp(ucled);
    }
    else
    {
        ucled = (ucled & ~0x04) | 0x80;
        Led_Disp(ucled);
    }

    if(NDA_view >= 3 || NDB_view >= 3)
    {
        ucled = (ucled & ~0x80) | 0x80;
        Led_Disp(ucled);
    }
    else
    {
        ucled = (ucled & ~0x80) | 0x00;
        Led_Disp(ucled);
    }
//        ucled = (ucled & ~0x80) | 0x80;
//        Led_Disp(ucled);
}
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    Key_Serv();

//    if(BTimer3S_EN)
//    {
//        BTimer3S++;
//        if(BTimer3S < 300 && B_MIN_F == 1 && B_MAX_F == 1)
//        {
//            if(B_MAX - B_MIN >= PD_view)
//            {
//                NDB_view++;
//                BTimer3S = 300;
//            }
//        }
//        if(BTimer3S == 300)
//        {
//            B_MIN_F = B_MAX_F = 0;
//            BTimer3S = 0;
//            BTimer3S_EN = 0;
//        }
//    }

//    if(ATimer3S_EN)
//    {
//        ATimer3S++;
//        if(ATimer3S < 300 && A_MIN_F == 1 && A_MAX_F == 1)
//        {
//            if(A_MAX - A_MIN >= PD_view)
//            {
//                NDA_view++;
//                ATimer3S = 300;
//            }
//        }
//        if(ATimer3S == 300)
//        {
//            B_MIN_F = B_MAX_F = 0;
//            ATimer3S = 0;
//            ATimer3S_EN = 0;
//        }
//    }

    if(BTimer3S_EN)
    {
        BTimer3S++;

        if(BTimer3S >= 300)   //3秒
        {
            if(B_MAX - B_MIN >= PD_view)
            {
                NDB_view++;
            }

            BTimer3S = 0;
            BTimer3S_EN = 0;
        }
    }

    if(ATimer3S_EN)
    {
        ATimer3S++;

        if(ATimer3S >= 300)   //3秒
        {
            if(A_MAX - A_MIN >= PD_view)
            {
                NDA_view++;
            }

            ATimer3S = 0;
            ATimer3S_EN = 0;
        }
    }
}


