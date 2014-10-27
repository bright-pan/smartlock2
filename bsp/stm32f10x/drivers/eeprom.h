/*********************************************************************
 * Filename:    eeprom.h
 * 
 *
 * Author:        wangzw <wangzw@yuettak.com>
 * Created at:    2013-12-11 14:33:16
 *
 *                
 * Change Log:
 *
 * Copyright (C) 2013 Yuettak Co.,Ltd
 ********************************************************************/

#ifndef __EEPROM_H__
#define __EEPROM_H__
#include "stm32f10x.h"
#define DEVICE_NAME_EEPROM    "EEPROM"


#define STM32_FLASH_SIZE     512         //��ѡSTM32��FLASH������С(��λΪK)
#define STM32_FLASH_WREN     1           //ʹ��FLASHд��(0��������;1��ʹ��)
#define STM32_FLASH_BASE     0x08000000  //STM32 FLASH����ʼ��ַ
#define FLASH_EEPROM_ADDR    (STM32_FLASH_BASE + (508*1024))//ģ��EEPROM������ַ510k
 

u16 STMFLASH_ReadHalfWord(u32 faddr);     //��������  
void STMFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite);   
void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead);      

void rt_hw_eeprom_register(void);

#endif

