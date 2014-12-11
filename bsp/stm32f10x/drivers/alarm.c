/*********************************************************************
 * Filename:			alarm.c
 *
 * Description:
 *
 * Author:              Bright Pan
 * Email:				bright_pan@yuettak.com
 * Date:				2014-03-12
 *
 * Modify:
 *
 * Copyright (C) 2014 Yuettak Co.,Ltd
 ********************************************************************/

#include "alarm.h"
#include "gpio_exti.h"
#include "local.h"
#include "untils.h"
#include "sms.h"
#include "config.h"
#include "fprint.h"

#define ALARM_DEBUG 1

#define ALARM_MAIL_MAX_MSGS 20
static rt_mq_t alarm_mq = RT_NULL;

const char *alarm_help_map[] = {
	"ALARM_TYPE_LOCK_SHELL",// lock shell alarm type
	"ALARM_TYPE_LOCK_TEMPERATURE",// lock temperatrue
	"ALARM_TYPE_GATE_TEMPERATURE",// lock temperatrue
	"ALARM_TYPE_LOCK_GATE",// lock gate status
	"ALARM_TYPE_GSM_RING",// lock gate status
	"ALARM_TYPE_SWITCH1",// switch1 alarm type
	"ALARM_TYPE_CAMERA_PHOTOSENSOR", // camera photo sensor
	"ALARM_TYPE_CAMERA_IRDASENSOR", // camera irda sensor
	"ALARM_TYPE_MOTOR_STATUS", // motor status sensor
	"ALARM_TYPE_BATTERY_WORKING_20M",

	"ALARM_TYPE_BATTERY_REMAIN_50P",//10
	"ALARM_TYPE_BATTERY_REMAIN_20P",
	"ALARM_TYPE_BATTERY_REMAIN_5P",
	"ALARM_TYPE_BATTERY_SWITCH",
	"ALARM_TYPE_FPRINT_INPUT",
	"ALARM_TYPE_FPRINT_KEY_ADD",
	"ALARM_TYPE_FPRINT_KEY_RIGHT",
	"ALARM_TYPE_FPRINT_KEY_ERROR",
	"ALARM_TYPE_CODE_KEY_ADD",     //ÃÜÂëÌí¼Ó
	"ALARM_TYPE_CODE_KEY_ERROR",   //ÃÜÂë´íÎó

	"ALARM_TYPE_CODE_KEY_RIGHT",   //ÕýÈ·ÃÜÂë¿ªËø 20
	"ALARM_TYPE_RFID_KEY_ERROR",// rfid key detect error alarm type 14
	"ALARM_TYPE_RFID_KEY_SUCCESS",// rfid key detect success alarm type
	"ALARM_TYPE_RFID_KEY_PLUGIN",// rfid key detect plugin alarm type
	"ALARM_TYPE_RFID_FAULT",
	"ALARM_TYPE_CAMERA_FAULT",
	"ALARM_TYPE_MOTOR_FAULT",
	"ALARM_TYPE_POWER_FAULT",
	"ALARM_TYPE_GPRS_AUTH",
	"ALARM_TYPE_GPRS_HEART",

	"ALARM_TYPE_GPRS_LIST_TELEPHONE",//30
	"ALARM_TYPE_GPRS_LIST_RFID_KEY",
	"ALARM_TYPE_GPRS_SET_TELEPHONE_SUCCESS",
	"ALARM_TYPE_GPRS_SET_TELEPHONE_FAILURE",
	"ALARM_TYPE_GPRS_SET_RFID_KEY_SUCCESS",
	"ALARM_TYPE_GPRS_SET_RFID_KEY_FAILURE",
	"ALARM_TYPE_GPRS_LIST_USER_PARAMETERS",
	"ALARM_TYPE_GPRS_SET_USER_PARAMETERS_SUCCESS",
	"ALARM_TYPE_GPRS_SET_USER_PARAMETERS_FAILURE",
	"ALARM_TYPE_GPRS_SYS_TIME_UPDATE",

	"ALARM_TYPE_GPRS_SET_TIME_SUCCESS",//40
	"ALARM_TYPE_GPRS_SET_TIME_FAILURE",
	"ALARM_TYPE_GPRS_SET_KEY0_SUCCESS",
	"ALARM_TYPE_GPRS_SET_KEY0_FAILURE",
	"ALARM_TYPE_GPRS_SET_HTTP_SUCCESS",
	"ALARM_TYPE_GPRS_SET_HTTP_FAILURE",
	"ALARM_TYPE_GPRS_UPLOAD_PIC",
	"ALARM_TYPE_GPRS_SEND_PIC_DATA",
	"ALARM_TYPE_GPRS_SLEEP",
	"ALARM_TYPE_GPRS_WAKE_UP",

	"ALARM_TYPE_GPRS_CAMERA_OP",//50
	"ALARM_TYPE_BUTTON_ADJUST_IR",
	"ALARM_TYPE_LOCK_PROCESS",
	"ALARM_TYPE_KEY_ADD",
	"ALARM_TYPE_KEY_DEL",
	"ALARM_TYPE_KEY_RIGHT",
	"ALARM_TYPE_KEY_ERROR",
	"ALARM_TYPE_SMS_RF433_ERROR",
	"ALARM_TYPE_SMS_KEY_ERROR",
	"ALARM_TYPE_SMS_FPRINT_ERROR",

	"ALARM_TYPE_SMS_REQ_IN_PHONE_CALL",//60
	"ALARM_TYPE_SMS_REP_IN_PHONE_CALL",
	"ALARM_TYPE_GSM_RING_REQUEST",
	"ALARM_TYPE_GPRS_ADD_ACCOUNT",
	"ALARM_TYPE_GPRS_ADD_PHONE",
	"ALARM_TYPE_SYSTEM_FREEZE",
	"ALARM_TYPE_SYSTEM_UNFREEZE",
  "ALARM_TYPE_FPRINT_INFORM",
  "ALARM_TYPE_SYSTEM_RESET",
  "ALARM_TYPE_GPRS_DATAMAP_UPLOAD",
  "ALARM_TYPE_IRQ",
};

