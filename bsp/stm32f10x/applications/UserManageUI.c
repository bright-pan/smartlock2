#include "UserManageUI.h"
#include"menu.h"

/**************************************************************************************************/
#define PASSWORD_MAX_LEN						7   //������󳤶�
#define PHONE_MAX_LEN               13  //�ֻ���󳤶�
#define INPUT_PW_WAIT_TIME         30   //��������ȴ�ʱ��

/**************************************************************************************************/
/* �û�����˵��б� */
static const rt_uint8_t UserManageListText[][LCD_LINE_MAX_LEN] =
{
	{"1.�������"},
	{"2.ָ�ƹ���"},
	{"3.�ֻ�����"},
};

/* ���������� */
static const rt_uint8_t PasswordManageText[][LCD_LINE_MAX_LEN] =
{
	{"1.���"},
	{"2.ɾ��"},
};

/* ���������� */
static const rt_uint8_t PasswordAddText[][LCD_LINE_MAX_LEN] =
{
	{"�û�ID:"},
	{"����Կ������"},
	{"������������"},
	{"��������һ��"},
	{"�����Ѵ���"},
	{"���볤�ȴ���"},
	{"��ӳɹ�"},
	{"���ʧ��������"},
	{"�������벻ƥ��"},
};

/* ɾ��������� */
static const rt_uint8_t PasswordDelText[][LCD_LINE_MAX_LEN] =
{
	{"�û�ID"},
	{"������"},
	{"��Ҫɾ��������:"},
	{"����ɾ���ɹ�"},
	{"���볤�ȴ���"},
	{"���û�û�д�����"},
	{"����ɾ��ʧ��"},
};

/* ָ�ƹ������ */
static const rt_uint8_t FprintManageText[][LCD_LINE_MAX_LEN] =
{
	{"1.���"},
	{"2.ɾ��"},
};

/* ָ����ӽ��� */
static const rt_uint8_t FprintAddText[][LCD_LINE_MAX_LEN] =
{
	{"�û�ID"},
	{"�ɼ���ָ��"},
	{"��*����ʼ�ɼ�"},
	{"���ڲɼ�����..."},
	{"ָ�Ʋɼ�ʧ��"},
	{"��ָ���Ѵ���"},
	{"��ӳɹ�"},
	{"���ʧ��"},
};

/* ָ��ɾ������ */
static const rt_uint8_t FprintDelText[][LCD_LINE_MAX_LEN] =
{
	{"�û�ID"},
	{"�ɼ�Ҫɾ����ָ��"},
	{"�밴*����ʼ�ɼ�"},
	{"�ɼ�ʧ��"},
	{"���û�û�д�ָ��"},
	{"ɾ��ʧ��"},
};


/* �绰������� */
static const rt_uint8_t PhoneManageText[][LCD_LINE_MAX_LEN] =
{
	{"1.���"},
	{"2.ɾ��"},
};

/* �绰��ӽ��� */
static const rt_uint8_t PhoneAddText[][LCD_LINE_MAX_LEN] =
{
	{"�û�ID"},
	{"���������ֻ�����"},
	{"��������һ��"},
	{"�ֻ��ų��ȴ���"},
	{"�ú����Ѿ�����"},
	{"��ӳɹ�"},
	{"���ʧ��"},
	{"�������벻ƥ��"},
};

/* ɾ���绰���� */
static const rt_uint8_t PhoneDelText[][LCD_LINE_MAX_LEN] =
{
	{"�û�ID"},
	{"������"},
	{"��Ҫɾ�����ֻ���"},
	{"ɾ���ɹ�"},
	{"�ֻ��ų��ȴ���"},
	{"���û�û�д˺���"},
	{"ɾ��ʧ��"},
};


/**************************************************************************************************/

/** 
@brief  �˵��б���ʾ
				���
				ɾ��
				�޸�
@param  none
@retval none
*/
static void menu_list_show_ui(const rt_uint8_t list[][LCD_LINE_MAX_LEN],	
																								rt_uint8_t InPOS,
																								rt_uint8_t ListSize)
{
	rt_uint8_t page;
	rt_uint8_t pos;
	rt_uint8_t i;
	rt_int32_t CurUserPos;

	if(InPOS >= ListSize)
	{
		rt_kprintf("%s InPos > %d is Error",__FUNCTION__,ListSize);
		return ;
	}
	CurUserPos = account_cur_pos_get();
	page = InPOS /PAGE_MAX_SHOW_NUM;//������ʾ����һҳ
	pos = InPOS % PAGE_MAX_SHOW_NUM;//��ǰѡ�е�λ��
	gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
	for(i=0;i<PAGE_MAX_SHOW_NUM;i++)
	{
		if(page*PAGE_MAX_SHOW_NUM+i >= ListSize)
		{
			break;
		}
		if(CurUserPos == 0)
		{
      if(i == pos)
      {
      	// ��ǰѡ����
        gui_display_string(SHOW_X_CENTERED((const char*)list[page*PAGE_MAX_SHOW_NUM+i]),
                           SHOW_Y_LINE(i),
                           (rt_uint8_t *)list[page*PAGE_MAX_SHOW_NUM+i],
                           GUI_BLACK);
      }
      else
      {
        gui_display_string(SHOW_X_CENTERED((const char*)list[page*PAGE_MAX_SHOW_NUM+i]),
                           SHOW_Y_LINE(i),
                           (rt_uint8_t *)list[page*PAGE_MAX_SHOW_NUM+i],
                           GUI_WIHIT);
      }
		}
		else
		{
			if(i == pos)
			{
			  // ��ǰѡ����
				gui_display_string(SHOW_X_CENTERED((const char*)list[page*PAGE_MAX_SHOW_NUM+i]),
				                   SHOW_Y_LINE(i),
				                   (rt_uint8_t *)list[page*PAGE_MAX_SHOW_NUM+i],
				                   GUI_BLACK);
			}
			else
			{
			gui_display_string(SHOW_X_CENTERED((const char*)list[page*PAGE_MAX_SHOW_NUM+i]),
			                   SHOW_Y_LINE(i),
			                   (rt_uint8_t *)list[page*PAGE_MAX_SHOW_NUM+i],
			                   GUI_WIHIT);
			}
		}
	}
	gui_display_update();
}


