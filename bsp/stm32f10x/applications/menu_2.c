/*********************************************************************
 * Filename:      menu_2.c
 *
 * Description:
 *
 * Author:        wangzw <wangzw@yuettak.com>
 * Created at:    2013-04-22
 *
 * Modify:
 *
 * 2014-08-19 wangzw <wangzw@yuettak.com> 
 							���������Ŀ¼�µĲ˵�
 							
 * Copyright (C) 2013 Yuettak Co.,Ltd
 ********************************************************************/
#include "menu_2.h"
#include "KeyModifyUI.h"
//#define USEING_MODIF_UI           

#define MENU2_DEBUG_ARG						1
#define FPRIN_OP_OUTIME           5

#define PAGE_MAX_SHOW_NUM					4
#define PHONE_STAND_LEN						11
#define ADMIN_DATA_POS						0

#define AOTU_LOCK_MIN_TIME				3//�Զ�����ʱ��

#define STRING_BT_MAC							"MAC:"
#define STRING_DEVICE_ID					"ID :"

#define STRING_USER_NO						"�û����:"
#define STRING_NEW_PASSWORD 			"����������:"
#define STRING_NEW_REPASSWORD		 	"������һ��:"
#define SHOW_PW_HIDE_CH						'*'
#define SHOW_EXIT_HINT						"�����뷵�ؼ��˳�"
#define SHOW_INPUT_ERR						"�������!!!"
#define SHOW_ADD_PS_ERR						"���ʧ��!!!"

static const rt_uint8_t MenuCommText[][LCD_LINE_MAX_LEN] = 
{
  {"�����뷵�ؼ��˳�"},
  {"�����ɹ�!"},
  {"����ʧ��!"},
  {"�Ƿ�����!!!"},
};

//��������
static const rt_uint8_t PasswordAddText[][LCD_LINE_MAX_LEN]=
{
	{"�û����:"},
	{"����������:"},
	{"������һ��:"},
	{"�������!!!"},
	{"���ʧ��!!!"},
};

//ָ�������ı�
static const rt_uint8_t FrintAddText[][LCD_LINE_MAX_LEN] = 
{
	{"�û����:"},
	{"ָ������:"},
	{"��ȷ������ʼ"},
	{"���ڲɼ�..."},
};

//�ֻ����������ı�
static const rt_uint8_t PhoneAddText[][LCD_LINE_MAX_LEN] = 
{
	{"�û����:"},
	{"�������µ绰:"},
	{"�������"},
	{"������һ��"},
};

//�����˳������ı�
static const rt_uint8_t SaveEixtText[][LCD_LINE_MAX_LEN] = 
{
	{"�û����:"},
	{"����ɹ�!"},
	{"�밴ȷ�����˳�"},
};

//�˳������ı�
static const rt_uint8_t EixtText[][LCD_LINE_MAX_LEN] = 
{
  {"�˳�ע��!"},
  {"�밴ȷ�����˳�"},
};

static const rt_uint8_t UserSearchText[][LCD_LINE_MAX_LEN] =
{
	{"�����û����:"},
	{"�Ҳ����˱��!"},
	{"�Ƿ�����������"},
};


//�û�����б�
#define MENU_USER_ADD_NUM						6
static const rt_uint8_t MenuUserAdd_list[MENU_USER_ADD_NUM][8*2] = 
{
	{"1.�½�����"},
	{"2.�½�ָ��"},
	{"3.�½��绰"},
	{"4.�����˳�"},
	{"5.�û���Ϣ"},
	{"6.>>>>�˳�"},
};

#define MENU_USER_MODIFY_TYPE_NUM		5
static const rt_uint8_t MenuUserModifyList[MENU_USER_MODIFY_TYPE_NUM][8*2] = 
{
	{"1.�������"},
	{"2.���ָ��"},
	{"3.��ӵ绰"},
	{"4.�����˳�"},
	{"5.ɾ���˻�"},
};

static const rt_uint8_t MenuUserModifyList1[MENU_USER_MODIFY_TYPE_NUM][8*2] = 
{
	{"1.�޸�����"},
	{"2.�޸�ָ��"},
	{"3.�޸ĵ绰"},
	{"4.�����˳�"},
	{"5.�˳�>>>>"},
};

#define MENU_USER_INFO_TEST_LEN_NUM      3
static const rt_uint8_t MenuUserInfoText[][8*2] = 
{
	{"�û����:"},
	{"Կ������:"},
	{"�绰����:"},
};

#ifdef USEING_MODIF_UI
static const rt_uint8_t UserModifyText[][8*2] = 
{
	{"��������:"},
	{"��ǰ���:"},
};
static const rt_uint8_t AdminModifyText[][8*2] =
{
	{"����Ա�����޸�"},
	{"������:"},
};
#endif
static const rt_uint8_t PasswordModifyText[][8*2] =
{
	{"��ǰ�û����:"},
	{"��������:"},
	{"�밴ȷ�����޸�"},
	{"����������:"},
	{"���볤�ȴ���"},
	{"������������:"},
	{"�������!!!"},
	{"���벻�Ϸ�!!!"},
	{"��������һ��:"},
	{"������ƥ��ʧ��"},
	{"��Ӵ������밴*"},
	{"��*ɾ�� #����"},
	{"ɾ���ɹ�"},
};

//ϵͳ����Ŀ¼
#define MENU_SYSTEM_SET_LIST_NUM             2
static const rt_uint8_t MenuSystemSetListText[MENU_SYSTEM_SET_LIST_NUM][8*2] =
{
	{"1.�Զ�����"},
	{"2.�Զ�����"},
};

//�Զ����߽���
static const rt_uint8_t MenuAutoSleepText[][8*2]=
{
	{"����ʱ��"},
};

//�Զ�����ʱ��
static const rt_uint8_t MenuAutoLockText[][8*2] = 
{
	{"����ʱ��"},
};
static void menu_user_add_ui(rt_uint8_t InPOS)
{
	rt_uint8_t page;
	rt_uint8_t pos;
	rt_uint8_t i;

	//�����û�������
	account_add_enter();

	page = InPOS /PAGE_MAX_SHOW_NUM;//������ʾ����һҳ
	pos = InPOS % PAGE_MAX_SHOW_NUM;//��ǰѡ�е�λ��
	gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
	for(i=0;i<PAGE_MAX_SHOW_NUM;i++)
	{
		if(page*PAGE_MAX_SHOW_NUM+i >= MENU_USER_ADD_NUM)
		{
			break;
		}
		if(i == pos)
		{
			gui_display_string(SHOW_X_CENTERED((const char*)MenuUserAdd_list[page*PAGE_MAX_SHOW_NUM+i]),
												 SHOW_Y_LINE(i),
												 (rt_uint8_t *)MenuUserAdd_list[page*PAGE_MAX_SHOW_NUM+i],
												 GUI_BLACK);
		}
		else
		{
			gui_display_string(SHOW_X_CENTERED((const char*)MenuUserAdd_list[page*PAGE_MAX_SHOW_NUM+i]),
												 SHOW_Y_LINE(i),
												 (rt_uint8_t *)MenuUserAdd_list[page*PAGE_MAX_SHOW_NUM+i],
												 GUI_WIHIT);
		}
	}
	gui_display_update();
}

//ѡ������
void menu_8_processing(void)
{
  menu_user_add_ui(0);
}

//ѡ��ָ��
void menu_9_processing(void)
{
  menu_user_add_ui(1);
}


//ѡ������ֻ���
void menu_10_processing(void)
{	
  menu_user_add_ui(2);
}


//ѡ�б����˳�
void menu_11_processing(void)
{
  menu_user_add_ui(3);
}


//ѡ���˳�
void menu_12_processing(void)
{
  menu_user_add_ui(4);
}

//ѡ�в鿴��Ϣ
void menu_13_processing(void)
{
  menu_user_add_ui(5);
}

//������������Ƿ�Ϸ�
rt_err_t add_new_password_check(rt_uint8_t password[])
{
	rt_err_t result = RT_EOK;
	
	if(rt_strlen((const char*)password) < CONFIG_PASSWORD_LEN)
	{
		//��������Ϊ��
		return RT_ERROR;
	}
	result = key_add_password_check(password);
	
	return result;
}

