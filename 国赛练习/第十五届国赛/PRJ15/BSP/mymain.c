//十五届国赛

/*--------------------------------------------------------------------------------
 * 1. <string.h> ：字符与内存底层操作函数
 *--------------------------------------------------------------------------------*/

/**
 * @brief strtok(char *str, const char *delim)
 * @note  字符串拆分函数（具有破坏性，会修改原字符串）
 * @param str:   第一次调用传入待拆分数组名；后续循环调用必须传入 NULL。
 * @param delim: 分隔符字符串（例如 "_" 或 ","）。
 * @return 成功返回当前切出的碎片子串首地址；当完全切光或找不到分隔符时返回 NULL。
 * @example
 * char *token = strtok(buf, ",");   // 第一次调用，初始化并切出第一段
 * while(token != NULL) {            // 只要返回值不为 NULL，说明切到了数据
 * // 使用数据，如：atoi(token);
 * token = strtok(NULL, ",");    // 固定搭配：传入 NULL，顺着记忆切下一段
 * }
 */

/**
 * @brief strchr(const char *str, int c)
 * @note  字符精准查找定位函数（注意拼写，末尾带r）
 * @param str: 待查找的字符串。
 * @param c:   要寻找的单个目标字符（例如 '_' 或 '('）。
 * @return 找到了则返回该字符在内存中的【指针地址】；若字符串中没有该字符则返回 NULL。
 * @purpose 常用于检查数据包的合法边界（如通过查找 '(' 和 ')' 来锁定有效数据区间）。
 */

/**
 * @brief strcmp(const char *str1, const char *str2)
 * @note  纯文本字符串比较函数
 * @return == 0 : 两个字符串内容、长度完全一模一样。
 * != 0 : 两个字符串不相同（可通过返回值正负判断字典序，但MCU开发通常只关心是否等于0）。
 * @warning 绝对不能用 if(str1 == str2) 来比较字符串，那是比较两者的硬件内存地址，永远为假。
 */

/**
 * @brief memset(void *ptr, int value, size_t num)
 * @note  内存一键填充函数（嵌入式中最常用于缓冲区清零）
 * @param ptr:   指向要填充的内存块指针（数组名、结构体变量地址等）。
 * @param value: 要设置的单字节值（通常传 0 代表清零）。
 * @param num:   要填充的连续字节数（强烈推荐配合 sizeof() 获取，防止漏写）。
 * @example memset(rx_buf, 0, sizeof(rx_buf)); // 串口接收完并解析后，一键清空缓冲区
 * @warning 严禁用来把 int 或 uint16_t 数组初始化为非0值（如填1），因其按单字节填充会导致多字节数值乱套。
 */

/**
 * @brief memcmp(const void *str1, const void *str2, size_t n)
 * @note  底层内存块二进制比较函数
 * @param n: 强制比较的前 n 个字节数（按字节为单位，而非元素个数）。
 * @return == 0 : 代表这两块物理内存里的二进制数据完全一模一样，中途遇到 0x00 (\0) 绝不刹车。
 * @purpose C语言不支持用 '==' 直接比对两个结构体变量。
 * 通过 if(memcmp(&zb, &zb_zero, sizeof(zb)) == 0) 可完美判断结构体是否全零或相等。
 */


/*--------------------------------------------------------------------------------
 * 2. <stdlib.h> ：数据类型转换函数
 *--------------------------------------------------------------------------------*/

/**
 * @brief atoi(const char *str)  int
 * @note  文本数字转整型数 (Ascii to Integer)
 * @param str: 纯数字字符组成的字符串首地址（如 "48"）。
 * @return 返回转换后的标准有符号整型数 (int)。
 * @feature 1. 从左往右解析，遇到正负号、数字以外的字符（如逗号、字母）会立刻“刹车”并返回前面转好的部分。
 * 2. 如果一开头就是字母、彻底无法转换，则安全返回数字 0。
 * @example atoi("48,92") -> 返回数字 48（遇到逗号自动刹车，完美配合 strtok 出来的碎片）。
 */

