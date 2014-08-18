/**
  ******************************************************************************
  * @file    apppublic.c
  * @author  wangzw <wangzw@yuettak.com>
  * @version v0.1
  * @date    2014-4-21
  * @brief   This document provides some common functions.This All the function 
  *					 of city opening to the outside world
  ******************************************************************************
  * @attention
  *
	*
  ******************************************************************************
  */
#include "apppubulic.h"
#include "alarm.h"
#include "local.h"
#include "untils.h"
#include "voice.h"

#define SHOW_APP_DEBUG_INFO  	1

/** 
@brief  get new add key pos
@param  void
@retval spoce key pos
*/
rt_uint16_t get_average_value(rt_uint16_t dat[],rt_uint8_t num)
{
	rt_uint8_t  i = 0,j = 0;
	rt_uint16_t temp;
	rt_uint32_t	result = 0;

	for(j = 0; j < num-1; j++)
	for(i = 0; i < num-1; i++)
	{
		if(dat[i] > dat[i+1])
		{
			temp = dat[i];
			dat[i] = dat[i+1];
			dat[i + 1] = temp;
		}
	}
	for(i = 2;i< num-2 ;i++)
	{
		result += dat[i];
	}
	result /= (num-4);

	return result;
}

/** 
@brief  operation device_config mutex
@param  way: RT_TRUE take;  RT_FALSE release;
@retval void
*/
void config_file_mutex_op(rt_bool_t way)
{
	RT_ASSERT(device_config.mutex != RT_NULL);
	if(way == RT_TRUE)
	{
    rt_mutex_take(device_config.mutex,RT_WAITING_FOREVER);
	}
	else if(way == RT_FALSE)
	{
		rt_mutex_release(device_config.mutex);
	}
}

/** 
@brief  get new add key pos
@param  void
@retval spoce key pos
*/
rt_uint16_t get_new_key_pos(void)
{
	rt_uint16_t i;

	config_file_mutex_op(RT_TRUE);
	for(i = 0 ;i < KEY_NUMBERS;i++)
	{
		if(device_config.param.key[i].flag == 0)
		{
				config_file_mutex_op(RT_FALSE);
        return i;
		}
	}
  config_file_mutex_op(RT_FALSE);

	return KEY_NUMBERS;
}

/** 
@brief  get this positional key type
@param  pos: positional
@retval KEY_TYPE_FPRINT
@retval KEY_TYPE_RFID
@retval KEY_TYPE_KBOARD
@retval KEY_TYPE_ERROR
*/
KEY_TYPE get_key_type(rt_uint16_t pos)
{
	KEY_TYPE type;
	
	if(pos < KEY_NUMBERS)
	{
		config_file_mutex_op(RT_TRUE);
		
   	type = device_config.param.key[pos].key_type;

		config_file_mutex_op(RT_FALSE);
		
		return type;
	}
	
	RT_DEBUG_LOG(SHOW_APP_DEBUG_INFO,("Key Position serious error!!!\n"));
	return KEY_TYPE_INVALID;
}

/** 
@brief  get fprint need update position
@param  void
@retval KEY_NUMBERS :none need update key 
@retval 0 ~ < KEY_NUMBERS:need update key pos
*/
rt_uint16_t get_key_update_pos(void)
{
	rt_uint16_t i;

	config_file_mutex_op(RT_TRUE);
	for(i = 0 ;i < KEY_NUMBERS;i++)
	{
		if(device_config.param.key[i].flag == 1)
		{			
			if(device_config.param.key[i].is_updated == 1)
			{
				RT_DEBUG_LOG(SHOW_APP_DEBUG_INFO,("update pos : %d\n",i));
				config_file_mutex_op(RT_FALSE);
				
				return i;
			}
		}
	}
  config_file_mutex_op(RT_FALSE);

	return KEY_NUMBERS;
}

/** 
@brief  set key update flag 
@param  pos: key of position
@param  new_status: new status
@retval void
*/
void set_key_update_flag(rt_uint16_t pos,rt_uint8_t new_status)
{
  if(device_config.param.key[pos].is_updated == 1)
  {
    config_file_mutex_op(RT_TRUE);
    
		device_config.param.key[pos].is_updated = new_status;

		config_file_mutex_op(RT_FALSE);
		
		device_config_file_operate(&device_config,1);
  }
}

/** 
@brief  get fprint key number 
@param  void
@retval all fprint key number
*/
rt_uint16_t get_fprint_key_num(void)
{
  rt_uint16_t i;
  rt_uint16_t num = 0;

  config_file_mutex_op(RT_TRUE);
	for(i = 0 ;i < KEY_NUMBERS;i++)
	{
	  if(device_config.param.key[i].flag == 1)
	  {
	  	if(device_config.param.key[i].key_type == KEY_TYPE_FPRINT)
	  	{
	  		RT_DEBUG_LOG(SHOW_APP_DEBUG_INFO,("Have Key: %d\n",i));
        num++;
	  	}
	  }
	}
  config_file_mutex_op(RT_FALSE);

	return num;
}

