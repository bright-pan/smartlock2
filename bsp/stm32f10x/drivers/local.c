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

#ifdef USEING_BUZZER_FUN
#include "buzzer.h"
#endif

#define LOCAL_DEBUG 1

#define MOTOR_WORK_CUT													180					//电机转动次数

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


typedef struct
{
	rt_sem_t 		StatusSem;
	rt_uint8_t  Status;
	rt_uint16_t LockTime;
}MotorDevDef;

typedef struct
{
	rt_uint8_t ErrorCnt;
}KeyErrorDef;

static KeyErrorDef KeyErrorData = 
{
	0,
};

static MotorDevDef MotorManage =
{
	RT_NULL,
	LOCK_OPERATION_OPEN,
	0,
};

void lock_operation(s32 status, u16 pluse);

rt_bool_t key_error_alarm_manage(rt_uint8_t mode)
{
	switch(mode)
	{
		case 0:
		{	
			//计数
			KeyErrorData.ErrorCnt++;
			if(KeyErrorData.ErrorCnt > 3)
			{
				return RT_TRUE;
			}
			break;
		}
		case 1:
		{
			//清除计数
			KeyErrorData.ErrorCnt = 0;
			break;
		}
		default:
		{
			break;
		}
	}

	return RT_FALSE;
}

void motor_status_set(rt_uint8_t status)
{
	MotorManage.Status = status;
}

rt_uint8_t motor_status_get(void)
{
	return MotorManage.Status;
}
void motor_status_open_send(void)
{
	if(MotorManage.StatusSem == RT_NULL)
	{
		MotorManage.StatusSem  = rt_sem_create("motor",1,RT_IPC_FLAG_FIFO);
		RT_ASSERT(MotorManage.StatusSem );
	}

	rt_sem_release(MotorManage.StatusSem );
	MotorManage.LockTime = 0;
}

void motor_status_manage(void)
{
	rt_err_t result;
	
	if(MotorManage.LockTime == 0)
	{
    result = rt_sem_take(MotorManage.StatusSem ,RT_WAITING_NO);
    if(result == RT_EOK)
    {
      MotorManage.LockTime  = 1; 
    }
	}
	else
	{
		MotorManage.LockTime ++;
		if(MotorManage.LockTime  > 10)
		{
			//上锁
			MotorManage.LockTime  = 0;
      lock_operation(LOCK_OPERATION_CLOSE,MOTOR_WORK_CUT);
		}
	}

}
void motor_status_sem_init(void)
{
	if(MotorManage.StatusSem  == RT_NULL)
	{
		MotorManage.StatusSem  = rt_sem_create("motor",1,RT_IPC_FLAG_FIFO);
		RT_ASSERT(MotorManage.StatusSem );
	}
}
void
local_thread_entry(void *parameter)
{
	rt_err_t result;
	LOCAL_MAIL_TYPEDEF local_mail_buf;

	//fprint_module_init();
	motor_status_sem_init();
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
        case ALARM_TYPE_LOCK_PROCESS: 
        {
        	lock_process(&local_mail_buf);
        	break;
        }
				default :
                {
                    RT_DEBUG_LOG(LOCAL_DEBUG,("this alarm is not process...\n"));
                    break;
                };
			}
		}
		else
		{
			motor_status_manage();
		}
		//motor_auto_lock(RT_FALSE);
	}
}

void
lock_operation(s32 status, u16 pluse)
{
    if (status == LOCK_OPERATION_CLOSE)
    {
    	if(motor_status_get() == LOCK_OPERATION_OPEN)
    	{
        motor_rotate(pluse);
    		motor_status_set(LOCK_OPERATION_CLOSE);
    	}
			#ifdef USEING_BUZZER_FUN
      buzzer_send_mail(BZ_TYPE_LOCK);
			#endif
    }
    else
    {
    	if(motor_status_get() == LOCK_OPERATION_CLOSE)
    	{
        motor_rotate(-pluse);
        motor_status_set(LOCK_OPERATION_OPEN); 
        motor_status_open_send();
    	}
			#ifdef USEING_BUZZER_FUN
			buzzer_send_mail(BZ_TYPE_UNLOCK);
			#endif
    }
       
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
            lock_operation(local_mail->data.lock.operation, MOTOR_WORK_CUT);
            // send lock log
            break;
        }
        case KEY_OPERATION_TYPE_ONCE : {
            if (local_mail->time >= k.head.start_time && local_mail->time <= k.head.end_time) {
                lock_operation(local_mail->data.lock.operation, MOTOR_WORK_CUT);
                // send lock log
            }
            break;
        }
        case KEY_OPERATION_TYPE_WEEKLY : {
            if (check_time(k.head.start_time, k.head.end_time, localtime(&local_mail->time)) > 0) {
                lock_operation(local_mail->data.lock.operation, MOTOR_WORK_CUT);
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
    buf.data = *data;
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
									1024*2, 102, 5);
    if (local_thread == RT_NULL)
        return -1;

    rt_thread_startup(local_thread);

    return 0;
}

INIT_APP_EXPORT(rt_local_init);





#ifdef RT_USING_FINSH
#include <finsh.h>

void lock_unlock(rt_uint8_t mode,rt_uint32_t time)
{
	lock_operation(mode,time);
}
FINSH_FUNCTION_EXPORT(lock_unlock,"lock_unlock(rt_uint8_t mode)");

#endif
