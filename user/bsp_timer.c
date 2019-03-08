/**
  **********************************  STM8S  ***********************************
  * @文件名     ： bsp_timer.c
  * @作者       ： strongerHuang
  * @库版本     ： V2.2.0
  * @文件版本   ： V1.0.0
  * @日期       ： 2017年04月10日
  * @摘要       ： TIM定时器源文件
  ******************************************************************************/
/*----------------------------------------------------------------------------
  更新日志:
  2017-04-10 V1.0.0:初始版本
  ----------------------------------------------------------------------------*/
/* 包含的头文件 --------------------------------------------------------------*/
#include "bsp_timer.h"

#define BSP_IRQ_TIME	1		//ms


typedef struct
{
	//unsigned short millTime;
	unsigned char  used;
	unsigned short irqCount;
	unsigned short irqCountMax;
	void (*callback)(int timeId);
} bsp_timer_t;

static bsp_timer_t	bsp_timer_array[BSP_TIMER_MAX];

void bsp_timer_array_init(void)
{
	int i;

	for(i=0; i<BSP_TIMER_MAX; i++)
	{
		bsp_timer_array[i].used = 0;
		bsp_timer_array[i].irqCount = 0;
		bsp_timer_array[i].irqCountMax = 0;
		bsp_timer_array[i].callback = 0;
	}
}

void bsp_timer_update_count(int timerId)
{
	//if(timerId < BSP_TIMER_MAX)
	//{
		bsp_timer_array[timerId].irqCount++;
	//}
}

void bsp_start_timer(int timerId, unsigned short millTime, void (*callback)(int id))
{
	if(timerId >= BSP_TIMER_MAX)
		return;

	bsp_timer_array[timerId].irqCount = 0;
	if(millTime%BSP_IRQ_TIME)
		bsp_timer_array[timerId].irqCountMax = ((unsigned short)(millTime/BSP_IRQ_TIME))+1;
	else
		bsp_timer_array[timerId].irqCountMax = ((unsigned short)(millTime/BSP_IRQ_TIME));
	
	bsp_timer_array[timerId].callback = callback;
	bsp_timer_array[timerId].used = 1;
	bsp_timer_hw_enable(timerId, 1);
}

void bsp_stop_timer(int timerId)
{
	if(timerId >= BSP_TIMER_MAX)
		return;

	bsp_timer_array[timerId].used = 0;
	bsp_timer_array[timerId].callback = 0;
	bsp_timer_hw_enable(timerId, 0);
}


void bsp_timer_proc(void)
{
	int i;

	for(i=0; i<BSP_TIMER_MAX; i++)
	{
		if(bsp_timer_array[i].used)
		{
			if(bsp_timer_array[i].irqCount >= bsp_timer_array[i].irqCountMax)
			{
				bsp_timer_array[i].irqCount = 0;
				if(bsp_timer_array[i].callback)
				{
					//console_write("timer 0 callback \n");
					bsp_timer_array[i].callback(i);
				}
			}
		}
	}
}

void bsp_timer_hw_enable(int timerId, unsigned char flag)
{
	if(timerId >= BSP_TIMER_MAX)
		return;

	if(flag)
	{
		if(timerId == BSP_TIMER_0)
		{
			TIM1_SetCounter(0); 						   //计数值归零
			TIM1_Cmd(ENABLE);							   //启动定时器
		}
		else if(timerId == BSP_TIMER_1)
		{
			TIM2_SetCounter(0); 						   //计数值归零
			TIM2_Cmd(ENABLE);							   //启动定时器
		}
		else if(timerId == BSP_TIMER_2)
		{
#if defined(STM8S105)
			TIM3_SetCounter(0); 						   //计数值归零
			TIM3_Cmd(ENABLE);							   //启动定时器
#endif
		}
		else if(timerId == BSP_TIMER_3)
		{
			TIM4_SetCounter(0); 						   //计数值归零
			TIM4_Cmd(ENABLE);							   //启动定时器
		}
	}
	else
	{
		if(timerId == BSP_TIMER_0)
		{
			TIM1_SetCounter(0);
			TIM1_Cmd(DISABLE);
		}
		else if(timerId == BSP_TIMER_1)
		{
			TIM2_SetCounter(0);
			TIM2_Cmd(DISABLE);
		}
		else if(timerId == BSP_TIMER_2)
		{
#if defined(STM8S105)
			TIM3_SetCounter(0);
			TIM3_Cmd(DISABLE);
#endif
		}
		else if(timerId == BSP_TIMER_3)
		{
			TIM4_SetCounter(0);
			TIM4_Cmd(DISABLE);
		}
	}
}



/************************************************
函数名称 ： bsp_timer_hw_init
功    能 ： 定时器初始化
参    数 ： 无
返 回 值 ： 无
作    者 ： strongerHuang
*************************************************/
void bsp_timer_hw_init(void)
{
	TIM1_TimeBaseInit(16-1,TIM1_COUNTERMODE_UP, 1000, 0);  //定时1ms (16M/16/1000 = 1000)
	TIM1_ARRPreloadConfig(ENABLE);
	TIM1_ClearFlag(TIM1_FLAG_UPDATE);			   //清除标志位
	TIM1_ITConfig(TIM1_IT_UPDATE, ENABLE);		   //使能更新UPDATE中断

	TIM2_TimeBaseInit(TIM2_PRESCALER_128, 125-1);  //定时1ms (16M/128/125 = 1000)
	TIM2_ClearFlag(TIM2_FLAG_UPDATE);			   //清除标志位
	TIM2_ITConfig(TIM2_IT_UPDATE, ENABLE);		   //使能更新UPDATE中断

#if defined(STM8S105)
	TIM3_TimeBaseInit(TIM3_PRESCALER_128, 125-1);  //定时1ms (16M/128/125 = 1000)
	TIM3_ClearFlag(TIM3_FLAG_UPDATE);			   //清除标志位
	TIM3_ITConfig(TIM3_IT_UPDATE, ENABLE);		   //使能更新UPDATE中断
#endif	
	TIM4_TimeBaseInit(TIM4_PRESCALER_128, 125-1);  //定时1ms (16M/128/125 = 1000)
	TIM4_ClearFlag(TIM4_FLAG_UPDATE);              //清除标志位
	TIM4_ITConfig(TIM4_IT_UPDATE, ENABLE);         //使能更新UPDATE中断

	bsp_timer_array_init();

	enableInterrupts();                           //使能全局中断
}


/**** Copyright (C)2017 strongerHuang. All Rights Reserved **** END OF FILE ****/
