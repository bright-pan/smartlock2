/*********************************************************************
 * Filename:			keyboard.h
 *
 * Description:
 *
 * Author:				BRIGHT PAN
 * Email:				bright_pan@yuettak.com
 * Date:				2014-04-18
 *
 * Modify:
 *
 * Copyright (C) 2014 Yuettak Co.,Ltd
 ********************************************************************/
#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include <rthw.h>
#include <rtthread.h>
#include <stm32f10x.h>

#define KB_DEBUG 0
#define KB_MAIL_TYPE_INPUT 1
#define KB_MAIL_TYPE_SETMODE 2
#define KB_MAIL_TYPE_TIMEOUT 3

typedef enum
{
	KEY_NOTIFYSOUND,		//提示声音
	KEY_UNLOCK_OK,			//解锁成功
	KEY_CODE_ERROR,			//密码错误
	KEY_UNLOCK_FAIL,		//开门失败
	KEY_SET_MODE,				//密码设置模式
	KEY_NORMAL_MODE,    //普通模式
	KEY_INPUT_NEW_CODE, //提示输入新密码
	KEY_REINPUT_NEW_CODE,//提示重新输入
	KEY_CHOOSE_MODE,     //选择模式
	KEY_MODE_INPUT_ERROR,//模式选择错误
	KEY_REGISTER_OK,     //注册成功
	KEY_REGISTER_FAIL,   //注册失败
	KEY_LIB_FULL,    //钥匙库已满
}KEYBOARD_EVENT_TYPE;

typedef enum {
	KB_MODE_NORMAL_AUTH = 0,
	KB_MODE_SETTING_AUTH,
    KB_MODE_SETTING,
	KB_MODE_ADD_PASSWORD,
	KB_MODE_MODIFY_SUPERPWD,
	KB_MODE_ADD_FPRINT,
}KB_MODE_TYPEDEF;

typedef  rt_err_t (*keyboard_call_back)(void *user);

typedef struct
{
	rt_uint16_t KeyPos;
	KEYBOARD_EVENT_TYPE event;
}KEYBOARD_USER,*KEYBOARD_USER_P;

void key_api_port_callback(keyboard_call_back fun);

__INLINE void
kb_data_init(void);

__INLINE rt_err_t
send_kb_mail(uint16_t type, KB_MODE_TYPEDEF mode, uint8_t c);

#endif /* _KEYBOARD_H_ */
