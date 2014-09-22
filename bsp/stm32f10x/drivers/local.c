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
//#include "voice.h"
//#include "unlockprocess.h" //unlock process fun
#include "fprint.h"
#include "untils.h"
#include "config.h"
#include "gpio_pwm.h"

#define LOCAL_DEBUG 1

#define KEY_NOT_PULL_REVOKE_TIME								100*60*30		// 30min
#define KEY_READ_TIMER_BASE											100					// 1s
#ifndef  TEST_LOCK_GATE_TIME
#define LOCK_GATE_TIMER_BASE										1000				// 10s
#else
#define LOCK_GATE_TIMER_BASE										100				// 1s debug use
#endif
#define BATTERY_CHECH_TIMER_BASE								6000				// 1min

#define LOCAL_MAIL_MAX_MSGS 20
void lock_process(LOCAL_MAIL_TYPEDEF *);
// local msg queue for local alarm
static rt_mq_t local_mq;

void
local_thread_entry(void *parameter)
{
	rt_err_t result;
	LOCAL_MAIL_TYPEDEF local_mail_buf;

	//fprint_module_init();
	while (1)
	{
		// receive mail
		rt_memset(&local_mail_buf, 0, sizeof(local_mail_buf));
		result = rt_mq_recv(local_mq, &local_mail_buf, sizeof(local_mail_buf), 100);
		if (result == RT_EOK)
		{
			// process mail
            
            RT_DEBUG_LOG(LOCAL_DEBUG,("receive local mail < time: %d alarm_type: %s >\n",\
                            local_mail_buf.time, alarm_help_map[local_mail_buf.alarm_type]));
			switch (local_mail_buf.alarm_type)
			{
				case ALARM_TYPE_SWITCH1:
					{
                        RT_DEBUG_LOG(LOCAL_DEBUG,("process alarm switch...\n"));
						break;
					}
				case ALARM_TYPE_CAMERA_IRDASENSOR:
				{
					//motor_rotate(RT_FALSE);
					//send_voice_mail(VOICE_TYPE_CCDIR);
					break;
				}
				case ALARM_TYPE_FPRINT_INPUT:
				{
					//fprint_unlock_process(&local_mail_buf);
					
					break;
				}
				case ALARM_TYPE_FPRINT_KEY_ADD:
				{
					//fprint_key_add_porcess(&local_mail_buf);
					
					break;
				}
                case ALARM_TYPE_LOCK_PROCESS: {
                    lock_process(&local_mail_buf);
                }
				default :
                {
                    RT_DEBUG_LOG(LOCAL_DEBUG,("this alarm is not process...\n"));
                    break;
                };
			}
		}
		//motor_auto_lock(RT_FALSE);
	}
}

void
lock_operation(s32 status, u16 pluse)
{
    if (status == LOCK_OPERATION_CLOSE)
        motor_rotate(-pluse);
    else
        motor_rotate(pluse);
}

s32
check_time(u32 start_time, u32 end_time, struct tm *tm_time)
{
    s32 result = -1;
    u32 s_hour = (end_time >> 24) & 0x000000ff;
    u32 s_min = (end_time >> 16) & 0x000000ff;
    u32 e_hour = (end_time >> 8) & 0x000000ff;
    u32 e_min = (end_time >> 0) & 0x000000ff;
    if (start_time & bits_mask(tm_time->tm_wday)) {
        if (tm_time->tm_hour >= s_hour && tm_time->tm_min >= s_min 
            && tm_time->tm_hour <= e_hour && tm_time->tm_min <= e_min) {
            result = 1;
        }
    }
    
    return result;
    
}

s32
device_config_key_operate(u16 key_id, struct key *k, u8 flag);
void lock_process(LOCAL_MAIL_TYPEDEF *local_mail)
{
    struct key k;
    //time_t time = sys_cur_date();
    device_config_key_operate(local_mail->data.lock.key_id, &k, 0);
    switch (k.head.operation_type)
    {
        case KEY_OPERATION_TYPE_FOREVER : {
            lock_operation(local_mail->data.lock.operation, 500);
            // send lock log
            break;
        }
        case KEY_OPERATION_TYPE_ONCE : {
            if (local_mail->time >= k.head.start_time && local_mail->time <= k.head.end_time) {
                lock_operation(local_mail->data.lock.operation, 500);
                // send lock log
            }
            break;
        }
        case KEY_OPERATION_TYPE_WEEKLY : {
            if (check_time(k.head.start_time, k.head.end_time, localtime(&local_mail->time)) > 0) {
                lock_operation(local_mail->data.lock.operation, 500);
                // send lock log
            }
            break;
        }
    }

}

void
send_local_mail(ALARM_TYPEDEF alarm_type, time_t time, union alarm_data *data)
{
	LOCAL_MAIL_TYPEDEF buf;
	rt_err_t result;
	//send mail
	buf.alarm_type = alarm_type;
	if (time) {
		buf.time = time;
	} else {
        buf.time = sys_cur_date();
	}
	if (local_mq != NULL) {
		result = rt_mq_send(local_mq, &buf, sizeof(LOCAL_MAIL_TYPEDEF));
		if (result == -RT_EFULL) {
            RT_DEBUG_LOG(LOCAL_DEBUG,("local_mq is full!!!\n"));
		}
	} else {
            RT_DEBUG_LOG(LOCAL_DEBUG,("local_mq is RT_NULL!!!\n"));
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
