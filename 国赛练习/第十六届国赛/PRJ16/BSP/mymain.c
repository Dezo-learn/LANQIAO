#include "mymain.h"

void LedDisp(uint8_t ucled)
{
    uint8_t adr = GPIOC -> ODR;
    GPIOC -> ODR = 0xFF00;
    GPIOC->ODR = (~ucled << 8);
    HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
    GPIOC -> ODR = adr;
}

uint8_t KeyRead(void)
{
    uint8_t Val = 0x00;
    uint8_t b1 = (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0) == GPIO_PIN_RESET);
    uint8_t b2 = (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1) == GPIO_PIN_RESET);
    uint8_t b3 = (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2) == GPIO_PIN_RESET);
    uint8_t b4 = (HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0) == GPIO_PIN_RESET);
    
    if(b1)  Val |= 0x01;
    if(b2)  Val |= 0x02;
    if(b3)  Val |= 0x04;
    if(b4)  Val |= 0x08;
    
    return Val;
}

struct BKEY
{
    uint8_t state;
    uint16_t timer;
    
    uint8_t short_flag;
    uint8_t long_flag;
    uint8_t long_flag2;
    uint8_t long_flag3;
    uint8_t double_flag;
};

struct BKEY keys[5] = {0};

void KeyServ(void)
{
    uint8_t key_value = KeyRead();
    for(uint8_t i = 1;i <= 4;i++)
    {
        uint8_t pressed = (key_value & (0x01 << (i-1)))? 1:0;
        switch(keys[i].state)
        {
            case 0:
                if(pressed)
                {
                    keys[i].timer = 0;
                    keys[i].state = 1;
                }
                break;
            
            case 1:
                if(pressed)
                {
                    keys[i].timer++;
                    if(keys[i].timer >= 100)
                    {
                        keys[i].long_flag3 = 1; 
                    }
                }
                else 
                {
                    if(keys[i].timer >= 300)
                    {
                        keys[i].long_flag = 1; 
                        keys[i].long_flag3 = 0; 
                    }
                    else if(keys[i].timer >= 200)
                    {
                        keys[i].long_flag2 = 1; 
                        keys[i].long_flag3 = 0; 
                    }

                    else if(keys[i].timer >= 3)
                    {
                        
                        keys[i].timer = 0;
                        keys[i].state = 2; 
                        break; 
                    }
                    keys[i].timer = 0;
                    keys[i].state = 0;
                }
                break;
                
            case 2:
                if(pressed)
                {
                    keys[i].double_flag = 1;
                    keys[i].timer = 0;
                    keys[i].state = 0;
                }
                else
                {
                    keys[i].timer++;
                    if(keys[i].timer >= 20)
                    {
                        keys[i].short_flag = 1;
                        keys[i].timer = 0;
                        keys[i].state = 0;
                    }
                }
                break;
        }
    }
}


//================变量区================//
//-------LCD变量-------
char lcd_buf[50];
uint32_t lcdtick;
uint8_t lcd_page;

//-------LED变量-------
uint8_t ucled;

//-------时间变量-------
struct TIME_S
{
    uint8_t Seconds;
    uint8_t Minutes;
    uint8_t Hours;
};

//-------页面变量-------
//page 0
char password_user[50] = "A35#";
RTC_TimeTypeDef time;
RTC_DateTypeDef date;

//page 1
uint8_t F = 10; //输出频率
uint8_t D = 20; //输出占空比
uint8_t T_view = 5;   uint8_t T = 0; //输出控制字
uint8_t CH = 0; //0:PA6 1:PA7

//page 2 (主状态机模式变量)
uint8_t page_mode; //0:SUCCESS, 1:FAIL, 2:LOCK

float OF;
uint8_t OD;
float CF;
uint8_t retry = 4;
uint8_t wait = 10;