void alarm_thread_entry(void *parameter)
{
	rt_err_t result;
	ALARM_MAIL_TYPEDEF alarm_mail_buf;

  if(system_power_Insufficient() == RT_TRUE)
	{
	  return ;
	}

  system_init();

	while (1)
	{
		if(system_power_Insufficient() == RT_TRUE)
		{
			rt_thread_delay(100);
			rt_thread_entry_sleep(rt_thread_self());
			continue;
		}
		rt_memset(&alarm_mail_buf, 0, sizeof(alarm_mail_buf));
		result = rt_mq_recv(alarm_mq, &alarm_mail_buf, sizeof(alarm_mail_buf), 100);

		if (result == RT_EOK)
		{
#if (defined RT_USING_FINSH) && (defined ALARM_DEBUG)
			rt_kprintf("receive alarm mail < time: %d alarm_type: %s >\n",
					   alarm_mail_buf.time, alarm_help_map[alarm_mail_buf.alarm_type]);
#endif
            switch (alarm_mail_buf.alarm_type)
            {
                case ALARM_TYPE_FPRINT_INFORM: {
                    fp_inform();
                    break;
                }
                default : {
                    break;
                }
            }
		}/* msg receive error */
	}
}

void send_alarm_mail(ALARM_TYPEDEF alarm_type, ALARM_PROCESS_FLAG_TYPEDEF alarm_process_flag, rt_int8_t gpio_value, time_t time)
{
	extern rt_device_t rtc_device;
	ALARM_MAIL_TYPEDEF mail;
	rt_err_t result;

	mail.alarm_type = alarm_type;
	if (time) {
		mail.time = time;
	} else {
        mail.time = sys_cur_date();
	}
	mail.alarm_process_flag = alarm_process_flag;
	mail.gpio_value = gpio_value;
	if (alarm_mq != RT_NULL)
	{
		result = rt_mq_send(alarm_mq, &mail, sizeof(mail));
		if (result == -RT_EFULL)
		{
            RT_DEBUG_LOG(ALARM_DEBUG,("alarm_mq is full!!!\n"));
		}
	}
	else
	{
            RT_DEBUG_LOG(ALARM_DEBUG,("alarm_mq is RT_NULL!!!\n"));
	}
}

int
rt_alarm_init(void)
{
    rt_thread_t alarm_thread;

	// initial alarm msg queue
	alarm_mq = rt_mq_create("alarm", sizeof(ALARM_MAIL_TYPEDEF),
							ALARM_MAIL_MAX_MSGS,
							RT_IPC_FLAG_FIFO);
    if (alarm_mq == RT_NULL)
        return -1;
    // init alarm thread
    alarm_thread = rt_thread_create("alarm",
									alarm_thread_entry, RT_NULL,
									1024, 90, 5);
    if (alarm_thread == RT_NULL)
        return -1;

    rt_thread_startup(alarm_thread);
    return 0;
}

INIT_APP_EXPORT(rt_alarm_init);
