/*********************************************************************
 * Filename:			comm_window.h
 *
 * Description:
 *
 * Author:				Bright Pan
 * Email:				bright_pan@yuettak.com
 * Date:				2014-03-17
 *
 * Modify:
 *
 * Copyright (C) 2014 Yuettak Co.,Ltd
 ********************************************************************/
#ifndef _COMM_WINDOW_H_
#define _COMM_WINDOW_H_

#include <rthw.h>
#include <rtthread.h>
#include <stm32f10x.h>

#include "board.h"
#include "list.h"

#define CW_FLAG_REQUEST 0
#define CW_FLAG_RESPONSE 1

typedef struct {

	struct list_head list;
	COMM_MAIL_TYPEDEF mail;
	uint8_t flag;
	uint8_t order;
	uint8_t r_cnts;// resend counts
	uint8_t cnts;// counts for timer
	uint16_t delay;
}COMM_WINDOW_NODE;

typedef struct {

	struct list_head list;
	rt_mutex_t mutex;
	rt_timer_t timer;
	rt_device_t device;
	uint8_t size;

}COMM_WINDOW_LIST;

extern COMM_WINDOW_LIST cw_list;

CW_STATUS
cw_list_init(COMM_WINDOW_LIST *cw_list);

CW_STATUS
cw_list_new(COMM_WINDOW_NODE **node, COMM_WINDOW_LIST *cw_list);

#endif /* _COMM_WINDOW_H_ */
