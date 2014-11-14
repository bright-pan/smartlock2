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
#include "sms.h"
#include "gsm.h"
#include "gprsmailclass.h"
#ifdef USEING_BUZZER_FUN
#include "buzzer.h"
#endif
#include "rf433.h"
#include "PVDProcess.h"
#include "eeprom_process.h"

#ifdef   USEING_RAM_DEBUG
#include "untils.h" //主要使用里面的 rt_dprintf
#endif

#ifndef USEING_RAM_DEBUG
#define rt_dprintf    RT_DEBUG_LOG
#endif


#define LOCAL_DEBUG_THREAD 											16
#define LOCAL_DEBUG_MAIL                        17

#define MOTOR_WORK_CUT													180					//电机转动次数

#define KEY_NOT_PULL_REVOKE_TIME								100*60*30		// 30min
#define KEY_READ_TIMER_BASE											100					// 1s
#ifndef  TEST_LOCK_GATE_TIME
#define LOCK_GATE_TIMER_BASE										5				   // 5s debug use
#endif
#define BATTERY_CHECH_TIMER_BASE								6000				// 1min
#define AUTO_UNFREEZE_TIME                      60*3          //30s
#define LOCAL_MAIL_MAX_MSGS 										20
void lock_process(LOCAL_MAIL_TYPEDEF *);
// local msg queue for local alarm
static rt_mq_t local_mq;
static rt_uint16_t AutoLockTime = LOCK_GATE_TIMER_BASE;
static rt_uint16_t AutoUnfreezeTime = AUTO_UNFREEZE_TIME;

void motor_status_open_send(void);

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
	LOCK_GATE_TIMER_BASE-3,
};
static rt_event_t   local_evt = RT_NULL;

void lock_operation(s32 status, u16 pluse);





/**
  * @brief  掉电紧急处理函数
  * @param  None
  * @retval None
  */
void PVD_IRQCallbackFun(void)
{
	/* save current system time */
	system_time_save();

	rt_kprintf("save current system time\n");
}

/**
  * @brief  系统冻结管理函数
  * @param  None
  * @retval None
  */
void system_freeze_manage(void)
{
	//如果被冻结
	if(local_event_process(1,LOCAL_EVT_SYSTEM_FREEZE) == 0)
	{
		//解冻倒计时
		AutoUnfreezeTime--;
		if(AutoUnfreezeTime == 0)
		{
			AutoUnfreezeTime = AUTO_UNFREEZE_TIME;
			local_event_process(2,LOCAL_EVT_SYSTEM_FREEZE);

			//发送开锁信号
			motor_status_open_send();
			rt_kprintf("System In Unfreeze\n");
		}
	}
}

/*
功能:local线程中事件
参数:mode 模式  type 事件类型
返回: -------------------------
		 |模式 |成功|失败|功能    |
		 |0    |0   |1   |发送事件|
		 |1    |0   |1   |收到事件|
		 |2    |0   |1   |清除事件|
		 --------------------------
*/
rt_uint8_t local_event_process(rt_uint8_t mode,rt_uint32_t type)
{
	rt_uint32_t value;
	rt_err_t    result;
	rt_uint8_t  return_data = 1;
	
	//net_evt_mutex_op(RT_TRUE);

	if(local_evt == RT_NULL)
	{
    local_evt = rt_event_create("local",RT_IPC_FLAG_FIFO);
    RT_ASSERT(local_evt != RT_NULL);
	}
	switch(mode)
	{
		case 0:	//set event 
		{
			result = rt_event_send(local_evt,type);
			if(result == RT_EOK)
			{
				return_data = 0;
			}
			break;
		}
		case 1:	//get event 
		{
			result = rt_event_recv(local_evt,
			                       type,
			                       RT_EVENT_FLAG_OR,
			                       RT_WAITING_NO,&value);
			if(result == RT_EOK)
			{
				return_data = 0;
			}
			else if(result == -RT_ETIMEOUT)
			{
				return_data = 1;
			}
			break;
		}
		case 2://clean event
		{
			result = rt_event_recv(local_evt,
			                       type,
			                       RT_EVENT_FLAG_OR | 
			                       RT_EVENT_FLAG_CLEAR,
			                       RT_WAITING_NO,&value);
			if(result == RT_EOK)
			{
				return_data = 0;
			}
			break;
		}
    case 3://clean all event 
    {
      result = rt_event_recv(local_evt,
                             0xffffffff,
                             RT_EVENT_FLAG_OR | 
                             RT_EVENT_FLAG_CLEAR,
                             RT_WAITING_NO,&value);
      if(result == RT_EOK)
      {
        return_data = 0;
      }
      break;
    }
    default:
    {
			break;
    }
	}

	//net_evt_mutex_op(RT_FALSE);
	return return_data;
}