/**
 * @brief atof(const char *str)
 * @note  文本数字转浮点数 (Ascii to Float)
 * @return 返回转换后的双精度/单精度浮点数 (double / float)。
 * @purpose 当题目需要通过串口修改带有小数点的系统参数、PID参数或阈值（如 SE = 42.3）时，必须使用此函数。
 */



#include "mymain.h"

void Led_Disp(uint8_t ucled)
{
    HAL_GPIO_WritePin(GPIOC,0xFF00,GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOC,ucled << 8,GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
}


//按键V2   长按、短按、组合键、双击
uint8_t Key_Read(void)
{
    uint8_t Val = 0;
    uint8_t b1 = (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_RESET);
    uint8_t b2 = (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_RESET);
    uint8_t b3 = (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2) == GPIO_PIN_RESET);
    uint8_t b4 = (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET);
    
    if(b2 && b3) 
    {
        Val = 0x10; 
    }
    else 
    {
        if(b1) Val |= 0x01;
        if(b2) Val |= 0x02;
    }
    if(b3) Val |= 0x04;
    if(b4) Val |= 0x08;

    return Val;
}

struct KEYS
{
    uint8_t state;
    uint16_t timer;
    
    uint8_t short_flag;
    uint8_t long_flag;
    uint8_t double_flag;

};

struct KEYS bkey[6] = {0};

//按键V2.1   长按、短按、组合键 （标准状态机）（题目用）
void Key_Serv(void)
{
    uint8_t key_val = Key_Read();
    for(uint8_t i = 1; i<= 5; i++)
    {
        uint8_t pressed = (key_val & (0x01 << (i - 1))) ? 1:0;
        
        switch(bkey[i].state)
        {
            case 0:
            {
                if(pressed)
                {
                    bkey[i].timer = 0;
                    bkey[i].state = 1;
                }
                break;
            }
            
            case 1://按下
            {
                if(pressed)
                {
                    bkey[i].timer++;
                    if(bkey[i].timer >= 200)
                    {
                        bkey[i].state = 2;
                        bkey[i].timer = 0;
                    }
                }
                else //松手
                {
                    if(bkey[i].timer >= 3)
                    {
                        bkey[i].short_flag = 1;
                        bkey[i].state = 0; 
                        bkey[i].timer = 0;
                    }
                    else
                    {
                        bkey[i].state = 0;
                        bkey[i].timer = 0;
                    }
                }
                break;
            }
            
            case 2: //长按松手后触发
            {
                if(!pressed)
                {
                    bkey[i].state = 0; 
                    bkey[i].timer = 0;
                    bkey[i].long_flag = 1;
                }
                break;
            }
        }
    }
}


//全局变量申明
/*--- 1. UI 界面与显示缓存 ---*/
uint8_t lcd_page = 0;       // LCD当前页面索引（0:DATA数据页, 1:PARA参数页, 2:RECD记录页）
char lcd_buf[50];           // LCD字符串格式化缓存区（用于 sprintf）
uint32_t lcd_tick = 0;
/*--- 2. 系统核心运行状态机 ---*/
uint8_t ST;                 // 系统主状态（0:Idle空闲/FREE, 1:Busy运行, 2:Wait暂停）
/*--- 3. 坐标与运动轨迹控制 ---*/
float CP_x;                 // 当前实际坐标 X (Current Position X)
float CP_y;                 // 当前实际坐标 Y (Current Position Y)
float TP_x;                 // 最终目标坐标 X (Target Position X，即当前坐标组的最后一组)
float TP_y;                 // 最终目标坐标 Y (Target Position Y)
float SE;                   // 当前系统运行速度 (Speed, 单位: cm/s)
uint8_t RN;                 // 当前接收到的有效坐标组总数 (Route Number)
uint8_t RN_View;            // 经过修正后的RN_View (Route Number)
struct ZB
{
    int x;                  // 坐标点 X 值
    int y;                  // 坐标点 Y 值
    uint8_t pass;           // 坐标点到达标记（0:未到达, 1:已通过/已到达）
};
struct ZB zb[100];          // 历史接收坐标点结构体数组（最大存储100组）
const struct ZB zb_zero = {0}; // 全零结构体常量（用于 memcmp 快速判断清零状态）
uint8_t zb_empty;           // 坐标缓存空标志位（1:当前无有效目标坐标, 0:有坐标数据）
/*--- 4. 题目参数配置 ---*/
float R;                    // 参数R：控制速度的半径系数（步长0.1, 范围1.0~2.0）
uint8_t B;                  // 参数B：控制速度的偏置常数（步长10, 范围10~100）
uint8_t Select_Para = 0;    // 参数页当前选中行高亮标记（0:选中R, 1:选中B）
uint8_t work_space;         // 当前工作空间/场景配置编号（1~4，对应不同的PWM输出频率）
/*--- 5. 运动数据记录与统计 ---*/
float TS;                   // 累计运行总距离 (Total Distance, 单位: cm)
uint16_t TT;                // 累计运行总时间 (Total Time, 单位: 秒)

