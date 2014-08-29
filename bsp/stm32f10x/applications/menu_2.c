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
#define PAGE_MAX_SHOW_NUM					4
#define LCD_LINE_MAX_LEN					17							//����һ����������λ��
#define PHONE_STAND_LEN						11

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
	{"�½�����"},
	{"�½�ָ��"},
	{"�½��绰"},
	{"�����˳�"},
	{"�û���Ϣ"},
	{">>>>�˳�"},
};


static void menu_user_add_ui(rt_uint8_t InPOS)
{
	rt_uint8_t page;
	rt_uint8_t pos;
	rt_uint8_t i;

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
	if(password[0] == 0)
	{
		//��������Ϊ��
		return RT_ERROR;
	}
	
	return RT_EOK;
}

//�������봦��
void menu_14_processing(void)
{
	rt_uint8_t buf[MENU_PASSWORD_MAX_LEN];
	rt_uint8_t buf1[MENU_PASSWORD_MAX_LEN];
	rt_uint8_t ShowBuf[MENU_PASSWORD_MAX_LEN];
	rt_uint8_t GlintStatus;

	while(1)
	{
    gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
    gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(0),PasswordAddText[0],GUI_WIHIT);
    gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(1),PasswordAddText[1],GUI_WIHIT);
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
        else if(KeyValue == '*')
        {
          //�������������Ƿ�Ϸ�
          result = add_new_password_check(buf);
          if(result != RT_EOK)
          {
          	//���벻�Ϸ�
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
        else if(KeyValue == '#')
        {
          rt_kprintf("ɾ��\nn");
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
        //��˸��ʾ
        GlintStatus++;
        menu_inputchar_glint(SHOW_X_ROW8(rt_strlen((const char *)ShowBuf)),SHOW_Y_LINE(2),GlintStatus%2);
      }
      //������ʾ
      
      gui_display_update();
    }
    
    //�ڶ�����������
    rt_memset(buf,0,MENU_PASSWORD_MAX_LEN);
    rt_memset(ShowBuf,0,MENU_PASSWORD_MAX_LEN);
    gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
    gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(0),PasswordAddText[0],GUI_WIHIT);
    gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(1),PasswordAddText[2],GUI_WIHIT);
    gui_display_update();
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
        else if(KeyValue == '*')
        {
          //�������������Ƿ�Ϸ�
          rt_uint32_t temp;
          
          temp = rt_memcmp(buf1,buf,rt_strlen((const char *)buf));
          if(temp != 0)
          {
            //����ƥ�����
            gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PasswordAddText[3],GUI_WIHIT);
    				gui_display_update();
    				rt_thread_delay(RT_TICK_PER_SECOND);
    				gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(3),SHOW_X_ROW8(15),SHOW_Y_LINE(4));
          }
          else
          {
            //��ӳɹ��������
          }
          //������������� ������֤��
          break;
        }
        else if(KeyValue == '#')
        {
          rt_kprintf("ɾ��\nn");
          result = string_del_char(buf,8);
          if(result == RT_EOK)
          {
            string_hide_string((const rt_uint8_t *)buf,ShowBuf,SHOW_PW_HIDE_CH,8);
            gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(2),SHOW_X_ROW8(15),SHOW_Y_LINE(3));
            gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(2),ShowBuf,GUI_WIHIT);
          }
          else
          {
          	rt_kprintf("�˳�Կ�����\n");
						//return ;
          }
        }
      }
      else
      {
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
	rt_uint8_t FrintNum;
	rt_err_t	 result;
	rt_uint8_t buf[LCD_LINE_MAX_LEN];
	
  //gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
  //��ȡ��ǰָ������
  FrintNum = 0;
  while(1)
  {
  	gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
    gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(0),FrintAddText[0],GUI_WIHIT);
    gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(1),FrintAddText[1],GUI_WIHIT);
    rt_sprintf((char *)buf,"%d",FrintNum);
    gui_display_string(SHOW_X_ROW8(rt_strlen((const char *)FrintAddText[1])),SHOW_Y_LINE(1),buf,GUI_WIHIT);
    gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(2),FrintAddText[2],GUI_WIHIT);
    gui_display_update();
		while(1)
		{
			result = gui_key_input(&KeyValue);
			if(result == RT_EOK)
			{
				if(KeyValue == '*')
				{
					//�ɼ�ָ��
					gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(2),SHOW_X_ROW8(15),SHOW_Y_LINE(3));
					gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(2),FrintAddText[3],GUI_WIHIT);
					gui_display_update();
					result = RT_EOK;
					if(result == RT_EOK)
					{
						//ָ�Ʋɼ��ɹ�
						rt_thread_delay(100);
						gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(2),SHOW_X_ROW8(15),SHOW_Y_LINE(3));
						gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(2),MenuCommText[1],GUI_WIHIT);
						gui_display_update();
						rt_thread_delay(100);
						FrintNum++;
						break;
					}
					else
					{
						//ָ�Ʋɼ�ʧ��
					}
				}
				else if(KeyValue == '#')
				{
					gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),MenuCommText[0],GUI_WIHIT);
         	gui_display_update();
					return ;
				}
			}
			else
			{

			}
		}
		rt_thread_delay(1);
  }
  //gui_display_update();
}

