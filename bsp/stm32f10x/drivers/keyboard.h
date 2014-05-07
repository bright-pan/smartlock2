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

typedef  rt_err_t (*keyboard_call_back)(void *user);

typedef struct
{
	rt_uint16_t KeyPos;
	KEYBOARD_EVENT_TYPE event;
}KEYBOARD_USER,*KEYBOARD_USER_P;

__INLINE
void kb_detect(void);

void key_api_port_callback(keyboard_call_back fun);

#endif /* _KEYBOARD_H_ */
