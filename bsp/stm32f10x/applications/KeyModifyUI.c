#include "KeyModifyUI.h"
#include "fprint.h"
#include "local.h"
#define ADMIN_DATA_POS						0
#include "menu_2.h"



static const rt_uint8_t FprintModifyText[][8*2] = 
{
	{"1.����ָ��"},
	{"2.�޸�ָ��"},
	{"3.ɾ��ָ��"},
	{"��ǰ�û����:"},
	{"�밴*�ɼ���ָ��"},
	{"���ڲɼ�. . ."},
	{"ע��ɹ�^_^"},
	{"ע��ʧ��@@@"},
	{"������ԭָ��"},
	{"��ȷ������ʼ"},
	{"��������ָ��"},
	{"�޸ĳɹ�"},
	{"�޸�ʧ��"},
};

//ָ���޸Ľ���
// 0  �½�
// 1  �޸�
// 2  ɾ��
// 0xff ����
static rt_uint8_t fprint_modify_fun_choose_ui(void)
{
	rt_uint8_t i;

	gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
	for(i = 0;i < 3;i++)
	{
		gui_display_string(SHOW_X_CENTERED(FprintModifyText[i]),SHOW_Y_LINE(i+1),FprintModifyText[i],GUI_WIHIT);
	}
	gui_display_update();
	while(1)
	{
		rt_err_t    InResult;
		rt_uint8_t  KeyValue;

		InResult = gui_key_input(&KeyValue);
		if(InResult == RT_EOK)
		{
			if(KeyValue == '1')
			{
				return 0;
			}
			else if(KeyValue == '2')
			{
				return 1;
			}
			else if(KeyValue == '3')
			{
				return 2;
			}
			else if(KeyValue == '#')
			{
				return 0xff;
			}
		}
		else
		{
			//������ʱ
    	if(menu_event_process(2.,MENU_EVT_OP_OUTTIME) == 0)
    	{
				return 0xff;
    	}
		}
	}
}

static rt_mq_t 				FpRight_mq = RT_NULL;
static rt_event_t 		FpRight_evnt = RT_NULL;

#define FP_EVNT_NORMAL_MODE					(1<<0)
#define FP_EVNT_REGISTER_MODE				(1<<1)
#define FP_EVNT_ALL									(1<<2)

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
	rt_kprintf("��ָ������");
	if(FpRight_mq == RT_NULL)
	{
		FpRight_mq = rt_mq_create("fpright",sizeof(FPINTF_USER),1,RT_IPC_FLAG_FIFO);
		RT_ASSERT(FpRight_mq != RT_NULL);
	}

	if(fp_event_process(1,FP_EVNT_NORMAL_MODE) == RT_EOK)
	{
		rt_uint16_t *keypos = RT_NULL;
		
		#ifdef _LOCAL_H_
		union alarm_data KeyData;

		KeyData.lock.key_id = key.KeyPos;
	  KeyData.lock.operation = LOCK_OPERATION_OPEN;
	  rt_kprintf("unlock id %d\n",key.KeyPos);
	  send_local_mail(ALARM_TYPE_LOCK_PROCESS,(time_t)menu_get_cur_date,&KeyData);
	  #endif

		#ifdef __GPRS_H__
    //GPRS�ʼ�
    keypos = rt_calloc(1,sizeof(*keypos));
    *keypos = key.KeyPos;
    send_gprs_mail(ALARM_TYPE_CODE_KEY_RIGHT,menu_get_cur_date(),(void *)keypos);
		#endif

		key_error_alarm_manage(1);
		rt_mq_send(FpRight_mq,user,sizeof(FPINTF_USER));
	}
  
  return RT_EOK;
}

rt_err_t fprint_input_error_trigger(void *user)
{	
  union alarm_data data;		

  data.key.ID = KEY_TYPE_INVALID;
	data.key.Type = KEY_TYPE_FPRINT;
	//�����ۼ�
	if(key_error_alarm_manage(0) == RT_TRUE)
	{
		data.key.sms = 1;
	}
	else
	{
		data.key.sms = 0;
	}
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

//�½�ָ��
static void fprint_new_create_ui(void)
{
	rt_int32_t CurUserPos;
	rt_uint8_t *ShowBuf;
	
	ShowBuf = rt_calloc(1,MENU_PASSWORD_MAX_LEN);
	RT_ASSERT(ShowBuf != RT_NULL);

	while(1)
	{
    gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
    gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(0),FprintModifyText[3],GUI_WIHIT);
    
    CurUserPos = account_cur_pos_get();
    rt_sprintf((char *)ShowBuf,"%03d",CurUserPos);
    gui_display_string(SHOW_X_ROW8(12),SHOW_Y_LINE(0),ShowBuf,GUI_WIHIT);
    
    gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(1),FprintModifyText[4],GUI_WIHIT);
    gui_display_update();
    while(1)
    {
      rt_err_t   result;
      rt_uint8_t KeyValue;
      rt_uint8_t CrateResult = 0;
      
      result = gui_key_input(&KeyValue);
      if(result == RT_EOK)
      {
        if(KeyValue == '*')
        {
          //�ɼ�ָ��
          gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(2),SHOW_X_ROW8(15),SHOW_Y_LINE(3));
          gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(2),FprintModifyText[5],GUI_WIHIT);
          gui_display_update();
          if(CurUserPos == ADMIN_DATA_POS)
          {
            result = admin_modify_fprint(30*RT_TICK_PER_SECOND);
          }
          else
          {
            result = user_add_fprint(30*RT_TICK_PER_SECOND);
          }
          if(result == RT_EOK)
          {
            //ָ�Ʋɼ��ɹ�
            rt_kprintf("�ɼ��ɹ�\n");
            CrateResult = 6;
          }
          else
          {
            //ָ�Ʋɼ�ʧ��
            rt_kprintf("�ɼ�ʧ��\n");
            CrateResult = 7;  
          }
          gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(2),SHOW_X_ROW8(15),SHOW_Y_LINE(3));
          gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(2),FprintModifyText[CrateResult],GUI_WIHIT);
          gui_display_update();
          rt_thread_delay(RT_TICK_PER_SECOND*2);
          break;
        }
        else if(KeyValue == '#')
        {
          //gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),MenuCommText[0],GUI_WIHIT);
          gui_display_update();
          
          rt_free(ShowBuf);
          return ;
        }
      }
      else
      {
    		//������ʱ
	    	if(menu_event_process(2.,MENU_EVT_OP_OUTTIME) == 0)
	    	{
					return ;
	    	}
      }
    }

	}
	
	
}