/*--- 6. 实时时钟（RTC） ---*/
RTC_TimeTypeDef time;       // HAL库 RTC 时间结构体（时、分、秒）
RTC_DateTypeDef date;       // HAL库 RTC 日期结构体（年、月、日、星期）

/*--- 7. 模拟量采集（ADC） ---*/
uint32_t adc2[1];           // ADC2 DMA 接收原始数据缓冲区
float V;                    // 经过换算后的实际电压值（0.0V ~ 3.3V）

/*--- 8. 脉冲捕获与驱动输出（TIM/PWM） ---*/
uint16_t F;                 // 输入捕获测量得到的信号频率 (Frequency, 单位: Hz)
uint16_t A_Freq;            // 通道A (TIM16) 期望输出 PWM 频率
uint8_t A_Duty;             // 通道A (TIM16) 期望输出 PWM 占空比 (0~100)
uint16_t B_Freq;            // 通道B (TIM17) 期望输出 PWM 频率
uint8_t B_Duty;             // 通道B (TIM17) 期望输出 PWM 占空比 (0~100)

/*--- 9. 定时器基础时间片与软件计时器 ---*/
uint8_t TIMER;              // 定时器内部 10ms 计数器（累加到100即为1秒）
uint8_t TIMER_ENABLE;       // 系统运行时间累计使能控制（1:开始累计, 0:停止累计）
uint32_t task_tick;        // 定时器 100ms 溢出放行标志位（1:放行 100ms 核心算法任务）
uint32_t dt;
/*--- 10. 轨迹算法中间变量 ---*/
uint16_t last_time;         // 上一次算法迭代的时间戳（预留）
uint16_t time_target;       // 算法目标预期到达时间（预留）
double delta_distance;      // 当前点与当前目标坐标点之间的欧几里得距离

/*--- 11. 局部控制与硬件指示灯（LED） ---*/
static uint8_t index = 0;   // 坐标追踪当前正在处理的节点索引（指向 zb 数组）
uint8_t led1_timer = 0;     // LED1 闪烁软件计数器（以 10ms 为步长，最大周期为40，即400ms）
uint8_t led1_enable;        // LED1 闪烁使能标志位（1:开启闪烁状态）
uint8_t ucled;              // LED硬件输出控制字（按位对应LD1~LD8，1:点亮, 0:熄灭）

/*--- 12. 串口接收与解析（UART） ---*/
char rx_buf[50];            // 串口接收缓冲区（用于 DMA 到空闲中断接收原生字符串）
char tx_buf[50];            // 串口发送缓冲区（预留）
uint8_t uart_rx_flag = 0;   // 串口数据包接收完成标志包（0:无数据, 1:收到完整新数据包等待解析）

void Task_Key(void);
void Task_Hardware(void);
void Task(void);
void UART_Callback(void);
void Task_LCD(void);