rt_err_t add_new_phone_check(rt_uint8_t phone[])
{
	rt_uint8_t len;

	len = rt_strlen((const char *)phone);
	if(len < PHONE_STAND_LEN)
	{
		return RT_ERROR;
	}	
	
	return RT_EOK;
}
//����ֻ�����
void menu_16_processing(void)
{
	rt_uint8_t PhoneNum = 0;
	rt_uint8_t buf[LCD_LINE_MAX_LEN];
	rt_uint8_t PhoneBuf[MENU_PHONE_MAX_LEN+1];
	rt_uint8_t KeyValue;
	rt_uint8_t result;
	rt_uint8_t GlintStatus;
	
  //gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);

  while(1)
  {
  	gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
    gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(0),PhoneAddText[0],GUI_WIHIT);
    gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(1),PhoneAddText[1],GUI_WIHIT);
    rt_sprintf((char *)buf,"%d",PhoneNum);
    gui_display_string(SHOW_X_ROW8(rt_strlen((const char *)PhoneAddText[1])),SHOW_Y_LINE(1),buf,GUI_WIHIT);
    gui_display_update();

    rt_memset(PhoneBuf,0,MENU_PHONE_MAX_LEN);
		while(1)
		{
			result = gui_key_input(&KeyValue);
      if(RT_EOK == result)
      {
        //�а���
        if(KeyValue >= '0' && KeyValue <= '9')
        {
          result = string_add_char(PhoneBuf,KeyValue,MENU_PHONE_MAX_LEN);
          if(result == RT_EOK)
          {
            gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(2),SHOW_X_ROW8(15),SHOW_Y_LINE(3));
            gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(2),PhoneBuf,GUI_WIHIT);
          }
          else
          {
            //������������8��
          }
        }
        else if(KeyValue == '*')
        {
          //�������������Ƿ�Ϸ�
          result = add_new_phone_check(PhoneBuf);
          if(result != RT_EOK)
          {
          	//���벻�Ϸ�
    				gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PhoneAddText[2],GUI_WIHIT);
    				gui_display_update();
    				rt_thread_delay(RT_TICK_PER_SECOND);
    				gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(3),SHOW_X_ROW8(15),SHOW_Y_LINE(4));
          }
          else
          {
            //�������ǺϷ���
            //rt_memcpy(buf1,buf,MENU_PASSWORD_MAX_LEN);
            break;
          }
          //������������� ������֤��
        }
        else if(KeyValue == '#')
        {
          rt_kprintf("ɾ��\nn");
          result = string_del_char(PhoneBuf,MENU_PHONE_MAX_LEN);
          if(result == RT_EOK)
          {
            gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(2),SHOW_X_ROW8(15),SHOW_Y_LINE(3));
            gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(2),PhoneBuf,GUI_WIHIT);
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
        //��˸��ʾ
        GlintStatus++;
        menu_inputchar_glint(SHOW_X_ROW8(rt_strlen((const char *)PhoneBuf)),SHOW_Y_LINE(2),GlintStatus%2);
      }
      //������ʾ
      
      gui_display_update();
			rt_thread_delay(1);
		}
		rt_thread_delay(1);
  }
  //gui_display_update();
}