//�������봦��
void menu_14_processing(void)
{
	rt_uint8_t buf[MENU_PASSWORD_MAX_LEN];
	rt_uint8_t buf1[MENU_PASSWORD_MAX_LEN];
	rt_uint8_t ShowBuf[MENU_PASSWORD_MAX_LEN];
	rt_uint8_t GlintStatus;
	rt_uint8_t CurUserPos;
	rt_uint8_t UserMaxPsNum;

	while(1)
	{
		CurUserPos = account_cur_pos_get();
		UserMaxPsNum = user_valid_password_num();
		
    gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
    
    gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(0),PasswordAddText[0],GUI_WIHIT);
		rt_sprintf((char *)buf,"%03d",CurUserPos);
    gui_display_string(SHOW_X_ROW8(12),SHOW_Y_LINE(0),buf,GUI_WIHIT);
    
    gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(1),PasswordAddText[1],GUI_WIHIT);
    rt_sprintf((char *)buf,"%02d",UserMaxPsNum);
    gui_display_string(SHOW_X_ROW8(13),SHOW_Y_LINE(1),buf,GUI_WIHIT);
    gui_display_update();
    //��һ����������
    rt_memset(buf,0,MENU_PASSWORD_MAX_LEN);
    rt_memset(ShowBuf,0,MENU_PASSWORD_MAX_LEN);
    while(1)
    {
      rt_err_t result;
      rt_uint8_t KeyValue;
      
      result = gui_key_input(&KeyValue);
      if(RT_EOK == result)
      {
        //�а���
        if(KeyValue >= '0' && KeyValue <= '9')
        {
          result = string_add_char(buf,KeyValue,7);
          if(result == RT_EOK)
          {
            string_hide_string((const rt_uint8_t *)buf,ShowBuf,SHOW_PW_HIDE_CH,8);
            gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(2),SHOW_X_ROW8(15),SHOW_Y_LINE(3));
            gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(2),ShowBuf,GUI_WIHIT);
          }
          else
          {
            //������������8��
          }
        }
        else if(KeyValue == MENU_SURE_VALUE)
        {
          //�������������Ƿ�Ϸ�
          result = add_new_password_check(buf);
          if(result != RT_EOK)
          {
          	//���벻�Ϸ�
          	menu_operation_result_handle(1);
    				gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PasswordAddText[3],GUI_WIHIT);
    				gui_display_update();
    				rt_thread_delay(RT_TICK_PER_SECOND);
    				gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(3),SHOW_X_ROW8(15),SHOW_Y_LINE(4));
          }
          else
          {
            //�������ǺϷ���
            rt_memcpy(buf1,buf,MENU_PASSWORD_MAX_LEN);
            break;
          }
          //������������� ������֤��
        }
        else if(KeyValue == MENU_DEL_VALUE)
        {
          result = string_del_char(buf,8);
          if(result == RT_EOK)
          {
            string_hide_string((const rt_uint8_t *)buf,ShowBuf,SHOW_PW_HIDE_CH,8);
            gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(2),SHOW_X_ROW8(15),SHOW_Y_LINE(3));
            gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(2),ShowBuf,GUI_WIHIT);
          }
         	else
         	{
         		gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),MenuCommText[0],GUI_WIHIT);
         		gui_display_update();
						return ;
         	}
        }
      }
      else
      {
      	//������ʱ
	    	if(menu_event_process(2.,MENU_EVT_OP_OUTTIME) == 0)
	    	{
					return ;
	    	}
        //��˸��ʾ
        GlintStatus++;
        menu_inputchar_glint(SHOW_X_ROW8(rt_strlen((const char *)ShowBuf)),SHOW_Y_LINE(2),GlintStatus%2);
      }
      //������ʾ
      
      gui_display_update();
    }
    
    //�ڶ�����������
    gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
    
    gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(0),PasswordAddText[0],GUI_WIHIT);
		rt_sprintf((char *)buf,"%03d",CurUserPos);
    gui_display_string(SHOW_X_ROW8(12),SHOW_Y_LINE(0),buf,GUI_WIHIT);
    
    gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(1),PasswordAddText[2],GUI_WIHIT);
    rt_sprintf((char *)buf,"%02d",UserMaxPsNum);
    gui_display_string(SHOW_X_ROW8(13),SHOW_Y_LINE(1),buf,GUI_WIHIT);
    gui_display_update();

    rt_memset(buf,0,MENU_PASSWORD_MAX_LEN);
    rt_memset(ShowBuf,0,MENU_PASSWORD_MAX_LEN);
    while(1)
    {
      rt_err_t result;
      rt_uint8_t KeyValue;
      
      result = gui_key_input(&KeyValue);
      if(RT_EOK == result)
      {
        //�а���
        if(KeyValue >= '0' && KeyValue <= '9')
        {
          result = string_add_char(buf,KeyValue,7);
          if(result == RT_EOK)
          {
            string_hide_string((const rt_uint8_t *)buf,ShowBuf,SHOW_PW_HIDE_CH,8);
            gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(2),SHOW_X_ROW8(15),SHOW_Y_LINE(3));
            gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(2),ShowBuf,GUI_WIHIT);
          }
          else
          {
            //������������8��
          }
        }
        else if(KeyValue == MENU_SURE_VALUE)
        {
          //�������������Ƿ�Ϸ�
          rt_uint32_t temp;
          
          temp = rt_memcmp(buf1,buf,rt_strlen((const char *)buf1));
          if(temp != 0)
          {
            //����ƥ�����
            menu_operation_result_handle(1);
            gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PasswordAddText[3],GUI_WIHIT);
    				gui_display_update();
    				rt_thread_delay(RT_TICK_PER_SECOND);
    				gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(3),SHOW_X_ROW8(15),SHOW_Y_LINE(4));
          }
          else
          {
          	rt_uint8_t save = 1;

						//����Ա�û�
          	if(CurUserPos == ADMIN_DATA_POS)
          	{
							result = admin_modify_password(buf1);
							if(result == RT_ERROR)
							{
								save = 2;
							}
          	}
          	else
          	{
							//��ӳɹ��������
	            result = key_add_password(buf1);
	            if(result != RT_EOK)
	            {
								//����ʧ��
								save = 2;
								rt_kprintf("Password save fail\n");
	            }
	          
	            result = account_cur_add_password(buf1);
	            if(result != RT_EOK)
	            {
								//����ʧ��
								rt_kprintf("Password save fail\n");
								save = 2;
	            }
          	}
          	menu_operation_result_handle(2);
            gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),MenuCommText[save],GUI_WIHIT);
    				gui_display_update();
    				rt_thread_delay(RT_TICK_PER_SECOND);
    				gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(3),SHOW_X_ROW8(15),SHOW_Y_LINE(4));
          }
          //������������� ������֤��
          break;
        }
        else if(KeyValue == MENU_DEL_VALUE)
        {
          result = string_del_char(buf,8);
          if(result == RT_EOK)
          {
            string_hide_string((const rt_uint8_t *)buf,ShowBuf,SHOW_PW_HIDE_CH,8);
            gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(2),SHOW_X_ROW8(15),SHOW_Y_LINE(3));
            gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(2),ShowBuf,GUI_WIHIT);
          }
          else
          {
          	rt_dprintf(MENU_DEBUG_THREAD,("Add key is exit\n"));
						//return ;
          }
        }
      }
      else
      {
      	//������ʱ
	    	if(menu_event_process(2.,MENU_EVT_OP_OUTTIME) == 0)
	    	{
					return ;
	    	}
        //��˸��ʾ
        GlintStatus++;
        menu_inputchar_glint(SHOW_X_ROW8(rt_strlen((const char *)ShowBuf)),SHOW_Y_LINE(2),GlintStatus%2);
      }
      //������ʾ
      
      gui_display_update();
    }
	}
}