//===============================主调================================
void Set_Up(void)
{
    HAL_TIM_Base_Start_IT(&htim6);
    HAL_TIM_IC_Start(&htim2,TIM_CHANNEL_1);
    HAL_ADCEx_Calibration_Start(&hadc2,ADC_SINGLE_ENDED);
    HAL_ADC_Start_DMA(&hadc2,adc2,1);
    HAL_TIM_PWM_Start(&htim16,TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim17,TIM_CHANNEL_1);
    HAL_UARTEx_ReceiveToIdle_IT(&huart1,(uint8_t *)rx_buf,sizeof(rx_buf) - 1);
    Led_Disp(0x00);
    
    LCD_Init();
    HAL_Delay(100);
    LCD_SetBackColor(Black);
    LCD_SetTextColor(White);
    LCD_Clear(Black);
    
//*************显示初始化**************
    //PAGE 0
    ST = 0; //0:Idle 1:Busy 2:Wait
    CP_x = 0;
    CP_y = 0;
    TP_x = 0;
    TP_y = 0;
    SE = 0;
    RN = 0;
    
    //PAGE 1
    R = 1.0f;
    B = 10;
    
    //PAGE 2
    TS = 0;
    TT = 0;
//*************参数初始化**************
    work_space = 1;
}

void Loop(void)
{
    Task_Key();

    Task_Hardware();
    if(task_tick - uwTick >= 100.0f)
    {
        Task();
        task_tick = uwTick;
    }

    
    UART_Callback();
    

    if(uwTick - lcd_tick >= 200) //设定刷新时间
    {
        Task_LCD();
        lcd_tick = uwTick;
    }
}



void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim->Instance == TIM6)
    {
        Key_Serv(); 
        
        if(TIMER_ENABLE)
        {
            TIMER++;
            if(TIMER == 100) // 10ms * 100 = 1s
            {
                TT++;
                TIMER = 0;
            }
        }
        

        if(led1_enable)
        {
            led1_timer++;
            if(led1_timer > 40) 
            {
                led1_timer = 1;
            }
        }
        else
        {
            led1_timer = 0;
        }
    }
}

// 串口空闲中断回调
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if(huart->Instance == USART1)
    {
        rx_buf[Size] = '\0'; 
        uart_rx_flag = 1;    
    }
}



//自定义函数
int fputc(int ch,FILE *f)
{
    HAL_UART_Transmit(&huart1,(uint8_t *)&ch,1,50);
    return ch;
}

void Task_Key(void)
{
    if(bkey[1].short_flag == 1)
    {
        if(ST == 0)
        {
            zb_empty = (TP_x == 0 && TP_y == 0) ? 1 : 0;
            if(!zb_empty) ST = 1;
        }
        else if(ST == 1) ST = 2; // 运行 -> 暂停
        else if(ST == 2) ST = 1; // 暂停 -> 运行
        bkey[1].short_flag = 0;
    }

    if(bkey[2].short_flag == 1)
    {
        lcd_page = (lcd_page + 1) % 3;
        LCD_Clear(Black); 
        bkey[2].short_flag = 0;
    }
    
    // B3: 选择参数
    if(bkey[3].short_flag == 1)
    {
        if(lcd_page == 1)
        {
            Select_Para = (Select_Para + 1) % 2; // 0：R 1：B
        }
        bkey[3].short_flag = 0;
    }
    
    // B4: 修改参数
    if(bkey[4].short_flag == 1)
    {
        if(lcd_page == 1)
        {            
            if(Select_Para == 0)
            {
                R += 0.1f;
                if(R >= 2.1f) R = 1.0f;
            }
            else
            {
                B += 10;
                if(B == 110) B = 10;
            }
        }
        bkey[4].short_flag = 0;
    }
    
    // B5: 长按复位
    if(bkey[5].long_flag == 1)
    {
        if(ST == 0 && lcd_page == 0)
        {
            TS = 0;
            TT = 0;
            TIMER = 0;
        }
        bkey[5].long_flag = 0;
    }
}