/** 
@brief  change key use status
@param  key :key position
@param	status :this position key new status
@retval void 
*/
void set_key_using_status(rt_uint16_t key,KEY_TYPE type,rt_uint8_t status)
{
	if(key < KEY_NUMBERS)
	{
		config_file_mutex_op(RT_TRUE);
		
    device_config.param.key[key].flag = status;
    device_config.param.key[key].key_type = type;
    device_config.param.key[key].created_time = status?sys_cur_date():0;
   	device_config.param.key[key].is_updated = 1;
   	
		config_file_mutex_op(RT_FALSE);
	}
	else
	{
		RT_DEBUG_LOG(SHOW_APP_DEBUG_INFO,("Key Pos Can,t > KEY_NUMBERS!!!\n"));
	}
	device_config_file_operate(&device_config,1);
}

/** 
@brief  Check to see if the fingerprint has been registered
@param  pos :key position
@retval RT_TRUE :  have 
@retval RT_FALSE : none
*/
rt_bool_t check_fprint_pos_inof(rt_uint16_t pos)
{
  rt_uint16_t i;

  config_file_mutex_op(RT_TRUE);
	for(i = 0 ;i < KEY_NUMBERS;i++)
	{
	  if(device_config.param.key[i].flag == 1)
	  {
	  	if(device_config.param.key[i].key_type == KEY_TYPE_FPRINT)
	  	{
        if(i == pos)
        {
        	config_file_mutex_op(RT_FALSE);
        	
					return RT_TRUE;
        }
	  	}
	  }
	}
	config_file_mutex_op(RT_FALSE);
	RT_DEBUG_LOG(SHOW_APP_DEBUG_INFO,("Key lib none find fprint\n\n"));
	
	return RT_FALSE;	
}


/** 
@brief  Testing the key at this time of the open access
@param  pos :key position
@retval RT_TRUE :  have 
@retval RT_FALSE : none
*/
rt_bool_t check_open_access(rt_uint16_t pos)
{
	rt_bool_t 	result = RT_FALSE;
	rt_uint16_t start_h;
	rt_uint16_t start_m;
	rt_uint16_t end_h;
	rt_uint16_t end_m;
	struct tm   *timep;
	rt_uint32_t time;
	
	if(pos >= KEY_NUMBERS)
	{
		return RT_FALSE;
	}
	//获取时间数据
	start_h = (device_config.param.key[pos].start_time & 0xffff0000)>>16;
	start_m = (device_config.param.key[pos].start_time & 0x0000ffff);

	end_h = (device_config.param.key[pos].end_time & 0xffff0000)>>16;
	end_m = (device_config.param.key[pos].end_time & 0x0000ffff);

	time = sys_cur_date();
	timep = localtime((const time_t *)&time);

	RT_DEBUG_LOG(SHOW_APP_DEBUG_INFO
								,("start_h:%d start_m:%d end_h:%d end_m:%d\n"
								,start_h
								,start_m
								,end_h
								,end_m));
								
	RT_DEBUG_LOG(SHOW_APP_DEBUG_INFO,("%d:%d:%d\n"
								,timep->tm_hour
								,timep->tm_min
								,timep->tm_wday));
	

  config_file_mutex_op(RT_TRUE);

	switch(device_config.param.key[pos].operation_type)
	{
		case OPERATION_TYPE_FOREVER:
		{
			result = RT_TRUE;
			
			break;
		}
		case OPERATION_TYPE_EVERYDAY:
		{
			if((timep->tm_hour >= start_h) && (timep->tm_hour <= end_h))
			{
				if(timep->tm_hour == start_h)
				{
					if(timep->tm_min >= start_m)
					{
						result =  RT_TRUE;
					}
					else
					{
						result = RT_FALSE;
					}
				}
				else if(timep->tm_hour == end_h)
				{
          if(timep->tm_min <= end_m)
	        {
	          result =  RT_TRUE;
	        }
	        else
	        {
	          result = RT_FALSE;
	        }
				}
				else
				{
					result = RT_TRUE;
				}
			}
			else
			{
				result = RT_FALSE;
			}
			
			break;
		}
		case OPERATION_TYPE_WORKDAY:
		{
			if((timep->tm_wday > 0) && (timep->tm_wday < 6))
			{
				if((timep->tm_hour >= start_h) && (timep->tm_hour <= end_h))
				{
					if(timep->tm_hour == start_h)
					{
						if(timep->tm_min >= start_m)
						{
							result =  RT_TRUE;
						}
						else
						{
							result = RT_FALSE;
						}
					}
					else if(timep->tm_hour == end_h)
					{
	          if(timep->tm_min <= end_m)
		        {
		          result =  RT_TRUE;
		        }
		        else
		        {
		          result = RT_FALSE;
		        }
					}
					else
					{
						result = RT_TRUE;
					}
				}
				else
				{
					result = RT_FALSE;
				}
			}
			else
			{
				result = RT_FALSE;
			}
			
			break;
		}
		case OPERATION_TYPE_ONCE:
		{
			if((time >= device_config.param.key[pos].start_time) 
					&& (time <= device_config.param.key[pos].end_time))
			{
				result = RT_TRUE;
			}
			else
			{
				result = RT_FALSE;
			}
			
			break;
		}
		default:
		{
			break;
		}
	}

	config_file_mutex_op(RT_FALSE);
	
	return result;
}

