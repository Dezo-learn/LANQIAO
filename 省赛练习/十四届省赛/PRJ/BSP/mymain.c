#include "mymain.h"



uint8_t K_view = 1;  //K显示变量
uint8_t R_view = 1;  //R显示变量
uint8_t K = 1;       //K存储变量
uint8_t R = 1;       //R存储变量
uint8_t Select = 0 ; //选择R or K    0;R  1:K
uint8_t Para = 0;    //0:数据界面 1:参数界面 2:记录界面
uint8_t N = 0;       //L H 切换计数
uint8_t Locked;      //锁定变量
uint8_t dsled = 0x00;//LED缓存变量
uint8_t mode = 0;    //控制高速低速显示
uint8_t last_mode =0;
uint8_t enable_5s_counter = 0; //5s定时器使能
uint16_t _5s_counter = 0;      //5s定时器
//uint8_t test = 0;  //测试变量

uint32_t timer_100ms;//100ms闪灯计时器

char text[30];          //LCD临时缓存
char M[2] = {'L','H'};  //高速低速数组 <- mode控制 

float R37_value = 0;    //ADC读取的R37的值
float ML = 0;           //MAX_L低速下的速度最大值
float MH = 0;           //MAX_H高速下的速度最大值
float last_ML = 0;
float last_MH = 0;


float V = 0;            //输出实时速度（计算后的）
float P = 0;            //输出实时占空比

uint32_t Freq_T = 4000;
uint8_t Freq_T_counter_EN;
uint16_t Freq_T_counter;
uint8_t Up_Flag = 0;
uint8_t Down_Flag = 0;
uint8_t MHL_counter_EN;
uint16_t ML_counter;
uint16_t MH_counter;

void setup(void)
{
  HAL_TIM_Base_Start_IT(&htim6);
  HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_2);
  HAL_TIM_IC_Start(&htim3,TIM_CHANNEL_1);
  HAL_TIM_IC_Start(&htim3,TIM_CHANNEL_2);

  HAL_ADCEx_Calibration_Start(&hadc2,ADC_SINGLE_ENDED);
  LCD_Init();
  LCD_SetBackColor(Black);
  LCD_SetTextColor(White);
  LCD_Clear(Black);
  Led_Disp(0x00); //灯全灭
  timer_100ms = uwTick;
}


float Adc_GetValue()
{
    uint16_t adc_val;
    HAL_ADC_Start(&hadc2);
    adc_val = HAL_ADC_GetValue(&hadc2);
    return adc_val * 3.3f / 65536.0f;
}