void Task(void)
{

    if(ST != 1) 
    {
        TIMER_ENABLE = 0;
        SE = 0;
        return;
    }

    TIMER_ENABLE = 1;
    
    // 1. 轨迹与坐标更新核心算法
    TS += SE / 10.0f;    // 累计距离计算

    if(zb[index].x == -1 && zb[index].y == -1)
    {
        zb[index].pass = 1;
        index++;
    }
    else
    {  //行进
        double delta_x = zb[index].x - CP_x;
        double delta_y = zb[index].y - CP_y;
        
        delta_distance = sqrt(delta_x * delta_x + delta_y * delta_y);
        
        if(delta_distance <= 1.5f)
        {
            zb[index].pass = 1;
            if(index == (RN - 1))
            {
                ST = 0;
                index = 0;
            }
            else
            {
                index++;
                RN_View -= 1;
            }
        }
        else
        {
            double step = SE / 10.0f;
            CP_x += (delta_x / delta_distance) * step;
            CP_y += (delta_y / delta_distance) * step;
        }
    }
}

//硬件部分 除了LCD的外设读取
void Task_Hardware(void)
{
    F = 1000000.0f / (HAL_TIM_ReadCapturedValue(&htim2, TIM_CHANNEL_1) + 1);
    V = adc2[0] / 65535.0f * 3.3f;    // 电压值
    if(ST == 1)
    {
        SE = 3.14f * R * F / 100.0f + B;  // 运行速度 cm/s
        
        // 场景配置
        switch(work_space)
        {
            case 1: A_Freq = 1000; B_Freq = 1000; break;
            case 2: A_Freq = 4000; B_Freq = 1000; break;
            case 3: A_Freq = 1000; B_Freq = 4000; break;
            case 4: A_Freq = 4000; B_Freq = 4000; break;
        }
        // 驱动控制
        if(V >= 2.5f)      { A_Duty = 95; B_Duty = 90; }
        else if(V >= 2.0f) { A_Duty = 90; B_Duty = 85; }
        else if(V >= 1.5f) { A_Duty = 70; B_Duty = 65; }
        else if(V >= 1.0f) { A_Duty = 50; B_Duty = 45; }
        else if(V >= 0.5f) { A_Duty = 30; B_Duty = 25; }
        else               { A_Duty = 10; B_Duty = 5;  }
        
        // 输出更新
        __HAL_TIM_SET_AUTORELOAD(&htim16, (1000000.0f / A_Freq) - 1);
        __HAL_TIM_SET_COMPARE(&htim16, TIM_CHANNEL_1, 1000000.0f / A_Freq * A_Duty / 100.0f - 1);
        __HAL_TIM_SET_AUTORELOAD(&htim17, (1000000.0f / B_Freq) - 1);
        __HAL_TIM_SET_COMPARE(&htim17, TIM_CHANNEL_1, 1000000.0f / B_Freq * B_Duty / 100.0f - 1);
        
        HAL_TIM_PWM_Start(&htim16, TIM_CHANNEL_1);
        HAL_TIM_PWM_Start(&htim17, TIM_CHANNEL_1);
    }
    else  //关闭必须要用HAL的STOP函数！！！
    {
        HAL_TIM_PWM_Stop(&htim16, TIM_CHANNEL_1);
        HAL_TIM_PWM_Stop(&htim17, TIM_CHANNEL_1);
        SE = 0;
    }

    // 2. LED 指示灯控制逻辑
    if(ST == 0)       ucled = (ucled & ~0x01) | 0x00;
    else if (ST == 1) ucled = (ucled & ~0x01) | 0x01;
    else              // Wait 状态闪烁
    {
        led1_enable = 1;
        ucled = (led1_timer <= 20 && led1_timer != 0) ? ((ucled & ~0x01) | 0x01) : ((ucled & ~0x01) | 0x00);
    }

    // 场景灯控制
    ucled = (work_space == 1) ? ((ucled & ~0x10) | 0x10) : (ucled & ~0x10);
    ucled = (work_space == 2) ? ((ucled & ~0x20) | 0x20) : (ucled & ~0x20);
    ucled = (work_space == 3) ? ((ucled & ~0x40) | 0x40) : (ucled & ~0x40);
    ucled = (work_space == 4) ? ((ucled & ~0x80) | 0x80) : (ucled & ~0x80);
    
    Led_Disp(ucled);
    sprintf(lcd_buf, "     V=%.1f,%d,%.1f     ", V,F,3.14f * R * F / 100.0f + B);
    LCD_DisplayStringLine(Line9, (unsigned char *)lcd_buf);
}


