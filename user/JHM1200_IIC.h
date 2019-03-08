/*
****************************************************************************
*
* JHM1200_IIC.h
* Date: 2015/06/26
* Revision: 1.0.0
*
* Usage: IIC read and write interface
*
**************************************************************************/
#ifndef __JHM1200_IIC_H_
#define __JHM1200_IIC_H_

uint8_t BSP_IIC_Write(uint8_t IIC_Address, uint8_t *buffer, uint8_t count);
uint8_t BSP_IIC_Read(uint8_t IIC_Address, uint8_t *buffer, uint8_t count);

#endif
