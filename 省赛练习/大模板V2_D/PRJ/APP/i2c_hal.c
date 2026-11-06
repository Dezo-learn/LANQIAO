/**
 * CT117E-M4 / GPIO - I2C
*/

#include "i2c_hal.h"

#define DELAY_TIME	20

//
void SDA_Input_Mode()
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    GPIO_InitStructure.Pin = GPIO_PIN_7;
    GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
    GPIO_InitStructure.Pull = GPIO_PULLUP;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
}

//
void SDA_Output_Mode()
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    GPIO_InitStructure.Pin = GPIO_PIN_7;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
}

//
void SDA_Output( uint16_t val )
{
    if ( val )
    {
        GPIOB->BSRR |= GPIO_PIN_7;
    }
    else
    {
        GPIOB->BRR |= GPIO_PIN_7;
    }
}

//
void SCL_Output( uint16_t val )
{
    if ( val )
    {
        GPIOB->BSRR |= GPIO_PIN_6;
    }
    else
    {
        GPIOB->BRR |= GPIO_PIN_6;
    }
}

//
uint8_t SDA_Input(void)
{
	if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7) == GPIO_PIN_SET){
		return 1;
	}else{
		return 0;
	}
}

//
static void delay1(volatile unsigned int n)
{
    volatile uint32_t i;
    for ( i = 0; i < n; ++i);
}

//
void I2CStart(void)
{
    SDA_Output(1);
    delay1(DELAY_TIME);
    SCL_Output(1);
    delay1(DELAY_TIME);
    SDA_Output(0);
    delay1(DELAY_TIME);
    SCL_Output(0);
    delay1(DELAY_TIME);
}

//
void I2CStop(void)
{
    SCL_Output(0);
    delay1(DELAY_TIME);
    SDA_Output(0);
    delay1(DELAY_TIME);
    SCL_Output(1);
    delay1(DELAY_TIME);
    SDA_Output(1);
    delay1(DELAY_TIME);

}

//
unsigned char I2CWaitAck(void)
{
    unsigned short cErrTime = 5;
    SDA_Input_Mode();
    delay1(DELAY_TIME);
    SCL_Output(1);
    delay1(DELAY_TIME);
    while(SDA_Input())
    {
        cErrTime--;
        delay1(DELAY_TIME);
        if (0 == cErrTime)
        {
            SDA_Output_Mode();
            I2CStop();
            return ERROR;
        }
    }
    SCL_Output(0);
    SDA_Output_Mode();
    delay1(DELAY_TIME);
    return SUCCESS;
}

//
void I2CSendAck(void)
{
    SDA_Output(0);
    delay1(DELAY_TIME);
    delay1(DELAY_TIME);
    SCL_Output(1);
    delay1(DELAY_TIME);
    SCL_Output(0);
    delay1(DELAY_TIME);

}

//
void I2CSendNotAck(void)
{
    SDA_Output(1);
    delay1(DELAY_TIME);
    delay1(DELAY_TIME);
    SCL_Output(1);
    delay1(DELAY_TIME);
    SCL_Output(0);
    delay1(DELAY_TIME);

}

//
void I2CSendByte(unsigned char cSendByte)
{
    unsigned char  i = 8;
    while (i--)
    {
        SCL_Output(0);
        delay1(DELAY_TIME);
        SDA_Output(cSendByte & 0x80);
        delay1(DELAY_TIME);
        cSendByte += cSendByte;
        delay1(DELAY_TIME);
        SCL_Output(1);
        delay1(DELAY_TIME);
    }
    SCL_Output(0);
    delay1(DELAY_TIME);
}

//
unsigned char I2CReceiveByte(void)
{
    unsigned char i = 8;
    unsigned char cR_Byte = 0;
    SDA_Input_Mode();
    while (i--)
    {
        cR_Byte += cR_Byte;
        SCL_Output(0);
        delay1(DELAY_TIME);
        delay1(DELAY_TIME);
        SCL_Output(1);
        delay1(DELAY_TIME);
        cR_Byte |=  SDA_Input();
    }
    SCL_Output(0);
    delay1(DELAY_TIME);
    SDA_Output_Mode();
    return cR_Byte;
}