static void fprint_modify_ui(void)
{
	rt_int32_t CurUserPos;
	rt_uint8_t *ShowBuf;
	rt_err_t   result;
	
	ShowBuf = rt_calloc(1,MENU_PASSWORD_MAX_LEN);
	RT_ASSERT(ShowBuf != RT_NULL);
	
	gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
	gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(0),FprintModifyText[3],GUI_WIHIT);

	CurUserPos = account_cur_pos_get();
	rt_sprintf((char *)ShowBuf,"%03d",CurUserPos);
	gui_display_string(SHOW_X_ROW8(12),SHOW_Y_LINE(0),ShowBuf,GUI_WIHIT);

	gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(1),FprintModifyText[8],GUI_WIHIT);
	gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(2),FprintModifyText[9],GUI_WIHIT);
	gui_display_update();

  if(FpRight_mq == RT_NULL)
  {
    FpRight_mq = rt_mq_create("fpright",sizeof(FPINTF_USER),1,RT_IPC_FLAG_FIFO);
    RT_ASSERT(FpRight_mq != RT_NULL);
  }
	while(1)
	{
		FPINTF_USER FpKeyDat;
		
    //����ָ��
    result = rt_mq_recv(FpRight_mq,&FpKeyDat,sizeof(FPINTF_USER),RT_WAITING_NO);
    
		result = menu_input_sure_key(0);
		if(result == RT_ERROR)
		{			
      rt_free(ShowBuf);
			return ;
		}
		gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(2),FprintModifyText[5],GUI_WIHIT);
		gui_display_update();
		//����ԭʼָ��
		result = rt_mq_recv(FpRight_mq,&FpKeyDat,sizeof(FPINTF_USER),RT_TICK_PER_SECOND*30);
		if(result == RT_ERROR)
		{
			rt_kprintf("ԭʼָ���������\n");
			return ;
		}

		gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(1),FprintModifyText[10],GUI_WIHIT);
		gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(2),FprintModifyText[9],GUI_WIHIT);
		gui_display_update();
		result = menu_input_sure_key(0);
		
		if(result == RT_ERROR)
		{	
			rt_free(ShowBuf);
			return ;
		}
		gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(2),FprintModifyText[5],GUI_WIHIT);
		gui_display_update();
		//����ָ��
		rt_kprintf("modify pos is %d\n",FpKeyDat.KeyPos);
		result = user_modify_fprint(FpKeyDat.KeyPos,RT_TICK_PER_SECOND*30);
		if(result == RT_EOK)
		{
			rt_kprintf("�޸ĳɹ�\n");

			gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(2),FprintModifyText[11],GUI_WIHIT);
			gui_display_update();
			rt_free(ShowBuf);
			return;
		}
		rt_kprintf("�޸�ʧ��\n");
		
		gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(2),FprintModifyText[12],GUI_WIHIT);
		gui_display_update();
    rt_free(ShowBuf);
		return ;
	}
}

static void fprint_delete_ui(void)
{
	
}

void menu_fprint_modify_ui(void)
{
	rt_uint8_t *ShowBuf;
	rt_uint8_t FunChoose;
	
	ShowBuf = rt_calloc(1,MENU_PASSWORD_MAX_LEN);
	RT_ASSERT(ShowBuf != RT_NULL);

	FunChoose = fprint_modify_fun_choose_ui();
	if(FunChoose == 0xff)
	{
		return ;
	}
	else if(FunChoose == 0)
	{
    fprint_new_create_ui();
	}
	else if(FunChoose == 1)
	{
    fprint_modify_ui();
	}
	else if(FunChoose == 2)
	{
		fprint_delete_ui();
	}
  rt_free(ShowBuf);
}

