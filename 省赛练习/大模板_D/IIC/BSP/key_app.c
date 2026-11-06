#include "key_app.h"

struct Bkeys bkey[5] = {0,0};

//uint8_t Key_Up , Key_Down , Key_Old, Key_Val;

uint8_t key_read()
{
  if(!(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0))) return 1;
  else if(!(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1))) return 2;
  else if(!(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2))) return 3;
  else if(!(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0))) return 4;
  else return 0;
}

//按键服务函数 包含长短按 双击
void key_serv(void)
{
  uint8_t key_sta = key_read();
  if(key_sta!=0) //如果按下
  {
    bkey[key_sta].age++;
    if(bkey[key_sta].age == 2) bkey[key_sta].press = 1; //消抖完后的有效按下
  } 
  else  //松开按键
  {
    for(int i=0;i<5;i++) //遍历结构体中所有元素
    {
      if(bkey[i].double_ageEN == 1 && bkey[i].press == 1)                        // 3.1.双击计时中 被再次按下（有效）则 产生双击状态 复原按下状态和使能
      {
        bkey[i].double_flag = 1;  //正常复位与置位
        bkey[i].press = 0;//由于后面需要用到press位所以需要将其置零以防产生短按位
        bkey[i].double_age = 0;   //正常复位与置位
        bkey[i].double_ageEN = 0; //正常复位与置位
      }
      if(bkey[i].press == 1 && bkey[i].long_flag == 0) bkey[i].double_ageEN = 1; // 1.若按下为有效且长按未被激活 -> 启动双击计时
      if(bkey[i].double_ageEN ==1)bkey[i].double_age++;                          // 2.双击计时开始
      if(bkey[i].double_ageEN == 1 && bkey[i].double_age > 20)                   // 3.2.双击计时中 计时超时 则为短按不在等待 -> 复原标志位（双击间隔计时位 与双击计时使能位）和 拉高短按状态
      {
        bkey[i].short_flag = 1;  //正常复位与置位
        bkey[i].double_age = 0;  //正常复位与置位
        bkey[i].double_ageEN = 0;//正常复位与置位
      }
      bkey[i].age = 0;           //主函数中的复位
      bkey[i].press = 0;         //主函数中的复位
      bkey[i].long_flag = 0;     //主函数中的复位
    }
  }
  if(bkey[key_sta].age > 69) bkey[key_sta].long_flag = 1; //超过690ms则为长按 无需等待后续
}