//page 3
uint8_t SN;//成功计数器
uint8_t ST; struct TIME_S ST_record;
uint8_t FN;//失败计数器
uint8_t FT; struct TIME_S FT_record;
uint8_t NA_S = 1;
uint8_t NA_F = 1;

//-------标志变量-------
uint8_t fun_mode; //
uint8_t call_flag;
uint8_t output_flag;
uint16_t timer1s_op;
//-------硬件变量-------
uint32_t adc1[1];
uint32_t adc2[2];
uint8_t adc_a;
float adc_b;

uint16_t FREQ; //中间变量
uint32_t r39_value;

//-------串口变量-------
char rx_buf[50];
uint8_t LOCKED;
//-------任务变量-------
char password[50] = "A35#";
uint16_t timer1s;
uint16_t timer3s;

void KeyProc(void);
void HardwareProc(void);
void LcdPorc(void);
void Uart_Callback(void);

//================主调区================//
void SetUp(void)
{
    HAL_TIM_Base_Start_IT(&htim6);
    HAL_ADCEx_Calibration_Start(&hadc1,ADC_SINGLE_ENDED);
    HAL_ADCEx_Calibration_Start(&hadc2,ADC_SINGLE_ENDED);
    HAL_ADC_Start_DMA(&hadc1,adc1,1);
    HAL_ADC_Start_DMA(&hadc2,adc2,1);
    HAL_TIM_IC_Start(&htim3,TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop(&htim16,TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop(&htim17,TIM_CHANNEL_1);
    HAL_UARTEx_ReceiveToIdle_IT(&huart1,(uint8_t *)rx_buf,sizeof(rx_buf) - 1);
    
    LCD_Init();
    HAL_Delay(100);
    LCD_SetBackColor(Black);
    LCD_SetTextColor(White);
    LCD_Clear(Black);
    LedDisp(0x00);
    
    date.Year = 25;date.Month = 06;date.Date = 15;
    HAL_RTC_SetDate(&hrtc,&date,RTC_FORMAT_BIN);
    time.Hours = 23;time.Minutes = 59;time.Seconds = 50;
    HAL_RTC_SetTime(&hrtc,&time,RTC_FORMAT_BIN);
}

void Loop(void)
{
    if(timer3s == 0 && LOCKED == 0) KeyProc();
    HardwareProc();
    
    if(uwTick - lcdtick >= 20)
    {
        LcdPorc();
        lcdtick = uwTick;
    }
}

//================回调区================//
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) //10ms 中断
{
    if(htim -> Instance == TIM6)
    {
        KeyServ();
        if(T > 0 && output_flag)
        {
            timer1s_op++;
            if(timer1s_op >= 100)
            {
                timer1s_op = 0;
                T--;
                if(T == 0)
                {
                    output_flag = 0;            //关闭输出
                    lcd_page = 0;
                    page_mode = 0;
                    memset(password,0,sizeof(password));
                }
            }
        }
        
        // 10s 锁定倒计时逻辑
        if(LOCKED)
        {
            timer1s++;
            if(timer1s >= 100) // 10ms * 100 = 1s
            {
                timer1s = 0;
                if(wait > 0) wait --;
                
                if(wait == 0)
                {
                    LOCKED = 0;     // 10s到了，解开
                    retry = 4;      // 重置重试次数
                    page_mode = 0;
                    lcd_page = 0;   // 返回主页
                    LCD_Clear(Black);
                }
            }
        }
        else timer1s = 0;
        
        // FAIL界面 3s 倒计时逻辑（事件发生处）
        if(timer3s > 0)
        {
            timer3s--;
            if(timer3s == 0 && lcd_page == 3 && page_mode == 1)
            {
                memset(password_user, 0, sizeof(password_user));
                lcd_page = 0;       // 3秒时间到，退回主页
                LCD_Clear(Black);
            }
        }
    }
    
    if(tim)
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if(huart -> Instance == USART1)
    {
        rx_buf[Size] = '\0';
        Uart_Callback();
        memset(rx_buf,0,sizeof(rx_buf));
        HAL_UARTEx_ReceiveToIdle_IT(&huart1,(uint8_t *)rx_buf,sizeof(rx_buf) - 1);
    }
}

//================函数区================//
char get_char(uint8_t val)
{
    if(val >= 250) return '+';
    else if(val >= 240) return '=';
    else if(val >= 230) return '&';
    else if(val >= 220) return '%';
    else if(val >= 210) return '$';
    else if(val >= 200) return '#';
    else if(val >= 190) return '@';
    else if(val >= 180) return 'Z';
    else if(val >= 170) return 'Y';
    else if(val >= 160) return 'X';
    else if(val >= 150) return 'F';
    else if(val >= 140) return 'E';
    else if(val >= 130) return 'D';
    else if(val >= 120) return 'B';
    else if(val >= 110) return 'C';
    else if(val >= 100) return 'A';
    else if(val >= 90) return '9';
    else if(val >= 80) return '8';
    else if(val >= 70) return '7';
    else if(val >= 60) return '6';
    else if(val >= 50) return '5';
    else if(val >= 40) return '4';
    else if(val >= 30) return '3';
    else if(val >= 20) return '2';
    else if(val >= 10) return '1';
    else return '0';
}

void KeyProc(void)
{
    // B1 短按：切换菜单页面
    if(keys[1].short_flag == 1)
    {
        keys[1].short_flag = 0;
        
        if(lcd_page != 3) 
        {
            lcd_page = (lcd_page + 1) % 3;
            LCD_Clear(Black);
        }
        if(lcd_page == 1) fun_mode = 0;
    }
    
    
    // B2 短按：输入字符 或 模式项切换
    if(keys[2].short_flag == 1)
    {
        keys[2].short_flag = 0;
        
        // 输入
        if(lcd_page == 0 && !LOCKED) 
        {
            char str_new[2] = {0};
            str_new[0] = get_char(adc_a);
            str_new[1] = '\0';
            
            if(str_new[0] != '0' && strlen(password_user) < 49) 
            {
                strcat(password_user, str_new);
            }
        }
        
        else if(lcd_page == 1) 
        {
            fun_mode  = (fun_mode + 1) % 4;
        }
        else if(lcd_page == 3 && page_mode == 0) 
        {
            T = 0;
            output_flag = 0;
            memset(password,0,sizeof(password));
            lcd_page = 0;

        }
    }
    
    // B2 长按：清空输入 或 清空历史记录
    if(keys[2].long_flag2 == 1)
    {
        keys[2].long_flag2 = 0;
        if(lcd_page == 0)
        {
            memset(password_user, 0, sizeof(password_user));
        }
    }
    
    if(keys[2].long_flag == 1)
    {
        keys[2].long_flag = 0;
        
        if(lcd_page == 2)
        {
            NA_F = 1; //F:NA
            NA_S = 1;
            SN = 0;
            FN = 0;
            
            ST = 0;
            ST_record.Seconds = 0;
            ST_record.Minutes = 0;
            ST_record.Hours = 0;
            
            FT = 0;
            FT_record.Seconds = 0;
            FT_record.Minutes = 0;
            FT_record.Hours = 0;
        }
        
    }
    
    // B3 短按：确认密码 或 参数增加
    if(keys[3].short_flag == 1)
    {
        keys[3].short_flag = 0;
        
        // 密码校验及页面无缝跳转
        if(lcd_page == 0) 
        {
            if(LOCKED) return; // 被锁定时无法提交校验
            
            if(strcmp(password, password_user) == 0) // 1. 密码正确
            {
                page_mode = 0; // 进入 SUCCESS 状态
                lcd_page = 3;
                LCD_Clear(Black);
                retry = 4;     // 刷新重试机会
                T = T_view; //送入真实寄存器计时
                
                // 触发 Record 成功记录更新
                NA_S = 0; 
                ST_record.Minutes = time.Minutes;
                ST_record.Hours = time.Hours;
                ST_record.Seconds = time.Seconds;
                ST = r39_value / 1000.f;
                SN++;
                
                //触发输出
                output_flag = 1;
                
            }
            else // 2. 密码错误
            {
                if(retry > 0) retry--;
                
                if(retry == 0) // 错误扣完，直接上锁
                {
                    page_mode = 2; // 进入 LOCK 状态
                    LOCKED = 1;
                    wait = 10;     // 10秒锁定
                }
                else // 还有机会，进入失败提示
                {
                    page_mode = 1; // 进入 FAIL 状态
                    timer3s = 300; // 开启 3s 倒计时退回
                }
                
                lcd_page = 3;
                
                // TRACE：触发 Record 失败记录更新
                NA_F = 0;
                FT_record.Minutes = time.Minutes;
                FT_record.Hours = time.Hours;
                FT_record.Seconds = time.Seconds;
                FT = r39_value / 1000.f;
                FN++;
            }
            LCD_Clear(Black);
        }
        
        else if(lcd_page == 1) // PARA 界面加数据
        {
            switch(fun_mode)
            {
                case 0: F++; break;
                case 1: D += 5; break;
                case 2: T_view++; break;
                case 3: CH = !CH; break;
            }
        }
    }
    
    // B4 短按：参数减少
    if(keys[4].short_flag == 1)
    {
        keys[4].short_flag = 0;

        if(lcd_page == 1) // PARA 界面减数据
        {
            switch(fun_mode)
            {
                case 0: if(F > 0) F--; break;
                case 1: if(D >= 5) D -= 5; break;
                case 2: if(T_view > 0) T--; break;
                case 3: CH = !CH; break;
            }
        }
    }
    
    // B4 长按：呼叫
    if(keys[4].long_flag3 == 1)
    {

        if(lcd_page == 0) // 初始界面
        {
            led3 = 1;  //呼叫内部
        }
    }
    else
    {
        led3 = 0;
    }
}

void LcdPorc(void)
{
    switch(lcd_page)
    {
        case 0:
            LCD_DisplayStringLine(Line1, (unsigned char *)"        IDLE       ");
            LCD_DisplayStringLine(Line4, (unsigned char *)"                  ");
            sprintf(lcd_buf,"     UN:%s         ",password_user);
            LCD_DisplayStringLine(Line3, (unsigned char *)lcd_buf);
            
            sprintf(lcd_buf,"     %02d-%02d-%02d   ",date.Year,date.Month,date.Date);
            LCD_DisplayStringLine(Line5, (unsigned char *)lcd_buf);
            sprintf(lcd_buf,"     %02d-%02d-%02d   ",time.Hours,time.Minutes,time.Seconds);
            LCD_DisplayStringLine(Line6, (unsigned char *)lcd_buf);
        break;
        
        case 1:
            sprintf(lcd_buf,"        PARA       ");
            LCD_DisplayStringLine(Line1, (unsigned char *)lcd_buf);
            sprintf(lcd_buf,"     F:%dKHZ      ",F);
            LCD_DisplayStringLine(Line3, (unsigned char *)lcd_buf);
            sprintf(lcd_buf,"     D:%d%%      ",D);
            LCD_DisplayStringLine(Line4, (unsigned char *)lcd_buf);
            sprintf(lcd_buf,"     T:%d      ",T_view);
            LCD_DisplayStringLine(Line5, (unsigned char *)lcd_buf);
            CH? sprintf(lcd_buf,"     CH:PA7      "):sprintf(lcd_buf,"     CH:PA6      ");
            LCD_DisplayStringLine(Line6, (unsigned char *)lcd_buf);
            break;
        case 3:
            sprintf(lcd_buf,"        OUTP       ");
            LCD_DisplayStringLine(Line1, (unsigned char *)lcd_buf);
            switch (page_mode)
            {
                case 0:
                    sprintf(lcd_buf,"     T:%02d         ",T);
                    LCD_DisplayStringLine(Line3, (unsigned char *)lcd_buf);
                    sprintf(lcd_buf,"     OF:%.1fKHZ         ",OF);
                    LCD_DisplayStringLine(Line4, (unsigned char *)lcd_buf);
                    sprintf(lcd_buf,"     OD:%d%%         ",OD);
                    LCD_DisplayStringLine(Line5, (unsigned char *)lcd_buf);
                    sprintf(lcd_buf,"     CF:%.1fKHZ         ",CF);
                    LCD_DisplayStringLine(Line6, (unsigned char *)lcd_buf);
                    break;
                case 1:
                    sprintf(lcd_buf,"        FAIL         ");
                    LCD_DisplayStringLine(Line3, (unsigned char *)lcd_buf);
                    sprintf(lcd_buf,"      RETRY:%d      ",retry);
                    LCD_DisplayStringLine(Line4, (unsigned char *)lcd_buf);
                    break;
                case 2:
                    sprintf(lcd_buf,"        LOCK         ");
                    LCD_DisplayStringLine(Line3, (unsigned char *)lcd_buf);
                    sprintf(lcd_buf,"      RETRY:%d      ",retry);
                    LCD_DisplayStringLine(Line4, (unsigned char *)lcd_buf);
                    sprintf(lcd_buf,"      WAIT:%02d      ",wait);
                    LCD_DisplayStringLine(Line5, (unsigned char *)lcd_buf);
                    break;
            }
            break;
        case 2:
            sprintf(lcd_buf,"        RECD       ");
            LCD_DisplayStringLine(Line1, (unsigned char *)lcd_buf);
            
            NA_S? sprintf(lcd_buf,"   SN:NA          "):sprintf(lcd_buf,"   SN:%d          ",SN);
            LCD_DisplayStringLine(Line3, (unsigned char *)lcd_buf);
        
            NA_S? sprintf(lcd_buf,"   ST:NA          "):sprintf(lcd_buf,"   ST:%d@%02d:%02d:%02d",ST,ST_record.Hours,ST_record.Minutes,ST_record.Seconds);
            LCD_DisplayStringLine(Line4, (unsigned char *)lcd_buf);
        
            NA_F? sprintf(lcd_buf,"   FN:NA          "):sprintf(lcd_buf,"   FN:%d          ",FN);
            LCD_DisplayStringLine(Line5, (unsigned char *)lcd_buf);
        
            NA_F? sprintf(lcd_buf,"   FT:NA          "):sprintf(lcd_buf,"   FT:%d@%02d:%02d:%02d",FT,FT_record.Hours,FT_record.Minutes,FT_record.Seconds);
            LCD_DisplayStringLine(Line6, (unsigned char *)lcd_buf);
            break;
    }
    
    // 底部的 Debug 数据同步显示
    sprintf(lcd_buf, "   U:%s       ", password);
    LCD_DisplayStringLine(Line9, (unsigned char *)lcd_buf);

    sprintf(lcd_buf, "   T:%d,%d       ", T_view, T);
    LCD_DisplayStringLine(Line8, (unsigned char *)lcd_buf);
}

void HardwareProc(void)
{
    HAL_RTC_GetTime(&hrtc,&time,RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc,&date,RTC_FORMAT_BIN);
    
    float r38_value_volt = adc1[0] * 3.3f / 65535;
    float r37_value_volt = adc2[0] * 3.3f / 65535; 

    adc_a = r37_value_volt * 255 / 3.3f;
    

    
    adc_b = 1000.0f / 3.3f * r38_value_volt - 500.0f;
    FREQ = adc_b + FREQ;

    CF = r39_value / 1000.0f;
    OF = F;
    OD = D;
    FREQ = F * 1000;
    if(SN!=0)
    {
        if(T > 0)
        {
            if(CH == 0) // TIM16 配置 pa6
            {
                TIM16->ARR  = (uint32_t)(1000000.0f / FREQ - 1);
                TIM16->CCR1 = (uint32_t)(1000000.0f / FREQ * OD / 100.0f - 1);
                HAL_TIM_PWM_Start(&htim16,TIM_CHANNEL_1);
                HAL_TIM_PWM_Stop(&htim17,TIM_CHANNEL_1);
            }
            else // TIM17 配置
            {
                TIM17->ARR  = (uint32_t)(1000000.0f / FREQ - 1);
                TIM17->CCR1 = (uint32_t)(1000000.0f / FREQ * OD / 100.0f - 1);
                HAL_TIM_PWM_Start(&htim17,TIM_CHANNEL_1);
                HAL_TIM_PWM_Stop(&htim16,TIM_CHANNEL_1);
            }
        }
        else
        {
            HAL_TIM_PWM_Stop(&htim17,TIM_CHANNEL_1);
            HAL_TIM_PWM_Stop(&htim16,TIM_CHANNEL_1);
        }
    }
    else
    {
        HAL_TIM_PWM_Stop(&htim17,TIM_CHANNEL_1);
        HAL_TIM_PWM_Stop(&htim16,TIM_CHANNEL_1);
    }
    
    r39_value = (uint32_t)(1000000.0f / ((TIM3 -> CCR1) + 1));
    
    
    if(output_flag) ucled = (ucled & ~0x01) & 0x01;
    else ucled = (ucled & ~0x01) & 0x00;
   
    if(LOCKED == 1 || timer3s) ucled = (ucled & ~0x02) & 0x02;
    else ucled = (ucled & ~0x02) & 0x00;
    
    if(led3_timer <= 10 && led3_timer != 0)  ucled = (ucled & ~0x04) & 0x04;
    else ucled = (ucled & ~0x04) & 0x00;
    
    LedDisp(ucled);
}

void Uart_Callback(void)
{
    char tim[50];
    char psw[50];
    char date_t[50];
    int freq_temp;
    int duty_temp;
    int csec;
    char channel_temp[20];
    
    if(strcmp(rx_buf,"?") == 0)
    {
        printf("%s",password);
    }
    else if(sscanf(rx_buf,"SET:%s",psw) == 1)
    {
        char* hdat = psw;
        uint8_t num_dat = 0;
        uint8_t char_dat = 0;
        uint8_t spchar_dat = 0;
        uint8_t error = 0;
        hdat = psw;
        while(*hdat != '\0')
        {
            if(*hdat >= '1' && *hdat <= '9') { num_dat++; }
            else if(*hdat == 'A'|| *hdat == 'B'|| *hdat == 'C'|| *hdat == 'D'|| *hdat == 'E'|| *hdat == 'F'|| *hdat == 'X'|| *hdat == 'Y'||  *hdat == 'Z') { char_dat++; }
            else if(*hdat == '@'|| *hdat == '#'|| *hdat == '$'|| *hdat == '%'|| *hdat == '&'|| *hdat == '='|| *hdat == '+') { spchar_dat++; }
            else { error = 1; break; }
            hdat++;
         }
        
         if(num_dat == 0 || char_dat == 0 || spchar_dat == 0) {error = 1;}

         if(!error)
         {
             int len = strlen(psw);
             for(int i = 0; i <= len - 3; i++)
             {
                 if(psw[i] == psw[i+1] && psw[i+1] == psw[i+2]) { error = 1; break; }
                 if((psw[i+1] - psw[i] == 1) && (psw[i+2] - psw[i+1] == 1)) { error = 1; break; }
             }
         }
         
        if(error) {printf("ERROR\r\n");}
        else 
        {
            memset(password,0,sizeof(password));
            sprintf(password,"%s",psw);
            memset(psw,0,sizeof(psw));
            printf("OK");
        }
    }
    else if(sscanf(rx_buf,"TIME:%s",tim) == 1)
    {
        char *hdat = tim;
        uint8_t error = 0;
        uint8_t len = strlen(tim);
        uint8_t num_c = 0;
        int hour; int min; int sec;
        
        if(len != 6) error = 1;
        while(*hdat != '\0'&& !error)
        {
            if(*hdat >= '0'&& *hdat<= '9') num_c++;
            else{error = 1; break;}
            hdat++;
        }
        
        if(!error)
        {
            if(sscanf(tim, "%2d%2d%2d", &hour, &min, &sec) != 3) {error = 1;}
            else if(hour > 24 || min > 60 || sec > 60) error = 1;
        }
        
        if(error)
        {
            printf("Error\r\n");
            memset(tim,0,sizeof(tim));
        }
        else
        {
            time.Hours= hour; time.Minutes = min; time.Seconds = sec;
            memset(tim,0,sizeof(tim));
            HAL_RTC_SetTime(&hrtc,&time,RTC_FORMAT_BIN);
            printf("OK\r\n");
        }
    }
    else if(sscanf(rx_buf,"DATE:%s",date_t) == 1)
    {
        char *hdat = date_t;
        uint8_t error = 0;
        uint8_t len = strlen(date_t);
        int year; int month; int day;

        if(len != 6) error = 1;
        while(*hdat != '\0'&& !error)
        {
            if(*hdat >= '0'&& *hdat<= '9') hdat++;
            else{error = 1; break;}
        }
        
        if(!error)
        {
            if(sscanf(date_t, "%2d%2d%2d", &year, &month, &day) != 3) {error = 1;}
            else if(month > 12 ) error = 1;

            switch(month)
            {
                case 1:{if(day > 31) error = 1;break;}
                case 2:
                {
                    if (((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) && day > 29) error = 1;
                    else if(day > 28) error = 1;
                    break;
                }
                case 3:{if(day > 31) error = 1;break;}
                case 4:{if(day > 30) error = 1;break;}case 5:{if(day > 31) error = 1;break;}case 6:{if(day > 30) error = 1;break;}
                case 7:{if(day > 31) error = 1;break;}case 8:{if(day > 31) error = 1;break;}case 9:{if(day > 30) error = 1;break;}
                case 10:{if(day > 31) error = 1;break;}case 11:{if(day > 30) error = 1;break;}case 12:{if(day > 31) error = 1;break;}
            }
        }
        
        if(error)
        {
            printf("ERROR\r\n");
            memset(date_t,0,sizeof(date_t));
        }
        else
        {
            date.Year = year; date.Month = month; date.Date = day;
            HAL_RTC_SetDate(&hrtc,&date,RTC_FORMAT_BIN);
            memset(date_t,0,sizeof(date_t));
            printf("OK\r\n");
        }
    }
    else if(sscanf(rx_buf,"%3s:%d:%d%%:%d",channel_temp,&freq_temp,&duty_temp,&csec) == 4)
    {
        uint8_t error = 0;
        if(strcmp(channel_temp,"PA6") != 0 && strcmp(channel_temp,"PA7") != 0) error = 1;
        if(duty_temp > 100) error = 1;
        
        if(!error)
        {
            F = freq_temp / 1000;
            D = duty_temp;
            T_view = csec * 100;
            if(strcmp(channel_temp,"PA6") == 0){CH = 0;}
            else if(strcmp(channel_temp,"PA7") == 0){CH = 1;}
            printf("OK\r\n");
        }
        else { printf("ERROR\r\n"); }
    }
    else { printf("ERROR\r\n"); }
}

int fputc(int ch,FILE* file)
{
    HAL_UART_Transmit(&huart1,(uint8_t *)&ch,1,50);
    return ch;
}