//����ָ��
void menu_15_processing(void)
{
	rt_uint8_t KeyValue;
	rt_uint8_t FPrintNum;
	rt_uint8_t CurUserPos;
	rt_err_t	 result;
	rt_uint8_t buf[LCD_LINE_MAX_LEN];
	
  //gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
  //��ȡ��ǰָ������
  FPrintNum = 0;
  while(1)
  {
  	FPrintNum = user_valid_fprint_num();
  	CurUserPos = account_cur_pos_get();
  	
  	gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
    gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(0),FrintAddText[0],GUI_WIHIT);
    rt_sprintf((char *)buf,"%03d",CurUserPos);
    gui_display_string(SHOW_X_ROW8(12),SHOW_Y_LINE(0),buf,GUI_WIHIT);
    
    gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(1),FrintAddText[1],GUI_WIHIT);
    rt_sprintf((char *)buf,"%02d",FPrintNum);
    gui_display_string(SHOW_X_ROW8(13),SHOW_Y_LINE(1),buf,GUI_WIHIT);
    
    gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(2),FrintAddText[2],GUI_WIHIT);
    gui_display_update();
    fp_event_process(0,FP_EVNT_REGISTER_MODE);
		while(1)
		{
			result = gui_key_input(&KeyValue);
			if(result == RT_EOK)
			{
				if(KeyValue == MENU_SURE_VALUE)
				{
					//�ɼ�ָ��
					gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(2),SHOW_X_ROW8(15),SHOW_Y_LINE(3));
					gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(2),FrintAddText[3],GUI_WIHIT);
					gui_display_update();
					if(CurUserPos == ADMIN_DATA_POS)
					{
						result = admin_modify_fprint(FPRIN_OP_OUTIME*RT_TICK_PER_SECOND);
					}
					else
					{
            result = user_add_fprint(FPRIN_OP_OUTIME*RT_TICK_PER_SECOND);
					}
					if(result == RT_EOK)
					{
						//ָ�Ʋɼ��ɹ�
						menu_operation_result_handle(2);
						gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(2),SHOW_X_ROW8(15),SHOW_Y_LINE(3));
						gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(2),MenuCommText[1],GUI_WIHIT);
						gui_display_update();
						rt_thread_delay(RT_TICK_PER_SECOND*2);
						rt_dprintf(MENU_DEBUG_THREAD,("Fprint add ok\n"));
						break;
					}
					else
					{
						//ָ�Ʋɼ�ʧ��
						menu_operation_result_handle(1);
						gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(2),SHOW_X_ROW8(15),SHOW_Y_LINE(3));
						gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(2),MenuCommText[2],GUI_WIHIT);
						gui_display_update();
						rt_thread_delay(RT_TICK_PER_SECOND*2);
						rt_kprintf("Fprint delete fail\n");
						break;
					}
				}
				else if(KeyValue == MENU_DEL_VALUE)
				{
					gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),MenuCommText[0],GUI_WIHIT);
         	gui_display_update();
         	fp_event_process(2,FP_EVNT_REGISTER_MODE);
					return ;
				}
			}
			else
			{
        //������ʱ
        if(menu_event_process(2.,MENU_EVT_OP_OUTTIME) == 0)
        {
        	fp_event_process(2,FP_EVNT_REGISTER_MODE);
          return ;
        }
			}
		}
		rt_thread_delay(1);
  }
  fp_event_process(2,FP_EVNT_REGISTER_MODE);
  //gui_display_update();
}

rt_err_t add_new_phone_check(rt_uint8_t phone[])
{
	rt_uint8_t len;
	rt_err_t result;

	len = rt_strlen((const char *)phone);
	if(len < PHONE_STAND_LEN)
	{
		return RT_EEMPTY;
	}	

	result = user_phone_add_check(phone);
	if(result != RT_EOK)
	{
		rt_kprintf("The mobile phone number ALREADY EXISTS\n");
		
    return RT_ERROR;
	}
	
	return RT_EOK;
}
//����ֻ�����
void menu_16_processing(void)
{
	rt_uint8_t PhoneNum = 0;
	rt_uint8_t CurUserPos = 0;
	rt_uint8_t buf[LCD_LINE_MAX_LEN];
	rt_uint8_t PhoneBuf[MENU_PHONE_MAX_LEN+1];
	rt_uint8_t KeyValue;
	rt_uint8_t result;
	rt_uint8_t GlintStatus;
	
  //gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);

  while(1)
  {
  	//��һ�������ֻ�����
  	CurUserPos = account_cur_pos_get();
    PhoneNum = user_valid_phone_num();
    
  	gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
    gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(0),PhoneAddText[0],GUI_WIHIT);
    rt_sprintf((char *)buf,"%03d",CurUserPos);
    gui_display_string(SHOW_X_ROW8(12),SHOW_Y_LINE(0),buf,GUI_WIHIT);

    gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(1),PhoneAddText[1],GUI_WIHIT);
    rt_sprintf((char *)buf,"%02d",PhoneNum);
    gui_display_string(SHOW_X_ROW8(13),SHOW_Y_LINE(1),buf,GUI_WIHIT);
    gui_display_update();

    rt_memset(buf,0,MENU_PHONE_MAX_LEN);
		while(1)
		{
			result = gui_key_input(&KeyValue);
      if(RT_EOK == result)
      {
        //�а���
        if(KeyValue >= '0' && KeyValue <= '9')
        {
          result = string_add_char(buf,KeyValue,MENU_PHONE_MAX_LEN);
          if(result == RT_EOK)
          {
            gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(2),SHOW_X_ROW8(15),SHOW_Y_LINE(3));
            gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(2),buf,GUI_WIHIT);
          }
          else
          {
            //������������8��
          }
        }
        else if(KeyValue == MENU_SURE_VALUE)
        {
          //���������ֻ��Ƿ�Ϸ�
          result = add_new_phone_check(buf);
          if(result != RT_EOK)
          {
          	//���Ϸ�
          	menu_operation_result_handle(1);
    				gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PhoneAddText[2],GUI_WIHIT);
    				gui_display_update();
    				rt_thread_delay(RT_TICK_PER_SECOND);
    				gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(3),SHOW_X_ROW8(15),SHOW_Y_LINE(4));
          }
          else
          {
          	//�Ϸ�
           	rt_memcpy(PhoneBuf,buf,MENU_PHONE_MAX_LEN);
            break;
          }
          //������������� ������֤��
        }
        else if(KeyValue == MENU_DEL_VALUE)
        {
          result = string_del_char(buf,MENU_PHONE_MAX_LEN);
          if(result == RT_EOK)
          {
            gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(2),SHOW_X_ROW8(15),SHOW_Y_LINE(3));
            gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(2),buf,GUI_WIHIT);
          }
         	else
         	{
         		gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),MenuCommText[0],GUI_WIHIT);
         		gui_display_update();
						return ;
         	}
        }
      }
      else
      {
      	//������ʱ
	    	if(menu_event_process(2.,MENU_EVT_OP_OUTTIME) == 0)
	    	{
					return ;
	    	}
        //��˸��ʾ
        GlintStatus++;
        menu_inputchar_glint(SHOW_X_ROW8(rt_strlen((const char *)buf)),SHOW_Y_LINE(2),GlintStatus%2);
      }
      //������ʾ
      
      gui_display_update();
			rt_thread_delay(1);
		}

		//�ڶ�����֤�ֻ�����
		CurUserPos = account_cur_pos_get();
    PhoneNum = user_valid_phone_num();
    
  	gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
    gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(0),PhoneAddText[0],GUI_WIHIT);
    rt_sprintf((char *)buf,"%03d",PhoneNum);
    gui_display_string(SHOW_X_ROW8(12),SHOW_Y_LINE(0),buf,GUI_WIHIT);

    gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(1),PhoneAddText[3],GUI_WIHIT);
    rt_sprintf((char *)buf,"%02d",PhoneNum);
    gui_display_string(SHOW_X_ROW8(13),SHOW_Y_LINE(1),buf,GUI_WIHIT);
    gui_display_update();

    rt_memset(buf,0,MENU_PHONE_MAX_LEN);
		while(1)
		{
			result = gui_key_input(&KeyValue);
      if(RT_EOK == result)
      {
        //�а���
        if(KeyValue >= '0' && KeyValue <= '9')
        {
          result = string_add_char(buf,KeyValue,MENU_PHONE_MAX_LEN);
          if(result == RT_EOK)
          {
            gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(2),SHOW_X_ROW8(15),SHOW_Y_LINE(3));
            gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(2),buf,GUI_WIHIT);
          }
          else
          {
            //������������8��
          }
        }
        else if(KeyValue == MENU_SURE_VALUE)
        {
        	rt_int32_t res;
          //���������ֻ��Ƿ�Ϸ�
          res = rt_memcmp(PhoneBuf,buf,11);
          if(res != 0)
          {
          	//���Ϸ�
          	menu_operation_result_handle(1);
    				gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PhoneAddText[2],GUI_WIHIT);
    				gui_display_update();
    				rt_thread_delay(RT_TICK_PER_SECOND);
    				gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(3),SHOW_X_ROW8(15),SHOW_Y_LINE(4));
          }
          else
          {
          	rt_uint8_t SaveShowFlag = 1;
          	
          	//�Ϸ�
          	if(CurUserPos == ADMIN_DATA_POS)
          	{
							result = admin_modify_phone(PhoneBuf);
          	}
          	else
          	{
          		rt_dprintf(MENU_DEBUG_THREAD,("Save phone %s ...\n",PhoneBuf));
              result = user_cur_add_phone(PhoneBuf);
          	}
           	
           	if(result == RT_ERROR)
           	{
							//����ʧ��
							menu_operation_result_handle(2);
							SaveShowFlag = 2;
           	}
           	menu_operation_result_handle(2);
           	gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),MenuCommText[SaveShowFlag],GUI_WIHIT);
    				gui_display_update();
    				rt_thread_delay(RT_TICK_PER_SECOND);
    				gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(3),SHOW_X_ROW8(15),SHOW_Y_LINE(4));
            break;
          }
          //������������� ������֤��
        }
        else if(KeyValue == MENU_DEL_VALUE)
        {
          result = string_del_char(buf,MENU_PHONE_MAX_LEN);
          if(result == RT_EOK)
          {
            gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(2),SHOW_X_ROW8(15),SHOW_Y_LINE(3));
            gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(2),buf,GUI_WIHIT);
          }
         	else
         	{
         		gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),MenuCommText[0],GUI_WIHIT);
         		gui_display_update();
						return ;
         	}
        }
      }
      else
      {
      	//������ʱ
	    	if(menu_event_process(2.,MENU_EVT_OP_OUTTIME) == 0)
	    	{
					return ;
	    	}
        //��˸��ʾ
        GlintStatus++;
        menu_inputchar_glint(SHOW_X_ROW8(rt_strlen((const char *)buf)),SHOW_Y_LINE(2),GlintStatus%2);
      }
      //������ʾ
      
      gui_display_update();
			rt_thread_delay(1);
		}
		rt_thread_delay(1);
  }
  //gui_display_update();
}

