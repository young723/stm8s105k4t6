#include "stdio.h"
//#include "stdlib.h"
#include <stdarg.h>
#include "stm8s.h"
#include "qst_i2c.h"
#include "delay.h"
#include "bsp_timer.h"

//#define STM_IRQ_RX_ENABLE
#define QST_UART_DEBUG
//#define QST_WRITE_EEPROM
#if defined(QST_WRITE_EEPROM)
#include "bsp_eeprom.h"
#define EEPROM_ADDR               0x4000         //EEPROM±‡≥Ãµÿ÷∑
#endif

enum
{
	DEVICE_ACC = 0x01,
	DEVICE_MAG = 0x02,
	DEVICE_PRESS = 0x04,
	DEVICE_GYRO = 0x08,

	DEVICE_MAX = 0xff
};

enum
{
  	CMD_NONE,
	CMD_WRITE_ROM,
	CMD_WITRE_REG,
	CMD_READ_REG,
	CMD_ACC_SET_PARA,
	CMD_MAG_SET_PARA,
	CMD_PRE_SET_PARA,

	CMD_MAX = 0xff
};

static u32 qst_run_count = 0;
static uint8_t device_type = 0;

#if defined(STM_IRQ_RX_ENABLE)
uint8_t RxTxbuf[40];
uint8_t RxTxLen = 0;
uint8_t uartCmd = CMD_NONE;
#endif

#if defined(QST_CONFIG_QMAX981)
extern uint8_t qmaX981_init(void);
extern void qmaX981_read_xyz(void);
unsigned char qmaX981_irq_hdlr(void);
#endif
#if defined(QST_CONFIG_QMP6988)
extern uint8_t qmp6988_init(void);
extern void qma6988_calc_press(void);
extern void qmp6988_set_power_mode(unsigned char power_mode);
extern void qmp6988_set_app(unsigned char app);
#endif
#if defined(QST_CONFIG_QMCX983)
extern uint8_t qmcX983_init(void);
extern uint8_t qmcX983_read_xyz(void);
#endif

#if defined(QST_CONFIG_FIS210X)
extern uint8_t fis210x_init(void);
extern void fis210x_read_gyro_xyz(void);
extern void fis210x_read_acc_xyz(void);
#endif
#if defined(QST_CONFIG_JHM1200)
unsigned char jhm1200_init(void);
void jhm1200_get_cal(void);
extern void jhm1200_reg_test(void);
#endif


#if defined(QST_UART_DEBUG)
uint8_t itoa10(uint8_t *buf, int32_t i)
{
     uint32_t rem;
     uint8_t *s,length=0;
	 uint8_t flag=0;

     s = buf;
     if (i == 0) 
     {
     	*s++ = '0'; 
     }
     else 
     { 
          if (i < 0) 
          {
	          *buf++ = '-';
			  flag = 1;
	          s = buf;
	          i = -i;
          }
          while (i) 
          {
	          ++length;
	          rem = i % 10;
	          *s++ = rem + '0';
	          i /= 10;
          }
          for(rem=0; ((unsigned char)rem)<length/2; rem++) 
          {
	          *(buf+length) = *(buf+((unsigned char)rem));
	          *(buf+((unsigned char)rem)) = *(buf+(length-((unsigned char)rem)-1));
	          *(buf+(length-((unsigned char)rem)-1)) = *(buf+length);
          }
     }
     *s=0;

	 return ((s-buf)+flag);
}

void ftoa(uint8_t *buf, float i)
{
	int32_t i_data;
	uint8_t length=0;

	if((i>=0.000001f) && (i<=0.000001f))		
	{
	   *buf++ = '0'; 
	   *buf++ = '.'; 
	   *buf++ = '0'; 
	}
	else
	{
		if (i < 0) 
		{
			*buf++ = '-';
			i = -i;
		}
		i_data = (int32_t)i;
		length = itoa10(buf, i_data);
		buf += length;
		*buf++ = '.';
		i_data = (int32_t)((i-i_data)*10000);
		length = itoa10(buf, i_data);
		buf += length;
	}
	
	*buf=0;
}

