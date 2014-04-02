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
#include "comm_window.h"

#define DEVICE_NAME_COMM "uart3"
#define COMM_TMAIL_MAX_MSGS 5

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



extern rt_mq_t comm_tx_mq;
extern rt_mutex_t comm_mutex;

void
comm_tx_thread_entry(void *parameters);
void
comm_rx_thread_entry(void *parameters);

CTW_STATUS
send_ctx_mail(COMM_TYPE_TYPEDEF comm_type, uint8_t order, uint16_t delay, uint8_t *buf, uint16_t len);

void
send_frame(rt_device_t device, COMM_TMAIL_TYPEDEF *mail, uint8_t order);

#endif /* _COMM_H_ */