static void update_account_new_data(rt_uint16_t pos)
{	
	rt_uint8_t           i;
	struct account_head  *data = RT_NULL;
	
	data = rt_calloc(1,sizeof(*data));
	RT_ASSERT(data != RT_NULL);

	//�ϴ��˻�
	gprs_account_add_mail(pos);
	device_config_account_operate(pos,data,0);
  //�ϴ�����Կ��
	for(i = 0;i< ACCOUNT_KEY_NUMBERS;i++)
	{
		if(data->key[i] != KEY_ID_INVALID)
		{
			struct key           *key;

			key = rt_calloc(1,sizeof(*key));
			RT_ASSERT(key != RT_NULL);
			
			device_config_key_operate(data->key[i],key,0);
			if(key->head.is_updated == 1)
			{
        gprs_Key_add_mail(data->key[i]);
			}
			rt_free(key);
		}
	}
	//�ϴ��绰
	for(i=0;i < ACCOUNT_PHONE_NUMBERS;i++)
	{
		if(data->phone[i] != PHONE_ID_INVALID)
		{
			struct phone_head *ph;

			ph = rt_calloc(1,sizeof(*ph));
			RT_ASSERT(ph != RT_NULL);
			
			device_config_phone_operate(data->phone[i],ph,0);
			if(ph->is_update == 1)
			{
				gprs_account_add_mail(data->phone[i]);
			}
			rt_free(ph);
		}
	}

	rt_free(data);
}
//���沢�˳�
void menu_17_processing(void)
{
	rt_uint8_t *buf;
	rt_int32_t CurUserPos;
	
	gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
	CurUserPos = account_cur_pos_get();

	buf = rt_calloc(1,LCD_LINE_MAX_LEN);
	RT_ASSERT(buf != RT_NULL);
  rt_sprintf((char *)buf,"%03d",CurUserPos);      
  gui_display_string(SHOW_X_ROW8(12),SHOW_Y_LINE(0),buf,GUI_WIHIT);
  rt_free(buf);
  
	gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(0),SaveEixtText[0],GUI_WIHIT);
	gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(1),SaveEixtText[1],GUI_WIHIT);
	gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(2),SaveEixtText[2],GUI_WIHIT);
  gui_display_update();
  account_add_exit(RT_TRUE);
  update_account_new_data(CurUserPos);
}

void menu_18_processing(void)
{
	rt_uint8_t *ShowBuf;
	rt_int16_t CurUserPos;
	rt_uint8_t UserKeyNum;
	rt_uint8_t UserPhNum;
	
  CurUserPos = account_cur_pos_get();
	UserKeyNum = user_vaild_key_num();
	UserPhNum = user_valid_phone_num();
	
	ShowBuf = rt_calloc(1,LCD_LINE_MAX_LEN);
	
  gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);

	gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(1),MenuUserInfoText[0],GUI_WIHIT);
	gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(2),MenuUserInfoText[1],GUI_WIHIT);
	gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),MenuUserInfoText[2],GUI_WIHIT);

  rt_sprintf((char *)ShowBuf,"%02d",CurUserPos);
	gui_display_string(SHOW_X_ROW8(rt_strlen((const char *)MenuUserInfoText[0])),SHOW_Y_LINE(1),ShowBuf,GUI_WIHIT);

	rt_sprintf((char *)ShowBuf,"%02d",UserKeyNum);
	gui_display_string(SHOW_X_ROW8(rt_strlen((const char *)MenuUserInfoText[1])),SHOW_Y_LINE(2),ShowBuf,GUI_WIHIT);

	rt_sprintf((char *)ShowBuf,"%02d",UserPhNum);
	gui_display_string(SHOW_X_ROW8(rt_strlen((const char *)MenuUserInfoText[2])),SHOW_Y_LINE(3),ShowBuf,GUI_WIHIT);
	
  gui_display_update();
  rt_free(ShowBuf);
}

void menu_19_processing(void)
{
  gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
  gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(0),EixtText[0],GUI_WIHIT);
	gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(1),EixtText[1],GUI_WIHIT);
	account_add_exit(RT_FALSE);
  gui_display_update();
}
rt_err_t search_user_id_check(rt_uint8_t *buf,rt_uint8_t *user)
{
	rt_dprintf(MENU_DEBUG_KEY,("Input ID %s",buf));
	sscanf((const char*)buf,"%d",user);
	if(rt_strlen((const char *)buf) == 0)
	{
		return RT_ERROR;
	}

	if(account_valid_check(*user)  == RT_ERROR)
	{
		return RT_ERROR;
	}
	
	return RT_EOK;
}
//static rt_uint8_t ModifyUserPos;

#define PAGE_MAX_SHOW_LINE	4
static void user_list_processing(void)
{
	rt_uint8_t KeyValue;
	rt_err_t	 result;
	rt_uint8_t buf[LCD_LINE_MAX_LEN];
	//rt_uint8_t ShowStart = 0;
	rt_int8_t CurLine = 0;
	rt_uint8_t UserMaxNum = 100;
	rt_int32_t Page;
	rt_int8_t CurPage;
	rt_int32_t surplus;
	UserInfoDef UserInfo[PAGE_MAX_SHOW_LINE];
	rt_int32_t start_id = 0;
	rt_int32_t start_id1;
	rt_uint8_t i;
	
	gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);

	//��ȡ�û��������
	UserMaxNum = user_valid_num();
	if(UserMaxNum == 0)
	{
		//û���û�
		gui_display_update();
		return ;
	}
	Page = UserMaxNum/PAGE_MAX_SHOW_LINE;
	surplus = UserMaxNum % PAGE_MAX_SHOW_LINE;
  CurPage = 0;
  start_id = 0;

	rt_dprintf(MENU_DEBUG_THREAD,("start_id = %d\n",start_id));
	user_get_info_continuous(UserInfo,&start_id,PAGE_MAX_SHOW_LINE,0);
	start_id1 = start_id;
	while(1)
	{
    result = gui_key_input(&KeyValue);
    if(result == RT_EOK)
    {
      if(KeyValue == MENU_UP_VALUE)
      {
        //��
        if(CurPage >= 0)
        {
          CurLine--;
          if(CurLine < 0)
          {
 						if(CurPage > 0)
 						{
 							CurLine = 3;
 							if(CurPage == Page)
 							{
								user_get_info_continuous(UserInfo,&start_id,surplus,1);
 							}
	            CurPage--;
	            user_get_info_continuous(UserInfo,&start_id,PAGE_MAX_SHOW_LINE,1);
 						}
 						else
 						{
              CurLine++;
 						}
          }
        }
      }
      else if(KeyValue == MENU_DOWN_VALUE)
      {
        //��
        CurLine++;
        if(CurLine > 3)
        {
        	if(CurPage*PAGE_MAX_SHOW_LINE+CurLine < UserMaxNum)
        	{
        		//rt_int32_t tmp;

        		if(CurPage == 0)
        		{
              start_id = start_id1;
        		}
        		CurLine = 0;
  					CurPage++;
  					user_get_info_continuous(UserInfo,&start_id,PAGE_MAX_SHOW_LINE,0);
        	}
        	else
        	{
						CurLine--;
        	}
        }
      }
      /*else if(KeyValue == '7')
      {
        //��
        if(CurPage > 1)
        {
					CurPage--;
					user_get_info_continuous(UserInfo,&start_id,PAGE_MAX_SHOW_LINE,1);
        }
      }
      else if(KeyValue == '9')
      {
        //��
        if(CurPage > Page)
        {
          CurPage++;
          user_get_info_continuous(UserInfo,&start_id,PAGE_MAX_SHOW_LINE,0);
        }
      }*/
      else if(KeyValue == MENU_SURE_VALUE)
      {
        //ȷ��
        rt_dprintf(MENU_DEBUG_THREAD,("Choose User:%d\n",CurPage*PAGE_MAX_SHOW_LINE+CurLine));
        //ModifyUserPos = CurPage*PAGE_MAX_SHOW_LINE+CurLine;
        account_set_use(CurPage*PAGE_MAX_SHOW_LINE+CurLine);
        menu_run_sure_process();
        return ;
      }
      else if(KeyValue == MENU_DEL_VALUE)
      {
        //ȡ��
      }

      rt_dprintf(MENU_DEBUG_THREAD,("Choose Pos CurPage = %d CurLine = %d\n",CurPage,CurLine));
    }
    else
    {
			//������ʱ
    	if(menu_event_process(2.,MENU_EVT_OP_OUTTIME) == 0)
    	{
				return ;
    	}
    }
    for(i = 0;i<PAGE_MAX_SHOW_LINE;i++)
		{
			rt_uint8_t color = GUI_WIHIT;
			
			rt_memset(buf,0,LCD_LINE_MAX_LEN);
			rt_sprintf((char *)buf,"%03d",(const char *)UserInfo[i].id);
			if(CurLine == i)
			{
				color = GUI_BLACK;
			}
			gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(i),buf,color);
			
			rt_memset(buf,0,LCD_LINE_MAX_LEN);
			rt_strncpy((char *)buf,(const char *)UserInfo[i].name,LCD_LINE_MAX_LEN);
			gui_display_string(SHOW_X_ROW16(4),SHOW_Y_LINE(i),buf,GUI_WIHIT);
		}
    gui_display_update();
	}
}

