#ifndef  __DELAY_H__
#define  __DELAY_H__

//#include "stm8l15x.h"

void delay_init(unsigned char clk);	//��ʱ������ʼ��
void delay_us(unsigned short nus);       //us����ʱ����,���65536us.
void delay_ms(unsigned long nms);	//ms����ʱ����
#endif
