/*********************************************************************
 * Filename:			alarm.h
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
#ifndef _ALARM_H_
#define _ALARM_H_

#include <rtthread.h>
#include <rthw.h>
#include <rtdevice.h>
#include <time.h>

#define ALARM_INTERVAL 10

/* not sleep status */
#define MACHINE_ON_WORK							(0X01<<0) //wakeup is 1

/* have call */
#define IN_CALL_EVENT								(0X01<<1) //have call is 1

/* need exit picture send*/
#define EXIT_GPRS_PIC_SEND					(0X01<<2)

/* key deal status */
#define KEY_DEAL_STATUS							(0X01<<3)

/* photograph whether status */
#define PHOTOGRAPH_STATUS						(0X01<<4)

/* device activate event*/
#define DEVICE_ACTIVATE							(0X01<<5)

#if (defined USE_SMS_SEND_TYPE2)||(defined USE_SMS_SEND_TYPE3)
#define SMS_TYPE1										(0X01<<6)
#define SMS_TYPE2										(0X01<<7)
#define SMS_TYPE3										(0X01<<8)
#define SMS_TYPE4										(0X01<<9)
#define SMS_TYPE5										(0X01<<10)
#define SMS_TYPE6										(0X01<<11)
#define SMS_TYPE7										(0X01<<12)
#define SMS_TYPE8										(0X01<<13)
#define SMS_TYPE9										(0X01<<14)
#define SMS_TYPE10									(0X01<<15)
#define SMS_TYPE11									(0X01<<16)
#define SMS_TYPE12									(0X01<<17)
#define SMS_TYPE13									(0X01<<18)
#define SMS_TYPE_ALL								(SMS_TYPE1)|(SMS_TYPE2)|(SMS_TYPE3)|(SMS_TYPE4)	\
	|(SMS_TYPE5)|(SMS_TYPE6)|(SMS_TYPE7)|(SMS_TYPE8)					\
	|(SMS_TYPE9)|(SMS_TYPE10)|(SMS_TYPE11)|(SMS_TYPE12)					\
	|(SMS_TYPE13)
#define SMS_TYPE_PIC								(SMS_TYPE1)|(SMS_TYPE2)|(SMS_TYPE5)|(SMS_TYPE7)
#define SMS_TYPE_NO_PIC							(SMS_TYPE3)|(SMS_TYPE4)|(SMS_TYPE6)|(SMS_TYPE8)|(SMS_TYPE9)| \
	(SMS_TYPE10)|(SMS_TYPE11)|(SMS_TYPE12)
#endif
#define BATTERY_FULL							(0X01<<19)  //have electricity is 1










/*
 * alarm type and items
 *
 */