/** 
@brief  ����������
				�������
				ָ�ƹ���
				�ֻ�����
@param  none
@retval none
*/
void password_manage_ui(void)
{
	menu_list_show_ui(UserManageListText,0,sizeof(UserManageListText)/LCD_LINE_MAX_LEN);
}


/** 
@brief  ָ�ƹ������
				�������
				ָ�ƹ���
				�ֻ�����

@param  none
@retval none
*/
void fprint_manage_ui(void)
{
  menu_list_show_ui(UserManageListText,1,sizeof(UserManageListText)/LCD_LINE_MAX_LEN);
}


/** 
@brief  �ֻ��Ź������
				�������
				ָ�ƹ���
				�ֻ�����

@param  none
@retval none
*/
void phone_manage_ui(void)
{
	menu_list_show_ui(UserManageListText,2,sizeof(UserManageListText)/LCD_LINE_MAX_LEN);
}


/** 
@brief  ���������ӽ���
				���
				ɾ��
@param  none
@retval none
*/
void password_add_ui(void)
{
	menu_list_show_ui(PasswordManageText,0,sizeof(PasswordManageText)/LCD_LINE_MAX_LEN);
}


/** 
@brief  �������ɾ������
				���
				ɾ��
@param  none
@retval none
*/
void password_del_ui(void)
{
	menu_list_show_ui(PasswordManageText,1,sizeof(PasswordManageText)/LCD_LINE_MAX_LEN);
}


/** 
@brief  ������Ӳ�ͬҳ����ʾ����
				���
				ɾ��
@param  buf ��ʾ������
@param  page ��ͬ����ʾ����
				@arg 0 �������������
				@arg 1 ������һ�εĽ���
				@arg 2 ��ʾ�����Ѵ���
				@arg 3 ��ʾ���볤�Ȳ���
				@arg 4 ¼��������ɹ�
				@arg 5 ¼��������ʧ��
				@arg 6 �������벻ƥ��
@retval none
*/
static void password_add_ui_page(rt_uint8_t *buf,rt_uint8_t page)
{
	rt_uint8_t CurUserPos;
	
	rt_uint8_t UserMaxPsNum;
  CurUserPos = account_cur_pos_get();
  
  UserMaxPsNum = user_valid_password_num();

	//��ʼ��������Ҫ����
  if((page == 1)||(page == 0))
  {
    gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
  }

	menu_clear_line(3);
  switch(page)
  {
		case 0:
		{
			//����������
			gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(0),PasswordAddText[0],GUI_WIHIT);
			rt_sprintf((char *)buf,"%03d",CurUserPos);
			gui_display_string(SHOW_X_ROW8(12),SHOW_Y_LINE(0),buf,GUI_WIHIT);

			//gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(1),PasswordAddText[1],GUI_WIHIT);
			//rt_sprintf((char *)buf,"%02d",UserMaxPsNum);
			//gui_display_string(SHOW_X_ROW8(13),SHOW_Y_LINE(1),buf,GUI_WIHIT);

			gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(1),PasswordAddText[2],GUI_WIHIT);
			break;
		}
		case 1:
		{
			//������һ��
	    gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(0),PasswordAddText[0],GUI_WIHIT);
			rt_sprintf((char *)buf,"%03d",CurUserPos);
	    gui_display_string(SHOW_X_ROW8(12),SHOW_Y_LINE(0),buf,GUI_WIHIT);
	    
	    //gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(1),PasswordAddText[1],GUI_WIHIT);
	    //rt_sprintf((char *)buf,"%02d",UserMaxPsNum);
	    //gui_display_string(SHOW_X_ROW8(13),SHOW_Y_LINE(1),buf,GUI_WIHIT);

	    gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(1),PasswordAddText[3],GUI_WIHIT);
			break;
		}
		case 2:
		{
			gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PasswordAddText[4],GUI_WIHIT);
			gui_display_update();
			menu_input_sure_key(RT_TICK_PER_SECOND*2);
			//gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(3),SHOW_X_ROW8(15),SHOW_Y_LINE(4));

			break;
		}
		case 3:
		{
			gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PasswordAddText[5],GUI_WIHIT);
			gui_display_update();
			menu_input_sure_key(RT_TICK_PER_SECOND*2);
			//gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(3),SHOW_X_ROW8(15),SHOW_Y_LINE(4));

			break;
		}
		case 4:
		{
			gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PasswordAddText[6],GUI_WIHIT);
			gui_display_update();
			menu_input_sure_key(RT_TICK_PER_SECOND*2);
			//gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(3),SHOW_X_ROW8(15),SHOW_Y_LINE(4));

			break;
		}
		case 5:
		{
			gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PasswordAddText[7],GUI_WIHIT);
			gui_display_update();
			menu_input_sure_key(RT_TICK_PER_SECOND*2);
			//gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(3),SHOW_X_ROW8(15),SHOW_Y_LINE(4));

			break;
		}
		case 6:
		{
			gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PasswordAddText[8],GUI_WIHIT);
			gui_display_update();
			menu_input_sure_key(RT_TICK_PER_SECOND*2);
			//gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(3),SHOW_X_ROW8(15),SHOW_Y_LINE(4));

			break;
		}
		default:
		{
			break;
		}
  }
  gui_display_update();
}


