#include"menu.h"
#include"menu_1.h"
#include"menu_2.h"
#include"menu_3.h"
#include"unlock_ui.h"
#ifdef USEING_BUZZER_FUN 	
#include "buzzer.h"
#endif

#define SHOW_GLINT_CH							"_"

typedef void (*fun1)(void);

MenuManageDef MenuManage = 
{
	RT_NULL,
};

rt_uint8_t KeyFuncIndex = 0;
fun1 current_operation_index = RT_NULL;

fun1 cur_run_processing = RT_NULL;


//void(*current_operation_index)(void);


//�˵��б�
KbdTabStruct	KeyTab[KEY_MAX_MENU_NUM] = 
{
	//����
	{0,0,0,1,0,menu_0_processing},//��½����

	//����
	{1,2,3,4,3,menu_1_processing},//�û�����
	{2,3,1,6,3,menu_2_processing},//ϵͳ���� 
  {3,1,2,0,3,menu_3_processing},//�û�����

	//����
	{4,5,5,8,1,menu_4_processing},//�û�����
	{5,4,4,22,1,menu_5_processing},//�û��޸�
	
	{6,7,7,20,2,menu_6_processing},//ϵͳ��Ϣ
	{7,6,6,7,2,menu_7_processing},//ϵͳ����

	//�ļ�
	{8,9,13,14,13,menu_8_processing},//�������� >>ͬʱ�����˺�
	{9,10,8,15,13,menu_9_processing},//����ָ��
	{10,11,9,16,13,menu_10_processing},//�����ֻ�
	{11,12,10,17,13,menu_11_processing},//�����˳�
	{12,13,11,18,12,menu_12_processing},//�鿴��Ϣ
	{13,8,12,19,13,menu_13_processing},//�˳�

	//�弶
	{14,14,14,14,8,menu_14_processing},//¼������
	{15,15,15,15,9,menu_15_processing},//¼��ָ��
	{16,16,16,16,10,menu_16_processing},//¼��绰
	{17,17,17,4,4,menu_17_processing},//�����˳�
	{18,18,18,18,12,menu_18_processing},//�鿴��Ϣ
	{19,19,19,4,13,menu_19_processing},//�˳�

	//�ļ�
	{20,20,20,20,6,menu_20_processing},//��ʾ������Ϣ
	{21,21,21,21,7,menu_21_processing},//��ʾ������Ϣ

	{22,22,22,23,5,menu_22_processing},//�û���������

	//�弶
	{23,24,27,28,26,menu_23_processing},//�޸�����
	{24,25,23,29,26,menu_24_processing},//�޸�ָ��
	{25,26,24,30,26,menu_25_processing},//�޸ĵ绰
	{26,27,25,5,26,menu_26_processing},//�����˳�
	{27,23,26,31,27,menu_27_processing},//�˳�
	
	{28,23,24,25,23,menu_28_processing},//�޸����봦��
	{29,23,24,25,24,menu_29_processing},//�޸�ָ�ƴ���
	{30,23,24,25,25,menu_30_processing},//�޸ĵ绰����
	{31,31,31,5,26,menu_31_processing},//ɾ���û�����

	//�����˵�
	//{32,4,5,33,1,menu_32_processing},//����Ա�޸�
	//{33,33,33,33,32,menu_33_processing},//����Ա�����޸�
};

//ϵͳ����˵�
#define SYSTEM_ENTER_MENU_NUM				10
rt_uint8_t	SystemFuncIndex = 0;
fun1 System_menu_index = RT_NULL;

KbdTabStruct	SystemMenu[SYSTEM_ENTER_MENU_NUM] = 
{
  {0,1,1,2,0,system_menu1_show},//��ʾ����
  {1,0,0,3,1,system_menu2_show},//ϵͳ����
  {2,2,2,2,0,unlock_process_ui},//����
  {3,3,3,3,1,system_manage_processing},//����ϵͳ����
};

