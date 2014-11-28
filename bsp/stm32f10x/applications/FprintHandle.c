/*********************************************************************
 * Filename:      FprintHandle.c
 *
 * Description:
 *
 * Author:        wangzw <wangzw@yuettak.com>
 * Created at:    2014-10-22
 *
 * Modify:
 * 2014-10-22     添加指纹识别正确和错误的回调处理函数
 * 
 							
 * Copyright (C) 2014 Yuettak Co.,Ltd
 ********************************************************************/

#include "FprintHandle.h"
#include "menu.h"

static rt_event_t 		FpRight_evnt = RT_NULL;


/*
功能:指纹事件处理函数
参数:mode 模式  type 事件类型
返回: -------------------------
		 |模式 |成功|失败|功能 |
		 |0    |RT_EOK|RT_ERROR|发送事件|
		 |1    |RT_EOK|RT_ERROR|收到事件|
		 |2    |RT_EOK|RT_ERROR|清除事件|
		 --------------------------
*/
rt_err_t fp_event_process(rt_uint8_t mode,rt_uint32_t type)
{
	rt_uint32_t value;
	rt_err_t    result;
	rt_err_t    return_data = RT_ERROR;

  //system_evt_mutex_op(RT_TRUE);

	if(FpRight_evnt == RT_NULL)
	{
		FpRight_evnt = rt_event_create("fp",RT_IPC_FLAG_FIFO);
		RT_ASSERT(FpRight_evnt != RT_NULL);
	}
	switch(mode)
	{
		case 0:	//set event 
		{
			result = rt_event_send(FpRight_evnt,type);
			if(result == RT_EOK)
			{
				return_data = RT_EOK;
			}
			break;
		}
		case 1:	//get event 
		{
			result = rt_event_recv(FpRight_evnt,
			                       type,
			                       RT_EVENT_FLAG_OR,
			                       RT_WAITING_NO,&value);
			if(result == RT_EOK)
			{
				return_data = RT_EOK;
			}
			else if(result == -RT_ETIMEOUT)
			{
				return_data = RT_ERROR;
			}
			break;
		}
		case 2://clean event
		{
			result = rt_event_recv(FpRight_evnt,
			                       type,
			                       RT_EVENT_FLAG_OR | 
			                       RT_EVENT_FLAG_CLEAR,
			                       RT_WAITING_NO,&value);
			if(result == RT_EOK)
			{
				return_data = RT_EOK;
			}
			break;
		}
    case 3://clean all event 
    {
      result = rt_event_recv(FpRight_evnt,
                             FP_EVNT_ALL,
                             RT_EVENT_FLAG_OR | 
                             RT_EVENT_FLAG_CLEAR,
                             RT_WAITING_NO,&value);
      if(result == RT_EOK)
      {
        return_data = RT_EOK;
      }
      break;
    }
    default:
    {
			break;
    }
	}

  //system_evt_mutex_op(RT_FALSE);

	return return_data;
}


//指纹输入回调函数
rt_err_t fprint_input_ok_trigger(void *user)
{
	FPINTF_USER  key;

	key = *(FPINTF_USER*)user;

	//注册模式
	if(fp_event_process(1,FP_EVNT_REGISTER_MODE) == RT_EOK)
	{
		rt_kprintf("fprint in register mode\n");
	}
	else if(fp_event_process(1,FP_EVNT_NORMAL_MODE) == RT_EOK)
	{
		//正常指纹采集模式
		union alarm_data KeyData;

		// 打开lcd
		gui_open_lcd_show();
		
		KeyData.key.ID = key.KeyPos;
	  KeyData.key.Type = KEY_TYPE_FPRINT;

		if(key_permission_check(KeyData.key.ID) != RT_EOK)
		{
			//计数累加
			if(key_error_alarm_manage(KEY_ERRNUM_MODE_ADDUP,&KeyData.key.sms) == RT_TRUE)
			{
				menu_operation_result_handle(3);
			}

			menu_event_process(0,MENU_EVT_FP_ERROR);
			if(KeyData.key.sms)
			{
        send_local_mail(ALARM_TYPE_KEY_ERROR,0,&KeyData); 
			}
		  
			return RT_EOK; 
		}
		
	  send_local_mail(ALARM_TYPE_KEY_RIGHT,(time_t)menu_get_cur_date,&KeyData);

		//发送解锁事件给UI
		menu_event_process(0,MENU_EVT_FP_UNLOCK);
		
    //清除开锁错误次数
		key_error_alarm_manage(KEY_ERRNUM_MODE_CLAER,RT_NULL);
	}
  
  return RT_EOK;
}

rt_err_t fprint_input_error_trigger(void *user)
{	
  union alarm_data data;		
	rt_err_t 			   result;
	
	result = fp_event_process(1,FP_EVNT_REGISTER_MODE);
	if(result == RT_EOK)
	{
		return RT_ERROR;
	}
	// 打开lcd
	gui_open_lcd_show();
	
  data.key.ID = KEY_TYPE_INVALID;
	data.key.Type = KEY_TYPE_FPRINT;
	//计数累加
	if(key_error_alarm_manage(KEY_ERRNUM_MODE_ADDUP,&data.key.sms) == RT_TRUE)
	{
		menu_operation_result_handle(3);
	}

	// 处理指纹错误结果
	menu_operation_result_handle(1);

	// 发送指纹处理事件给UI
	menu_event_process(0,MENU_EVT_FP_ERROR);
	
  send_local_mail(ALARM_TYPE_KEY_ERROR,0,&data);	
  
  return RT_EOK;
}
//指纹回调函数初始化
int fprint_call_back_init(void)
{
	fp_ok_callback(fprint_input_ok_trigger);
	fp_error_callback(fprint_input_error_trigger);
	fp_event_process(0,FP_EVNT_NORMAL_MODE);
  
  return 0;
}
INIT_APP_EXPORT(fprint_call_back_init);