/** 
@brief  ����һ���ַ�����ȷ�����˳�
@param  ShowBuf   �Դ�
@param  DataBuf   ���ݻ�����
@param  InputLen  ��Ҫ���볤��
@param  StartLine ��ʾ��ʼ��
@param  ShowMode  ��ʾģʽ
				@arg 0    ��������ʾ
				@arg 1		������ʾ
@retval none
*/
static rt_err_t gui_input_string(rt_uint8_t *ShowBuf,
																		 	rt_uint8_t *DataBuf,
																		 	rt_uint8_t InputLen,
																		 	rt_uint8_t StartLine,
																		 	rt_uint8_t ShowMode)
{
	rt_uint8_t KeyValue; //��������ֵ
	rt_err_t   result;   //���ս����
	rt_err_t   FunResult; //�������н��
  rt_uint8_t GlintStatus;//��˸״̬

	while(1)
	{
		result = gui_key_input(&KeyValue);
		if(result == RT_EOK)
		{
			// ��Ч����
      if(KeyValue >= '0' && KeyValue <= '9')
      {
      	// ����0~9
        result = string_add_char(DataBuf,KeyValue,InputLen);
        if(result == RT_EOK)
        {
        	if(ShowMode == 1)
        	{
        		//�����ַ���
            string_hide_string((const rt_uint8_t *)DataBuf,ShowBuf,SHOW_PW_HIDE_CH,PASSWORD_MAX_LEN);
        	}
          
          gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(StartLine),SHOW_X_ROW8(15),SHOW_Y_LINE(StartLine+1));
          gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(StartLine),ShowBuf,GUI_WIHIT);
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
        result = string_del_char(DataBuf,PASSWORD_MAX_LEN);
        if(result == RT_EOK)
        {
        	if(ShowMode == 1)
        	{
        		//�����ַ���
            string_hide_string((const rt_uint8_t *)DataBuf,ShowBuf,SHOW_PW_HIDE_CH,PASSWORD_MAX_LEN);
        	}
          gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(StartLine),SHOW_X_ROW8(15),SHOW_Y_LINE(StartLine+1));
          gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(StartLine),ShowBuf,GUI_WIHIT);
        }
        else
        {
        	//û�������ַ�
          FunResult = RT_EEMPTY;
          break;
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
      GlintStatus++;
      // ��˸��ʾ
      menu_inputchar_glint(SHOW_X_ROW8(rt_strlen((const char *)ShowBuf)),SHOW_Y_LINE(StartLine),GlintStatus%2);
		}
		//������ʾ
		gui_display_update();
	}

	return FunResult;
}


/** 
@brief  ������Ӽ��
@param  password ����
@retval RT_EEMPTY ���볤�Ȳ���
				RT_ERROR  �����Ѵ���
				RT_EOK    ����Ϸ�
*/
static rt_err_t Password_add_check(rt_uint8_t *password)
{
	rt_err_t result = RT_EOK;
	rt_int32_t Kresult;

	if(rt_strlen((const char*)password) < CONFIG_PASSWORD_LEN)
	{
		//��������Ϊ��
		rt_kprintf("rt_strlen((const char*)password) = %d\n",rt_strlen((const char*)password));
		return RT_EEMPTY;
	}

	Kresult = device_config_key_verify(KEY_TYPE_KBOARD,password,rt_strlen((const char*)password));
	if(Kresult >= 0)
	{
		result = RT_ERROR;
	}
	
	return result;
}


