
#if defined(QST_CONFIG_JHM1200)
#include "stm8s.h"
#include "qst_i2c.h"
#if defined(USE_SPI)
#include "qst_spi.h"
#endif
#include "jhm1200.h"
#include "delay.h"


extern void qst_printf(const char *format, ...);
uint8_t jhm1200_iic_write(uint8_t Addr, uint8_t* Buff, uint8_t Len);
uint8_t jhm1200_iic_read(uint8_t *pData, uint16_t Length);

#define OSRx4
#define JHM_SLAVE_ADDR		0xf0

NVM14_Define NVM14;
static int NewPressData=0;
static int NewTempData=0;
#define CAL_L 20000  //20Kpa
#define CAL_H 120000 //120Kpa

extern uint8_t BSP_IIC_Write(uint8_t address, uint8_t *buf, uint8_t count);
extern uint8_t BSP_IIC_Read(uint8_t *buf, uint8_t count);

unsigned char jhm1200_write_regs(unsigned char *buf, unsigned char num)
{
	return jhm1200_iic_write(buf[0], &buf[1], num-1);
}

unsigned char jhm1200_read_regs(unsigned char *buf,unsigned short num)
{
	return jhm1200_iic_read(buf, num);
}

unsigned char jhm1200_is_busy(void)
{
	unsigned char status, ret;

	ret = jhm1200_read_regs(&status, 1);
	if(ret==0)	
		return 1;
	qst_printf("status = %d\n", status);
	status = (status>>5)&0x01;

	return status;
}

void jhm1200_get_raw_data(void)
{
	unsigned char i=0;
	unsigned char buffer[6];
	unsigned short *tmp=(unsigned short*)(&NVM14);

	buffer[0]=0xA5;
	buffer[1]=0x07|(NVM14.OSR_Temp<<6);
	buffer[2]=0x00;
	jhm1200_write_regs(buffer,3);
	delay_ms(30);
	while(1)
	{
		if(jhm1200_is_busy())
		{
			delay_ms(10);
			i++;
		}
		else
			break;

		if(i==100)
		{
			qst_printf("jhm1200 t busy!\n");
			return;
		}
	}

	jhm1200_read_regs(buffer, 3);
	NewTempData = ((int)buffer[1]<<8)|buffer[2]<<0;
	qst_printf("jhm1200 get temp raw %d \n", NewTempData);

	buffer[0]=0xA3;
	buffer[1]=(*tmp)>>8;
	buffer[2]=(*tmp)&0xFF;
	jhm1200_write_regs(buffer,3);
	delay_ms(50);
	while(1)
	{
		if(jhm1200_is_busy())
		{
			delay_ms(20);
			i++;
		}
		else
			break;

		if(i==100)
		{
			qst_printf("jhm1200 p busy!\n");
			return;
		}
	}
	//pjhm1200->IIC_Read(pjhm1200->Device_Address,buffer,4);
	jhm1200_read_regs(buffer, 4);
	NewPressData=((int)buffer[1]<<16)|((int)buffer[2]<<8)|buffer[3];
	
	NewPressData+= (-7)*524288;
	if(NewPressData<0)  NewPressData+=16777216;
	//*dataout = NewPressData;	
	qst_printf("jhm1200 get press raw %d \n", NewPressData);
}


void jhm1200_get_cal(void)
{
	int i=0;
	uint8_t buffer[6] = {0};
	uint32_t press_raw = 0;
	uint16_t temp_raw = 0;
	double press = 0.0, temp = 0.0;

	//jhm1200_get_raw_data();

	//Send 0xAC command and read the returned six-byte data
	buffer[0] = 0xAC;
	jhm1200_write_regs(buffer, 1);
	delay_ms(500);
	while(1)
	{
		if(jhm1200_is_busy())
		{
			qst_printf("jhm1200 t busy!\n");
			delay_ms(10);
			i++;
		}
		else
		{
			break;
		}
		if(i==100)
		{
			qst_printf("jhm1200 t busy return!\n");
			return;
		}
	}
	jhm1200_read_regs(buffer, 6);

	//The returned pressure and temperature values are converted into actual values according to the calibration range
	press_raw = ((uint32_t)buffer[1] << 16) | ((uint16_t)buffer[2] << 8) | buffer[3];
	temp_raw = ((uint16_t)buffer[4] << 8) | (buffer[5] << 0);
	press = (double)press_raw / 16777216;
	press = press * (CAL_H - CAL_L) + CAL_L;
	temp = (double)temp_raw / 65536;
	temp = temp * 19000 - 4000;

	qst_printf("jhm1200 press=%f, temp=%f\n",press, temp);
}


void jhm1200_reg_test(void)
{
	int i=0;
	uint8_t buffer[6] = {0};

	for(i=0; i<0x1f; i++)
	{
		buffer[0] = i;
		jhm1200_write_regs(buffer, 1);
		delay_ms(500);
		jhm1200_read_regs(buffer, 3);
		
		qst_printf("jhm1200 %d:[%d %d %d]\n",i, buffer[0],buffer[1],buffer[2]);
	}
}


unsigned char jhm1200_init(void)
{
//	unsigned char buffer[6];
//	unsigned short *tmp=(unsigned short*)(&NVM14);


	NVM14.OSR_Temp=Temp_Oversamp_4;
#ifdef OSRx0
	NVM14.OSR_Pressure=Press_Oversamp_0;
#elif defined(OSRx1)
	NVM14.OSR_Pressure=Press_Oversamp_1;
#elif defined(OSRx2)
	NVM14.OSR_Pressure=Press_Oversamp_2;
#elif defined(OSRx4)
	NVM14.OSR_Pressure=Press_Oversamp_4;
#elif defined(OSRx8)
	NVM14.OSR_Pressure=Press_Oversamp_8;
#elif defined(OSRx16)
	NVM14.OSR_Pressure=Press_Oversamp_16;
#elif defined(OSRx32)
	NVM14.OSR_Pressure=Press_Oversamp_32;
#elif defined(OSRx64)
	NVM14.OSR_Pressure=Press_Oversamp_64;
#else
	#error "No OSR defined"
#endif

	/*	
	NVM14.Gain_stage1=   
	NVM14.Gain_stage2=   
	NVM14.Gain_Polarity= 
	NVM14.Clk_divider=   
	NVM14.A2D_Offset= 	
	*/
/*
	tmp=(unsigned short*)(&NVM14);
	buffer[0]=0xA5;
	buffer[1]=0x07|(NVM14.OSR_Temp<<6);
	buffer[2]=0x00;
	jhm1200_write_regs(buffer,3);
	
	buffer[0]=0xA3;
	buffer[1]=(*tmp)>>8;
	buffer[2]=(*tmp)&0xFF;
	jhm1200_write_regs(buffer,3);
	*/
	return 1;
}
#endif
