#include "mymain.h"

char uart_tx[50];
char uart_rx[50];
char text[30];
uint16_t i = 5;
uint8_t led_sta;
uint32_t time500ms;
uint8_t eep_num = 0;

char passwd[10];


float adc_read(ADC_HandleTypeDef * hadc)
{
  uint16_t adc_val;
  float adc_f;
  HAL_ADC_Start(hadc);
  adc_val = HAL_ADC_GetValue(hadc);
  adc_f = adc_val * 3.3f /4096.0f;
  return adc_f;
 
}

float adc_read_os(ADC_HandleTypeDef * hadc)
{
  uint16_t adc_val;
  float adc_f;
  HAL_ADC_Start(hadc);
  adc_val = HAL_ADC_GetValue(hadc);
  adc_f = adc_val * 3.3f /65536.0f;
  return adc_f;
 
}

void setup(void){
    HAL_TIM_Base_Start_IT(&htim6);
    HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_2);
    HAL_TIM_IC_Start(&htim15,TIM_CHANNEL_1);
    HAL_TIM_IC_Start(&htim15,TIM_CHANNEL_2);
    
    HAL_UARTEx_ReceiveToIdle_IT(&huart1,(uint8_t * )uart_rx,50);
    LED_Disp(0X00);
    HAL_ADCEx_Calibration_Start(&hadc2,ADC_SINGLE_ENDED);
    LCD_Init();
    LCD_SetBackColor(Black);
    LCD_SetTextColor(White);
    LCD_Clear(Black);
    I2CInit();
    LED_Chg(4,1);
    sprintf(passwd,"123");
    time500ms = uwTick;
    eep_num = eeprom_read(8);
}

void loop(void){
    float frq1 = 0;
    float duty1 = 0;
    float adc_data_pb15;



    frq1 = 1000000.0f/ (HAL_TIM_ReadCapturedValue(&htim15,TIM_CHANNEL_1) + 1);
    duty1 = (HAL_TIM_ReadCapturedValue(&htim15,TIM_CHANNEL_2) + 1) * 100.0f / (HAL_TIM_ReadCapturedValue(&htim15,TIM_CHANNEL_1) + 1);

    
    sprintf(text,"FREQ:%.2f",frq1);
    LCD_DisplayStringLine(Line4, (unsigned char *)"    Hello,world.   ");
//    sprintf(text,"CNBR:%d     ",i);
    LCD_DisplayStringLine(Line1, (unsigned char *)text);
    sprintf(text,"DUTY:%.2f%%",duty1);
    LCD_DisplayStringLine(Line2, (unsigned char *)text);
    
    sprintf(text,"EEP_NUM:%d",eep_num);
    LCD_DisplayStringLine(Line9, (unsigned char *)text);

    adc_data_pb15 = adc_read_os(&hadc2);
    sprintf(text,"PB15:%.4fV",adc_data_pb15);
    LCD_DisplayStringLine(Line6, (unsigned char *)text);



    if(bkey[1].short_flag == 1)
    {
      LCD_DisplayStringLine(Line5, (unsigned char *)" KEY1 DOWN     ");
      bkey[1].short_flag =0;
      __HAL_TIM_SET_AUTORELOAD(&htim2,250-1);
      __HAL_TIM_SetCompare(&htim2,TIM_CHANNEL_2,125);
    }
    if(bkey[2].short_flag == 1)
    {
      LCD_DisplayStringLine(Line5, (unsigned char *)" KEY2 DOWN     ");
      bkey[2].short_flag =0;
      eep_num+=1;
      eeprom_write(8,eep_num);

    }
    if(bkey[3].long_flag == 1)
    {
      LCD_DisplayStringLine(Line5, (unsigned char *)" KEY3 HOLD");
      bkey[3].long_flag =0;
    }
    if(bkey[4].long_flag == 1)
    {
      LCD_DisplayStringLine(Line5, (unsigned char *)" KEY4 HOLD");
      bkey[4].long_flag =0;
    }
    if(bkey[1].double_flag == 1)
    {
      LCD_DisplayStringLine(Line5, (unsigned char *)" KEY1 Double");
      bkey[1].double_flag =0;
    }
    if(bkey[2].double_flag == 1)
    {
      LCD_DisplayStringLine(Line5, (unsigned char *)" KEY2 Double");
      bkey[2].double_flag =0;
    }
    
    sprintf(uart_tx,"PB15:%.4fV     \r\n",adc_data_pb15);
    HAL_UART_Transmit(&huart1,(uint8_t*)uart_tx,strlen(uart_tx),50);
    
    
    if(uwTick - time500ms > 500)
    {
     static uint8_t LED_sta = 0;
     LED_Chg(2,LED_sta);
     LED_sta = !LED_sta;
     time500ms = uwTick;
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  key_serv();
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{ 
    
    char old_pswd[10];
    char new_pswd[10];
    sscanf(uart_rx,"%3s-%3s",old_pswd,new_pswd); 

    if(strcmp(old_pswd,passwd) == 0)
    {
      for(int i=0;i<3 ;i++)passwd[i] = new_pswd[i];
      sprintf(text,"new:%s        ",passwd);
      LCD_DisplayStringLine(Line8, (unsigned char *)text);
    }
    else
    {

      LCD_DisplayStringLine(Line7, (unsigned char *)" Fail     ");
    }
    HAL_UARTEx_ReceiveToIdle_IT(&huart1,(uint8_t * )uart_rx,50);
}