//���ؽ���
void menu_22_processing(void)
{
	rt_uint8_t KeyValue;
	rt_err_t   result;
	rt_uint8_t GlintStatus;
	rt_uint8_t UserPos;
	rt_uint8_t buf[LCD_LINE_MAX_LEN];
	
  gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);

  gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(0),UserSearchText[0],GUI_WIHIT);
  gui_display_update();
  rt_memset((void *)buf,0,LCD_LINE_MAX_LEN);
 	while(1)
	{
		result = gui_key_input(&KeyValue);
  	if(RT_EOK == result)
	  {
	    //�а���
	    if(KeyValue >= '0' && KeyValue <= '9')
	    {
	      result = string_add_char(buf,KeyValue,MENU_PHONE_MAX_LEN);
	      if(result == RT_EOK)
	      {
	        gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(1),SHOW_X_ROW8(15),SHOW_Y_LINE(2));
	        gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(1),buf,GUI_WIHIT);
	      }
	      else
	      {
	        //������������8��
	      }
	    }
	    else if(KeyValue == MENU_SURE_VALUE)
	    {
	      //����Ƿ����ҵ������
	      result = search_user_id_check(buf,&UserPos);
	      if(result != RT_EOK)
	      {
	      	//����Ҳ��������������������
					gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(2),UserSearchText[1],GUI_WIHIT);
					gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),UserSearchText[2],GUI_WIHIT);
					gui_display_update();
					//rt_thread_delay(RT_TICK_PER_SECOND);
					

					//ȷ����������� ���ؼ���������
					while(1)
					{
            result = gui_key_input(&KeyValue);
						if(result == RT_EOK)
						{
							if(KeyValue == MENU_SURE_VALUE)
							{
								//�����������
								user_list_processing();
								return ;
							}
							else if(KeyValue == MENU_DEL_VALUE)
							{
								//���ؼ�
								gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(2),SHOW_X_ROW8(16),SHOW_Y_LINE(3));
								gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(3),SHOW_X_ROW8(16),SHOW_Y_LINE(4));
								break;
							}
						}
					}
	      }
	      else
	      {
	        //������ҵ�����˽����޸Ľ���
	        
	        //ModifyUserPos = UserPos;

	        account_set_use(UserPos);
	        menu_run_sure_process();
	       	rt_dprintf(MENU_DEBUG_THREAD,("User Pos=%d\n",UserPos));
	        break;
	      }
	      //������������� ������֤��
	    }
	    else if(KeyValue == MENU_DEL_VALUE)
	    {
	      result = string_del_char(buf,MENU_PHONE_MAX_LEN);
	      if(result == RT_EOK)
	      {
	        gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(1),SHOW_X_ROW8(15),SHOW_Y_LINE(2));
	        gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(1),buf,GUI_WIHIT);
	      }
	     	else
	     	{
	     		gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),MenuCommText[0],GUI_WIHIT);
	     		gui_display_update();
					return ;
	     	}
	    }
	  }
	  else
	  {
	  	//������ʱ
    	if(menu_event_process(2.,MENU_EVT_OP_OUTTIME) == 0)
    	{
				return ;
    	}
	    //��˸��ʾ
	    GlintStatus++;
	    menu_inputchar_glint(SHOW_X_ROW8(rt_strlen((const char *)buf)),SHOW_Y_LINE(1),GlintStatus%2);
	  }
	  //������ʾ
  
  gui_display_update();
	rt_thread_delay(1);
	}
  gui_display_update();
}

static void menu_user_modify_ui(rt_uint8_t InPOS)
{
	rt_uint8_t page;
	rt_uint8_t pos;
	rt_uint8_t i;
	rt_int32_t CurUserPos;

	CurUserPos = account_cur_pos_get();
	page = InPOS /PAGE_MAX_SHOW_NUM;//������ʾ����һҳ
	pos = InPOS % PAGE_MAX_SHOW_NUM;//��ǰѡ�е�λ��
	gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
	for(i=0;i<PAGE_MAX_SHOW_NUM;i++)
	{
		if(page*PAGE_MAX_SHOW_NUM+i >= MENU_USER_MODIFY_TYPE_NUM)
		{
			break;
		}
		if(CurUserPos == 0)
		{
      if(i == pos)
      {
        gui_display_string(SHOW_X_CENTERED((const char*)MenuUserModifyList1[page*PAGE_MAX_SHOW_NUM+i]),
                           SHOW_Y_LINE(i),
                           (rt_uint8_t *)MenuUserModifyList1[page*PAGE_MAX_SHOW_NUM+i],
                           GUI_BLACK);
      }
      else
      {
        gui_display_string(SHOW_X_CENTERED((const char*)MenuUserModifyList1[page*PAGE_MAX_SHOW_NUM+i]),
                           SHOW_Y_LINE(i),
                           (rt_uint8_t *)MenuUserModifyList1[page*PAGE_MAX_SHOW_NUM+i],
                           GUI_WIHIT);
      }
		}
		else
		{
#ifdef USEING_MODIF_UI
			if(i == pos)
			{
			  
			gui_display_string(SHOW_X_CENTERED((const char*)MenuUserModifyList1[page*PAGE_MAX_SHOW_NUM+i]),
			                   SHOW_Y_LINE(i),
			                   (rt_uint8_t *)MenuUserModifyList1[page*PAGE_MAX_SHOW_NUM+i],
			                   GUI_BLACK);
			}
			else
			{
			gui_display_string(SHOW_X_CENTERED((const char*)MenuUserModifyList1[page*PAGE_MAX_SHOW_NUM+i]),
			                   SHOW_Y_LINE(i),
			                   (rt_uint8_t *)MenuUserModifyList1[page*PAGE_MAX_SHOW_NUM+i],
			                   GUI_WIHIT);
			}
#else
			if(i == pos)
			{
				
			gui_display_string(SHOW_X_CENTERED((const char*)MenuUserModifyList[page*PAGE_MAX_SHOW_NUM+i]),
			                   SHOW_Y_LINE(i),
			                   (rt_uint8_t *)MenuUserModifyList[page*PAGE_MAX_SHOW_NUM+i],
			                   GUI_BLACK);
			}
			else
			{
			gui_display_string(SHOW_X_CENTERED((const char*)MenuUserModifyList[page*PAGE_MAX_SHOW_NUM+i]),
			                   SHOW_Y_LINE(i),
			                   (rt_uint8_t *)MenuUserModifyList[page*PAGE_MAX_SHOW_NUM+i],
			                   GUI_WIHIT);
			}
#endif
		}
	}
	gui_display_update();
}


//�������
void menu_23_processing(void)
{
 menu_user_modify_ui(0);
}

void menu_24_processing(void)
{
 menu_user_modify_ui(1);
}

void menu_25_processing(void)
{
 menu_user_modify_ui(2);
}

void menu_26_processing(void)
{
 menu_user_modify_ui(3);
}

void menu_27_processing(void)
{
 menu_user_modify_ui(4);
}

void password_is_cur_user(rt_uint8_t *buf)
{
	
}

void menu_input_new_password_ui(rt_uint8_t *old_password)
{
	
}

