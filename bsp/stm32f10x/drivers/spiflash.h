/*********************************************************************
 * Filename:    spiflash.h
 *
 * Description:    Tihs flie include spi flash filesystem use function and
 *						   spiflash.c file need of file include and macro define
 *						
 * Author:        wangzw <wangzw@yuettak.com>
 * Created at:    2013-04-26 12:00:00
 *                
 * Modify:
 *
 * 
 *
 * Copyright (C) 2013 Yuettak Co.,Ltd
 ********************************************************************/
#ifndef	__SPIFLASH_H__
#define		__SPIFLASH_H__
#include "spibus.h"

#define SPI1_CS1_NAME									  ("w25q32")
#define SPI1_BUS_NAME										("spi1bus")
#define FLASH1_DEVICE_NAME							("flash1")

int rt_spi_flash_init(void);



#endif 





