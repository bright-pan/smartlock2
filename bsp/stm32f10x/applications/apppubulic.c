#include "apppubulic.h"
#include "alarm.h"
#include "local.h"
#include "untils.h"
#include "voice.h"

/*
@brief 求平均数
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
@brief  get new add key pos
@param  void
@retval spoce key pos
*/
rt_uint16_t get_new_key_pos(void)
{
	rt_uint16_t i;

	for(i = 0 ;i < KEY_NUMBERS;i++)
	{
		if(device_config.param.key[i].flag == 0)
		{
			if(device_config.param.key[i].key_type == 0)
			{
        return i;
			}
		}
	}

	return KEY_NUMBERS;
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
  
	for(i = 0 ;i < KEY_NUMBERS;i++)
	{
	  if(device_config.param.key[i].flag == 1)
	  {
	  	if(device_config.param.key[i].key_type == KEY_TYPE_FPRINT)
	  	{
	  		rt_kprintf("this i = %d\n",i);
        num++;
	  	}
	  }
	}

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
    device_config.param.key[key].flag = status;
    device_config.param.key[key].key_type = type;
    device_config.param.key[key].created_time = status?sys_cur_date():0;
   	device_config.param.key[key].is_updated = 1;
	}
	else
	{
		rt_kprintf("Key Pos Can,t > KEY_NUMBERS!!!\n");
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
  
	for(i = 0 ;i < KEY_NUMBERS;i++)
	{
	  if(device_config.param.key[i].flag == 1)
	  {
	  	if(device_config.param.key[i].key_type == KEY_TYPE_FPRINT)
	  	{
        if(i == pos)
        {
					return RT_TRUE;
        }
	  	}
	  }
	}

	rt_kprintf("Not find fprint\n\n");
	return RT_FALSE;	
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

	rt_kprintf("Current System Time: 0x%X\n",time);
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

#endif