/** 
@brief  operation system event mutex
@param  way: RT_TRUE take;  RT_FALSE release;
@retval void
*/
static void system_evt_mutex_op(rt_bool_t way)
{
	static rt_mutex_t system_evt = RT_NULL;
	
	if(system_evt == RT_NULL)
	{
		system_evt = rt_mutex_create("sysevt",RT_IPC_FLAG_FIFO);
	}
	if(way == RT_TRUE)
	{
    rt_mutex_take(system_evt,RT_WAITING_FOREVER);
	}
	else if(way == RT_FALSE)
	{
		rt_mutex_release(system_evt);
	}
}

/*
功能:操作网络协议中的各种事件
参数:mode 模式  type 事件类型
返回: -------------------------
		 |模式 |成功|失败|功能    |
		 |0    |0   |1   |发送事件|
		 |1    |0   |1   |收到事件|
		 |2    |0   |1   |清除事件|
		 --------------------------
*/
static rt_event_t system_event = RT_NULL;
rt_uint8_t system_event_process(rt_uint8_t mode,rt_uint32_t type)
{
	rt_uint32_t value;
	rt_err_t    result;
	rt_uint8_t  return_data = 1;

  system_evt_mutex_op(RT_TRUE);

	if(system_event == RT_NULL)
	{
		system_event = rt_event_create("system",RT_IPC_FLAG_FIFO);
		RT_ASSERT(system_event != RT_NULL);
	}
	switch(mode)
	{
		case 0:	//set event 
		{
			result = rt_event_send(system_event,type);
			if(result == RT_EOK)
			{
				return_data = 0;
			}
			break;
		}
		case 1:	//get event 
		{
			result = rt_event_recv(system_event,
			                       type,
			                       RT_EVENT_FLAG_OR,
			                       RT_WAITING_NO,&value);
			if(result == RT_EOK)
			{
				return_data = 0;
			}
			else if(result == -RT_ETIMEOUT)
			{
				return_data = 1;
			}
			break;
		}
		case 2://clean event
		{
			result = rt_event_recv(system_event,
			                       type,
			                       RT_EVENT_FLAG_OR | 
			                       RT_EVENT_FLAG_CLEAR,
			                       RT_WAITING_NO,&value);
			if(result == RT_EOK)
			{
				return_data = 0;
			}
			break;
		}
    case 3://clean all event 
    {
      result = rt_event_recv(system_event,
                             SYS_EVENT_ALL,
                             RT_EVENT_FLAG_OR | 
                             RT_EVENT_FLAG_CLEAR,
                             RT_WAITING_NO,&value);
      if(result == RT_EOK)
      {
        return_data = 0;
      }
      break;
    }
    default:
    {
			break;
    }
	}

  system_evt_mutex_op(RT_FALSE);

	return return_data;
}

/*
功能:获得当前时间
*/
rt_uint32_t sys_cur_date(void)
{
  rt_device_t device;
  rt_uint32_t time=0;

  device = rt_device_find("rtc");
  if (device != RT_NULL)
  {
      rt_device_control(device, RT_DEVICE_CTRL_RTC_GET_TIME, &time);
  }

	RT_DEBUG_LOG(SHOW_APP_DEBUG_INFO,("Current System Time: 0x%X\n",time));
  return time;
}
RTM_EXPORT(sys_cur_date);










#ifdef RT_USING_FINSH
#include <finsh.h>
#include "stm32f10x.h"
void sysreset()
{
  NVIC_SystemReset();
}
FINSH_FUNCTION_EXPORT(sysreset,sysreset() -- reset stm32);

void syskey(void)
{
	rt_kprintf("new key pos :%d\n",get_new_key_pos());
	rt_kprintf("cur key num :%d\n",get_fprint_key_num());
}
FINSH_FUNCTION_EXPORT(syskey,"show system key lib information");

void testu32(void)
{
	rt_uint32_t time;
	struct tm   *p;

	time = sys_cur_date();

	p = localtime((const time_t *)&time);
	rt_kprintf("%d :",p->tm_sec);
	rt_kprintf("%d :",p->tm_min);
	rt_kprintf("%d :",p->tm_hour);
	rt_kprintf("%d ",p->tm_mday);
	rt_kprintf("%d ",p->tm_mon);
	rt_kprintf("%d ",p->tm_year);
	rt_kprintf("%d ",p->tm_wday);
	rt_kprintf("%d ",p->tm_yday);
}
FINSH_FUNCTION_EXPORT(testu32,"test data");

#endif