static rt_err_t menu_intput_password_two(rt_uint8_t *buf,rt_uint8_t *ShowBuf,rt_uint8_t *password)
{
	rt_uint8_t GlintStatus;
	
  rt_memset(buf,0,MENU_PASSWORD_MAX_LEN);
	rt_memset(ShowBuf,0,MENU_PASSWORD_MAX_LEN);
	while(1)
	{
		rt_err_t   result;
		rt_uint8_t KeyValue;

		result = gui_key_input(&KeyValue);
		if(RT_EOK == result)
		{
		  //�а���
		  if(KeyValue >= '0' && KeyValue <= '9')
		  {
		    result = string_add_char(buf,KeyValue,8);
		    if(result == RT_EOK)
		    {
		      string_hide_string((const rt_uint8_t *)buf,ShowBuf,SHOW_PW_HIDE_CH,8);
		      gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(2),SHOW_X_ROW8(15),SHOW_Y_LINE(3));
		      gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(2),ShowBuf,GUI_WIHIT);
		    }
		    else
		    {
		      //������������8��
		    }
		  }
		  else if(KeyValue == MENU_SURE_VALUE)
		  {
		    //�������������Ƿ�Ϸ�
		    rt_uint32_t temp;
		    
		    temp = rt_memcmp(password,buf,rt_strlen((const char *)password));
		    if(temp != 0)
		    {
		    	//�������벻ƥ��
		    	RT_DEBUG_LOG(MENU2_DEBUG_ARG,("password1 : %s != password2 %s\n",password,buf));
		      return RT_ERROR;
		    }
		    else
		    {	
		    	RT_DEBUG_LOG(MENU2_DEBUG_ARG,("Password Change successfully\n"));
		      return RT_EOK;
		    }
		  }
		  else if(KeyValue == MENU_DEL_VALUE)
		  {
		    result = string_del_char(buf,8);
		    if(result == RT_EOK)
		    {
		      string_hide_string((const rt_uint8_t *)buf,ShowBuf,SHOW_PW_HIDE_CH,8);
		      gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(2),SHOW_X_ROW8(15),SHOW_Y_LINE(3));
		      gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(2),ShowBuf,GUI_WIHIT);
		    }
		    else
		    {
		      rt_dprintf(MENU_DEBUG_THREAD,("Add key Exit\n"));
		      return RT_ETIMEOUT;
		    }
		  }
		}
		else
		{
			//������ʱ
    	if(menu_event_process(2.,MENU_EVT_OP_OUTTIME) == 0)
    	{
				return RT_ETIMEOUT;
    	}
		  //��˸��ʾ
		  GlintStatus++;
		  menu_inputchar_glint(SHOW_X_ROW8(rt_strlen((const char *)ShowBuf)),SHOW_Y_LINE(2),GlintStatus%2);
		}
		//������ʾ

		gui_display_update();
	}

}

static rt_err_t menu_input_password_one(rt_uint8_t *buf,rt_uint8_t *ShowBuf)
{
	rt_uint8_t GlintStatus;

  rt_memset(buf,0,MENU_PASSWORD_MAX_LEN);
  rt_memset(ShowBuf,0,MENU_PASSWORD_MAX_LEN);
  while(1)
  {
    rt_err_t result;
    rt_uint8_t KeyValue;
    
    result = gui_key_input(&KeyValue);
    if(RT_EOK == result)
    {
      //�а���
      if(KeyValue >= '0' && KeyValue <= '9')
      {
        result = string_add_char(buf,KeyValue,8);
        if(result == RT_EOK)
        {
          string_hide_string((const rt_uint8_t *)buf,ShowBuf,SHOW_PW_HIDE_CH,8);
          gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(2),SHOW_X_ROW8(15),SHOW_Y_LINE(3));
          gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(2),ShowBuf,GUI_WIHIT);
        }
        else
        {
          //������������8��
        }
      }
      else if(KeyValue == MENU_SURE_VALUE)
      {
        //�������������Ƿ�Ϸ�
        if(rt_strlen((const char*)buf) < CONFIG_PASSWORD_LEN)
        {
        	//�����ǿյ�
					return RT_ENOMEM;
        }
        result = add_new_password_check(buf);
        if(result != RT_EOK)
        {
          //���벻�Ϸ�
          return RT_ERROR;
        }
        else
        {
          //�������ǺϷ���
          return RT_EOK;
        }
        //������������� ������֤��
      }
      else if(KeyValue == MENU_DEL_VALUE)
      {
        result = string_del_char(buf,8);
        if(result == RT_EOK)
        {
          string_hide_string((const rt_uint8_t *)buf,ShowBuf,SHOW_PW_HIDE_CH,8);
          gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(2),SHOW_X_ROW8(15),SHOW_Y_LINE(3));
          gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(2),ShowBuf,GUI_WIHIT);
        }
        else
        {
          return RT_ETIMEOUT;
        }
      }
    }
    else
    {
			//������ʱ
			if(menu_event_process(2.,MENU_EVT_OP_OUTTIME) == 0)
			{
					return RT_ETIMEOUT;
			}
      //��˸��ʾ
      GlintStatus++;
      menu_inputchar_glint(SHOW_X_ROW8(rt_strlen((const char *)ShowBuf)),SHOW_Y_LINE(2),GlintStatus%2);
    }
    //������ʾ
    gui_display_update();
  }
}

static menu_password_modify_uiinit(rt_uint8_t *buf,rt_uint8_t page)
{
	rt_uint8_t CurUserPos;
	
	rt_uint8_t UserMaxPsNum;
  CurUserPos = account_cur_pos_get();
  
  UserMaxPsNum = user_valid_password_num();
  
  gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);

  switch(page)
  {
		case 0:
		{
			gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(0),PasswordModifyText[0],GUI_WIHIT);
		  rt_sprintf((char *)buf,"%03d",CurUserPos);
		  gui_display_string(SHOW_X_ROW8(12),SHOW_Y_LINE(0),buf,GUI_WIHIT);
		  
		  gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(1),PasswordModifyText[1],GUI_WIHIT);
			break;
		}

		case 1:
		{
			gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(0),PasswordModifyText[0],GUI_WIHIT);
			rt_sprintf((char *)buf,"%03d",CurUserPos);
			gui_display_string(SHOW_X_ROW8(12),SHOW_Y_LINE(0),buf,GUI_WIHIT);

			gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(1),PasswordModifyText[5],GUI_WIHIT);
			rt_sprintf((char *)buf,"%02d",UserMaxPsNum);
			gui_display_string(SHOW_X_ROW8(13),SHOW_Y_LINE(1),buf,GUI_WIHIT);
			break;
		}
		case 2:
		{
	    gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(0),PasswordModifyText[0],GUI_WIHIT);
			rt_sprintf((char *)buf,"%03d",CurUserPos);
	    gui_display_string(SHOW_X_ROW8(12),SHOW_Y_LINE(0),buf,GUI_WIHIT);
	    
	    gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(1),PasswordModifyText[8],GUI_WIHIT);
	    rt_sprintf((char *)buf,"%02d",UserMaxPsNum);
	    gui_display_string(SHOW_X_ROW8(13),SHOW_Y_LINE(1),buf,GUI_WIHIT);
			break;
		}
		default:
		{
			break;
		}
  }
  gui_display_update();
}

static rt_err_t menu_input_string_ui1(rt_uint8_t *buf,rt_uint8_t InputLength)
{
	
	rt_uint8_t GlintStatus;
	rt_uint8_t *ShowBuf;
	rt_err_t 	 FunResult;

	ShowBuf = rt_calloc(1,MENU_PASSWORD_MAX_LEN);
	RT_ASSERT(ShowBuf != RT_NULL);
	while(1)
	{
			rt_err_t 	 result;
			rt_uint8_t KeyValue;

	    result = gui_key_input(&KeyValue);
	    if(RT_EOK == result)
	    {
	      //�а���
	      if(KeyValue >= '0' && KeyValue <= '9')
	      {
	        result = string_add_char(buf,KeyValue,InputLength);
	        if(result == RT_EOK)
	        {
	          string_hide_string((const rt_uint8_t *)buf,ShowBuf,SHOW_PW_HIDE_CH,8);
	          gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(2),SHOW_X_ROW8(15),SHOW_Y_LINE(3));
	          gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(2),ShowBuf,GUI_WIHIT);
	        }
	        else
	        {
	          //������������8��
	        }
	      }
	      else if(KeyValue == MENU_SURE_VALUE)
	      {
					FunResult = RT_EOK;
					break;
	      }
	      else if(KeyValue == MENU_DEL_VALUE)
	      {
	        result = string_del_char(buf,8);
	        if(result == RT_EOK)
	        {
	          string_hide_string((const rt_uint8_t *)buf,ShowBuf,SHOW_PW_HIDE_CH,8);
	          gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(2),SHOW_X_ROW8(15),SHOW_Y_LINE(3));
	          gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(2),ShowBuf,GUI_WIHIT);
	        }
	        else
	        {
						FunResult = RT_ERROR;
						break;
	        }
	      }
	    }
	    else
	    {
	    	//������ʱ
	    	if(menu_event_process(2.,MENU_EVT_OP_OUTTIME) == 0)
	    	{
					break ;
	    	}
	      //��˸��ʾ
	      GlintStatus++;
	      menu_inputchar_glint(SHOW_X_ROW8(rt_strlen((const char *)ShowBuf)),SHOW_Y_LINE(2),GlintStatus%2);
	    }
	    //������ʾ
	    
	    gui_display_update();
	  }

	  rt_free(ShowBuf);
	  
		return FunResult;
}

