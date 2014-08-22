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

#define STRING_BT_MAC							"MAC:"
#define STRING_DEVICE_ID					"ID :"

#define STRING_USER_NO						"�û����:"
#define STRING_NEW_PASSWORD 			"����������:"
#define STRING_NEW_REPASSWORD		 	"������һ��:"
#define SHOW_PW_HIDE_CH						'*'
#define SHOW_EXIT_HINT						"�����뷵�ؼ��˳�"
#define SHOW_INPUT_ERR						"�������!!!"
#define SHOW_ADD_PS_ERR						"���ʧ��!!!"

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
    gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(0),STRING_USER_NO,GUI_WIHIT);
    gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(1),STRING_NEW_PASSWORD,GUI_WIHIT);
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
    				gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),SHOW_INPUT_ERR,GUI_WIHIT);
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
         		gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),SHOW_EXIT_HINT,GUI_WIHIT);
         		gui_display_update();
						return ;
         	}
        }
      }
      else
      {
        //��˸��ʾ
        menu_inputchar_glint(SHOW_X_ROW8(rt_strlen(ShowBuf)),SHOW_Y_LINE(2),GlintStatus%2);
        GlintStatus++;
      }
      //������ʾ
      
      gui_display_update();
    }
    
    //�ڶ�����������
    rt_memset(buf,0,MENU_PASSWORD_MAX_LEN);
    rt_memset(ShowBuf,0,MENU_PASSWORD_MAX_LEN);
    gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
    gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(0),STRING_USER_NO,GUI_WIHIT);
    gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(1),STRING_NEW_REPASSWORD,GUI_WIHIT);
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
          
          temp = rt_memcmp(buf1,buf,rt_strlen(buf));
          if(temp != 0)
          {
            //����ƥ�����
            gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),SHOW_ADD_PS_ERR,GUI_WIHIT);
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
        menu_inputchar_glint(SHOW_X_ROW8(rt_strlen(ShowBuf)),SHOW_Y_LINE(2),GlintStatus%2);
        GlintStatus++;
      }
      //������ʾ
      
      gui_display_update();
    }

	}
}

void menu_15_processing(void)
{
  gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
  
  gui_display_update();
}

void menu_16_processing(void)
{
  gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
  
  gui_display_update();
}

void menu_17_processing(void)
{
  gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
  
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
  
  gui_display_update();
}