//LCD显示
void Task_LCD(void)
{
    switch(lcd_page)
    {
        case 0:
            LCD_DisplayStringLine(Line1, (unsigned char *)"        DATA        ");
            if(ST == 0)      sprintf(lcd_buf, "     ST=Idle       ");
            else if(ST == 1) sprintf(lcd_buf, "     ST=Busy       ");
            else             sprintf(lcd_buf, "     ST=Wait       ");
            LCD_DisplayStringLine(Line3, (unsigned char *)lcd_buf);
            
            sprintf(lcd_buf, "     CP=%.0f,%.0f       ", CP_x, CP_y);
            LCD_DisplayStringLine(Line4, (unsigned char *)lcd_buf);
            
            if(ST == 0) sprintf(lcd_buf, "     TP=NF       ");
            else        sprintf(lcd_buf, "     TP=%.0f,%.0f       ", TP_x, TP_y); 
            LCD_DisplayStringLine(Line5, (unsigned char *)lcd_buf);
            
            sprintf(lcd_buf, "     SE=%.1f       ", SE);
            LCD_DisplayStringLine(Line6, (unsigned char *)lcd_buf);
            
            if(ST == 0) sprintf(lcd_buf, "     RN=NF       ");
            else        sprintf(lcd_buf, "     RN=%d       ", RN_View);
            LCD_DisplayStringLine(Line7, (unsigned char *)lcd_buf);
            break;

        case 1:
            LCD_DisplayStringLine(Line1, (unsigned char *)"        PARA        ");
            sprintf(lcd_buf, "     R=%.1f         ", R);
            LCD_DisplayStringLine(Line3, (unsigned char *)lcd_buf);
            sprintf(lcd_buf, "     B=%d           ", B);
            LCD_DisplayStringLine(Line4, (unsigned char *)lcd_buf);
            // 清除多余行
            LCD_DisplayStringLine(Line5, (unsigned char *)"                    ");
            LCD_DisplayStringLine(Line6, (unsigned char *)"                    ");
            LCD_DisplayStringLine(Line7, (unsigned char *)"                    ");
            break;

        case 2:
            LCD_DisplayStringLine(Line1, (unsigned char *)"        RECD        ");
            sprintf(lcd_buf, "     TS=%.1f        ", TS);
            LCD_DisplayStringLine(Line3, (unsigned char *)lcd_buf);
            sprintf(lcd_buf, "     TT=%d          ", TT);
            LCD_DisplayStringLine(Line4, (unsigned char *)lcd_buf);
            // 清除多余行
            LCD_DisplayStringLine(Line5, (unsigned char *)"                    ");
            LCD_DisplayStringLine(Line6, (unsigned char *)"                    ");
            LCD_DisplayStringLine(Line7, (unsigned char *)"                    ");
            break;
    }
}


