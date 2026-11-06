#ifndef _KEY_H_
#define _KEY_H_


#include "mymain.h"

struct Bkeys
{
  uint8_t age;           //判断是否按键有效的计时位
  uint8_t press;         //按下有效位
  uint8_t short_flag;    //短按键状态位
  uint8_t long_flag;     //长按键状态位   
  uint8_t double_flag;

  uint8_t double_ageEN;  //双击计时使能位
  uint8_t double_age;    //双击按键状态位
};

void key_serv(void);
void key_serv_long(void);
void key_serv_double(void);

#endif