/** 
@brief  ��������봦��
				����������
				������һ��
				��ʾ���
@param  none
@retval none
*/
void password_add_process(void)
{
	rt_uint8_t *ShowBuf; 		//��ʾ������
	rt_uint8_t *DataBuf; 		//��������buf
	rt_uint8_t *Password; 	//����
	rt_err_t   result;   		//���ս����
	
	// ��ȡ�ڴ�
	ShowBuf = rt_calloc(1,LCD_LINE_MAX_LEN);
	DataBuf = rt_calloc(1,LCD_LINE_MAX_LEN);
	Password = rt_calloc(1,PASSWORD_MAX_LEN);
	RT_ASSERT(ShowBuf != RT_NULL);
	RT_ASSERT(DataBuf != RT_NULL);
	RT_ASSERT(Password!= RT_NULL);

  while(1)
  {
  	// ��ʾ��ʼ������
 		password_add_ui_page(ShowBuf,0);
 		
		rt_memset(ShowBuf,0,LCD_LINE_MAX_LEN);
		rt_memset(DataBuf,0,LCD_LINE_MAX_LEN);

		result = gui_input_string(ShowBuf,DataBuf,PASSWORD_MAX_LEN,2,1);
		if(result != RT_EOK)
		{
			break;
		}
		else
		{
			// ��������Ƿ�Ϸ�
			result = Password_add_check(DataBuf);
      if(result != RT_EOK)
      {
      	// ���벻�Ϸ�
      	menu_operation_result_handle(1);

				if(result == RT_EEMPTY)
				{
					//��ʾ���볤�Ȳ���
					password_add_ui_page(ShowBuf,3);
				}
				else
				{
					// ��ʾ���벻�Ϸ�
					password_add_ui_page(ShowBuf,2);
				}
			
				continue;
      }
      else
      {
        // �������ǺϷ���
        rt_memcpy(Password,DataBuf,PASSWORD_MAX_LEN);
      }
		}

		// ��ʾ�ٴ���������
 		password_add_ui_page(ShowBuf,1);
 		
		rt_memset(ShowBuf,0,LCD_LINE_MAX_LEN);
		rt_memset(DataBuf,0,LCD_LINE_MAX_LEN);

		result = gui_input_string(ShowBuf,DataBuf,PASSWORD_MAX_LEN,2,1);
		if(result == RT_EEMPTY)
		{
			// ȡ���ڶ�����������
			continue;
		}
		else if(result == RT_ETIMEOUT)
		{
			// ��ʱ
			break;
		}
		else if(result == RT_EOK)
		{
			// ����������ȷ
			if(rt_memcmp(Password,DataBuf,rt_strlen((const char*)Password)) == 0)
			{
		    result = key_add_password(Password);
		    if(result == RT_EOK)
		    {
					result = account_cur_add_password(Password);
			    if(result == RT_EOK)
			    {
						// ¼��ɹ�
						password_add_ui_page(ShowBuf,4);
			    }
			    else
			    {
            password_add_ui_page(ShowBuf,5);
			    }
		    }
		    else
		    {
					password_add_ui_page(ShowBuf,5);
		    }
			}
			else
			{
				// �����������벻һ��
				password_add_ui_page(ShowBuf,6);
			}
		}
  }

	
	
	// �ͷ��ڴ�
	rt_free(ShowBuf);
	rt_free(DataBuf);	
	rt_free(Password);
}


/** 
@brief  ����ɾ����ͬҳ����ʾ����
@param  buf �Դ�
@param  page ��ͬҳ��
				@arg 0 ��ʾ����Ҫɾ������Ľ���
				@arg 1 ��ʾ����ɾ���ɹ�
				@arg 2 ��ʾ���볤�ȴ���
				@arg 3 ��ʾ���벻����
				@arg 4 ��ʾɾ��ʧ��
@retval none
*/
static void password_del_ui_page(rt_uint8_t *buf,rt_uint8_t page)
{
	rt_uint8_t CurUserPos;
	
	rt_uint8_t UserMaxPsNum;
  CurUserPos = account_cur_pos_get();
  
  UserMaxPsNum = user_valid_password_num();

	// ��ʼ��������Ҫ����
  if((page == 0))
  {
    gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
  }

	menu_clear_line(3);
  switch(page)
  {
		case 0:
		{
			// ��ʾ����Ҫɾ�����������
			gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(0),PasswordDelText[0],GUI_WIHIT);
			rt_sprintf((char *)buf,"%03d",CurUserPos);
			gui_display_string(SHOW_X_ROW8(12),SHOW_Y_LINE(0),buf,GUI_WIHIT);

			gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(1),PasswordDelText[1],GUI_WIHIT);
			gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(2),PasswordDelText[2],GUI_WIHIT);
			break;
		}
		case 1:
		{
			// ��ʾ����ɾ���ɹ�
			gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PasswordDelText[3],GUI_WIHIT);
			gui_display_update();
			menu_input_sure_key(RT_TICK_PER_SECOND*2);

			break;
		}
		case 2:
		{
			// ��ʾ���볤�ȴ���
			gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PasswordDelText[4],GUI_WIHIT);
			gui_display_update();
			menu_input_sure_key(RT_TICK_PER_SECOND*2);
			break;
		}
		case 3:
		{ 
			// ��ʾ���벻����
			gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PasswordDelText[5],GUI_WIHIT);
			gui_display_update();
			menu_input_sure_key(RT_TICK_PER_SECOND*2);
			break;
		}
		case 4:
		{ 
			// ��ʾ���벻����
			gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PasswordDelText[6],GUI_WIHIT);
			gui_display_update();
			menu_input_sure_key(RT_TICK_PER_SECOND*2);
			break;
		}
		default:
		{
			break;
		}
  }
  gui_display_update();

}


