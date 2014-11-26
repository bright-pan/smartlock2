/*********************************************************************
 * Filename:      battery.c
 * 
 *
 * Author:        Bright Pan <loststriker@gmail.com>
 * Created at:    2013-06-08 11:17:29
 *
 *
 * Change Log:    
 *
 * Copyright (C) 2013 Yuettak Co.,Ltd
 ********************************************************************/

#include "battery.h"
#include "gpio_adc.h"
#include "untils.h"
#include "buzzer.h"
#include "alarm.h"


#define BATTERY_DEBUG_THREAD            31

#define BATTERY_FULL_ENERGE             1.3  //满电时测量得到的电压
#define BATTERY_LOW_ENERGE              0.8  //电量过低得到的电压
 
#define BATTERY_FILE_NAME								"/bat.txt"

#define BAT_DEVICE_NAME                 DEVICE_NAME_BAT

void bat_enable(void)
{
  rt_device_t device = RT_NULL;
  device = rt_device_find(BAT_DEVICE_NAME);
  if (device != RT_NULL)
  {
    rt_device_control(device, RT_DEVICE_CTRL_ENABLE_CONVERT, (void *)0);
    rt_kprintf("bat is starting convert!\n");
  }
  else
  {
    rt_kprintf("the device is not found!\n");
  }
}

void bat_disable(void)
{
  rt_device_t device = RT_NULL;
  device = rt_device_find(BAT_DEVICE_NAME);
  if (device != RT_NULL)
  {
    rt_device_control(device, RT_DEVICE_CTRL_DISABLE_CONVERT, (void *)0);
    rt_kprintf("bat is stoped convert!\n");
  }
  else
  {
    rt_kprintf("the device is not found!\n");
  }
}

void battery_get_data(Battery_Data* data)
{
	rt_device_t adc_dev;
	float temp = 0;
	
	adc_dev = rt_device_find(BAT_DEVICE_NAME);
	if(RT_NULL != adc_dev)
	{
		rt_uint16_t adc_temp[100];
		rt_uint8_t  i;

		rt_device_open(adc_dev,0);
		bat_enable();

		for(i = 0;i<100;i++)
		{
			rt_device_control(adc_dev, RT_DEVICE_CTRL_GET_CONVERT_VALUE, (void *)&adc_temp[i]);
		}
		data->adc_value = get_average_value(adc_temp,100);

		bat_disable();

		//计算电压
		data->voltage = (data->adc_value / 4095.0)*3.3;

		//计算剩余工作时间
		if(data->voltage > BATTERY_FULL_ENERGE)
		{
			temp = BATTERY_FULL_ENERGE;
		}
		else if(data->voltage < BATTERY_LOW_ENERGE)
		{
			//电量0%
			temp = BATTERY_LOW_ENERGE;
		}
		else if(data->voltage > BATTERY_FULL_ENERGE)
		{
			//电量100%
			temp = BATTERY_FULL_ENERGE;
		}
		else
		{
			temp = data->voltage;
		}
		data->DumpEnergy = (1-((BATTERY_FULL_ENERGE-temp)/
											 (BATTERY_FULL_ENERGE-BATTERY_LOW_ENERGE)))*100;
		/*data->work_time = 0xff;
		for(i = BATTERY_WORK_MAX_TIME ;i > 0;i--)
		{
			if(voltage > battery_table[BATTERY_WORK_MAX_TIME - i])
			{
				data->work_time = i;
				break;
			}
		}
		if(data->work_time == 0xff)
		{
			data->work_time = 0;
		}*/
	}
	else
	{
    rt_kprintf("%s device not find\n",DEVICE_NAME_BAT);
	}
}

/** 
@brief  电池信息保存到文件
@param  bat 电池 
@retval 0 :ok 1:error
*/
void battery_info_save_file(Battery_Data *bat)
{
	int fd;
	char *buf;
	rt_uint32_t date;

	date = sys_cur_date();
	
	fd = open(BATTERY_FILE_NAME,O_APPEND|O_RDWR,0x777);
	if(fd < 0)
	{
		fd = open(BATTERY_FILE_NAME,O_CREAT|O_RDWR,0x777);
		if(fd < 0)
		{
      rt_kprintf("Open %s file fial\n",BATTERY_FILE_NAME);
      return ;
		}
		
	}

	buf = rt_calloc(1,100);
	sprintf(buf,"Voltage>>%.04f Date>>%s \n\r",bat->voltage,ctime((const time_t *)&date));
	write(fd,buf,rt_strlen(buf));
	rt_free(buf);
	
	close(fd);
}

/** 
@brief  电池处理线程
@param  arg 
@retval 0 :ok 1:error
*/
void battery_thread_entry(void *arg)
{
	while(1)
	{
		Battery_Data bat;
		
		rt_thread_delay(100*60);

		// 线程进入工作
		rt_thread_entry_work(rt_thread_self());
		//battery_get_data(&bat);
		//battery_info_save_file(&bat);
		
		// 电压过低处理
		battery_too_low();

		// 线程进入休眠
		rt_thread_entry_sleep(rt_thread_self());
	}
}

int battery_thread_init(void)
{
	rt_thread_t id;

	
	id = rt_thread_create("bat",
                         battery_thread_entry, RT_NULL,
                         1024, 107, 20);
  if(id == RT_NULL)
  {
    rt_kprintf("bat thread init fail !\n");

    return 1;
  }

  rt_thread_startup(id);
	return 0;
}
INIT_APP_EXPORT(battery_thread_init);
/** 
@brief  电池电压过低
@param  bat 电池 
@retval 0 :ok 1:error
*/
int battery_too_low(void)
{
	Battery_Data bat;
	
  battery_get_data(&bat);
	if(bat.DumpEnergy <= 10)
	{
		send_local_mail(ALARM_TYPE_BATTERY_WORKING_20M,0,RT_NULL);
		while(1)
		{
			rt_kprintf("Battery too low\n");
			buzzer_work(BZ_TYPE_INIT);
			//sysreset();
			rt_thread_delay(RT_TICK_PER_SECOND*60*5);
			while(1);
		}
	}

	return ;
}
INIT_DEVICE_EXPORT(battery_too_low);








#ifdef RT_USING_FINSH
#include <finsh.h>

void bat_info(void)
{
	Battery_Data bat;
	char buf[20];
	
  battery_get_data(&bat);
	rt_kprintf("adc        : %d\n",bat.adc_value);
	rt_kprintf("DumpEnergy : %d\n",bat.DumpEnergy);

	sprintf(buf,"%.02f",bat.voltage);
	rt_kprintf("voltage    : %sv\n",buf);
}

FINSH_FUNCTION_EXPORT(bat_info,"battery info");

#endif