void loop(void)
{
  float frq1 = 6000;
  frq1 = 1000000.0f / (HAL_TIM_ReadCapturedValue(&htim3,TIM_CHANNEL_2)+1);
  V = frq1 * 2.0f * 3.14f * R / 100.0f / K ;
    
    
   
//  sprintf(text,"Frq1:%.2f         ",frq1);
//  LCD_DisplayStringLine(Line9, (uint8_t *)text);


//*******************PWM输出
    if(last_mode == 1 && mode == 0)
    {
        //Freq_T = 8000;//4K  1 000 000 / Frq_T = 4000
        Freq_T_counter_EN = 1;
        Down_Flag = 1;
    }
    else if(last_mode == 0 && mode == 1)
    {
        //Freq_T = 4000;// 8K
        Freq_T_counter_EN = 1;
        Up_Flag = 1;
    }

    __HAL_TIM_SET_AUTORELOAD(&htim2,1000000 / Freq_T -1);//4K 8K
    R37_value = Adc_GetValue();
//    sprintf(text,"     MODE=%d      ",ML_counter);
//    LCD_DisplayStringLine(Line8, (uint8_t *)text);

//    sprintf(text,"     LMODE=%.2f      ",last_ML);
//    LCD_DisplayStringLine(Line9, (uint8_t *)text);

//    sprintf(text,"     ML=%.2f      ",ML);
//    LCD_DisplayStringLine(Line7, (uint8_t *)text);

    if(!Locked)
    {
        if(R37_value <=  1.00f) {__HAL_TIM_SetCompare(&htim2,TIM_CHANNEL_2,(1000000 / Freq_T ) * 0.10f);P = 10.0f;}
        if(R37_value >=  3.00f) {__HAL_TIM_SetCompare(&htim2,TIM_CHANNEL_2,(1000000 / Freq_T ) * 0.85f);P = 85.0f;}
        if(R37_value <  3.00f && R37_value >  1.00f){__HAL_TIM_SetCompare(&htim2,TIM_CHANNEL_2,(0.375f * (R37_value-1.0f) + 0.10f) * (1000000 / Freq_T ) - 1.0f);P = 37.5f * (R37_value - 1.0f) + 10.0f ;}
    }
//*******************按键控制
  if(bkey[1].short_flag == 1) // 按键1
  {
   Para++;
   if(Para == 3)Para = 0;
   if(Para == 2)
    {
        K = K_view;
        R = R_view;
    }
   if(Para == 1) Select = 0;
   bkey[1].short_flag = 0;
  }



  if(bkey[2].short_flag == 1) //按键2
  {
    if(Para == 0 && enable_5s_counter == 0)
    {
     mode++;
     if(mode == 2) mode = 0;
     enable_5s_counter = 1;
     N++;
    }
    if(Para == 1)
    {
     Select++;
     if(Select == 2) Select = 0;
    }

     bkey[2].short_flag = 0;
  }


  if(bkey[3].short_flag == 1) //按键3
  {
   if(Select ==  0)
   {
     R_view ++;
     if(R_view == 11) R_view = 1;
   }
   else
   {
     K_view++;
     if(K_view == 11) K_view = 1;
   }

   bkey[3].short_flag = 0;
  }


  if(bkey[4].short_flag == 1)//按键4
  {
   if(Para == 1)
   {
       if(Select ==  0)
       {
         R_view --;
         if(R_view == 0) R_view = 10;
       }
       else
       {
         K_view --;
         if(K_view == 0) K_view = 10;
       }

       bkey[4].short_flag = 0;
   }
   if(Para == 0)
    {
       if(Locked == 1) Locked = 0;
    }


    bkey[4].short_flag = 0;
  }

  if(bkey[4].long_flag == 1) //按键4_长按
  {
    if(Para == 0)
    {
        Locked = 1;
    }
    bkey[4].long_flag = 0;
  }



//   sprintf(text,"     N=%d      ",_5s_counte r);
//   LCD_DisplayStringLine(Line9, (uint8_t *)text); //测试按键五秒计时


//*******************统计
//    if(mode)
//    {
//        if(V > MH)  MH = V;
//    }
//    else
//    {
//        if(V > ML)  ML = V;
//    }


    if(mode)
    {
        if(V > last_MH)
        {
            MH = V;
            MHL_counter_EN = 1;
        }
    }
    else
    {
        if(V > last_ML)
        {
            ML = V;
            MHL_counter_EN = 1;
        }
    }


//*******************LCD显示页面
  switch(Para){
    case 0:
    {
      LCD_DisplayStringLine(Line1, (uint8_t *)"        DATA         ");
      sprintf(text,"     M=%c      ",M[mode]);
      LCD_DisplayStringLine(Line3, (uint8_t *)text);
      sprintf(text,"     P=%.0f%%      ",P);
      LCD_DisplayStringLine(Line4, (uint8_t *)text);
      sprintf(text,"     V=%.1f      ",V);
      LCD_DisplayStringLine(Line5, (uint8_t *)text);
      break;
    }
    case 1:
    {
      LCD_DisplayStringLine(Line1, (uint8_t *)"        PARA         ");
      sprintf(text,"     R=%d      ",R_view);
      LCD_DisplayStringLine(Line3, (uint8_t *)text);
      sprintf(text,"     K=%d      ",K_view);
      LCD_DisplayStringLine(Line4, (uint8_t *)text);
      LCD_DisplayStringLine(Line5,(uint8_t *)"                   ");
      break;
    }
    case 2:
    {
      LCD_DisplayStringLine(Line1, (uint8_t *)"        RECD         ");
      sprintf(text,"     N=%d      ",N);
      LCD_DisplayStringLine(Line3, (uint8_t *)text);
      sprintf(text,"     MH=%.1f      ",last_MH);
      LCD_DisplayStringLine(Line4, (uint8_t *)text);
      sprintf(text,"     ML=%.1f      ",last_ML);
      LCD_DisplayStringLine(Line5, (uint8_t *)text);
      break;
    }
  }


//*******************LED显示
//LED1
    if(Para == 0)dsled  = (dsled & ~0x01) | 0x01;
    else dsled = (dsled & ~0x01) | 0x00;


    if(enable_5s_counter == 1)
    {   
        if(uwTick - timer_100ms > 100) //led2闪烁代码
        {
            static uint8_t led2_sta = 0;
            dsled  = (dsled & ~0x02) | (0x02 * led2_sta);
            led2_sta = !led2_sta;
            timer_100ms = uwTick;
        }
    }
    else dsled  = (dsled & ~0x02) | 0x00;


    if(Locked == 1) dsled  = (dsled & ~0x04) | 0x04;
    else dsled  = (dsled & ~0x04) | 0x00;
    Led_Disp(dsled);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) // 10ms定时器
{
    key_serv();
    if(enable_5s_counter)
      {
        _5s_counter++;
        if(_5s_counter == 500)
        {
          _5s_counter = 0;
          enable_5s_counter = 0;
          
        }
      }

    if(Freq_T_counter_EN)
    {   
        Freq_T_counter++;
        if(Freq_T_counter == 12)
        {
            if(Up_Flag == 1)
            {
                Freq_T += 100;
                if(Freq_T ==  8000)
                {
                    Up_Flag = 0;
                    last_mode = mode;
                    Freq_T_counter_EN = 0;
                }
            }
            else if(Down_Flag == 1)
            {
                Freq_T -= 100;
                if(Freq_T ==  4000)
                {
                    Down_Flag = 0;
                    last_mode = mode;
                    Freq_T_counter_EN = 0;
                }
            }
            Freq_T_counter = 0;
        }
    }
    if(MHL_counter_EN == 1)
    { 
        if(MH != last_MH) MH_counter ++;
        else {MHL_counter_EN = 0;MH_counter = 0;}
        if(MH_counter == 200){last_MH = MH;MH_counter = 0;MHL_counter_EN = 0;}

        if(ML != last_ML) ML_counter ++;
        else{MHL_counter_EN = 0;ML_counter = 0;}
        if(ML_counter == 200){last_ML = ML;ML_counter = 0;MHL_counter_EN = 0;}
    }
}