/** 
@brief  ɾ��������
@param  none
@retval none
*/
static rt_err_t password_del_check(rt_uint8_t *password)
{
	rt_err_t result = RT_ERROR;
	rt_int32_t Kresult;
  rt_uint8_t CurUserPos;
  struct account_head *ah;

  CurUserPos = account_cur_pos_get();

	if(rt_strlen((const char*)password) < CONFIG_PASSWORD_LEN)
	{
		// ��������벻��Ϊ��
		rt_kprintf("rt_strlen((const char*)password) = %d\n",rt_strlen((const char*)password));
		return RT_EEMPTY;
	}

	Kresult = device_config_key_verify(KEY_TYPE_KBOARD,password,rt_strlen((const char*)password));
	if(Kresult >= 0)
	{
		rt_uint8_t i;
		
		ah = rt_calloc(1,sizeof(*ah));
		RT_ASSERT(ah != RT_NULL);

		device_config_account_operate(CurUserPos,ah,0);

		for(i=0;i<ACCOUNT_KEY_NUMBERS;i++)
		{
			if(ah->key[i] == Kresult)
			{
				result = RT_EOK;
				break;
			}
		}
		rt_free(ah);
	}
	
	return result;
}


/** 
@brief  ɾ�������봦��
				����Ҫɾ������
				��ʾ���
@param  none
@retval none
*/
void password_del_process(void)
{
	rt_uint8_t *ShowBuf; 		//��ʾ������
	rt_uint8_t *DataBuf; 		//��������buf
  rt_err_t   result;      //���ս����

	// ��ȡ�ڴ�
	ShowBuf = rt_calloc(1,LCD_LINE_MAX_LEN);
	DataBuf = rt_calloc(1,LCD_LINE_MAX_LEN);
	RT_ASSERT(ShowBuf != RT_NULL);
	RT_ASSERT(DataBuf != RT_NULL);

	while(1)
	{
    password_del_ui_page(ShowBuf,0);

    rt_memset(ShowBuf,0,LCD_LINE_MAX_LEN);
		rt_memset(DataBuf,0,LCD_LINE_MAX_LEN);

		result = gui_input_string(ShowBuf,DataBuf,PASSWORD_MAX_LEN,3,1);
		if(result != RT_EOK)
		{
			// ��ʱ���߷���
			break;
		}
		else
		{
			result = password_del_check(DataBuf);
			if(result == RT_EEMPTY)
			{
				//���볤�ȴ���
				password_del_ui_page(ShowBuf,2);
			}
			else if(result == RT_ERROR)
			{
				//û���������
				password_del_ui_page(ShowBuf,3);
			}
			else if(result == RT_EOK)
			{
				//����ɾ��
				result = key_password_str_delete(DataBuf);
				if(result != RT_EOK)
				{
					//ɾ��ʧ��
					password_del_ui_page(ShowBuf,4);
					
				}
				password_del_ui_page(ShowBuf,1);
			}
			
		}
	}
	

	// �ͷ��ڴ�
	rt_free(ShowBuf);
	rt_free(DataBuf);	
}


/** 
@brief  ָ����Ӳ�ͬҳ����ʾ����
				���
				ɾ��
@param  buf ��ʾ������
@param  page ��ͬ����ʾ����
				@arg 0 ָ�Ʋɼ���ʼ����
				@arg 1 ָ�������ɼ�����
				@arg 2 �ɼ�����ʧ��
				@arg 3 ��ָ���Ѵ���
				@arg 4 ��ӳɹ�
				@arg 5 ���ʧ��
@retval none
*/
static void fprint_add_ui_page(rt_uint8_t *buf,rt_uint8_t page)
{
	rt_uint8_t CurUserPos;
	
	rt_uint8_t UserMaxPsNum;
  CurUserPos = account_cur_pos_get();
  
  UserMaxPsNum = user_valid_password_num();

	//��ʼ��������Ҫ����
  if((page == 0))
  {
    gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
  }

	menu_clear_line(3);
  switch(page)
  {
		case 0:
		{
			// ��ָ�Ʋɼ�����
			gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(0),FprintAddText[0],GUI_WIHIT);
			rt_sprintf((char *)buf,"%03d",CurUserPos);
			gui_display_string(SHOW_X_ROW8(12),SHOW_Y_LINE(0),buf,GUI_WIHIT);
			
			gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(1),FprintAddText[1],GUI_WIHIT);
			
			gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(3),FprintAddText[2],GUI_WIHIT);
			break;
		}
		case 1:
		{
			// ָ�����ɼ�����
	    gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),FprintAddText[3],GUI_WIHIT);
			gui_display_update();
			menu_input_sure_key(RT_TICK_PER_SECOND*2);
			break;
		}
		case 2:
		{ 
			// �ɼ�����ʧ��
			gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),FprintAddText[4],GUI_WIHIT);
			gui_display_update();
			menu_input_sure_key(RT_TICK_PER_SECOND*2);
			break;
		}
		case 3:
		{ 
			// ��ָ���Ѵ���
			gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),FprintAddText[5],GUI_WIHIT);
			gui_display_update();
			menu_input_sure_key(RT_TICK_PER_SECOND*2);
			break;
		}
		case 4:
		{ 
			// ��ʾ��ӳɹ�
			gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),FprintAddText[6],GUI_WIHIT);
			gui_display_update();
			menu_input_sure_key(RT_TICK_PER_SECOND*2);
			break;
		}
		case 5:
		{ 
			// ��ʾ���ʧ��
			gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),FprintAddText[7],GUI_WIHIT);
			gui_display_update();
			menu_input_sure_key(RT_TICK_PER_SECOND*2);
			break;
		}
		default:
		{
			break;
		}
  }
  gui_display_update();
}


