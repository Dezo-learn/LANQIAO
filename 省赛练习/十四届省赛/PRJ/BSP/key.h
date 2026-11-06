#ifndef _KEY_H_
#define _KEY_H_

#include "mymain.h"

struct BKEY
{
  uint8_t press;
  uint16_t age;

  uint8_t short_flag;
  uint8_t long_flag;
//  uint8_t double_flag;

//  uint8_t double_ageEN;  //双击计时使能位
//  uint8_t double_age;    //双击按键状态位

};

void key_serv(void);

#endif