//���沢�˳�
void menu_17_processing(void)
{
  gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
	gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(0),SaveEixtText[0],GUI_WIHIT);
	gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(1),SaveEixtText[1],GUI_WIHIT);
	gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(2),SaveEixtText[2],GUI_WIHIT);
  gui_display_update();
}

void menu_18_processing(void)
{
  gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
  
  gui_display_update();
}

void menu_19_processing(void)
{
  gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
  gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(0),EixtText[0],GUI_WIHIT);
	gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(1),EixtText[1],GUI_WIHIT);
  gui_display_update();
}

rt_err_t search_user_id_check(rt_uint8_t *buf,rt_uint8_t *user)
{
	sscanf((const char*)buf,"%s",user);
	if(rt_strlen((const char *)buf) == 0)
	{
		return RT_ERROR;
	}
	
	return RT_EOK;
}

static  rt_uint8_t  TextUser[100][LCD_LINE_MAX_LEN] = 
{
	{"user1"},
	{"user2"},
	{"user3"},
	{"user5"},
	{"user6"},
	{"user7"},
	{"user8"},
	{"user9"},
};
static void user_list_processing(void)
{
	rt_uint8_t KeyValue;
	rt_err_t	 result;
	rt_uint8_t buf[LCD_LINE_MAX_LEN];
	rt_uint8_t ShowStart = 0;
	rt_uint8_t i;
	
	gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);

	//��ʼ����������
	for(i= 0;i<100;i++)
	{
		rt_memset(buf,0,LCD_LINE_MAX_LEN);
		rt_sprintf(buf,"User_%d",i);
		rt_strncpy(TextUser[i],buf,LCD_LINE_MAX_LEN);
	}
	while(1)
	{
    result = gui_key_input(&KeyValue);
    if(result == RT_EOK)
    {
      if(KeyValue == '8')
      {
        //��
      }
      else if(KeyValue == '0')
      {
        //��
      }
      else if(KeyValue == '7')
      {
        //��
        if(ShowStart > 4)
        {
					ShowStart-=4;
        }
      }
      else if(KeyValue == '9')
      {
        //��
        if(ShowStart < 100)
        {
          ShowStart+=4;
        }
      }
      else if(KeyValue == '*')
      {
        //ȷ��
      }
      else if(KeyValue == '#')
      {
        //ȡ��
      }
    }
    for(i = 0;i<4;i++)
			{

				rt_memset(buf,0,LCD_LINE_MAX_LEN);
				rt_sprintf(buf,"%03d",ShowStart+i);
				gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(i),buf,GUI_WIHIT);
				
				rt_memset(buf,0,LCD_LINE_MAX_LEN);
				rt_strncpy(buf,TextUser[ShowStart+i],LCD_LINE_MAX_LEN);
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
  rt_memset(buf,0,LCD_LINE_MAX_LEN);
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
	    else if(KeyValue == '*')
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
							if(KeyValue == '*')
							{
								//�����������
								user_list_processing();
							}
							else if(KeyValue == '#')
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
	       	rt_kprintf("User Pos=%d\n",UserPos);
	        break;
	      }
	      //������������� ������֤��
	    }
	    else if(KeyValue == '#')
	    {
	      rt_kprintf("ɾ��\nn");
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

//�������
void menu_23_processing(void)
{
  gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
  
  gui_display_update();
}

//�޸Ľ���








