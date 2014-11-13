/**
  ******************************************************************************
  * @file    eeprom_process.h
  * @author  wangzw <wangzw@yuettak.com>
  * @version V1.1.0
  * @date    10/11/2014
  * @brief   提供需要对EEPROM进行操作的接口
  ******************************************************************************
  * @copy
  *
  *
  * <h2><center>&copy; COPYRIGHT 2010 Yuettalk</center></h2>
  */ 


#ifndef __EEPROM_PROCESS_H__
#define __EEPROM_PROCESS_H__
#include "rtthread.h"
#include "eeprom.h"
#include "untils.h"

rt_err_t system_time_save(void);
rt_err_t eeprom_debugmap_manage(MapByteDef_p map,rt_uint8_t cmd);

#endif

/**
  * @}
  */

/**
  * @}
  */

/******************* (C) COPYRIGHT 2014 Yuettalk *****END OF FILE****/


