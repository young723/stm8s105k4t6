/**
  **********************************  STM8S  ***********************************
  * @�ļ���     �� bsp_eeprom.h
  * @����       �� strongerHuang
  * @��汾     �� V2.2.0
  * @�ļ��汾   �� V1.0.0
  * @����       �� 2017��05��18��
  * @ժҪ       �� �ڲ�EEPROMͷ�ļ�
  ******************************************************************************/

/* �����ֹ�ݹ���� ----------------------------------------------------------*/
#ifndef _BSP_EEPROM_H
#define _BSP_EEPROM_H

/* ������ͷ�ļ� --------------------------------------------------------------*/
#include "stm8s.h"


/* �궨�� --------------------------------------------------------------------*/

/* �������� ------------------------------------------------------------------*/
void EEPROM_Initializes(void);
void EEPROM_WriteNByte(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t nByte);
void EEPROM_ReadNByte(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t nByte);


#endif /* _BSP_EEPROM_H */

/**** Copyright (C)2017 strongerHuang. All Rights Reserved **** END OF FILE ****/