//ϵͳ������洦��
void system_entry_ui_processing(void)
{
  rt_err_t result;
	rt_uint8_t KeyValue;

	result = gui_key_input(&KeyValue);
	if (result == RT_EOK) 
	{ 
	  switch(KeyValue)
	  {
	    case MENU_SURE_VALUE:
	    {
	      //ȷ��
	      if(System_menu_index == RT_NULL)
	      {
	        break;
	      }
	      SystemFuncIndex = SystemMenu[ SystemFuncIndex].SureState;
	      break;
	    }
	    case MENU_DEL_VALUE:
	    {
	      //ȡ��
	      SystemFuncIndex = SystemMenu[ SystemFuncIndex].BackState;
	      break;
	    }
	    case MENU_UP_VALUE:
	    {
	      //��
	      SystemFuncIndex = SystemMenu[ SystemFuncIndex].UpState;
	      break;
	    }
	    case MENU_DOWN_VALUE:
	    {
	      //��
	      SystemFuncIndex = SystemMenu[ SystemFuncIndex].DnState;
	      break;
	    }
	    default:
	    {
	    	SystemFuncIndex = SystemMenu[ SystemFuncIndex].SureState;
	      break;
	    }
	  }
	  System_menu_index = SystemMenu[SystemFuncIndex].CurrentOperate;
	  System_menu_index();
	}
	else
	{
		//������ʱ
		if(menu_event_process(2,MENU_EVT_OP_OUTTIME) == 0)
		{
      system_menu_choose(0);
		}
	}
}

//ϵͳ����˵�����
void system_manage_ui_processing(void)
{
	rt_err_t result;
	rt_uint8_t KeyValue;
	
	result = gui_key_input(&KeyValue);
	if (result == RT_EOK) 
	{	
		rt_kprintf("recv key value :%c\n",KeyValue);
		/*if(KeyValue > '0' && KeyValue < '8')
		{
			rt_uint8_t MenuPos;
			rt_uint8_t i;
			
			MenuPos -= '0';
			MenuPos -= 1;
			
			rt_kprintf("��ݼ�%d\n",MenuPos);
			for(i = 0;i<MenuPos%4;i++)
			{
        SystemFuncIndex = SystemMenu[ SystemFuncIndex].DnState;
			}
			SystemFuncIndex = SystemMenu[ SystemFuncIndex].SureState;
			System_menu_index = SystemMenu[SystemFuncIndex].CurrentOperate;
	 		System_menu_index();
		}*/
		switch(KeyValue)
		{
			case MENU_SURE_VALUE:
			{
				//ȷ��
				if(current_operation_index == RT_NULL)
				{
					break;
				}
				KeyFuncIndex = KeyTab[ KeyFuncIndex].SureState;
				break;
			}
			case MENU_DEL_VALUE:
			{
				//ȡ��
				KeyFuncIndex = KeyTab[ KeyFuncIndex].BackState;
				break;
			}
			case MENU_UP_VALUE:
			{
				//��
				KeyFuncIndex = KeyTab[ KeyFuncIndex].UpState;
				break;
			}
			case MENU_DOWN_VALUE:
			{
				//��
				KeyFuncIndex = KeyTab[ KeyFuncIndex].DnState;
				break;
			}
			default:
			{
				break;
			}
		}
		current_operation_index = KeyTab[KeyFuncIndex].CurrentOperate;
		current_operation_index();
	}
	else
	{
    //������ʱ
	  if(menu_event_process(2,MENU_EVT_OP_OUTTIME) == 0)
	  {
	    system_menu_choose(0);
	  }
	}
}

void system_menu_choose(rt_uint8_t menu)
{
	switch(menu)
	{
		case 0:
		{
			SystemFuncIndex = 0;
			KeyFuncIndex = 0;
			System_menu_index = RT_NULL;
			cur_run_processing = system_entry_ui_processing;
			break;
		}
		case 1:
		{
			KeyFuncIndex = 0;
			SystemFuncIndex = 0;
			current_operation_index = RT_NULL;
			cur_run_processing = system_manage_ui_processing;

			//KeyFuncIndex = KeyTab[ KeyFuncIndex].SureState;
			current_operation_index = KeyTab[KeyFuncIndex].CurrentOperate;
			current_operation_index();
			break;
		}
		default:
		{
			cur_run_processing = system_entry_ui_processing;
			break;
		}
	}
}

void key_input_processing_init(void)
{
	if(MenuManage.event == RT_NULL)
	{
		MenuManage.event = rt_event_create("menu",RT_IPC_FLAG_FIFO);
		RT_ASSERT(MenuManage.event != RT_NULL);
	}
  system_menu_choose(0);
}

