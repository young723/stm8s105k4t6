#ifndef JHM1200_H
#define JHM1200_H
#include "stm8s.h"

typedef enum
{
	Temp_Oversamp_4=0,
	Temp_Oversamp_8=1,
	Temp_Oversamp_16=2,
	Temp_Oversamp_32=3
}Temp_Oversamp;

typedef enum
{
	Press_Oversamp_0=7,
	Press_Oversamp_1=6,
	Press_Oversamp_2=5,
	Press_Oversamp_4=4,
	Press_Oversamp_8=3,
	Press_Oversamp_16=2,
	Press_Oversamp_32=1,
	Press_Oversamp_64=0
}Press_Oversamp;

#if 0
typedef signed int            s32;
typedef unsigned int          u32;
typedef signed short          s16;
typedef unsigned short        u16;
typedef	signed char           s8;
typedef unsigned char         u8;
#ifdef CPU_64Bits
typedef long                  s64;
#elif defined(CPU_32Bits)
typedef long long             s64;
#endif
#endif

typedef struct 
{
	u16 Gain_stage1:2;
	u16 Gain_stage2:3;
	u16 Gain_Polarity:1;
	u16 Clk_divider:2;
	u16 A2D_Offset:3;
	u16 OSR_Pressure:3;
	u16 OSR_Temp:2;
}NVM14_Define;

#endif
