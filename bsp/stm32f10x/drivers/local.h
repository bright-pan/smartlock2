/*********************************************************************
 * Filename:      local.h
 *
 *
 * Author:        Bright Pan <loststriker@gmail.com>
 * Created at:    2013-05-06 11:53:04
 *
 *
 *
 * Change Log:
 *
 * Copyright (C) 2013 Yuettak Co.,Ltd
 ********************************************************************/

#ifndef _LOCAL_H_
#define _LOCAL_H_

#include <rtthread.h>
#include <rthw.h>
#include <rtdevice.h>
#include <time.h>
#include <stm32f10x.h>
#include "alarm.h"

#define LOCK_OPERATION_OPEN  0
#define LOCK_OPERATION_CLOSE 1

struct lock_data {
    s32 key_id;
    s32 operation;
};
struct ring_data {
     u8 phone_call[20];
};

struct keyop_data
{
	rt_uint16_t ID;
	rt_uint8_t  Type;
	rt_uint8_t 	sms;
};

union alarm_data{
    struct lock_data lock;
    struct ring_data ring;
    struct keyop_data  key;
};

typedef struct
{
	time_t time;
	ALARM_TYPEDEF alarm_type;
    union alarm_data data;
}LOCAL_MAIL_TYPEDEF;

typedef struct
{
	rt_uint8_t key[4];
}GPRS_MAIL_USER;

/*
extern GPRS_MAIL_USER gprs_mail_user;

extern rt_timer_t lock_gate_timer;
extern rt_timer_t battery_switch_timer;
*/
void local_thread_entry(void *parameter);

void send_local_mail(ALARM_TYPEDEF alarm_type, time_t time, union alarm_data *data);

rt_uint8_t motor_status_get(void);

rt_bool_t key_error_alarm_manage(rt_uint8_t mode);
#endif
