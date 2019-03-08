/**
  **********************************  STM8S  ***********************************
  * @�ļ���     �� bsp_eeprom.c
  * @����       �� strongerHuang
  * @��汾     �� V2.2.0
  * @�ļ��汾   �� V1.0.0
  * @����       �� 2017��05��18��
  * @ժҪ       �� �ڲ�EEPROMԴ�ļ�
  ******************************************************************************/
/*----------------------------------------------------------------------------
  ������־:
  2017-05-18 V1.0.0:��ʼ�汾
  ----------------------------------------------------------------------------*/
/* ������ͷ�ļ� --------------------------------------------------------------*/
#include "bsp_eeprom.h"


/************************************************
�������� �� EEPROM_Initializes
��    �� �� �ڲ�EEPROM��ʼ��
��    �� �� ��
�� �� ֵ �� ��
��    �� �� strongerHuang
*************************************************/
void EEPROM_Initializes(void)
{
  FLASH_SetProgrammingTime(FLASH_PROGRAMTIME_STANDARD);
}

/************************************************
�������� �� EEPROM_WriteNByte
��    �� �� дEEPROM
��    �� �� pBuffer ----- д���������׵�ַ
            WriteAddr --- Ҫд��Flash�ĵ�ַ
            nByte ------- Ҫд����ֽ���
�� �� ֵ �� ��
��    �� �� strongerHuang
*************************************************/
void EEPROM_WriteNByte(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t nByte)
{
  FLASH_Unlock(FLASH_MEMTYPE_DATA);

  while(FLASH_GetFlagStatus(FLASH_FLAG_DUL) == RESET);

  while(nByte--)
  {
    FLASH_ProgramByte(WriteAddr, *pBuffer);
    WriteAddr++;
    pBuffer++;
    FLASH_WaitForLastOperation(FLASH_MEMTYPE_DATA);
  }
  FLASH_Lock(FLASH_MEMTYPE_DATA);
}

/************************************************
�������� �� EEPROM_ReadNByte
��    �� �� ��EEPROM
��    �� �� pBuffer ---- ����buf
            ReadAddr --- Ҫ��ȡFlash���׵�ַ
            nByte ------ Ҫ��ȡ���ֽ���
�� �� ֵ �� ��
��    �� �� strongerHuang
*************************************************/
void EEPROM_ReadNByte(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t nByte)
{
  while(nByte--)
  {
    *pBuffer = FLASH_ReadByte(ReadAddr);
    ReadAddr++;
    pBuffer++;
  }
}

/************************************************
�������� �� EEPROM_EraseNByte
��    �� �� ��������N�ֽ�
��    �� �� EraseAddr --- Ҫ�������׵�ַ
            nByte ------- Ҫ�������ֽ���
�� �� ֵ �� ��
��    �� �� strongerHuang
*************************************************/
void EEPROM_EraseNByte(uint32_t EraseAddr, uint16_t nByte)
{
  uint16_t i;

  FLASH_Unlock(FLASH_MEMTYPE_DATA);

  while(FLASH_GetFlagStatus(FLASH_FLAG_DUL) == RESET);

  for(i=0; i<nByte; i++)
  {
    *(PointerAttr uint8_t*) (MemoryAddressCast)EraseAddr = 0xFF;
    EraseAddr++;
    FLASH_WaitForLastOperation(FLASH_MEMTYPE_DATA);
  }
  FLASH_Lock(FLASH_MEMTYPE_DATA);
}


/**** Copyright (C)2017 strongerHuang. All Rights Reserved **** END OF FILE ****/