//报警管理
//错误计数累加  0
//清除计数      1
rt_bool_t key_error_alarm_manage(KeyErrCntManageMode mode,rt_uint8_t *smsflag)
{
	switch(mode)
	{
		case KEY_ERRNUM_MODE_ADDUP:
		{	
			//计数
			KeyErrorData.ErrorCnt++;
			if(KeyErrorData.ErrorCnt > 3)
			{
				//发送冻结事件
				send_local_mail(ALARM_TYPE_SYSTEM_FREEZE,0,RT_NULL);
				if(smsflag != RT_NULL)
				{
          if(KeyErrorData.ErrorCnt == 4)
          {
            *smsflag = 1;
          }
          else
          {
            *smsflag = 0;
          }
				}

       	return RT_TRUE;
			}
			else
			{
				*smsflag = RT_FALSE;
			}
			break;
		}
		case KEY_ERRNUM_MODE_CLAER:
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
static void motor_locktime_set(rt_uint16_t value)
{
  MotorManage.LockTime = value;
}
void motor_status_manage(void)
{
	rt_err_t result;
	
	if(MotorManage.LockTime == 0)
	{	
		//上次的开始过程结束后再一次收到开门信号
    result = rt_sem_take(MotorManage.StatusSem ,RT_WAITING_NO);
    if(result == RT_EOK)
    {
      MotorManage.LockTime  = 1; 
    }
	}
	else
	{
		MotorManage.LockTime ++;
		if(MotorManage.LockTime  > AutoLockTime)
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

static u8 gsm_ring_request = 1;

void gsm_ring_request_timeout(void *parameters)
{
    u8 *flag = parameters;
    *flag = 1;
}

int gsm_ring_request_enter(void)
{
    int result = -1;
    if (gsm_ring_request)
    {
        rt_timer_create("g_req", gsm_ring_request_timeout, &gsm_ring_request, 10000, RT_TIMER_FLAG_ONE_SHOT | RT_TIMER_FLAG_SOFT_TIMER);
        gsm_ring_request = 0;
        result = 0;
    }
    return result;
}

void gsm_ring_request_exit(void)
{
    gsm_ring_request = 1;
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
            
     	rt_dprintf(LOCAL_DEBUG_MAIL,("receive local mail < time: %d alarm_type: %s >\n",\
                            local_mail_buf.time, alarm_help_map[local_mail_buf.alarm_type]));
			switch (local_mail_buf.alarm_type)
			{
				case ALARM_TYPE_GSM_RING:
				{
	        s32 temp;
	        struct phone_head ph;
	        rt_memset(&ph, 0, sizeof(ph));
	        rt_memcpy(ph.address, "86", 2);
	        rt_memcpy(ph.address+2, local_mail_buf.data.ring.phone_call, rt_strlen((const char *)local_mail_buf.data.ring.phone_call));
					temp = device_config_phone_verify(local_mail_buf.data.ring.phone_call, 11);
	        if (temp >= 0)
	        {
	            if (device_config_phone_operate(temp, &ph, 0) >= 0) {
	                if (ph.account != PHONE_ID_INVALID && ph.auth & PHONE_AUTH_CALL) {
	                    //电机解锁。
	                    lock_operation(LOCK_OPERATION_OPEN, MOTOR_WORK_CUT);
	                    send_sms_mail(ALARM_TYPE_SMS_REP_IN_PHONE_CALL, 0, (u8 *)ph.address, 11, PHONE_AUTH_CALL);
	                }
	            }
	        }
	        else
	        {
						rt_dprintf(LOCAL_DEBUG_THREAD,("This is phone error %s\n",local_mail_buf.data.ring.phone_call));
	        }
                    send_gsm_ctrl_mail(GSM_CTRL_PHONE_CALL_HANG_UP,RT_NULL,0,1);
                    gsm_ring_request_exit();
					break;
				}
				case ALARM_TYPE_GSM_RING_REQUEST:
				{
                    if (gsm_ring_request_enter >= 0)
                        send_sms_mail(ALARM_TYPE_SMS_REQ_IN_PHONE_CALL, 0, RT_NULL, 0, PHONE_AUTH_CALL);
					break;
				}
				case ALARM_TYPE_FPRINT_KEY_ADD:
				{
					//fprint_key_add_porcess(&local_mail_buf);
					
					break;
				}
        case ALARM_TYPE_LOCK_PROCESS: 
        {
        	rt_uint8_t freeze;
        	
        	freeze = local_event_process(1,LOCAL_EVT_SYSTEM_FREEZE);
        	if(freeze == 0)
        	{
						//被冻结
						rt_kprintf("system is freeze !!! run lock \n");
						lock_operation(LOCK_OPERATION_CLOSE,MOTOR_WORK_CUT);
        	}
        	else
        	{
            lock_process(&local_mail_buf);
        	}
         	break;
        }
        case ALARM_TYPE_KEY_ERROR:
        {
        	//钥匙错误报警
        	union alarm_data data;
        	
					if(local_mail_buf.data.key.sms == 1)
					{
						switch(local_mail_buf.data.key.Type)
						{
							case KEY_TYPE_FPRINT:
							{
								send_sms_mail(ALARM_TYPE_SMS_FPRINT_ERROR,0, RT_NULL, 0, PHONE_AUTH_SMS);
								break;
							}
							case KEY_TYPE_KBOARD:
							{
								send_sms_mail(ALARM_TYPE_SMS_KEY_ERROR,0, RT_NULL, 0, PHONE_AUTH_SMS);
								break;
							}
							case KEY_TYPE_RF433:
							{
								send_sms_mail(ALARM_TYPE_SMS_RF433_ERROR,0, RT_NULL, 0, PHONE_AUTH_SMS);
								break;
							}
							default:
							{
								break;
							}
						}
						gprs_key_error_mail(local_mail_buf.data.key.Type);
					}

					data.lock.key_id = 0;
					data.lock.operation = LOCK_OPERATION_CLOSE;
					data.lock.CheckMode = LOCK_HAVE_AUTH_CHECK;
					send_local_mail(ALARM_TYPE_LOCK_PROCESS,0,&data);
					break;
        }
        case ALARM_TYPE_KEY_RIGHT:
        {
        	//钥匙正确
					union alarm_data data;

					if(local_mail_buf.data.key.Type == KEY_TYPE_RF433)
					{
						//433 钥匙
						rt_kprintf("This is RF433 GPRS Mail\n");
            gprs_key_right_mail(local_mail_buf.data.key.ID);
            buzzer_send_mail(BZ_TYPE_UNLOCK);
						break;
					}
					data.lock.key_id = local_mail_buf.data.key.ID;
					data.lock.operation = LOCK_OPERATION_OPEN;
					data.lock.operation = LOCK_HAVE_AUTH_CHECK;
					send_local_mail(ALARM_TYPE_LOCK_PROCESS,0,&data);
					gprs_key_right_mail(local_mail_buf.data.key.ID);
					
					motor_locktime_set(1);
					break;
        }
        case ALARM_TYPE_SYSTEM_FREEZE:
        {
        	//系统冻结
        	rt_kprintf("Password Error System Freeze!!!\n");
        	local_event_process(0,LOCAL_EVT_SYSTEM_FREEZE);
					break;
        }
        case ALARM_TYPE_SYSTEM_UNFREEZE:
        {
					//系统解冻
					rt_kprintf("Admin Login ");
					local_event_process(2,LOCAL_EVT_SYSTEM_FREEZE);
					break;
        }
       	case ALARM_TYPE_SYSTEM_RESET:
       	{
					sysinit();
					break;
       	}
				default :
        {
            rt_dprintf(LOCAL_DEBUG_THREAD,("this alarm is not process...\n"));
            break;
        };
			}
		}
		else
		{
			//电机状态管理
			motor_status_manage();

			//电机冻结管理
			system_freeze_manage();
		}
		//motor_auto_lock(RT_FALSE);
	}
}

void
lock_operation(s32 status, u16 pluse)
{
    if (status == LOCK_OPERATION_CLOSE)
    {
			#ifdef USEING_BUZZER_FUN
      buzzer_send_mail(BZ_TYPE_LOCK);
			#endif
			if(motor_status_get() == LOCK_OPERATION_OPEN)
    	{
        motor_rotate(pluse);
    		motor_status_set(LOCK_OPERATION_CLOSE);
    	}
    }
    else
    {
			#ifdef USEING_BUZZER_FUN
			buzzer_send_mail(BZ_TYPE_UNLOCK);
			#endif
			if(motor_status_get() == LOCK_OPERATION_CLOSE)
    	{
        motor_rotate(-pluse);
        motor_status_set(LOCK_OPERATION_OPEN); 
        motor_status_open_send();
    	}
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

    if(local_mail->data.lock.CheckMode)
    {
    	//电机工作在不做钥匙检查模式
      lock_operation(local_mail->data.lock.operation, MOTOR_WORK_CUT);
    }
    else
    {
    	//电机工作在权限检测开锁方式
			//time_t time = sys_cur_date();
			device_config_key_operate(local_mail->data.lock.key_id, &k, 0);
			switch (k.head.operation_type)
			{
				case KEY_OPERATION_TYPE_FOREVER : 
				{
					 lock_operation(local_mail->data.lock.operation, MOTOR_WORK_CUT);
					 // send lock log
					 break;
				}
				case KEY_OPERATION_TYPE_ONCE : 
				{
					 if (local_mail->time >= k.head.start_time && local_mail->time <= k.head.end_time)
					 {
				     lock_operation(local_mail->data.lock.operation, MOTOR_WORK_CUT);
				     // send lock log
					 }
					 break;
				}
				case KEY_OPERATION_TYPE_WEEKLY : 
				{
					if (check_time(k.head.start_time, k.head.end_time, localtime(&local_mail->time)) > 0) 
					{
						lock_operation(local_mail->data.lock.operation, MOTOR_WORK_CUT);
						// send lock log
					}
					break;
				}
				default:
				{
					rt_kprintf("%s Key Type Is Error!!!\n",__FUNCTION__);
					RT_ASSERT(RT_NULL);
					break;
				}
			}
    }
}

void
send_local_mail(ALARM_TYPEDEF alarm_type, time_t time, union alarm_data *data)
{
	LOCAL_MAIL_TYPEDEF mail;
	rt_err_t result;
	//send mail
	mail.alarm_type = alarm_type;
	if (time) {
		mail.time = time;
	} else {
        mail.time = sys_cur_date();
	}
    mail.data = *data;
	if (local_mq != NULL) {
		result = rt_mq_send(local_mq, &mail, sizeof(LOCAL_MAIL_TYPEDEF));
		if (result == -RT_EFULL) {
            rt_dprintf(LOCAL_DEBUG_THREAD,("local_mq is full!!!\n"));
		}
	} else {
            rt_dprintf(LOCAL_DEBUG_THREAD,("local_mq is RT_NULL!!!\n"));
	}
}

int
rt_local_init(void)
{
	rt_thread_t local_thread;

	PVD_IRQCallBackSet(PVD_IRQCallbackFun);
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

//系统自动上锁时间设置
void system_autolock_time_set(rt_uint16_t value)
{
	AutoLockTime = value;
}

//系统自动上锁时间获取
rt_uint16_t system_autolock_time_get(void)
{
	return AutoLockTime;
}





#ifdef RT_USING_FINSH
#include <finsh.h>

void lock_unlock(rt_uint8_t mode,rt_uint32_t time)
{
	lock_operation(mode,time);
}
FINSH_FUNCTION_EXPORT(lock_unlock,"lock_unlock(rt_uint8_t mode)");
FINSH_FUNCTION_EXPORT(send_local_mail,"send_local_mail");

void system_info(void)
{
	extern void bt_info(void);
	extern void net_info(void);
	
	//报警计数
	rt_kprintf("NET manage info:\n");
	net_info();
	rt_kprintf(">>>NET END\n");
	rt_kprintf("BT manage info:\n");
	bt_info();
	rt_kprintf(">>>BT END\n");
	rt_kprintf("Door manage info:\n");
	if(MotorManage.Status == LOCK_OPERATION_OPEN)
	{
		rt_kprintf("Now Door Status is Open \n");
	}
	else
	{
		rt_kprintf("Now Door Status is Close\n");
	}
	rt_kprintf("MotorManage.LockTime = %d\n",MotorManage.LockTime);
	rt_kprintf("KeyErrorData.ErrorCnt = %d\n",KeyErrorData.ErrorCnt);
	rt_kprintf(">>>DOOR END\n");
}
FINSH_FUNCTION_EXPORT(system_info,"show system info");

void int_to_uint(rt_uint8_t data)
{
	rt_int8_t sdata;

	sdata = data;
	rt_kprintf("int:%d hex:%x\nuint:%d hex:%x\n",sdata,sdata,data,data);
}
FINSH_FUNCTION_EXPORT(int_to_uint,"int to uint show");


#endif
