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

#include "list.h"
#include "untils.h"

#define CTW_FLAG_REQUEST 0
#define CTW_FLAG_RESPONSE 1

typedef enum {

	CTW_STATUS_OK = 0,
	CTW_STATUS_ERROR,
	CTW_STATUS_FULL,
	CTW_STATUS_INIT_ERROR,
	CTW_STATUS_NEW_ERROR,
	CTW_STATUS_SEND_ERROR,

}CTW_STATUS;

typedef struct {

	rt_sem_t result_sem;
	CTW_STATUS *result;
	uint8_t comm_type;
	uint8_t order;
	uint16_t delay;
	uint8_t *buf;
	uint16_t len;

}COMM_TMAIL_TYPEDEF;

typedef struct {

	COMM_TMAIL_TYPEDEF mail;
	uint8_t flag;
	uint8_t order;
	uint8_t r_cnts;// resend counts
	uint8_t cnts;// counts for timer
	uint16_t delay;

}COMM_TWINDOW_NODE_DATA_TYPEDEF;

typedef struct {

	struct list_head list;
    COMM_TWINDOW_NODE_DATA_TYPEDEF data;

}COMM_TWINDOW_NODE;

typedef struct {

	struct list_head list;
	rt_mutex_t mutex;
	rt_timer_t timer;
	rt_device_t device;
	uint8_t size;

}COMM_TWINDOW_LIST;

typedef enum {

	CRW_STATUS_OK = 0,
	CRW_STATUS_ERROR,
	CRW_STATUS_FULL,
	CRW_STATUS_INIT_ERROR,
	CRW_STATUS_NEW_ERROR,
	CRW_STATUS_SEND_ERROR,

}CRW_STATUS;

typedef struct {

    rt_sem_t sem;
    CRW_STATUS *result;
    uint8_t comm_type;
	uint8_t order;
    uint8_t *buf;

}COMM_RWINDOW_NODE_DATA_TYPEDEF;

typedef struct {

	struct list_head list;
    COMM_RWINDOW_NODE_DATA_TYPEDEF data;

}COMM_RWINDOW_NODE;

typedef struct {

	struct list_head list;
	rt_mutex_t mutex;
	rt_timer_t timer;
	uint8_t size;

}COMM_RWINDOW_LIST;

extern COMM_TWINDOW_LIST comm_twindow_list;
extern COMM_RWINDOW_LIST comm_rwindow_list;

CTW_STATUS
ctw_list_init(COMM_TWINDOW_LIST *ctw_list);

CTW_STATUS
ctw_list_new(COMM_TWINDOW_NODE **node, COMM_TWINDOW_LIST *ctw_list, COMM_TWINDOW_NODE_DATA_TYPEDEF *data);

CRW_STATUS
crw_list_init(COMM_RWINDOW_LIST *crw_list);

CRW_STATUS
crw_list_new(COMM_RWINDOW_NODE **node, COMM_RWINDOW_LIST *crw_list, COMM_RWINDOW_NODE_DATA_TYPEDEF *data);

#endif /* _COMM_WINDOW_H_ */
