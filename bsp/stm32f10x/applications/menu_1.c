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

#define SHOW_LAND_UI_STRING				"Admin Password"

void menu_1_ui_show(void)
{
	//请输入管理员密码
	//
	//
}

void menu_1_processing(void)
{
	rt_kprintf("进入一级1菜单\n");

	rt_kprintf("x=%d y=%d\n",SHOW_X_CENTERED(SHOW_LAND_UI_STRING),
								SHOW_Y_LINE(0));
	
	gui_display_string(SHOW_X_CENTERED(SHOW_LAND_UI_STRING),SHOW_Y_LINE(0),SHOW_LAND_UI_STRING);
										 
  gui_display_update();
	while(1)
	{
		
	}
}











//
void menu_2_processing(void)
{
	rt_kprintf("进入二级1菜单\n");
}

void menu_3_processing(void)
{
	rt_kprintf("进入二级2菜单\n");
}

void menu_4_processing(void)
{
	rt_kprintf("进入三级1菜单\n");
}

void menu_5_processing(void)
{
	rt_kprintf("进入三级3菜单\n");
}

