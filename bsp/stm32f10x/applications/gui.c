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
 * 2014-08-20 wangzw <wangzw@yuettak.com> 
 							GUI:人机接口，为输入输出提供接口
 							
 * Copyright (C) 2014 Yuettak Co.,Ltd
 ********************************************************************/

#include "gui.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//key api
static rt_mq_t key_mq;
typedef struct {
	uint16_t type;
	KB_MODE_TYPEDEF mode;
	uint8_t c;
}KB_MAIL_TYPEDEF;

rt_err_t send_key_value_mail(uint16_t type, KB_MODE_TYPEDEF mode, uint8_t c)
{
	rt_err_t result = -RT_EFULL;
	KB_MAIL_TYPEDEF mail;

	if(key_mq == RT_NULL)
	{
		key_mq = rt_mq_create("kboard", sizeof(KB_MAIL_TYPEDEF),
						 5, RT_IPC_FLAG_FIFO);
	}
	if (key_mq != RT_NULL)
	{
		mail.type = type;
		mail.mode = mode;
		mail.c = c;
		result = rt_mq_send(key_mq, &mail, sizeof(mail));
		if (result == -RT_EFULL)
		{
#if (defined RT_USING_FINSH) && (defined KB_DEBUG)
			rt_kprintf("kb_mq is full!!!\n");
#endif
		}
	}
	else
	{
#if (defined RT_USING_FINSH) && (defined KB_DEBUG)
		rt_kprintf("kb_mq is RT_NULL!!!!\n");
#endif
	}
    return result;
}

rt_err_t gui_key_input(rt_uint8_t *KeyValue)
{
	rt_err_t result;
	KB_MAIL_TYPEDEF mail;

	rt_memset(&mail, 0, sizeof(mail));

	if(key_mq == RT_NULL)
	{
		key_mq = rt_mq_create("kboard", sizeof(KB_MAIL_TYPEDEF),
						 5, RT_IPC_FLAG_FIFO);
	}
	result = rt_mq_recv(key_mq, &mail, sizeof(mail),RT_WAITING_NO);

	*KeyValue = mail.c;
	return result;
}
////////////////////////////////////////////////////////////////////////////////////////////////////

void gui_display_string(rt_uint8_t x,rt_uint8_t y,rt_uint8_t *string)
{	
  lcd_display_string(x,y,string,rt_strlen(string));
}

void gui_display_update(void)
{
	lcd_display(0,0,LCD_X_MAX,LCD_Y_MAX);
}
