/*
****************************************************************************
*
* JHM1200_IIC.c
* Date: 2015/06/26
* Revision: 1.0.0
*
* Usage: IIC read and write interface
*
**************************************************************************/
#include "stm32f10x.h"
#include "./jhm1200/JHM1200_IIC.h"

#define GPIO_PORT_I2C	GPIOB			/* GPIO端口 */
#define RCC_I2C_PORT 	RCC_APB2Periph_GPIOB		/* GPIO端口时钟 */
#define I2C_SCL_PIN		GPIO_Pin_6			/* 连接到SCL时钟线的GPIO */
#define I2C_SDA_PIN		GPIO_Pin_7			/* 连接到SDA数据线的GPIO */

/* 定义读写SCL和SDA的宏，已增加代码的可移植性和可阅读性 */
	#define I2C_SCL_OUTPUT()			
	#define I2C_SCL_INPUT()			
	#define I2C_SDA_OUTPUT()			
	#define I2C_SDA_INPUT()			
#if 1	/* 条件编译： 1 选择GPIO的库函数实现IO读写 */
	#define I2C_SCL_1()  GPIO_SetBits(GPIO_PORT_I2C, I2C_SCL_PIN)		/* I2C_SCL_1() */
	#define I2C_SCL_0()  GPIO_ResetBits(GPIO_PORT_I2C, I2C_SCL_PIN)		/* I2C_SCL_0() */
	
	#define I2C_SDA_1()  GPIO_SetBits(GPIO_PORT_I2C, I2C_SDA_PIN)		/* I2C_SDA_1() */
	#define I2C_SDA_0()  GPIO_ResetBits(GPIO_PORT_I2C, I2C_SDA_PIN)		/* I2C_SDA_0() */
	
	#define I2C_SDA_READ()  GPIO_ReadInputDataBit(GPIO_PORT_I2C, I2C_SDA_PIN)	/* 读SDA口线状态 */
#else	/* 这个分支选择直接寄存器操作实现IO读写 */
    /*　注意：如下写法，在IAR最高级别优化时，会被编译器错误优化 */
	#define I2C_SCL_1()  GPIO_PORT_I2C->BSRR = I2C_SCL_PIN				/* I2C_SCL_1() */
	#define I2C_SCL_0()  GPIO_PORT_I2C->BRR = I2C_SCL_PIN				/* I2C_SCL_0() */
	
	#define I2C_SDA_1()  GPIO_PORT_I2C->BSRR = I2C_SDA_PIN				/* I2C_SDA_1() */
	#define I2C_SDA_0()  GPIO_PORT_I2C->BRR = I2C_SDA_PIN				/* I2C_SDA_0() */
	
	#define I2C_SDA_READ()  ((GPIO_PORT_I2C->IDR & I2C_SDA_PIN) != 0)	/* 读SDA口线状态 */
#endif

void jhm1200_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_I2C_PORT, ENABLE);	/* 打开GPIO时钟 */

	GPIO_InitStructure.GPIO_Pin = I2C_SCL_PIN | I2C_SDA_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;  	/* 开漏输出 */
	GPIO_Init(GPIO_PORT_I2C, &GPIO_InitStructure);

	/* 给一个停止信号, 复位I2C总线上的所有设备到待机模式 */
	//i2c_Stop();
}


////Delay function needs to be defined
void DelayUs(unsigned char ms)
{
	uint8_t i;

	while(ms)
	{
		ms--;
		for (i = 0; i < 10; i++);
	}
}


//Start signal
void Start(void)
{
	I2C_SDA_1();
	DelayUs(2);
	I2C_SCL_1();
	DelayUs(2);
	I2C_SDA_0();
	DelayUs(2);
	I2C_SCL_0();
}

//Stop signal
void Stop(void)
{
	I2C_SDA_OUTPUT();
	I2C_SDA_0();
	DelayUs(2);
	I2C_SCL_1();
	DelayUs(2);
	I2C_SDA_1();
	DelayUs(2);
}

//Read ACK signal
unsigned char Check_ACK(void)
{
	unsigned char ack;
	I2C_SDA_INPUT();
	I2C_SCL_1();
	DelayUs(2);
	ack = I2C_SDA_READ();
	I2C_SCL_0();
	I2C_SDA_OUTPUT();
	return ack;
}

//Send ACK signal
void Send_ACK(void)
{
	I2C_SDA_OUTPUT();
	I2C_SDA_0();
	DelayUs(2);
	I2C_SCL_1();
	DelayUs(2);
	I2C_SCL_0();
	DelayUs(2);
}

//Send one byte
void SendByte(unsigned char byte)
{
	unsigned char i = 0;
	I2C_SDA_OUTPUT();
	do
	{
		if (byte & 0x80)
		{
			I2C_SDA_1();
		}
		else
		{
			I2C_SDA_0();
		}
		DelayUs(2);
		I2C_SCL_1();
		DelayUs(2);
		byte <<= 1;
		i++;
		I2C_SCL_0();
	} while (i < 8);
	I2C_SCL_0();
}

//Receive one byte
unsigned char ReceiveByte(void)
{
	unsigned char i = 0, tmp = 0;
	I2C_SDA_INPUT();
	do
	{
		tmp <<= 1;
		I2C_SCL_1();
		DelayUs(2);
		if (I2C_SDA_READ())
		{
			tmp |= 1;
		}
		I2C_SCL_0();
		DelayUs(2);
		i++;
	} while (i < 8);
	return tmp;
}

//Write a byte of data through IIC
uint8_t BSP_IIC_Write(uint8_t address, uint8_t *buf, uint8_t count)
{
	unsigned char timeout, ack;
	address &= 0xFE;
	Start();
	DelayUs(2);
	SendByte(address);
	I2C_SDA_INPUT();
	DelayUs(2);
	timeout = 0;
	do
	{
		ack = Check_ACK();
		timeout++;
		if (timeout == 10)
		{
			Stop();
			return 1;
		}
	} while (ack);
	while (count)
	{
		SendByte(*buf);
		I2C_SDA_INPUT();
		DelayUs(2);
		timeout = 0;
		do
		{
			ack = Check_ACK();
			timeout++;
			if (timeout == 10)
			{
				return 2; 
			}
		} while (0);
		buf++;
		count--;
	}
	Stop();
	return 0;
}

//Read a byte of data through IIC
uint8_t BSP_IIC_Read(uint8_t address, uint8_t *buf, uint8_t count)
{
	unsigned char timeout, ack;
	address |= 0x01;
	Start();
	SendByte(address);
	I2C_SDA_INPUT();
	DelayUs(2);
	timeout = 0;
	do
	{
		ack = Check_ACK();
		timeout++;
		if (timeout == 4)
		{
			Stop();
			return 1; 
		}
	} while (ack);
	DelayUs(2);
	while (count)
	{
		*buf = ReceiveByte();
		if (count != 1)
			Send_ACK();
		buf++;
		count--;
	}
	Stop();
	return 0;
}