typedef enum
{
	ALARM_TYPE_LOCK_SHELL,// lock shell alarm type
	ALARM_TYPE_LOCK_TEMPERATURE,// lock temperatrue
	ALARM_TYPE_GATE_TEMPERATURE,// lock temperatrue
	ALARM_TYPE_LOCK_GATE,// lock gate status
	ALARM_TYPE_GSM_RING,// lock gate status
	ALARM_TYPE_SWITCH1,// switch1 alarm type
	ALARM_TYPE_CAMERA_PHOTOSENSOR, // camera photo sensor
	ALARM_TYPE_CAMERA_IRDASENSOR, // camera irda sensor
	ALARM_TYPE_MOTOR_STATUS, // motor status sensor
	ALARM_TYPE_BATTERY_WORKING_20M,

	ALARM_TYPE_BATTERY_REMAIN_50P,//10
	ALARM_TYPE_BATTERY_REMAIN_20P,
	ALARM_TYPE_BATTERY_REMAIN_5P,
	ALARM_TYPE_BATTERY_SWITCH,
	ALARM_TYPE_FPRINT_INPUT,
	ALARM_TYPE_FPRINT_KEY_ADD,
	ALARM_TYPE_FPRINT_KEY_RIGHT,
	ALARM_TYPE_FPRINT_KEY_ERROR,
	ALARM_TYPE_CODE_KEY_ADD,     //ÃÜÂëÌí¼Ó
	ALARM_TYPE_CODE_KEY_ERROR,   //ÃÜÂë´íÎó

	ALARM_TYPE_CODE_KEY_RIGHT,   //ÕýÈ·ÃÜÂë¿ªËø 20
	ALARM_TYPE_RFID_KEY_ERROR,// rfid key detect error alarm type 14
	ALARM_TYPE_RFID_KEY_SUCCESS,// rfid key detect success alarm type
	ALARM_TYPE_RFID_KEY_PLUGIN,// rfid key detect plugin alarm type
	ALARM_TYPE_RFID_FAULT,
	ALARM_TYPE_CAMERA_FAULT,
	ALARM_TYPE_MOTOR_FAULT,
	ALARM_TYPE_POWER_FAULT,
	ALARM_TYPE_GPRS_AUTH,
	ALARM_TYPE_GPRS_HEART,

	ALARM_TYPE_GPRS_LIST_TELEPHONE,//30
	ALARM_TYPE_GPRS_LIST_RFID_KEY,
	ALARM_TYPE_GPRS_SET_TELEPHONE_SUCCESS,
	ALARM_TYPE_GPRS_SET_TELEPHONE_FAILURE,
	ALARM_TYPE_GPRS_SET_RFID_KEY_SUCCESS,
	ALARM_TYPE_GPRS_SET_RFID_KEY_FAILURE,
	ALARM_TYPE_GPRS_LIST_USER_PARAMETERS,
	ALARM_TYPE_GPRS_SET_USER_PARAMETERS_SUCCESS,
	ALARM_TYPE_GPRS_SET_USER_PARAMETERS_FAILURE,
	ALARM_TYPE_GPRS_SYS_TIME_UPDATE,

	ALARM_TYPE_GPRS_SET_TIME_SUCCESS,//40
	ALARM_TYPE_GPRS_SET_TIME_FAILURE,
	ALARM_TYPE_GPRS_SET_KEY0_SUCCESS,
	ALARM_TYPE_GPRS_SET_KEY0_FAILURE,
	ALARM_TYPE_GPRS_SET_HTTP_SUCCESS,
	ALARM_TYPE_GPRS_SET_HTTP_FAILURE,
	ALARM_TYPE_GPRS_UPLOAD_PIC,
	ALARM_TYPE_GPRS_SEND_PIC_DATA,
	ALARM_TYPE_GPRS_SLEEP,
	ALARM_TYPE_GPRS_WAKE_UP,

	ALARM_TYPE_GPRS_CAMERA_OP,//50
	ALARM_TYPE_BUTTON_ADJUST_IR,
	ALARM_TYPE_LOCK_PROCESS,
	ALARM_TYPE_KEY_ADD,
	ALARM_TYPE_KEY_DEL,
	ALARM_TYPE_KEY_RIGHT,
	ALARM_TYPE_KEY_ERROR,
	ALARM_TYPE_SMS_RF433_ERROR,
	ALARM_TYPE_SMS_KEY_ERROR,
	ALARM_TYPE_SMS_FPRINT_ERROR,

	ALARM_TYPE_SMS_REQ_IN_PHONE_CALL,//60
	ALARM_TYPE_SMS_REP_IN_PHONE_CALL,
	ALARM_TYPE_GSM_RING_REQUEST,
	ALARM_TYPE_GPRS_ADD_ACCOUNT,
	ALARM_TYPE_GPRS_ADD_PHONE,
	ALARM_TYPE_SYSTEM_FREEZE,
	ALARM_TYPE_SYSTEM_UNFREEZE,
    ALARM_TYPE_FPRINT_INFORM,
}ALARM_TYPEDEF;

/*
 *
 * alarm process flag type and items
 */
typedef enum
{
	ALARM_PROCESS_FLAG_SMS = 0x01,// sms process
	ALARM_PROCESS_FLAG_GPRS = 0x02,// gprs process
	ALARM_PROCESS_FLAG_LOCAL = 0x04,// local process
	ALARM_PROCESS_FLAG_MMS = 0x08,// local process
}ALARM_PROCESS_FLAG_TYPEDEF;

/*
 * alarm mail type
 *   --time: occured time
 *   --alarm_type: alarm event type
 *   --alarm_process_flag: howto process alarm mail
 */
typedef struct
{
	time_t time;
	ALARM_TYPEDEF alarm_type;
	ALARM_PROCESS_FLAG_TYPEDEF alarm_process_flag;
	rt_int8_t gpio_value;
}ALARM_MAIL_TYPEDEF;

/*
 * alarm msg queue
 */
extern const char *alarm_help_map[];
extern rt_event_t mail_extra_event;
/*
 * alarm mail process function
 *
 */
void alarm_thread_entry(void *parameter);
void send_alarm_mail(ALARM_TYPEDEF alarm_type, ALARM_PROCESS_FLAG_TYPEDEF alarm_process_flag, rt_int8_t gpio_value, time_t time);
/*rt_err_t machine_status_deal(rt_uint8_t operate,rt_uint8_t  option,rt_int32_t wait_time);
rt_err_t call_event_deal(rt_uint8_t mode,rt_uint8_t  option,rt_int32_t wait_time);
rt_err_t exit_gprs_pic_deal(rt_uint8_t mode,rt_uint8_t  option,rt_int32_t wait_time);
rt_err_t key_deal_stata(rt_uint8_t mode,rt_uint8_t  option,rt_int32_t wait_time);
rt_err_t camera_deal_status(rt_uint8_t mode,rt_uint8_t  option,rt_int32_t wait_time);
rt_err_t device_activate_deal(rt_uint8_t mode,rt_uint8_t  option,rt_int32_t wait_time);
rt_err_t sys_power_supply(rt_uint8_t mode,rt_uint8_t  option,rt_int32_t wait_time);

#if (defined USE_SMS_SEND_TYPE2)||(defined USE_SMS_SEND_TYPE3)
rt_uint8_t sms_type_event_flag(rt_uint8_t mode,rt_uint32_t type);
void test_file_save(const char *file,char *str,rt_uint16_t len);
void clear_picture_work_flag(void);

#endif
*/

#endif /* _ALARM_H_ */
