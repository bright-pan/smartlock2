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
	KEY_NOTIFYSOUND,		//��ʾ����
	KEY_UNLOCK_OK,			//�����ɹ�
	KEY_CODE_ERROR,			//�������
	KEY_UNLOCK_FAIL,		//����ʧ��
	KEY_SET_MODE,				//��������ģʽ
	KEY_NORMAL_MODE,    //��ͨģʽ
	KEY_INPUT_NEW_CODE, //��ʾ����������
	KEY_REINPUT_NEW_CODE,//��ʾ��������
	KEY_CHOOSE_MODE,     //ѡ��ģʽ
	KEY_MODE_INPUT_ERROR,//ģʽѡ�����
	KEY_REGISTER_OK,     //ע��ɹ�
	KEY_REGISTER_FAIL,   //ע��ʧ��
	KEY_LIB_FULL,    //Կ�׿�����
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
