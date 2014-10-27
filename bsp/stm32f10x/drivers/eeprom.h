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


#define STM32_FLASH_SIZE     512         //所选STM32的FLASH容量大小(单位为K)
#define STM32_FLASH_WREN     1           //使能FLASH写入(0，不是能;1，使能)
#define STM32_FLASH_BASE     0x08000000  //STM32 FLASH的起始地址
#define FLASH_EEPROM_ADDR    (STM32_FLASH_BASE + (508*1024))//模拟EEPROM参数地址510k
 

u16 STMFLASH_ReadHalfWord(u32 faddr);     //读出半字  
void STMFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite);   
void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead);      

void rt_hw_eeprom_register(void);

#endif