/** 
@brief  ָ����ӽ���
@param  none
@retval none
*/
void fprint_add_ui(void)
{
	menu_list_show_ui(FprintManageText,0,sizeof(FprintManageText)/LCD_LINE_MAX_LEN);
}


/** 
@brief  ָ����ӽ���
@param  none
@retval none
*/
void fprint_del_ui(void)
{
  menu_list_show_ui(FprintManageText,1,sizeof(FprintManageText)/LCD_LINE_MAX_LEN);
}

/** 
@brief  ָ����ӽ���
@param  none
@retval none
*/
void fprint_add_process(void)
{
	rt_uint8_t *ShowBuf; 		//��ʾ������
	rt_uint8_t KeyValue;		//����ֵ
	rt_err_t   result;			//���
	// ��ȡ�ڴ�
	ShowBuf = rt_calloc(1,LCD_LINE_MAX_LEN);
	
	while(1)
	{
		
		// ָ�Ʋɼ���ʼ������
		fprint_add_ui_page(ShowBuf,0);

		result = gui_key_input(&KeyValue);
		if(result == RT_EOK)
		{
			
		}
	}

	// �ͷ��ڴ�
	rt_free(ShowBuf);
}


/** 
@brief  ָ����ӽ���
@param  none
@retval none
*/
void fprint_del_process(void)
{

}


/** 
@brief  �ֻ�����ӽ���
@param  none
@retval none
*/
void phone_add_ui(void)
{
	menu_list_show_ui(PhoneManageText,0,sizeof(PhoneManageText)/LCD_LINE_MAX_LEN);
}


/** 
@brief  �ֻ�ɾ������
@param  none
@retval none
*/
void phone_del_ui(void)
{
	menu_list_show_ui(PhoneManageText,1,sizeof(PhoneManageText)/LCD_LINE_MAX_LEN);
}


/** 
@brief  ������Ӳ�ͬҳ����ʾ����
				���
				ɾ��
@param  buf ��ʾ������
@param  page ��ͬ����ʾ����
				@arg 0 �������ֻ��������
				@arg 1 ������һ���ֻ�����Ľ���
				@arg 2 ������ֻ����볤�Ȳ���
				@arg 3 ������ֻ������Ѿ�����
				@arg 4 �ֻ�����ӳɹ�
				@arg 5 �ֻ������ʧ��
				@arg 6 �������벻ƥ��
@retval none
*/
static void phone_add_ui_page(rt_uint8_t *buf,rt_uint8_t page)
{
	rt_uint8_t CurUserPos;
	
	rt_uint8_t UserMaxPsNum;
  CurUserPos = account_cur_pos_get();
  
  UserMaxPsNum = user_valid_password_num();

	// ��ʼ��������Ҫ����
  if((page == 1)||(page == 0))
  {
    gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
  }

	// ���������
	menu_clear_line(3);
  switch(page)
  {
		case 0:
		{
			//����������
			gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(0),PhoneAddText[0],GUI_WIHIT);
			rt_sprintf((char *)buf,"%03d",CurUserPos);
			gui_display_string(SHOW_X_ROW8(12),SHOW_Y_LINE(0),buf,GUI_WIHIT);
			
			gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(1),PhoneAddText[1],GUI_WIHIT);
			break;
		}
		case 1:
		{
			//������һ��
	    gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(0),PhoneAddText[0],GUI_WIHIT);
			rt_sprintf((char *)buf,"%03d",CurUserPos);
	    gui_display_string(SHOW_X_ROW8(12),SHOW_Y_LINE(0),buf,GUI_WIHIT);

	    gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(1),PhoneAddText[2],GUI_WIHIT);
			break;
		}
		case 2:
		{ 
			// ���ȴ���
			gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PhoneAddText[3],GUI_WIHIT);
			gui_display_update();
			menu_input_sure_key(RT_TICK_PER_SECOND*2);
			break;
		}
		case 3:
		{ 
			// ��ʾ�ֻ����Ѿ�����
			gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PhoneAddText[4],GUI_WIHIT);
			gui_display_update();
			menu_input_sure_key(RT_TICK_PER_SECOND*2);
			break;
		}
		case 4:
		{ 
			// ��ʾ��ӳɹ�
			gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PhoneAddText[5],GUI_WIHIT);
			gui_display_update();
			menu_input_sure_key(RT_TICK_PER_SECOND*2);
			break;
		}
		case 5:
		{ 
			// ��ʾ���ʧ��
			gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PhoneAddText[6],GUI_WIHIT);
			gui_display_update();
			menu_input_sure_key(RT_TICK_PER_SECOND*2);
			break;
		}
		case 6:
		{ 
			// ��ʾ�������벻ƥ��
			gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PhoneAddText[7],GUI_WIHIT);
			gui_display_update();
			menu_input_sure_key(RT_TICK_PER_SECOND*2);
			break;
		}
		default:
		{
			break;
		}
  }
  gui_display_update();
}