//����Ҫ�޸ĵ�����
// 1 ��������
// 2 �޸�����
// 3 �������
// 4 ��������
static rt_uint8_t password_modify_fun_chonse(rt_uint8_t *buf,rt_int32_t *KeyPos)
{
	rt_err_t result;
	rt_uint8_t CurUserPos;
#if 1
  CurUserPos++;
  CurUserPos = account_cur_pos_get();

 	if(rt_strlen((const char*)buf) < CONFIG_PASSWORD_LEN)
	{	
		//���������Ϊ��
		gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PasswordModifyText[4],GUI_WIHIT);
		gui_display_update();
		menu_input_sure_key(0);
		gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(3),SHOW_X_ROW8(15),SHOW_Y_LINE(4));
		return 4;
	}
	//��������Ƿ����
	result = add_new_password_check(buf);	
	if(result  == RT_EOK)
	{
	  //���Ǹ������� 
		gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PasswordModifyText[10],GUI_WIHIT);
		gui_display_update();

		if(menu_input_sure_key(0) == RT_EOK)
		{
			//ȷ������
			return 1;
		}
		else
		{
			//������������
			return 4;
		}	
	}
	else
	{
		//�Ѿ����ڵ�����
	  //rt_uint8_t save = 1;
	  
		//������ID
		*KeyPos = key_pos_get_password(buf);
		
		result = key_check_password_cur_pos(buf);
		if(result == RT_EOK)
		{
			//�ǵ�ǰ�û��µ�����
			gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PasswordModifyText[2],GUI_WIHIT);
			gui_display_update();

			if(menu_input_sure_key(0) == RT_EOK)
			{
				//ȷ���޸�
				return 2;
			}
			else
			{
				//����
			}	
			gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(3),SHOW_X_ROW8(15),SHOW_Y_LINE(4));
		}
		else
		{
			//��ǰ�û�û���������
			gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PasswordModifyText[6],GUI_WIHIT);
  		gui_display_update();
  		return 3;
		}
	}
#endif
  return 4;
}

//�����޸�GUI
void menu_password_modify_ui(void)
{
	rt_uint8_t CurUserPos;
	rt_int32_t OldKeyPos;
	rt_uint8_t buf[MENU_PASSWORD_MAX_LEN];
	rt_uint8_t *PasswordOne;
	rt_uint8_t ShowBuf[MENU_PASSWORD_MAX_LEN];
	rt_err_t 	 result;

	PasswordOne = rt_calloc(1,MENU_PASSWORD_MAX_LEN);
	
	RT_ASSERT(PasswordOne != RT_NULL);
	while(1)
	{	
	
    //�����޸ĵ�����
MENU_INPUT_PASSWORD_OLD:
    while(1)
    {	
    	//��ʼ������
			menu_password_modify_uiinit(buf,0);
      rt_memset(buf,0,MENU_PASSWORD_MAX_LEN);
      result  = menu_input_string_ui1(buf,7);
      if(result == RT_EOK)
      {
        rt_uint8_t OpResult;
        
        OpResult = password_modify_fun_chonse(buf,&OldKeyPos);
        RT_DEBUG_LOG(MENU2_DEBUG_ARG,("password:%s fun:%d Pos:%d\n",buf,OpResult,OldKeyPos));
        if(OpResult == 1)
        {
          //��������
          rt_memcpy(PasswordOne,buf,rt_strlen((const char *)buf));
          while(1)
          {
			      //��ʼ������
			      menu_password_modify_uiinit(buf,2);
			      result = menu_intput_password_two(buf,ShowBuf,PasswordOne);
			      if(result == RT_ERROR)
			      {
			        //����ƥ�����
			        gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PasswordModifyText[9],GUI_WIHIT);
			        gui_display_update();
			        rt_thread_delay(RT_TICK_PER_SECOND);
			        gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(3),SHOW_X_ROW8(15),SHOW_Y_LINE(4));
			      }
			      else if(result == RT_EOK)
			      {
			        rt_uint8_t save = 1;
			        
			        //����Ա�û�
			        if(CurUserPos == ADMIN_DATA_POS)
			        {
			          result = admin_modify_password(PasswordOne);
			          if(result == RT_ERROR)
			          {
			          	RT_DEBUG_LOG(MENU2_DEBUG_ARG,("Modify Admin Password Failure\n"));
			            save = 2;
			          }
			        }
			        else
			        {
			          //��ӳɹ��������
			          result = key_password_modify(OldKeyPos,PasswordOne);
			          if(result != RT_EOK)
			          {
			            //����ʧ��
			            RT_DEBUG_LOG(MENU2_DEBUG_ARG,("Modify User Password Failure\n"));
			            save = 2;
			          }
			        }
			        RT_DEBUG_LOG(MENU2_DEBUG_ARG,("Modify Password Succeed\n"));
			        gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),MenuCommText[save],GUI_WIHIT);
			        gui_display_update();
			        rt_thread_delay(RT_TICK_PER_SECOND);
			        gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(3),SHOW_X_ROW8(15),SHOW_Y_LINE(4));
			        
			        goto EXIT_MENU_PASSWORD_MODIFY_UI;;
			      }
			      else
			      {
							break;
			      }
					}
        }
        else if(OpResult == 2)
        {
          //ȷ���޸�
          RT_DEBUG_LOG(MENU2_DEBUG_ARG,("Modify %s\n",buf));
          break;
        }
        else if(OpResult == 3)
        {
          //�������
          gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PasswordModifyText[7],GUI_WIHIT);
          gui_display_update();
        }
        else if(OpResult == 4)
        {
          //��������
        }
      }
      else
      {
       //�˳��޸�����
       goto EXIT_MENU_PASSWORD_MODIFY_UI;
      }
    }

MENU2_INPUT_PASSWORD_ONE:
		//��һ������������
		while(1)
		{
      //��ʼ������
      menu_password_modify_uiinit(buf,1);
      result = menu_input_password_one(buf,ShowBuf);
      if(result == RT_ENOMEM)
      {
				//�յ�����
				if(CurUserPos == 0)
				{
					//����Ա����ɾ��
					continue;
				}
				gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PasswordModifyText[11],GUI_WIHIT);
				gui_display_update();
				result = menu_input_sure_key(0);
				if(result == RT_EOK)
				{
					//ɾ��
					result = key_password_delete(OldKeyPos);
					if(result == RT_EOK)
					{
						gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PasswordModifyText[12],GUI_WIHIT);
						gui_display_update();
						menu_input_sure_key(0);
						goto MENU_INPUT_PASSWORD_OLD;
					}
					else
					{
						rt_kprintf("system error\n");
						RT_ASSERT(RT_NULL != RT_NULL);
					}
				}
				else
				{
					continue;
				}
      }
      else if(result == RT_EOK)
      {
        //�����������Ҫ��
        RT_DEBUG_LOG(MENU2_DEBUG_ARG,("New Password %s is OK\n",buf));
        rt_memcpy(PasswordOne,buf,rt_strlen((const char *)buf));
        break;
      }
      else if(result == RT_ERROR)
      {
        gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PasswordModifyText[7],GUI_WIHIT);
        gui_display_update();
        rt_thread_delay(RT_TICK_PER_SECOND);
        gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(3),SHOW_X_ROW8(15),SHOW_Y_LINE(4));
      }
      else if(result == RT_ETIMEOUT)
      {
      	gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),MenuCommText[0],GUI_WIHIT);
      	gui_display_update();
      	result = menu_input_sure_key(0);
      	if(result == RT_EOK)
      	{
          gui_display_update();
          goto MENU_INPUT_PASSWORD_OLD;
      	}
      	goto MENU2_INPUT_PASSWORD_ONE;
      }
		}

    //�ڶ�����������
		while(1)
		{
      //��ʼ������
      menu_password_modify_uiinit(buf,2);
      result = menu_intput_password_two(buf,ShowBuf,PasswordOne);
      if(result == RT_ERROR)
      {
        //����ƥ�����
        gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PasswordModifyText[9],GUI_WIHIT);
        gui_display_update();
        rt_thread_delay(RT_TICK_PER_SECOND);
        gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(3),SHOW_X_ROW8(15),SHOW_Y_LINE(4));
      }
      else if(result == RT_EOK)
      {
        rt_uint8_t save = 1;
        
        //����Ա�û�
        if(CurUserPos == ADMIN_DATA_POS)
        {
          result = admin_modify_password(PasswordOne);
          if(result == RT_ERROR)
          {
          	RT_DEBUG_LOG(MENU2_DEBUG_ARG,("Modify Admin Password Failure\n"));
            save = 2;
          }
        }
        else
        {
          //��ӳɹ��������
          result = key_password_modify(OldKeyPos,PasswordOne);
          if(result != RT_EOK)
          {
            //����ʧ��
            RT_DEBUG_LOG(MENU2_DEBUG_ARG,("Modify User Password Failure\n"));
            save = 2;
          }
        }
        RT_DEBUG_LOG(MENU2_DEBUG_ARG,("Modify Password Succeed\n"));
        gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),MenuCommText[save],GUI_WIHIT);
        gui_display_update();
        rt_thread_delay(RT_TICK_PER_SECOND);
        gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(3),SHOW_X_ROW8(15),SHOW_Y_LINE(4));
        break;
      }
      else if(result == RT_ETIMEOUT)
      {
				goto MENU2_INPUT_PASSWORD_ONE;
      }
		}
	}