void qst_send_string(uint8_t *str)
{
	while(*str)
	{
#if defined(STM8S103)
		while((UART1_GetFlagStatus(UART1_FLAG_TXE)==RESET));
		UART1_SendData8(*str);
		while((UART1_GetFlagStatus(UART1_FLAG_TC)==RESET));
		str++;
#elif defined(STM8S105)
		while((UART2_GetFlagStatus(UART2_FLAG_TXE)==RESET));
		UART2_SendData8(*str);
		while((UART2_GetFlagStatus(UART2_FLAG_TC)==RESET));
		str++;
#endif
	}
}
#endif

void qst_printf(const char *format, ...)
{
#if defined(QST_UART_DEBUG)
	int8_t *pc;
	int32_t value;
	float f_value;
	uint8_t buf[50];
	
	va_list arg;
	va_start(arg, format);
	//buf[0]=va_arg(arg, char);
	while (*format)
	{
		int8_t ret = *format;
		if(ret == '%')
		{
			switch (*++format)
			{
			case 'c':
			{
				buf[0] = va_arg(arg, char);
				//putchar(ch);
				buf[1] = 0;
				qst_send_string(buf);
				break;
			}
			case 's':
			{
				pc = va_arg(arg, int8_t *);
				//while (*pc)
				//{
				//	putchar(*pc);
				//	pc++;
				//}
				qst_send_string((uint8_t*)pc);
				break;
			}
			case 'd':
			{
				value =	va_arg(arg, int16_t);
				itoa10(buf, value);
				qst_send_string(buf);
				break;
			}
			case 'l':
			{
				value =	va_arg(arg, int32_t);
				itoa10(buf, value);
				qst_send_string(buf);
				break;
			}			
			case 'f':
			{
				f_value = va_arg(arg, float);
				ftoa(buf, f_value);
				qst_send_string(buf);
				break;
			}			
			default:
				break;
			}
		}
		else
		{
			//putchar(*format);
			//qst_send_string((uint8_t*)&ret);	
#if defined(STM8S103)
			while((UART1_GetFlagStatus(UART1_FLAG_TXE)==RESET));
			UART1_SendData8(ret);
			while((UART1_GetFlagStatus(UART1_FLAG_TC)==RESET));
#elif defined(STM8S105)
			while((UART2_GetFlagStatus(UART2_FLAG_TXE)==RESET));
			UART2_SendData8(ret);
			while((UART2_GetFlagStatus(UART2_FLAG_TC)==RESET));
#endif
		}
		format++;
	}
	va_end(arg);
#endif
}

#if defined(STM_IRQ_RX_ENABLE)
INTERRUPT_HANDLER(UART1_RX_IRQHandler, ITC_IRQ_UART1_RX)
{
	if(UART1_GetITStatus(UART1_IT_RXNE_OR) != RESET)		// interrupt flag
	{
		UART1_ClearITPendingBit(UART1_IT_RXNE_OR);
	}
	RxTxbuf[RxTxLen] = UART1_ReceiveData8();
	if(RxTxbuf[RxTxLen] == '\n')
	{
		RxTxbuf[RxTxLen] = 0;
		if((RxTxLen>3))
		{
			if((RxTxbuf[0]=='P')&&(RxTxbuf[1]=='R')&&(RxTxbuf[2]=='E'))
			{
				uartCmd = CMD_PRE_SET_PARA;
			}
#if defined(QST_WRITE_EEPROM)
			else if((RxTxbuf[0]=='R')&&(RxTxbuf[1]=='O')&&(RxTxbuf[2]=='M'))
			{
				EEPROM_WriteNByte(RxTxbuf+3, EEPROM_ADDR, RxTxLen-3);
			}
#endif
		}
		RxTxLen = 0;
	}
	else
	{
		RxTxLen++;
	}
	//if(UART1_GetFlagStatus(UART1_FLAG_RXNE) == RESET)
	//{
	//	RxTxLen = 0;
	//}
}
#endif

