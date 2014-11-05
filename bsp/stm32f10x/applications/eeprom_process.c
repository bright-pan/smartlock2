#include "eeprom_process.h"
#include <time.h>
#define USEING_DEBUG_INFO        1

#define SYSTEM_TIME_SAVE_ADDR    0
#define SYSTEM_TIME_SAVE_SIZE    sizeof(rt_uint32_t)

/*
功能:获得当前时间
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

	RT_DEBUG_LOG(USEING_DEBUG_INFO,("System Time: %s\n",ctime((const time_t *)&time)));
  return time;
}

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

	RT_DEBUG_LOG(USEING_DEBUG_INFO,("System Time: %s\n",ctime((const time_t *)&date)));	
}

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

void eeprom_device_close(rt_device_t dev)
{
	if(dev != RT_NULL)
	{
		rt_device_close(dev);
	}
}

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

rt_err_t system_time_save(void)
{
	return eeprom_save_system_time(sys_date_get());
}

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
	eeprom_save_system_time(sys_date_get());
	return 0;
}
INIT_APP_EXPORT(system_time_error_process);
#endif