/** 
@brief  �ֻ�������Ӵ���
@param  none
@retval none
*/
void phone_add_process(void)
{
	rt_uint8_t *ShowBuf; 		//��ʾ������
	rt_uint8_t *DataBuf; 		//��������buf
	rt_uint8_t *phone; 			//�ֻ���
	rt_err_t   result;   		//���ս����
	
	// ��ȡ�ڴ�
	ShowBuf = rt_calloc(1,LCD_LINE_MAX_LEN);
	DataBuf = rt_calloc(1,LCD_LINE_MAX_LEN);
	phone = rt_calloc(1,PHONE_MAX_LEN);
	RT_ASSERT(ShowBuf != RT_NULL);
	RT_ASSERT(DataBuf != RT_NULL);
	RT_ASSERT(phone != RT_NULL);

  while(1)
  {
  	// ��ʾ��ʼ������
 		phone_add_ui_page(ShowBuf,0);
 		
		rt_memset(ShowBuf,0,LCD_LINE_MAX_LEN);
		rt_memset(DataBuf,0,LCD_LINE_MAX_LEN);

		result = gui_input_string(ShowBuf,DataBuf,PHONE_MAX_LEN,2,0);
		if(result != RT_EOK)
		{
			break;
		}
		else
		{
			// ����ֻ��Ƿ�Ϸ�
			result = user_phone_add_check(DataBuf);
      if(result != RT_EOK)
      {
      	// ������ֻ��Ų��Ϸ�
      	menu_operation_result_handle(1);

				if(result == RT_EEMPTY)
				{
					// ��ʾ�ֻ��ų��Ȳ���
					phone_add_ui_page(ShowBuf,2);
				}
				else
				{
					// ��ʾ�ֻ����Ѿ�����
					phone_add_ui_page(ShowBuf,3);
				}
			
				continue;
      }
      else
      {
        // �ֻ��źϷ�
        rt_memcpy(phone,DataBuf,PHONE_MAX_LEN);
      }
		}

		// ��ʾ�ٴ������ֻ���
 		phone_add_ui_page(ShowBuf,1);
 		
		rt_memset(ShowBuf,0,LCD_LINE_MAX_LEN);
		rt_memset(DataBuf,0,LCD_LINE_MAX_LEN);

		result = gui_input_string(ShowBuf,DataBuf,PHONE_MAX_LEN,2,0);
		if(result == RT_EEMPTY)
		{
			// ȡ���ڶ��������ֻ���
			continue;
		}
		else if(result == RT_ETIMEOUT)
		{
			// ��ʱ
			break;
		}
		else if(result == RT_EOK)
		{
			// �����ֻ������
			if(rt_memcmp(phone,DataBuf,rt_strlen(phone)) == 0)
			{
		    result = user_cur_add_phone(phone);
		    if(result == RT_EOK)
		    {
					// ��ӳɹ�
					phone_add_ui_page(ShowBuf,4);
		    }
		    else
		    {
		    	// ���ʧ��
					phone_add_ui_page(ShowBuf,5);
		    }
			}
			else
			{
				// �������벻һ��
				phone_add_ui_page(ShowBuf,6);
			}
		}
  }

	// �ͷ��ڴ�
	rt_free(ShowBuf);
	rt_free(DataBuf);	
	rt_free(phone);

}