void clk_config(void)
{
	CLK_DeInit(); 
	// system clock
	CLK_ClockSwitchConfig(CLK_SWITCHMODE_AUTO,CLK_SOURCE_HSI,ENABLE,CLK_CURRENTCLOCKSTATE_ENABLE);
	CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);
	CLK_HSICmd(ENABLE);
}

void uart_config(void)
{
#if defined(STM8S103)
	// uart1
	UART1_DeInit();
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_UART1,ENABLE);
	UART1_Init(115200,UART1_WORDLENGTH_8D,UART1_STOPBITS_1,UART1_PARITY_NO,UART1_SYNCMODE_CLOCK_DISABLE,UART1_MODE_TXRX_ENABLE);
	UART1_Cmd(ENABLE);
#if defined(STM_IRQ_RX_ENABLE)
	UART1_ITConfig(UART1_IT_RXNE_OR,ENABLE);
#else
	UART1_ITConfig(UART1_IT_RXNE_OR,DISABLE);
#endif
	
#elif defined(STM8S105)
		// uart1
	UART2_DeInit();
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_UART2,ENABLE);
	UART2_Init(115200,UART2_WORDLENGTH_8D,UART2_STOPBITS_1,UART2_PARITY_NO,UART2_SYNCMODE_CLOCK_DISABLE,UART2_MODE_TXRX_ENABLE);
	UART2_Cmd(ENABLE);
	UART2_ITConfig(UART2_IT_RXNE_OR,DISABLE);

#endif
}

void i2c_config(uint16_t addr)
{
	//hardware i2c
#if defined(QST_SW_IIC)||defined(QST_SW_IIC_MTK)
	GPIO_DeInit(GPIOB);
	GPIO_Init(GPIOB, GPIO_PIN_4, GPIO_MODE_OUT_PP_HIGH_FAST);	// clk
	GPIO_Init(GPIOB, GPIO_PIN_5, GPIO_MODE_OUT_OD_HIZ_FAST);	// data
#else
	I2C_DeInit();
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_I2C,ENABLE);	
	I2C_Init(100000, addr, I2C_DUTYCYCLE_2, I2C_ACK_CURR, I2C_ADDMODE_7BIT, 16);
	I2C_Cmd(ENABLE);
#endif
}

#if defined(USE_SPI)
void spi_config(void)
{
	//PA3:CS	PC5:CLK		PC6:MOSI	PC7:MISO
	SPI_DeInit();
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_SPI, ENABLE);
#if defined(STM8S103)
	GPIO_Init(GPIOA, GPIO_PIN_3, GPIO_MODE_OUT_PP_HIGH_FAST);		// CS
#elif defined(STM8S105)
	//GPIO_Init(GPIOE, GPIO_PIN_5, GPIO_MODE_OUT_PP_HIGH_FAST);		// CS
	//GPIO_Init(GPIOC, GPIO_PIN_4, GPIO_MODE_OUT_PP_HIGH_FAST);		// CS
	GPIO_Init(GPIOF, GPIO_PIN_4, GPIO_MODE_OUT_PP_HIGH_FAST);		// CS
#endif
    //GPIO_Init(GPIOC, GPIO_PIN_5, GPIO_MODE_OUT_PP_LOW_FAST);		//CLK
    //GPIO_Init(GPIOC, GPIO_PIN_6, GPIO_MODE_OUT_PP_HIGH_FAST);		//MOSI
    //GPIO_Init(GPIOC, GPIO_PIN_7, GPIO_MODE_IN_PU_NO_IT);			//MISO 