void key_input_processing(void)
{
	cur_run_processing();
}

//�˵�����ȷ����ť������
void menu_run_sure_process(void)
{
	KeyFuncIndex = KeyTab[ KeyFuncIndex].SureState;
	current_operation_index = KeyTab[KeyFuncIndex].CurrentOperate;
	current_operation_index();
}


//���ַ���������ַ�
rt_err_t string_add_char(rt_uint8_t str[],rt_uint8_t ch,rt_uint8_t str_size)
{
	rt_uint8_t i;

	RT_ASSERT(str_size > 1);
	
	for(i=0;i<str_size-1;i++)
	{
		if(str[i] == 0)
		{
			str[i] = ch;
			str[i+1] = 0;
			    
			return RT_EOK;
		}
	}

	return RT_ERROR;
}

//���ַ������ɾ��һ���ַ�
rt_err_t string_del_char(rt_uint8_t str[],rt_uint8_t str_size)
{
	rt_uint8_t i;

	RT_ASSERT(str_size > 1);
	
	for(i=0;i<str_size;i++)
	{
		if(i == 0)
		{
			if(str[i] == 0)
			{
				return RT_ERROR;
			}
		}
		else if(str[i] == 0)
		{
			str[i-1] = 0;

			return RT_EOK;
		}
	}

	return RT_ERROR;
}

//�ַ���ת��Ϊ�����ַ���
void string_hide_string(const rt_uint8_t src[],rt_uint8_t str[],rt_uint8_t ch,rt_uint8_t size)
{
  rt_uint8_t i;
  
	RT_ASSERT(size > 1);

  rt_memset(str,0,size);

  rt_kprintf("input :%s\n",src);  

	for(i=0;i<rt_strlen((const char*)src);i++)
	{
		if(src[i] != 0)
		{
			str[i] = ch;
		}
	}
	//rt_kprintf("hide  :%s\n",str);   
}

void menu_inputchar_glint(rt_uint8_t x,rt_uint8_t y,rt_uint8_t status)
{
	if(status == 1)
	{
		gui_display_string(x,y,SHOW_GLINT_CH,GUI_WIHIT);
	}
	else
	{
    gui_clear(x,y,x+8,y+16);
	}
}

void menu_error_handle(rt_uint8_t type)
{
	switch(type)
	{
		case 1:
		{
			#ifdef USEING_BUZZER_FUN
			buzzer_send_mail(BZ_TYPE_ERROR1);
			#endif
			break;
		}
		case 2:
		{
			break;
		}
		case 3:
		{
			//�������
			#ifdef USEING_BUZZER_FUN
			buzzer_send_mail(BZ_TYPE_ERROR3);
			gprs_key_error_mail(KEY_TYPE_KBOARD);
			#endif
			break;
		}
		default:
		{
			break;
		}
	}
}

/*
����:�����˵����¼�
����:mode ģʽ  type �¼�����
����: -------------------------
		 |ģʽ |�ɹ�|ʧ��|����    |
		 |0    |0   |1   |�����¼�|
		 |1    |0   |1   |�յ��¼�|
		 |2    |0   |1   |����¼�|
		 --------------------------
*/
rt_uint8_t menu_event_process(rt_uint8_t mode,rt_uint32_t type)
{
	rt_uint32_t value;
	rt_err_t    result;
	rt_uint8_t  return_data = 1;
	
	//net_evt_mutex_op(RT_TRUE);

	if(MenuManage.event == RT_NULL)
	{
    MenuManage.event  = rt_event_create("menu",RT_IPC_FLAG_FIFO);
    RT_ASSERT(MenuManage.event  != RT_NULL);
	}
	switch(mode)
	{
		case 0:	//set event 
		{
			result = rt_event_send(MenuManage.event ,type);
			if(result == RT_EOK)
			{
				return_data = 0;
			}
			break;
		}
		case 1:	//get event 
		{
			result = rt_event_recv(MenuManage.event ,
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
			result = rt_event_recv(MenuManage.event,
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
      result = rt_event_recv(MenuManage.event,
                             0xffffffff,
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

	//net_evt_mutex_op(RT_FALSE);
	return return_data;
}


