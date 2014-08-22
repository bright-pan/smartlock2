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
 							处理第三级目录下的菜单
 							
 * Copyright (C) 2013 Yuettak Co.,Ltd
 ********************************************************************/
#include "menu_2.h"
#define PAGE_MAX_SHOW_NUM					4

#define STRING_BT_MAC							"MAC:"
#define STRING_DEVICE_ID					"ID :"

#define STRING_USER_NO						"用户编号:"
#define STRING_NEW_PASSWORD 			"输入新密码:"
#define STRING_NEW_REPASSWORD		 	"再输入一次:"
#define SHOW_PW_HIDE_CH						'*'
#define SHOW_EXIT_HINT						"请输入返回键退出"
#define SHOW_INPUT_ERR						"输入错误!!!"
#define SHOW_ADD_PS_ERR						"添加失败!!!"

//用户添加列表
#define MENU_USER_ADD_NUM						6
static const rt_uint8_t MenuUserAdd_list[MENU_USER_ADD_NUM][8*2] = 
{
	{"新建密码"},
	{"新建指纹"},
	{"新建电话"},
	{"保存退出"},
	{"用户信息"},
	{">>>>退出"},
};

static void menu_user_add_ui(rt_uint8_t InPOS)
{
	rt_uint8_t page;
	rt_uint8_t pos;
	rt_uint8_t i;

	page = InPOS /PAGE_MAX_SHOW_NUM;//计算显示在那一页
	pos = InPOS % PAGE_MAX_SHOW_NUM;//当前选中的位置
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

//选中密码
void menu_8_processing(void)
{
  menu_user_add_ui(0);
}

//选中指纹
void menu_9_processing(void)
{
  menu_user_add_ui(1);
}


//选中添加手机号
void menu_10_processing(void)
{	
  menu_user_add_ui(2);
}


//选中保存退出
void menu_11_processing(void)
{
  menu_user_add_ui(3);
}


//选中退出
void menu_12_processing(void)
{
  menu_user_add_ui(4);
}

//选中查看信息
void menu_13_processing(void)
{
  menu_user_add_ui(5);
}

//检查新增密码是否合法
rt_err_t add_new_password_check(rt_uint8_t password[])
{
	if(password[0] == 0)
	{
		//新增密码为空
		return RT_ERROR;
	}
	
	return RT_EOK;
}

//输入密码处理
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
    //第一次输入密码
    rt_memset(buf,0,MENU_PASSWORD_MAX_LEN);
    rt_memset(ShowBuf,0,MENU_PASSWORD_MAX_LEN);
    while(1)
    {
      rt_err_t result;
      rt_uint8_t KeyValue;
      
      result = gui_key_input(&KeyValue);
      if(RT_EOK == result)
      {
        //有按键
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
            //输入数量超过8个
          }
        }
        else if(KeyValue == '*')
        {
          //检测输入的密码是否合法
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
            //新密码是合法的
            rt_memcpy(buf1,buf,MENU_PASSWORD_MAX_LEN);
            break;
          }
          //新密码输入完成 进入验证。
        }
        else if(KeyValue == '#')
        {
          rt_kprintf("删除\nn");
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
        //闪烁提示
        menu_inputchar_glint(SHOW_X_ROW8(rt_strlen(ShowBuf)),SHOW_Y_LINE(2),GlintStatus%2);
        GlintStatus++;
      }
      //更新显示
      
      gui_display_update();
    }
    
    //第二次输入密码
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
        //有按键
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
            //输入数量超过8个
          }
        }
        else if(KeyValue == '*')
        {
          //检测输入的密码是否合法
          rt_uint32_t temp;
          
          temp = rt_memcmp(buf1,buf,rt_strlen(buf));
          if(temp != 0)
          {
            //密码匹配错误
            gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),SHOW_ADD_PS_ERR,GUI_WIHIT);
    				gui_display_update();
    				rt_thread_delay(RT_TICK_PER_SECOND);
    				gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(3),SHOW_X_ROW8(15),SHOW_Y_LINE(4));
          }
          else
          {
            //添加成功保存对象
          }
          //新密码输入完成 进入验证。
          break;
        }
        else if(KeyValue == '#')
        {
          rt_kprintf("删除\nn");
          result = string_del_char(buf,8);
          if(result == RT_EOK)
          {
            string_hide_string((const rt_uint8_t *)buf,ShowBuf,SHOW_PW_HIDE_CH,8);
            gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(2),SHOW_X_ROW8(15),SHOW_Y_LINE(3));
            gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(2),ShowBuf,GUI_WIHIT);
          }
          else
          {
          	rt_kprintf("退出钥匙添加\n");
						//return ;
          }
        }
      }
      else
      {
        //闪烁提示
        menu_inputchar_glint(SHOW_X_ROW8(rt_strlen(ShowBuf)),SHOW_Y_LINE(2),GlintStatus%2);
        GlintStatus++;
      }
      //更新显示
      
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










