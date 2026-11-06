#include "mymain.h"


char text[30];
uint16_t i = 5;

void setup(void){
    LCD_Init();
    LCD_SetBackColor(Black);
    LCD_SetTextColor(White);
    LCD_Clear(Black);
    LED_Chg(4,1);

}

void loop(void){
    LCD_DisplayStringLine(Line4, (unsigned char *)"    Hello,world.   ");
    sprintf(text,"CNBR:%d     ",i);
    LCD_DisplayStringLine(Line9, (unsigned char *)text);
    
}