void I2CInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    GPIO_InitStructure.Pin = GPIO_PIN_7 | GPIO_PIN_6;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull = GPIO_PULLUP;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
}


/*
    主要逻辑为：
    先启动IIC —> IIC发送器件地址 —> 等待回应 —> IIC发送数据存放地址 —> 等待回应 —> IIC发送数据 —> 等待回应 —> 终止IIC的通信
*/

//EEPROM写操作 -基本写 -8位
/*
    这个函数的用法在于 其addr必须是 8 及整数倍以便对齐边界 后面的data是uint8_t型的数据
*/
void eeprom_write(uint8_t addr,uint8_t dat)
{
  I2CStart();
  I2CSendByte(0xA0);//写地址是0xA0
  I2CWaitAck();
  I2CSendByte(addr);  
  I2CWaitAck();
  I2CSendByte(dat);
  I2CWaitAck();
  I2CStop(); 
}

/*
    主要逻辑为：（有两步骤）
    先启动IIC —> IIC发送器件地址A0（写） —> 等待回应 —> IIC发送数据存放地址 —> 等待回应 —> 终止IIC的通信
    先启动IIC —> IIC发送器件地址A1（读） —> 等待回应 —> 存储IIC返回数据 —> 取消IIC回应 —> 终止IIC的通信 —> 返回对应数据（有两步骤）
*/

//EEPROM读操作 -基本读 -8位
/*
    这个函数的用法在于 读addr相应位置的数据
*/
uint8_t eeprom_read(uint8_t addr)
{
  uint8_t rec;
  I2CStart();
  I2CSendByte(0xA0); //写地址是0xA0
  I2CWaitAck();
  I2CSendByte(addr);  
  I2CWaitAck();
  I2CStop(); 

  I2CStart();
  I2CSendByte(0xA1); //读地址是0xA1
  I2CWaitAck();
  rec = I2CReceiveByte();
  I2CSendNotAck();
  I2CStop();
  return rec;
}



//EEPROM写16位操作—分开写
/*利用前面基本的8位读写操作进行 16位的拓展*/
void eeprom_write_16(uint8_t addr,uint8_t dat)
{
  uint8_t dat_h = dat >> 8;
  uint8_t dat_l = dat;
  eeprom_write(addr,dat_h);
  HAL_Delay(10);
  eeprom_write(addr+1,dat_l);
}


//EEPROM读16位操作 —分开读
/*
  利用基本的8位读操作进行 16位的读取
  将16位拆分为高8位和低8位
*/
uint16_t eeprom_read_16(uint8_t addr)
{
  uint8_t dat_h = eeprom_read(addr);
  uint8_t dat_l = eeprom_read(addr + 1);
  return (dat_h << 8) + dat_l;
}

//EEPROM写浮点数
/*
    将 浮点数先*100 （加两位小数）
    再将浮点数根据16位的存取方法进行分开存储
*/
void eeprom_write_f(uint8_t addr,float dat)
{
  uint16_t dat_f = dat * 100;
  uint8_t dat_h = dat_f >> 8;
  uint8_t dat_l = dat_f;
  eeprom_write(addr,dat_h);
  HAL_Delay(10);
  eeprom_write(addr+1,dat_l);
}



//EEPROM读浮点数
float eeprom_read_f(uint8_t addr)
{
  uint8_t dat_h = eeprom_read(addr);
  uint8_t dat_l = eeprom_read(addr + 1);
  return ((dat_h << 8) + dat_l) / 100.0f;
}


/*
    主要逻辑为：
    先启动IIC —> IIC发送器件地址 —> 等待回应 —> IIC发送数据 —> 等待回应 —> 终止IIC的通信
*/

//数字电位器写操作
void mcp_write(uint8_t dat)
{
  I2CStart();
  I2CSendByte(0x5e); //地址是0x5e
  I2CWaitAck();
  I2CSendByte(dat);
  I2CWaitAck();
  I2CStop(); 
}
