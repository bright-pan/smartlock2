/**
  ******************************************************************************
  * @file    eeprom_process.c 
  * @author  wangzw <wangzw@yuettak.com>
  * @version V1.1.0
  * @date    10/11/2014
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and peripherals
  *          interrupt service routine.
  ******************************************************************************
  * @copy
  *
  *
  * <h2><center>&copy; COPYRIGHT 2014 Yuettalk</center></h2>
  */ 

/* Includes ------------------------------------------------------------------*/

#include "eeprom_process.h"
#include <time.h>
#define USEING_DEBUG_INFO        0       //调试信息

#define SYSTEM_TIME_SAVE_ADDR    0       //系统时间保存地址
#define SYSTEM_TIME_SAVE_SIZE    sizeof(rt_uint32_t) //系统时间保存空间大小



/**
  * @brief  获取当前系统时间
  * @param  None
  * @retval 系统时间
  */
static rt_uint32_t sys_date_get(void)
{
  rt_device_t device;
  rt_uint32_t time=0;

  device = rt_device_find("rtc");
  if (device != RT_NULL)
  {
      rt_device_control(device, RT_DEVICE_CTRL_RTC_GET_TIME, &time);
  }

	RT_DEBUG_LOG(USEING_DEBUG_INFO,("System Time: %s",ctime((const time_t *)&time)));
  return time;
}

/**
  * @brief  设置系统时间
  * @param  系统时间
  * @retval None
  */
static void sys_date_set(rt_uint32_t date)
{
  rt_device_t device;

  device = rt_device_find("rtc");
  if (device != RT_NULL)
  {
  	if(!(device->open_flag & RT_DEVICE_OFLAG_OPEN))
  	{
      rt_device_open(device,RT_DEVICE_OFLAG_RDWR);
  	}
    rt_device_control(device, RT_DEVICE_CTRL_RTC_SET_TIME, &date);
    rt_device_close(device);
  }

	RT_DEBUG_LOG(USEING_DEBUG_INFO,("System Time: %s",ctime((const time_t *)&date)));	
}

/**
  * @brief  EEPROM 驱动打开函数
  * @param  None
  * @retval 驱动对象
  */
rt_device_t eeprom_device_open(void)
{
	rt_device_t device;

	device = rt_device_find("EEPROM");
  if (device != RT_NULL)
  {
  	if(!(device->open_flag & RT_DEVICE_OFLAG_OPEN))
  	{
      rt_device_open(device,RT_DEVICE_OFLAG_RDWR);
  	}
  }

  return device;
}

/**
  * @brief  EEPROM 驱动关闭
  * @param  EEPROM 驱动对象
  * @retval None
  */
void eeprom_device_close(rt_device_t dev)
{
	if(dev != RT_NULL)
	{
		rt_device_close(dev);
	}
}

/**
  * @brief  向EEPROM 中保存时间
  * @param  时间
  * @retval RT_EOK 成功
  * @retval RT_EERROR 失败
  */
rt_err_t eeprom_save_system_time(rt_uint32_t date)
{
	rt_device_t dev;
	
  dev = eeprom_device_open();
  if(dev == RT_NULL)
  {
		rt_kprintf("EEPROM Open Fail --%s\n",__FUNCTION__);
		return RT_ERROR;
  }
	rt_device_write(dev,SYSTEM_TIME_SAVE_ADDR,&date,SYSTEM_TIME_SAVE_SIZE);
	rt_device_close(dev);
	
	return RT_EOK;
}

/**
  * @brief  从EEPROM 中读取系统时间
  * @param  时间接收地址
  * @retval RT_EOK 成功
  * @retval RT_ERROR
  */
rt_err_t eeprom_read_system_time(rt_uint32_t *date)
{
	rt_device_t dev;
	
  dev = eeprom_device_open();
  if(dev == RT_NULL)
  {
		rt_kprintf("EEPROM Open Fail --%s\n",__FUNCTION__);
		return RT_ERROR;
  }
	rt_device_read(dev,SYSTEM_TIME_SAVE_ADDR,date,SYSTEM_TIME_SAVE_SIZE);
	rt_device_close(dev);

	return RT_EOK;
}
/**
  * @brief  保存当前系统时间到EEPROM指定位置
  					供系统上电后较时使用。
  * @param  None
  * @retval RT_EOK 成功
  * @retval RT_ERROR 失败
  */
rt_err_t system_time_save(void)
{
	return eeprom_save_system_time(sys_date_get());
}

/**
  * @brief  系统时间错误处理，系统启动后如果检测到时1970年，
  					系统会自动向EEPROM中读取时间参数进行时间校准。
  * @param  None
  * @retval 0
  */
#ifdef RT_USING_COMPONENTS_INIT
int system_time_error_process(void)
{
	rt_uint32_t date;

	//获得系统时间
	date = sys_date_get();

	//判断date是否小于100
	if(date < 3)
	{
		//时间丢失
		rt_kprintf("system time error date:%d!!! ",date);
		eeprom_read_system_time(&date);
		sys_date_set(date);
	}
	else
	{
		//时间正常
	}
	//eeprom_save_system_time(sys_date_get());
	return 0;
}
INIT_APP_EXPORT(system_time_error_process);
#endif

/**
  * @}
  */

/**
  * @}
  */

/******************* (C) COPYRIGHT 2014 Yuettalk *****END OF FILE****/

