#include "mymain.h"

char uart_tx[50];
char uart_rx[50];
char text[30];      //显示缓存区
uint16_t i = 5;
uint8_t led_sta;
uint32_t time500ms;
uint8_t Key_Up , Key_Down , Key_Old, Key_Val;
uint16_t adc_Buf[2];


void setup(void){
  HAL_TIM_Base_Start_IT(&htim6);              //启用按键扫描的中断
  HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_2);    //定时器PWM输出启用
  HAL_TIM_IC_Start(&htim15,TIM_CHANNEL_1);    //IC捕获通道1启用
  HAL_TIM_IC_Start(&htim15,TIM_CHANNEL_2);    //IC捕获通道2启用


  HAL_UARTEx_ReceiveToIdle_IT(&huart1,(uint8_t * )uart_rx,50); //串口接收空闲中断服务函数启动


  HAL_ADCEx_Calibration_Start(&hadc2,ADC_SINGLE_ENDED);        //ADC通道采集打开
  HAL_ADCEx_Calibration_Start(&hadc1,ADC_SINGLE_ENDED);        //ADC自校准通道打开
  HAL_ADC_Start_DMA(&hadc1,(uint32_t *)adc_Buf,2);


  LED_Disp(0X00);                             //LED灯初始化
  LCD_Init();                                 //LCD初始化
  LCD_SetBackColor(Black);                    //设置LCD背景为黑
  LCD_SetTextColor(White);                    //设置LCD字体为白
  LCD_Clear(Black);                           //清屏为黑
  I2CInit();                                  //IIC初始化
  time500ms = uwTick;
}

void loop(void){
  float frq1 = 0;
  float duty1 = 0;
  float adc_data_pb15;
  adc_data_pb15 = adc_read_os(&hadc2);
   
  sprintf(text,"PB15:%.4fV",adc_data_pb15);
  LCD_DisplayStringLine(Line6, (unsigned char *)text);

  frq1 = 1000000.0f / (HAL_TIM_ReadCapturedValue(&htim15,TIM_CHANNEL_1) + 1); // 捕获频率
  duty1 = (HAL_TIM_ReadCapturedValue(&htim15,TIM_CHANNEL_2) + 1) * 100.0f / (HAL_TIM_ReadCapturedValue(&htim15,TIM_CHANNEL_1) + 1); //捕获占空比



  sprintf(text,"FREQ:%.2f",frq1);
  LCD_DisplayStringLine(Line4, (unsigned char *)"    Hello,world.   ");
  
  
  if(uwTick - time500ms > 500) //闪灯用的非阻塞延迟
  {
    
  }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  key_serv();
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{ 
    HAL_UARTEx_ReceiveToIdle_IT(&huart1,(uint8_t * )uart_rx,50);
}


