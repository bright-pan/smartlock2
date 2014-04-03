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
	CW_STATUS_ERROR,
	CW_STATUS_FULL,
	CW_STATUS_INIT_ERROR,
	CW_STATUS_NEW_ERROR,
	CW_STATUS_SEND_ERROR,

}CW_STATUS;

typedef enum {

	COMM_TYPE_SMS = 0,
	COMM_TYPE_GPRS,
	COMM_TYPE_MMS,
	COMM_TYPE_GSM_CTRL_CLOSE,
	COMM_TYPE_GSM_CTRL_OPEN,
	COMM_TYPE_GSM_CTRL_RESET,
	COMM_TYPE_GSM_CTRL_DIALING,
	COMM_TYPE_GSM_CTRL_SWITCH_TO_CMD,
	COMM_TYPE_GSM_CTRL_SWITCH_TO_GPRS,
	COMM_TYPE_GSM_SMSC,
	COMM_TYPE_GSM_PHONE_CALL,
	COMM_TYPE_GSM_CTRL_PHONE_CALL_ANSWER,
	COMM_TYPE_GSM_CTRL_PHONE_CALL_HANG_UP,
	COMM_TYPE_VOICE_AMP,

}COMM_TYPE_TYPEDEF;

typedef struct {

	rt_sem_t result_sem;
	CW_STATUS *result;
	uint8_t comm_type;
	uint8_t order;
	uint16_t delay;
	uint8_t *buf;
	uint16_t len;

}COMM_MAIL_TYPEDEF;

extern rt_mq_t comm_tx_mq;
extern rt_mutex_t comm_mutex;

extern char smsc[20];
extern char phone_call[20];

void
comm_tx_thread_entry(void *parameters);
void
comm_rx_thread_entry(void *parameters);

rt_err_t
send_ctx_mail(COMM_TYPE_TYPEDEF comm_type, uint8_t order, uint16_t delay, uint8_t *buf, uint16_t len);

void
send_frame(rt_device_t device, COMM_MAIL_TYPEDEF *mail, uint8_t order);

rt_size_t comm_recv_gprs_data(rt_uint8_t *buffer,rt_size_t size);


#endif /* _COMM_H_ */