//串口解析
void UART_Callback(void)
{
    if(uart_rx_flag == 0) return; // 没有收到新数据，直接跳出
    
    char *pHead = rx_buf;
    // 过滤开头的无效空白字符
    while(*pHead == ' ' || *pHead == '\r' || *pHead == '\n') 
    {
        pHead++;
    }
    
    //----------------- 场景 1：坐标组解析 ( ) -----------------
    if(*pHead == '(')
    {
        if(ST != 0)
        {
            printf("Busy\r\n");
        }
        else
        {
            char *pStart = strchr(rx_buf, '(');
            char *pEnd = strchr(rx_buf, ')');  
            
            if(pStart != NULL && pEnd != NULL && pEnd > pStart)
            {
                *pEnd = '\0'; 
                char *data_ptr = pStart + 1; // 跳过 '('
                
                uint8_t count = 0;
                uint16_t temp_coords[20];   
            
                char *token = strtok(data_ptr, ",");
                while(token != NULL && count < 20)
                {
                    temp_coords[count++] = (uint16_t)atoi(token); 
                    token = strtok(NULL, ",");
                }
                
                if(count > 0 && (count % 2 == 0))
                {
                    RN = count / 2; 
                    RN_View = RN;
                    memset(zb, 0, sizeof(zb)); // 一键清空历史坐标
                
                    for(uint8_t i = 0; i < RN; i++)
                    {
                        zb[i].x = temp_coords[i * 2];   
                        zb[i].y = temp_coords[i * 2 + 1];
                    }
                
                    TP_x = zb[RN - 1].x; // 最后一组为终点坐标
                    TP_y = zb[RN - 1].y;
                
                    printf("Got it\r\n");
                }
                else
                {
                    printf("Error\r\n");
                }
            }
        }
    }
    
    //----------------- 场景 2：删除坐标解析 { } -----------------
    else if(*pHead == '{')
    {
        char *pStart = strchr(rx_buf, '{');
        char *pEnd = strchr(rx_buf, '}'); 
        
        if(pStart != NULL && pEnd != NULL && pEnd > pStart)
        {
            *pEnd = '\0';
            char *data_ptr = pStart + 1;
            
            uint16_t del_x = 0; uint16_t del_y = 0;
            char *token = strtok(data_ptr, ",");
            if(token != NULL) del_x = (uint16_t)atoi(token);
            token = strtok(NULL, ",");
            if(token != NULL) del_y = (uint16_t)atoi(token);
            
            uint8_t find_flag = 0;
            for(uint8_t i = 0; i < RN; i++)
            {
                if(zb[i].x == del_x && zb[i].y == del_y && zb[i].pass == 0)
                {
                    find_flag = 1;
                    zb[i].x = -1;
                    zb[i].y = -1;
                    RN_View = RN - 1;
                    break;
                }
            }
            
            if(find_flag) printf("Got it\r\n");
            else printf("Nonexistent\r\n");
        }
    }
    
    //----------------- 场景 3：工作空间配置 [ ] -----------------
    else if(*pHead == '[')
    {
        if(ST == 1)
        {
            char* pStart = strchr(rx_buf, '[');
            char* pEnd = strchr(rx_buf, ']');
            
            if(pStart != NULL && pEnd != NULL && pEnd > pStart)
            {
                *pEnd = '\0';
                char* data_ptr = pStart + 1;  
                
                uint8_t temp_space = (uint8_t)atoi(data_ptr);
                if(temp_space >= 1 && temp_space <= 4)
                {
                    work_space = temp_space;
                    printf("Got it\r\n");
                }
            }
        }
        else
        {
            printf("Device offline\r\n");
        }
    }
    
    //----------------- 场景 4：状态查询 ? -----------------
    else if(*pHead == '?')
    {
        if(strcmp(rx_buf, "?") == 0)
        {
            if(ST == 0)      printf("Idle\r\n");
            else if (ST == 1) printf("Busy\r\n");
            else if (ST == 2) printf("Wait\r\n");
        }
    }
    
    //----------------- 场景 5：当前坐标查询 # -----------------
    else if(*pHead == '#')
    {
        if(strcmp(rx_buf, "#") == 0)
        {
            printf("(%.0f,%.0f)", CP_x, CP_y);
        }
    }

    else
    {
        printf("Error\r\n");
    }
    memset(rx_buf, 0, sizeof(rx_buf));
    uart_rx_flag = 0;
    HAL_UARTEx_ReceiveToIdle_IT(&huart1, (uint8_t *)rx_buf, sizeof(rx_buf) - 1);
}

