/********************************************************************
 * Filename:			comm.h
 *
 * Description:
 *
 * Author:              Bright Pan
 * Email:				bright_pan@yuettak.com
 * Date:				2014-03-14
 *
 * Modify:
 *
 * Copyright (C) 2014 Yuettak Co.,Ltd
 ********************************************************************/
#ifndef _COMM_H_
#define _COMM_H_

#include <rthw.h>
#include <rtthread.h>
#include <stm32f10x.h>
#include "untils.h"
#include "board.h"

#define DEVICE_NAME_COMM "uart3"
#define COMM_MAIL_MAX_MSGS 10

typedef enum {

	CW_STATUS_OK = 0,
	CW_STATUS_FULL,
	CW_STATUS_INIT_ERROR,
	CW_STATUS_NEW_ERROR,
	CW_STATUS_SEND_ERROR,

}CW_STATUS;

typedef enum {

	COMM_TYPE_SMS = 0,
	COMM_TYPE_GPRS = 1,
	COMM_TYPE_MMS = 2,

}COMM_TYPE_TYPEDEF;

typedef struct {

	rt_sem_t result_sem;
	CW_STATUS *result;
	uint8_t comm_type;
	uint16_t len;
	uint8_t *buf;

}COMM_MAIL_TYPEDEF;



extern rt_mq_t comm_tx_mq;
extern rt_mutex_t comm_mutex;

void
comm_tx_thread_entry(void *parameters);
void
comm_rx_thread_entry(void *parameters);

rt_err_t
send_ctx_mail(COMM_TYPE_TYPEDEF comm_type, uint8_t *buf, uint16_t len);

void
send_frame(rt_device_t device, COMM_MAIL_TYPEDEF *mail, uint8_t order);

#endif /* _COMM_H_ */
