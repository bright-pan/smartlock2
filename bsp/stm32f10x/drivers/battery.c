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
#include "menu.h"
#include "local.h"

/***************************************************************************************************/

#define BATTERY_DEBUG_THREAD            32

#define BATTERY_FULL_ENERGE             1.3  //ÂúµçÊ±²âÁ¿µÃµ½µÄµçÑ¹
#define BATTERY_LOW_ENERGE              0.8  //µçÁ¿¹ýµÍµÃµ½µÄµçÑ¹
 
#define BATTERY_FILE_NAME								"/bat.txt"

#define BAT_DEVICE_NAME                 DEVICE_NAME_BAT


/**************************************************************************************************/
static rt_mq_t BatMail = RT_NULL;

/**************************************************************************************************/
void bat_enable(void)
{
  rt_device_t device = RT_NULL;
  device = rt_device_find(BAT_DEVICE_NAME);
  if (device != RT_NULL)
  {
    rt_device_control(device, RT_DEVICE_CTRL_ENABLE_CONVERT, (void *)0);
    rt_dprintf(BATTERY_DEBUG_THREAD,("bat is starting convert!\n"));
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
    rt_dprintf(BATTERY_DEBUG_THREAD,("bat is stoped convert!\n"));
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
		
		rt_device_control(adc_dev, RT_DEVICE_CTRL_GET_CONVERT_VALUE, (void *)&adc_temp[0]);
		for(i = 0;i<100;i++)
		{
			rt_device_control(adc_dev, RT_DEVICE_CTRL_GET_CONVERT_VALUE, (void *)&adc_temp[i]);
		}
		data->adc_value = get_average_value(adc_temp,100);

		rt_device_close(adc_dev);
		bat_disable();

		//¼ÆËãµçÑ¹
		data->voltage = (data->adc_value / 4095.0)*3.3;

		//¼ÆËãÊ£Óà¹¤×÷Ê±¼ä
		if(data->voltage > BATTERY_FULL_ENERGE)
		{
			temp = BATTERY_FULL_ENERGE;
		}
		else if(data->voltage < BATTERY_LOW_ENERGE)
		{
			//µçÁ¿0%
			temp = BATTERY_LOW_ENERGE;
		}
		else if(data->voltage > BATTERY_FULL_ENERGE)
		{
			//µçÁ¿100%
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
@brief  µç³ØÐÅÏ¢±£´æµ½ÎÄ¼þ
@param  bat µç³Ø 
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
@brief  µç³ØµçÁ¿µÍÓÚ20%
@param  arg 
@retval 0 :ok 1:error
*/
static void battery_low_20p_process(void)
{
	Battery_Data bat;
	
  battery_get_data(&bat);
  //rt_kprintf("DumpEnergy = %d%\n",bat.DumpEnergy);
	if(bat.DumpEnergy <= 20)
	{
		// Ö»Ê£ÏÂ20å%
		if(local_event_process(1,LOCAL_EVT_SYSTEM_BAT20P) == 1)
		{
			send_local_mail(ALARM_TYPE_BATTERY_WORKING_20M,0,RT_NULL);
			// ÉèÖÃµçÑ¹¹ýµÍ
			local_event_process(0,LOCAL_EVT_SYSTEM_BAT20P);
		}
	}
	else if(bat.DumpEnergy >= 50)
	{
		// ³äµç50%
		if(local_event_process(1,LOCAL_EVT_SYSTEM_BAT20P) == 0)
		{
			// Çå³ýµçÑ¹¹ýµÍ
			local_event_process(2,LOCAL_EVT_SYSTEM_BAT20P);
		}
		if(local_event_process(1,LOCAL_EVT_SYSTEM_BATLOW) == 0)
		{
			sysreset();
		}
	}
}

/** 
@brief  ¼ì²âµç³ØµçÁ¿20%
@param  mail  ÓÊ¼þ 
@retval none
*/
void battery_energy_check_20p(void)
{
	BatteryMailDef SendMail;

	SendMail.Type = BAT_MAILTYPE_20P;
	SendMail.result = RT_NULL;
	battery_send_mail(&SendMail);
}

/** 
@brief  ¼ì²âµç³ØµçÑ¹¹ýµÍ
@param  mail  ÓÊ¼þ 
@retval none
*/
void battery_energy_too_low(void)
{
	BatteryMailDef SendMail;

	SendMail.Type = BAT_MAILTYPE_LowEnergy;
	SendMail.result = RT_NULL;
	battery_send_mail(&SendMail);
}


/** 
@brief  µç³ØµçÑ¹¹ýµÍ
@param  bat µç³Ø 
@retval 0 :ok 1:error
*/
int battery_too_low(void)
{
	Battery_Data bat;
	
  battery_get_data(&bat);
  
	if(bat.DumpEnergy <= 5)
	{
		if(local_event_process(1,LOCAL_EVT_SYSTEM_BATLOW) == 1)
		{
			rt_kprintf("DumpEnergy = %d%\n",bat.DumpEnergy);
      local_event_process(0,LOCAL_EVT_SYSTEM_BATLOW);
      rt_kprintf("Battery too low\n");
      //buzzer_work(BZ_TYPE_INIT);
      gui_open_lcd_show();
      menu_event_process(0,MENU_EVT_BAT_LOW);
		}
	}
	else
	{
		if(local_event_process(1,LOCAL_EVT_SYSTEM_BATLOW) == 0)
		{
      local_event_process(2,LOCAL_EVT_SYSTEM_BATLOW);
      sysreset();
		}
	}

	return ;
}
INIT_APP_EXPORT(battery_too_low);


/** 
@brief  µç³Ø´¦ÀíÏß³Ì
@param  arg 
@retval 0 :ok 1:error
*/
void battery_thread_entry(void *arg)
{
	rt_thread_delay(RT_TICK_PER_SECOND*3);
	// ¿ª»ú¾Í¼ì²âµç³ØµçÁ¿
	battery_energy_too_low();
	while(1)
	{
		rt_err_t result;
		BatteryMailDef mail;

		result = rt_mq_recv(BatMail,&mail,sizeof(mail),RT_TICK_PER_SECOND*2);
		if(result == RT_EOK)
		{
			// Ïß³Ì½øÈë¹¤×÷
			rt_thread_entry_work(rt_thread_self());
			switch(mail.Type)
			{
				case BAT_MAILTYPE_SAMPLE0:
				{
					// ²ÉÑùÒ»´Î
					break;
				}
				case BAT_MAILTYPE_20P:
				{
					battery_low_20p_process();
					break;
				}
				case BAT_MAILTYPE_LowEnergy:
				{
					// µçÁ¿¹ýµÍ´¦Àí
					battery_too_low();
					break;
				}
				default:
				{
					rt_kprintf("%s mail Type Error!!!\n",BatMail->parent.parent.name);
					break;
				}
			}
			// Ïß³Ì½øÈëÐÝÃß
			rt_thread_entry_sleep(rt_thread_self());
		}
		else
		{
			battery_energy_too_low();
		}
	}
}

int battery_thread_init(void)
{
	rt_thread_t id;

	BatMail = rt_mq_create("battery",sizeof(BatteryMailDef),2,RT_IPC_FLAG_FIFO);
	RT_ASSERT(BatMail != RT_NULL);
	
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
@brief  ¸øµç³Ø¹ÜÀíÏß³Ì·¢ËÍÓÊ¼þ
@param  mail  ÓÊ¼þ 
@retval none
*/
rt_err_t battery_send_mail(BatteryMailDef_p mail)
{	
	rt_err_t result;
	
	if(BatMail == RT_NULL)
	{
		//´´½¨ÓÊ¼þ
		BatMail = rt_mq_create("battery",sizeof(*mail),2,RT_IPC_FLAG_FIFO);
		RT_ASSERT(BatMail != RT_NULL);
	}
	result = rt_mq_send(BatMail,(void *)mail,sizeof(mail));
	if(result != RT_EOK)
	{
		rt_kprintf("%s mail send fail!!!\n",BatMail->parent.parent.name);
	}

	return result;
}





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



