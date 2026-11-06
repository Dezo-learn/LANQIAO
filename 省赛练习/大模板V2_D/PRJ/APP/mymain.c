//含IC PWM ADC UART KEY LED LCD  //IIC (EPPROM MCP(数字电位器))
#include "mymain.h"


uint8_t ucled;
uint8_t enable;
uint32_t ucadc1[1];
uint32_t ucadc2[1];
uint32_t Real_Freq;// 实际频率值
uint8_t Real_Duty; // 10 20 30 40 50
float R37_value;
float R38_value;
float FREQ = 0;
float DUTY = 0;
char uart_tx[50];
char uart_rx[50];
char lcd_temp[50];
char stdstring[50];

void Setup(void)
{
    //按键10ms定时器
    HAL_TIM_Base_Start_IT(&htim6);

    //IC捕获
    HAL_TIM_IC_Start(&htim15,TIM_CHANNEL_1);//启用IC捕获1 捕捉上升沿
    HAL_TIM_IC_Start(&htim15,TIM_CHANNEL_2);//启用IC捕获2 捕捉下降沿

    //PWM输出
    HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_2);    //定时器PWM输出启用

    //ADC+超采样+DMA方案
    HAL_ADCEx_Calibration_Start(&hadc1,ADC_SINGLE_ENDED);
    HAL_ADCEx_Calibration_Start(&hadc2,ADC_SINGLE_ENDED);
    HAL_ADC_Start_DMA(&hadc1,(uint32_t *)ucadc1,1);
    HAL_ADC_Start_DMA(&hadc2,(uint32_t *)ucadc2,1);

    //串口 9600 PA9 PA10
    HAL_UARTEx_ReceiveToIdle_IT(&huart1,(uint8_t * )uart_rx,50); //串口接收空闲中断服务函数启动

    //LED与LCD的初始化
    Led_Disp(0x00);
    LCD_Init();
    LCD_SetBackColor(Black);
    LCD_SetTextColor(White);
    LCD_Clear(Black);
    LCD_DisplayStringLine(Line2, (uint8_t *)"     Hello World");//测试LCD
}


void Loop(void)
{
    //IC采集显示 HAL_TIM_ReadCapturedValue
    FREQ = 1000000.0f / (HAL_TIM_ReadCapturedValue(&htim15,TIM_CHANNEL_1) + 1); // 捕获频率 1000000.0f / 上升沿
    DUTY = (HAL_TIM_ReadCapturedValue(&htim15,TIM_CHANNEL_2) + 1) * 100.0f / (HAL_TIM_ReadCapturedValue(&htim15,TIM_CHANNEL_1) + 1); //捕获占空比 下降沿 / 上升沿 * 100

    sprintf(lcd_temp,"     FREQ = %.2f",FREQ);
    LCD_DisplayStringLine(Line7, (uint8_t *)lcd_temp);
    sprintf(lcd_temp,"     DUTY = %.2f",DUTY);
    LCD_DisplayStringLine(Line8, (uint8_t *)lcd_temp);

    //动态调整PWM输出   __HAL_TIM_SetAutoreload 有大写有小写的是大栏的   __HAL_TIM_SET_COMPARE 全小写的是小栏里的
    __HAL_TIM_SetAutoreload(&htim2,(1000000.0f / Real_Freq) - 1); // 这个相当于CubeMX的Counter Period 
    __HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_1,(1000000.0f / Real_Freq * Real_Duty) - 1); //这个相当于CubeMX的CHANNLE 栏下的Pulse


    //ADC采集结果显示
    R37_value = ucadc2[0] * 3.3f / 65536.0f;
    R38_value = ucadc1[0] * 3.3f / 65536.0f;
    
    sprintf(lcd_temp,"     R37 = %.2f",R37_value);
    LCD_DisplayStringLine(Line5, (uint8_t *)lcd_temp);

    sprintf(lcd_temp,"     R38 = %.2f",R38_value);
    LCD_DisplayStringLine(Line6, (uint8_t *)lcd_temp);
 
    //按键模块
    if(bkey[1].short_flag == 1)  //短按检测
    {
        enable = !enable ;
        ucled = (ucled & ~ 0x04) | (0x04 * enable);
        Led_Disp(ucled);


        bkey[1].short_flag = 0;
    }





}

//定时器中断
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    Key_Proc();
}

//串口接收中断
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    //输入处理 sscanf strcmp
    char string[10];
    sscanf(uart_rx,"%3s",string);
    if(strcmp(uart_rx,stdstring) == 1)
    {
    
    }
    //输出上位机
    sprintf(uart_tx,"     R38 = %.2f",R38_value);
    HAL_UART_Transmit(&huart1,(uint8_t * )uart_rx,strlen(uart_rx),50);

    //重新使能中断
    HAL_UARTEx_ReceiveToIdle_IT(&huart1,(uint8_t * )uart_rx,50);
}