#if defined(QST_CONFIG_FIS210X)
	SPI_Init(SPI_FIRSTBIT_MSB,SPI_BAUDRATEPRESCALER_4,SPI_MODE_MASTER,SPI_CLOCKPOLARITY_HIGH,SPI_CLOCKPHASE_2EDGE,
		SPI_DATADIRECTION_2LINES_FULLDUPLEX,SPI_NSS_SOFT,0x07);
#else
	SPI_Init(SPI_FIRSTBIT_MSB,SPI_BAUDRATEPRESCALER_4,SPI_MODE_MASTER,SPI_CLOCKPOLARITY_LOW,SPI_CLOCKPHASE_1EDGE,
		SPI_DATADIRECTION_2LINES_FULLDUPLEX,SPI_NSS_SOFT,0x07);		//SPI_NSS_HARD SPI_NSS_SOFT
#endif
	SPI_Cmd(ENABLE);
#if defined(STM8S103)
	GPIO_WriteHigh(GPIOA,GPIO_PIN_3);		// CS	
#elif defined(STM8S105)
	//GPIO_WriteHigh(GPIOE,GPIO_PIN_5);		// CS	
	//GPIO_WriteHigh(GPIOC,GPIO_PIN_4);		// CS	
	GPIO_WriteHigh(GPIOF,GPIO_PIN_4);		// CS	
#endif	
}
#endif

void gpio_config(void)
{
#if defined(STM8S103)
	GPIO_Init(GPIOA, GPIO_PIN_3, GPIO_MODE_OUT_PP_HIGH_FAST);
	GPIO_WriteLow(GPIOA,GPIO_PIN_3);
#endif
#if defined(STM8S105)
// led
	GPIO_Init(GPIOD, GPIO_PIN_2, GPIO_MODE_OUT_PP_HIGH_FAST);
	GPIO_WriteLow(GPIOD,GPIO_PIN_2);
// led
#endif
}


void qst_gpio_test(void)
{
#if defined(QST_SW_IIC)||defined(QST_SW_IIC_MTK)
	GPIO_WriteHigh(GPIOA,GPIO_PIN_3);
	//delay_ms(100);
#if 0
	GPIO_WriteHigh(GPIOB, GPIO_PIN_5);
	delay_ms(5);
	for(index=0; index<8; index++)
	{
		GPIO_WriteLow(GPIOB, GPIO_PIN_4);
		delay_ms(5);
		GPIO_WriteHigh(GPIOB, GPIO_PIN_4);
		delay_ms(5);
	}
#endif
#else	
	GPIO_DeInit(GPIOB);
	GPIO_Init(GPIOB, GPIO_PIN_4, GPIO_MODE_OUT_PP_HIGH_FAST);	// clk
	GPIO_Init(GPIOB, GPIO_PIN_5, GPIO_MODE_OUT_OD_HIZ_FAST);	// data
 #if 1
	GPIO_WriteHigh(GPIOB, GPIO_PIN_5);
	delay_ms(5);
	for(index=0; index<8; index++)
	{
		GPIO_WriteLow(GPIOB, GPIO_PIN_4);
		delay_ms(5);
		GPIO_WriteHigh(GPIOB, GPIO_PIN_4);
		delay_ms(5);
	}

	 //GPIO_Init(GPIOB, GPIO_PIN_4, GPIO_MODE_OUT_OD_HIZ_SLOW);	// clk
     //GPIO_Init(GPIOB, GPIO_PIN_5, GPIO_MODE_OUT_OD_HIZ_SLOW);	// data
#endif
	GPIO_DeInit(GPIOB);
#endif
	//delay_ms(200);
}


void qmaX981_timer_read_xyz(int timeId)
{
#if defined(QST_CONFIG_QMAX981)
	qmaX981_read_xyz();
#endif
}

void mcu_gpiod_irq_hdlr(void)
{
#if defined(QST_CONFIG_QMAX981)
	qst_run_count = 0;
	 qmaX981_irq_hdlr();
#endif
}