EXIT_MENU_PASSWORD_MODIFY_UI:
	rt_free(PasswordOne);
}

//�ֻ��޸Ľ���
void menu_phone_modify_ui(void)
{
	gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
  
  gui_display_update();	
}

//�������
void menu_28_processing(void)
{
#ifdef USEING_MODIF_UI
	menu_password_modify_ui();
#else
	menu_14_processing();
#endif
}

//���ָ��
void menu_29_processing(void)
{
#ifdef USEING_MODIF_UI
	menu_fprint_modify_ui();
#else
	menu_15_processing();
#endif
}

//��ӵ绰
void menu_30_processing(void)
{
#ifdef USEING_MODIF_UI
	menu_phone_modify_ui();
#else
	menu_16_processing();
#endif
}



void menu_31_processing(void)
{
	rt_err_t result;

	gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);

	if(account_cur_pos_get() == 0)
	{
		gui_display_string(SHOW_X_CENTERED(EixtText[1]),SHOW_Y_LINE(2),EixtText[1],GUI_WIHIT);
	}
	else
	{
    result = account_cur_delete();
    if(result != RT_EOK)
    {
      gui_display_string(SHOW_X_CENTERED(MenuCommText[2]),SHOW_Y_LINE(2),MenuCommText[2],GUI_WIHIT);
    }
    else
    {
      gui_display_string(SHOW_X_CENTERED(MenuCommText[1]),SHOW_Y_LINE(2),MenuCommText[1],GUI_WIHIT);
    
    }
	}

	gui_display_update();
}

//�޸����뱣���˳�����
void menu_32_processing(void)
{
  menu_17_processing();
}

//�˵�ϵͳ�����б�˵�ѡ��
static void menu_system_setlist_ui(rt_uint8_t InPOS)
{
	rt_uint8_t page;
	rt_uint8_t pos;
	rt_uint8_t i;

	page = InPOS /PAGE_MAX_SHOW_NUM;//������ʾ����һҳ
	pos = InPOS % PAGE_MAX_SHOW_NUM;//��ǰѡ�е�λ��
	gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
	for(i=0;i<PAGE_MAX_SHOW_NUM;i++)
	{
		if(page*PAGE_MAX_SHOW_NUM+i >= MENU_SYSTEM_SET_LIST_NUM)
		{
			break;
		}
		if(i == pos)
		{
			gui_display_string(SHOW_X_CENTERED((const char*)MenuSystemSetListText[page*PAGE_MAX_SHOW_NUM+i]),
												 SHOW_Y_LINE(i),
												 (rt_uint8_t *)MenuSystemSetListText[page*PAGE_MAX_SHOW_NUM+i],
												 GUI_BLACK);
		}
		else
		{
			gui_display_string(SHOW_X_CENTERED((const char*)MenuSystemSetListText[page*PAGE_MAX_SHOW_NUM+i]),
												 SHOW_Y_LINE(i),
												 (rt_uint8_t *)MenuSystemSetListText[page*PAGE_MAX_SHOW_NUM+i],
												 GUI_WIHIT);
		}
	}
	gui_display_update();
}


//�Զ������б���ѡ��
void menu_33_processing(void)
{
  menu_system_setlist_ui(0);
}

//�Զ������б���ѡ��
void menu_34_processing(void)
{
  menu_system_setlist_ui(1);
}

//�޸��Զ�����ʱ�����
void menu_35_processing(void)
{
	rt_uint8_t  *ShowBuf;
	rt_uint8_t  SleepTime = 0;
	rt_err_t    result;
	
	ShowBuf = rt_calloc(1,LCD_LINE_MAX_LEN);
	RT_ASSERT(ShowBuf != RT_NULL);

	SleepTime = gui_sleep_time_get();
	
	gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
  gui_display_string(SHOW_X_CENTERED(MenuAutoSleepText[0]),SHOW_Y_LINE(1),MenuAutoSleepText[0],GUI_WIHIT);
  rt_sprintf((char *)ShowBuf,"<  %03d  >",SleepTime);
  gui_display_string(SHOW_X_CENTERED(ShowBuf),SHOW_Y_LINE(2),ShowBuf,GUI_WIHIT);
  gui_display_update();
	while(1)
	{
		rt_uint8_t data;
		
		result = gui_key_input(&data);
		if(result == RT_EOK)
		{
			if(data == MENU_SURE_VALUE)
			{
				break;
			}
			else if(data == MENU_DEL_VALUE)
			{
				break;
			}
			else if(data == MENU_UP_VALUE)
			{
				SleepTime++;
			}	
			else if(data == MENU_DOWN_VALUE)
			{
				SleepTime--;
			}
			rt_sprintf((char *)ShowBuf,"<  %03d  >",SleepTime);
  		gui_display_string(SHOW_X_CENTERED(ShowBuf),SHOW_Y_LINE(2),ShowBuf,GUI_WIHIT);
  		gui_display_update();
		}
	}
	gui_sleep_time_set(SleepTime);
	gui_display_update();

	rt_free(ShowBuf);
}

//�޸��Զ�����ʱ�����
void menu_36_processing(void)
{
	rt_uint8_t  *ShowBuf;
	rt_uint8_t  LockTime = 0;
	rt_err_t    result;
	
	ShowBuf = rt_calloc(1,LCD_LINE_MAX_LEN);
	RT_ASSERT(ShowBuf != RT_NULL);

	LockTime = system_autolock_time_get();
	
	gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
  gui_display_string(SHOW_X_CENTERED(MenuAutoLockText[0]),SHOW_Y_LINE(1),MenuAutoLockText[0],GUI_WIHIT);
  rt_sprintf((char *)ShowBuf,"<  %03d  >",LockTime);
  gui_display_string(SHOW_X_CENTERED(ShowBuf),SHOW_Y_LINE(2),ShowBuf,GUI_WIHIT);
  gui_display_update();
	while(1)
	{
		rt_uint8_t data;
		
		result = gui_key_input(&data);
		if(result == RT_EOK)
		{
			if(data == MENU_SURE_VALUE)
			{
				break;
			}
			else if(data == MENU_DEL_VALUE)
			{
        break;
			}
			else if(data == MENU_UP_VALUE)
			{
				LockTime++;
			}	
			else if(data == MENU_DOWN_VALUE)
			{
				if(LockTime > AOTU_LOCK_MIN_TIME)
				{
          LockTime--;
				}
				
			}
			rt_sprintf((char *)ShowBuf,"<  %03d  >",LockTime);
  		gui_display_string(SHOW_X_CENTERED(ShowBuf),SHOW_Y_LINE(2),ShowBuf,GUI_WIHIT);
  		gui_display_update();
		}
	}
	system_autolock_time_set(LockTime);
	gui_display_update();

	rt_free(ShowBuf);

}


//�޸Ľ���



#ifdef RT_USING_FINSH
#include <finsh.h>

void config_test(void)
{
	int user_num;
	rt_int32_t key_num;
	
	user_num = device_config_account_counts();
	rt_kprintf("user num:%d\n",user_num);

	key_num = device_config_account_key_counts(0);
	rt_kprintf("user key:%d\n",key_num);

	key_num = device_config_account_next_valid(11,0);
	rt_kprintf("next %d\n",key_num);

	key_num = device_config_account_next_valid(11,1);
	rt_kprintf("next %d\n",key_num);
}
FINSH_FUNCTION_EXPORT(config_test,config api test);

#endif


