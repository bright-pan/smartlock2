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
 * 2014-09-11 wangzw <wangzw@yuettak.com> 
 							用于显示开锁界面
 							
 * Copyright (C) 2014 Yuettak Co.,Ltd
 ********************************************************************/

#ifndef __UNLOCK_UI_H__
#define __UNLOCK_UI_H__
#include "rtthread.h"

rt_err_t unlock_process_ui(void);
void unlock_process_ui1(void);
void system_menu1_show(void);
void system_menu2_show(void);
void system_manage_processing(void);
rt_bool_t fprint_unlock_result_show(void);
#endif

