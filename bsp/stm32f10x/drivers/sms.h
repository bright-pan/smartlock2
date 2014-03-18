/*********************************************************************
 * Filename:      sms.h
 *
 *
 * Author:        Bright Pan <loststriker@gmail.com>
 * Created at:    2013-05-06 09:32:02
 *
 *
 *
 * Change Log:
 *
 * Copyright (C) 2013 Yuettak Co.,Ltd
 ********************************************************************/

#ifndef _SMS_H_
#define _SMS_H_

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <rthw.h>
#include <rtthread.h>
#include "stm32f10x.h"

#include "alarm.h"
#include "untils.h"
#include "comm.h"

typedef struct {

	time_t time;
	ALARM_TYPEDEF alarm_type;

}SMS_MAIL_TYPEDEF;

#define SMS_MAIL_MAX_MSGS 20

extern rt_mq_t sms_mq;

void
send_sms_mail(ALARM_TYPEDEF alarm_type, time_t time);
void
sms_thread_entry(void *parameter);

#endif
