/*********************************************************************
 * Filename:      FprintHandle.c
 *
 * Description:
 *
 * Author:        wangzw <wangzw@yuettak.com>
 * Created at:    2014-10-22
 *
 * Modify:
 * 2014-10-22     ���ָ��ʶ����ȷ�ʹ���Ļص�������
 * 
 							
 * Copyright (C) 2014 Yuettak Co.,Ltd
 ********************************************************************/

#include "FprintHandle.h"
#include "menu.h"

static rt_event_t 		FpRight_evnt = RT_NULL;


/*
����:ָ���¼�������
����:mode ģʽ  type �¼�����
����: -------------------------
		 |ģʽ |�ɹ�|ʧ��|���� |
		 |0    |RT_EOK|RT_ERROR|�����¼�|
		 |1    |RT_EOK|RT_ERROR|�յ��¼�|
		 |2    |RT_EOK|RT_ERROR|����¼�|
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


//ָ������ص�����
rt_err_t fprint_input_ok_trigger(void *user)
{
	FPINTF_USER  key;

	key = *(FPINTF_USER*)user;

	//ע��ģʽ
	if(fp_event_process(1,FP_EVNT_REGISTER_MODE) == RT_EOK)
	{
		rt_kprintf("fprint in register mode\n");
	}
	else if(fp_event_process(1,FP_EVNT_NORMAL_MODE) == RT_EOK)
	{
		//����ָ�Ʋɼ�ģʽ
		union alarm_data KeyData;

		// ��lcd
		gui_open_lcd_show();
		
		KeyData.key.ID = key.KeyPos;
	  KeyData.key.Type = KEY_TYPE_FPRINT;

		if(key_permission_check(KeyData.key.ID) != RT_EOK)
		{
			//�����ۼ�
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

		//���ͽ����¼���UI
		menu_event_process(0,MENU_EVT_FP_UNLOCK);
		
    //��������������
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
	// ��lcd
	gui_open_lcd_show();
	
  data.key.ID = KEY_TYPE_INVALID;
	data.key.Type = KEY_TYPE_FPRINT;
	//�����ۼ�
	if(key_error_alarm_manage(KEY_ERRNUM_MODE_ADDUP,&data.key.sms) == RT_TRUE)
	{
		menu_operation_result_handle(3);
	}

	// ����ָ�ƴ�����
	menu_operation_result_handle(1);

	// ����ָ�ƴ����¼���UI
	menu_event_process(0,MENU_EVT_FP_ERROR);
	
  send_local_mail(ALARM_TYPE_KEY_ERROR,0,&data);	
  
  return RT_EOK;
}
//ָ�ƻص�������ʼ��
int fprint_call_back_init(void)
{
	fp_ok_callback(fprint_input_ok_trigger);
	fp_error_callback(fprint_input_error_trigger);
	fp_event_process(0,FP_EVNT_NORMAL_MODE);
  
  return 0;
}
INIT_APP_EXPORT(fprint_call_back_init);

