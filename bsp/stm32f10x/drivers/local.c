/*********************************************************************
 * Filename:      local.c
 *
 *
 * Author:        Bright Pan <loststriker@gmail.com>
 * Created at:    2013-05-06 11:52:53
 *
 *
 * Change Log:
 *
 * Copyright (C) 2013 Yuettak Co.,Ltd
 ********************************************************************/

#include "local.h"
#include "gpio_exti.h"
#include "voice.h"
#include "unlockprocess.h" //unlock process fun
#include "fprint.h"

#define KEY_NOT_PULL_REVOKE_TIME								100*60*30		// 30min
#define KEY_READ_TIMER_BASE											100					// 1s
#ifndef  TEST_LOCK_GATE_TIME
#define LOCK_GATE_TIMER_BASE										1000				// 10s
#else
#define LOCK_GATE_TIMER_BASE										100				// 1s debug use
#endif
#define BATTERY_CHECH_TIMER_BASE								6000				// 1min

#define LOCAL_MAIL_MAX_MSGS 20

// local msg queue for local alarm
static rt_mq_t local_mq;

void
local_thread_entry(void *parameter)
{
	rt_err_t result;
	LOCAL_MAIL_TYPEDEF local_mail_buf;

	fprint_module_init();
	while (1)
	{
		// receive mail
		rt_memset(&local_mail_buf, 0, sizeof(local_mail_buf));
		result = rt_mq_recv(local_mq, &local_mail_buf, sizeof(local_mail_buf), 100);
		if (result == RT_EOK)
		{
			// process mail
#if (defined RT_USING_FINSH) && (defined LOCAL_DEBUG)
			rt_kprintf("receive local mail < time: %d alarm_type: %s >\n",\
					   local_mail_buf.time, alarm_help_map[local_mail_buf.alarm_type]);
#endif
			switch (local_mail_buf.alarm_type)
			{
				case ALARM_TYPE_SWITCH1:
					{
#if (defined RT_USING_FINSH) && (defined LOCAL_DEBUG)
						rt_kprintf("process alarm switch...\n");
#endif
						break;
					}
				case ALARM_TYPE_CAMERA_IRDASENSOR:
				{
					motor_rotate(RT_FALSE);
					send_voice_mail(VOICE_TYPE_CCDIR);
					break;
				}
				case ALARM_TYPE_FPRINT_INPUT:
				{
					fprint_unlock_process(&local_mail_buf);
					break;
				}
				case ALARM_TYPE_FPRINT_KEY_ADD:
				{
					fprint_key_add(&local_mail_buf);
					break;
				}
				default :
                {
#if (defined RT_USING_FINSH) && (defined LOCAL_DEBUG)
                    rt_kprintf("this alarm is not process...\n");
#endif
                    break;
                };
			}
		}
	}
}

void
send_local_mail(ALARM_TYPEDEF alarm_type, time_t time)
{
	LOCAL_MAIL_TYPEDEF buf;
	extern rt_device_t rtc_device;
	rt_err_t result;
	//send mail
	buf.alarm_type = alarm_type;
	if (!time)
	{
		RT_ASSERT(rtc_device != RT_NULL);
		rt_device_control(rtc_device, RT_DEVICE_CTRL_RTC_GET_TIME, &(buf.time));
	}
	else
	{
		buf.time = time;
	}
	if (local_mq != NULL)
	{
		result = rt_mq_send(local_mq, &buf, sizeof(LOCAL_MAIL_TYPEDEF));
		if (result == -RT_EFULL)
		{
#if (defined RT_USING_FINSH) && (defined LOCAL_DEBUG)
			rt_kprintf("local_mq is full!!!\n");
#endif
		}
	}
	else
	{
#if (defined RT_USING_FINSH) && (defined LOCAL_DEBUG)
		rt_kprintf("local_mq is RT_NULL!!!\n");
#endif
	}
}

int
rt_local_init(void)
{
	rt_thread_t local_thread;

    // initial local msg queue
	local_mq = rt_mq_create("local", sizeof(LOCAL_MAIL_TYPEDEF),
							LOCAL_MAIL_MAX_MSGS, RT_IPC_FLAG_FIFO);
    if (local_mq == RT_NULL)
        return -1;

    // init local thread
    local_thread = rt_thread_create("local",
									local_thread_entry, RT_NULL,
									1024, 102, 5);
    if (local_thread == RT_NULL)
        return -1;

    rt_thread_startup(local_thread);

    return 0;
}

INIT_APP_EXPORT(rt_local_init);
