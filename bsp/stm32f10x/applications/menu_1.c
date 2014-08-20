/*********************************************************************
 * Filename:      gpio_pin.c
 *
 * Description:
 *
 * Author:        wangzw <wangzw@yuettak.com>
 * Created at:    2013-04-22
 *
 * Modify:
 *
 * 2014-08-19 wangzw <wangzw@yuettak.com> 
 							管理员登陆界面
 							
 * Copyright (C) 2013 Yuettak Co.,Ltd
 ********************************************************************/

#include"menu_1.h"

#define SHOW_LAND_UI_STRING1				"Admin Landing"
#define SHOW_LAND_UI_STRING2				"KEY:"

#define SHOW_MENU_UI_SYSMANAGE			"System Manage"

void menu_1_ui_show(void)
{
	//请输入管理员密码
	//
	//
	
}

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

rt_err_t string_del_char(rt_uint8_t str[],rt_uint8_t ch,rt_uint8_t str_size)
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

void string_hide_string(const rt_uint8_t src[],rt_uint8_t str[],rt_uint8_t size)
{
  rt_uint8_t i;
  
	RT_ASSERT(size > 1);

  rt_memset(str,0,size);

	for(i=0;i<rt_strlen(src);i++)
	{
		if(src[i] != 0)
		{
			str[i] = '*';
		}
	}
}
void menu_1_processing(void)
{
	rt_uint8_t buf[8];
	rt_uint8_t ShowBuf[8];

	rt_kprintf("进入一级1菜单\n");

	gui_clear(0,0,128,64);
	gui_display_string(SHOW_X_CENTERED(SHOW_LAND_UI_STRING1),SHOW_Y_LINE(0),SHOW_LAND_UI_STRING1,GUI_WIHIT);

  gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(2),SHOW_LAND_UI_STRING2,GUI_WIHIT);

	gui_box(SHOW_X_ROW8(5)-1,SHOW_Y_LINE(2)-1,SHOW_X_ROW8(15)+1,SHOW_Y_LINE(3)+1,1,0);

  gui_display_update();

	rt_memset(buf,0,8);
	rt_memset(ShowBuf,0,8);
	while(1)
	{
		rt_err_t result;
		rt_uint8_t KeyValue;

		result = gui_key_input(&KeyValue);
		if(result == RT_EOK)
		{
			if(KeyValue >= '0' && KeyValue <= '9')
			{
				rt_kprintf("input :%s\n",buf);
				rt_kprintf("input :%s\n",ShowBuf);
				result = string_add_char(buf,KeyValue,8);
				if(result == RT_EOK)
				{
					string_hide_string((const rt_uint8_t *)buf,ShowBuf,8);
					gui_clear(SHOW_X_ROW8(5),SHOW_Y_LINE(2),SHOW_X_ROW8(15),SHOW_Y_LINE(3));
					gui_display_string(SHOW_X_ROW8(5),SHOW_Y_LINE(2),ShowBuf,GUI_WIHIT);
				}
				else
				{
					//输入数量超过8个
				}
			}
			else if(KeyValue == '#')
			{
				rt_kprintf("核对密码\n");
				KeyFuncIndex = KeyTab[ KeyFuncIndex].SureState;
				current_operation_index = KeyTab[KeyFuncIndex].CurrentOperate;
				current_operation_index();
				return ;
			}
			else if(KeyValue == '*')
			{
				rt_kprintf("删除\nn");
				result = string_del_char(buf,KeyValue,8);
				if(result == RT_EOK)
				{
					string_hide_string((const rt_uint8_t *)buf,ShowBuf,8);
					gui_clear(SHOW_X_ROW8(5),SHOW_Y_LINE(2),SHOW_X_ROW8(15),SHOW_Y_LINE(3));
					gui_display_string(SHOW_X_ROW8(5),SHOW_Y_LINE(2),ShowBuf,GUI_WIHIT);
				}
			}
			gui_display_update();
		}
	}
}



//系统管理
void menu_2_processing(void)
{
	rt_kprintf("进入二级1菜单\n");
	gui_clear(0,0,128,64);
	gui_display_string(SHOW_X_ROW8(5),SHOW_Y_LINE(1),"menu1",GUI_BLACK);
	gui_display_string(SHOW_X_ROW8(5),SHOW_Y_LINE(2),"menu2",GUI_WIHIT);
	gui_display_update();
}


void menu_3_processing(void)
{
	rt_kprintf("进入二级2菜单\n");
	gui_clear(0,0,128,64);
	gui_display_string(SHOW_X_ROW8(5),SHOW_Y_LINE(1),"menu1",GUI_WIHIT);
	gui_display_string(SHOW_X_ROW8(5),SHOW_Y_LINE(2),"menu2",GUI_BLACK);
	gui_display_update();
}

void menu_4_processing(void)
{
	rt_kprintf("进入三级1菜单\n");
}

void menu_5_processing(void)
{
	rt_kprintf("进入三级3菜单\n");
}