void mcu_gpioe_irq_hdlr(void)
{
#if defined(QST_CONFIG_QMAX981)
	qst_run_count = 0;
	qmaX981_irq_hdlr();	
#endif
}

void EXTI_Configuration(void)
{
#if defined(STM8S103)
	GPIO_Init(GPIOD, GPIO_PIN_2, GPIO_MODE_IN_FL_IT);
	EXTI_SetExtIntSensitivity(EXTI_PORT_GPIOD, EXTI_SENSITIVITY_RISE_ONLY);
	EXTI_SetTLISensitivity(EXTI_TLISENSITIVITY_RISE_ONLY);
#endif
#if defined(STM8S105)
	GPIO_Init(GPIOE, GPIO_PIN_5, GPIO_MODE_IN_FL_IT);
	EXTI_SetExtIntSensitivity(EXTI_PORT_GPIOE, EXTI_SENSITIVITY_FALL_ONLY);
	EXTI_SetTLISensitivity(EXTI_TLISENSITIVITY_FALL_ONLY);
#endif
}


void mcu_reset_counter(void)
{
	qst_run_count = 0;
}

void mcu_timer_sensor_cbk(int timeId)
{
#if defined(QST_CONFIG_QMP6988)
	if(device_type & DEVICE_PRESS)
	{
		qma6988_calc_press();
	}
#endif
}

void mcu_enter_sleep(void)
{
	qst_printf("mcu_enter_sleep\n");
	bsp_stop_timer(0);
	bsp_stop_timer(1);
	bsp_stop_timer(2);
	bsp_stop_timer(3);
	halt();
}

void mcu_exit_sleep(void)
{
	clk_config();
	qst_printf("mcu_exit_sleep\n");
	
	//bsp_start_timer(2, 500, mcu_timer_sensor_cbk);
}

void sys_init(void)
{
	clk_config();
	disableInterrupts();
	delay_init(16);
	gpio_config();
	uart_config();
#if defined(USE_SPI)	
	spi_config();
#endif
	i2c_config(0x0c<<1);
#if defined(QST_WRITE_EEPROM)
	EEPROM_Initializes();
#endif

	EXTI_Configuration();
	bsp_timer_hw_init();

	enableInterrupts();
	delay_ms(1000);
}

void main( void )
{
	uint8_t chip_id = 0;
	//uint8_t	index, value;
	//uint8_t ret = 0;

	sys_init();
	
#if defined(STM_IRQ_RX_ENABLE)
	RxTxbuf[0] = 0;
	RxTxLen = 0;
#endif
	qst_printf("qst sensor entry %s-%s!\n",__DATE__, __TIME__);
#if defined(QST_CONFIG_QMAX981)
	if((chip_id=qmaX981_init())!=0)
	{
		device_type |= DEVICE_ACC;
		qst_printf("qmaX981 OK! chip_id=%d\n",chip_id);
	}
#endif
#if defined(QST_CONFIG_QMP6988)
	if((chip_id=qmp6988_init())!=0)
	{
		device_type |= DEVICE_PRESS;
		qst_printf("qmp6988 OK! chip_id=%d\n",chip_id);
	}
#endif

#if defined(QST_WRITE_EEPROM)
	EEPROM_ReadNByte(RxTxbuf, EEPROM_ADDR, 20);
	qst_printf("EEPROM:%s\n", RxTxbuf);
	delay_ms(200);
#endif

	if(device_type)
	{
		//bsp_start_timer(2, 500, mcu_timer_sensor_cbk);
		while(1)
		{
			qst_run_count++;
			if(qst_run_count > 200000)
			{
				qst_run_count = 0;
				mcu_enter_sleep();
				mcu_exit_sleep();
			}
			bsp_timer_proc();
		}
	}
	else
	{
		while(1)
		{
			qst_printf("no sensor\n");
			delay_ms(200);
		}
	}
}