/** 
@brief  �绰ɾ����ͬҳ����ʾ����
@param  buf �Դ�
@param  page ��ͬҳ��
				@arg 0 ��ʾ����Ҫɾ���绰��Ľ���
				@arg 1 ��ʾ�ֻ���ɾ���ɹ�
				@arg 2 ��ʾ�����ֻ��ų��ȴ���
				@arg 3 ��ʾ�ֻ��Ų�����
				@arg 4 ��ʾɾ��ʧ��
@retval none
*/
static void phone_del_ui_page(rt_uint8_t *buf,rt_uint8_t page)
{
	rt_uint8_t CurUserPos;
	
	rt_uint8_t UserMaxPsNum;
  CurUserPos = account_cur_pos_get();
  
  UserMaxPsNum = user_valid_password_num();

	// ��ʼ��������Ҫ����
  if((page == 0))
  {
    gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
  }

	gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(3),SHOW_X_ROW8(15),SHOW_Y_LINE(4));
  switch(page)
  {
		case 0:
		{
			// ��ʾ����Ҫɾ�����������
			gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(0),PhoneDelText[0],GUI_WIHIT);
			rt_sprintf((char *)buf,"%03d",CurUserPos);
			gui_display_string(SHOW_X_ROW8(12),SHOW_Y_LINE(0),buf,GUI_WIHIT);

			gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(1),PhoneDelText[1],GUI_WIHIT);
			gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(2),PhoneDelText[2],GUI_WIHIT);
			break;
		}
		case 1:
		{
			// ��ʾ����ɾ���ɹ�
			gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PhoneDelText[3],GUI_WIHIT);
			gui_display_update();
			menu_input_sure_key(RT_TICK_PER_SECOND*2);

			break;
		}
		case 2:
		{
			// ��ʾ���볤�ȴ���
			gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PhoneDelText[4],GUI_WIHIT);
			gui_display_update();
			menu_input_sure_key(RT_TICK_PER_SECOND*2);
			break;
		}
		case 3:
		{ 
			// ��ʾ���벻����
			gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PhoneDelText[5],GUI_WIHIT);
			gui_display_update();
			menu_input_sure_key(RT_TICK_PER_SECOND*2);
			break;
		}
		case 4:
		{ 
			// ��ʾɾ��ʧ��
			gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PhoneDelText[6],GUI_WIHIT);
			gui_display_update();
			menu_input_sure_key(RT_TICK_PER_SECOND*2);
			break;
		}
		default:
		{
			break;
		}
  }
  gui_display_update();

}


/** 
@brief  ɾ���ֻ����
@param  none
@retval none
*/
static rt_err_t phone_del_check(rt_uint8_t *phone)
{
	rt_err_t result = RT_ERROR;
	rt_int32_t PHresult;
  rt_uint8_t CurUserPos;
  struct account_head *ah;

  CurUserPos = account_cur_pos_get();

	if(rt_strlen((const char*)phone) < CONFIG_PHONE_LET)
	{
		// ������ֻ�����Ϊ��
		rt_kprintf("rt_strlen((const char*)phone) = %d\n",rt_strlen((const char*)phone));
		return RT_EEMPTY;
	}

	PHresult = device_config_phone_verify(phone,rt_strlen((const char*)phone));
	if(PHresult >= 0)
	{
		rt_uint8_t i;
		
		ah = rt_calloc(1,sizeof(*ah));
		RT_ASSERT(ah != RT_NULL);

		device_config_account_operate(CurUserPos,ah,0);

		for(i=0;i<ACCOUNT_KEY_NUMBERS;i++)
		{
			if(ah->phone[i] == PHresult)
			{
				result = RT_EOK;
				break;
			}
		}
		rt_free(ah);
	}
	
	return result;
}


/** 
@brief  �ֻ�����ɾ������
@param  none
@retval none
*/
void phone_del_process(void)
{
	rt_uint8_t *ShowBuf; 		//��ʾ������
	rt_uint8_t *DataBuf; 		//��������buf
  rt_err_t   result;      //���ս����

	// ��ȡ�ڴ�
	ShowBuf = rt_calloc(1,LCD_LINE_MAX_LEN);
	DataBuf = rt_calloc(1,LCD_LINE_MAX_LEN);
	RT_ASSERT(ShowBuf != RT_NULL);
	RT_ASSERT(DataBuf != RT_NULL);

	while(1)
	{
    phone_del_ui_page(ShowBuf,0);

    rt_memset(ShowBuf,0,LCD_LINE_MAX_LEN);
		rt_memset(DataBuf,0,LCD_LINE_MAX_LEN);

		result = gui_input_string(ShowBuf,DataBuf,PHONE_MAX_LEN,3,1);
		if(result != RT_EOK)
		{
			// ��ʱ���߷���
			break;
		}
		else
		{
			result = phone_del_check(DataBuf);
			if(result == RT_EEMPTY)
			{
				//���볤�ȴ���
				phone_del_ui_page(ShowBuf,2);
			}
			else if(result == RT_ERROR)
			{
				//û���������
				phone_del_ui_page(ShowBuf,3);
			}
			else if(result == RT_EOK)
			{
				//����ɾ��
				result = user_phone_string_delete(DataBuf);
				if(result != RT_EOK)
				{
					//ɾ��ʧ��
					phone_del_ui_page(ShowBuf,4);
					
				}
				phone_del_ui_page(ShowBuf,1);
			}
			
		}
	}
	

	// �ͷ��ڴ�
	rt_free(ShowBuf);
	rt_free(DataBuf);	
}




