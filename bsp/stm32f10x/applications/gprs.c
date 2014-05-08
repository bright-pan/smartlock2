/**
  ******************************************************************************
  * @file    gprs.c
  * @author  wangzw <wangzw@yuettak.com>
  * @version v0.1
  * @date    2014-4-21
  * @brief   This file provides gprs mail functions.Implementation of message 
  *          sending and The key synchronization, and other functions.
  ******************************************************************************
  * @attention
  *
	*
  ******************************************************************************
  */
#include "gprs.h"
#include "unlockprocess.h"
#include "netmailclass.h"
#include "netfile.h"
#include "untils.h"
#include "fprint.h"
#include "keyboard.h"
#include "apppubulic.h"

#define UPDATE_KEY_CNT    60 //钥匙同步周期

static rt_mq_t gprs_mq = RT_NULL;//gprs data mail 

/** 
@brief  key upload process
@param  mail :gprs thread mail
@param  keytype :key of type
@retval void
*/
static void key_upload_process(rt_uint8_t keytype,rt_uint16_t UpdatePos)
{
	net_keyadd_user *data;
	rt_uint16_t keypos;
	rt_bool_t result;

	data = rt_calloc(1,sizeof(*data));
	RT_ASSERT(data != RT_NULL);

	keypos = UpdatePos;
  keypos = net_rev16(keypos);
  rt_memcpy(data->data.col,&keypos,2);

	data->data.type = keytype;
  net_uint32_copy_string(data->data.createt,device_config.param.key[UpdatePos].created_time);
	data->data.accredit = 0;

  net_uint32_copy_string(data->data.start_t,device_config.param.key[UpdatePos].start_time);
  
  net_uint32_copy_string(data->data.stop_t,device_config.param.key[UpdatePos].end_time);

	switch(keytype)
	{
		case KEY_TYPE_FPRINT:
		{
			data->DataLen = KEY_FPRINT_CODE_SIZE;
			break;
		}
		case KEY_TYPE_KBOARD:
		{
			data->DataLen = KEY_KBOARD_CODE_SIZE;
			break;
		}
		case KEY_TYPE_RFID:
		{
			data->DataLen = KEY_RFID_CODE_SIZE;
			break;
		}
		default:
		{
			
			rt_kprintf("Upload key type is error !!!\nfunction:%s line:%d\n",__FUNCTION__, __LINE__);
			rt_free(data);
			
			return ;
		}
	}
	
	data->data.data = rt_calloc(1,data->DataLen);
	
	RT_ASSERT(data->data.data != RT_NULL);

	device_config_key_operate(UpdatePos,(KEY_TYPE)keytype, data->data.data,0);
	
	result = msg_mail_keyadd(data);
	if(result == RT_TRUE)
	{
		set_key_update_flag(UpdatePos,0);
	}

	rt_free(data->data.data);
	rt_free(data);

}

/** 
@brief  update key lib remote
@param  mail :gprs thread mail
@retval void
*/
void update_key_lib_remote(void)
{   
	rt_uint8_t run = KEY_NUMBERS;
	KEY_TYPE   KeyType;
	
	while(run-- < KEY_NUMBERS);
	{
		rt_uint16_t pos;
		
		pos = get_key_update_pos();
		if(pos < KEY_NUMBERS)
		{  		
      KeyType = get_key_type(pos);
      key_upload_process(KeyType,pos);
 		}
	}
}


/** 
@brief  gprs thread mail process
@param  mail :gprs thread mail
@retval void
*/
static void gprs_mail_process(GPRS_MAIL_TYPEDEF *mail)
{
	switch(mail->alarm_type)
	{
		case ALARM_TYPE_CAMERA_IRDASENSOR:
		{
			msg_mail_alarm(0,motor_status(),mail->time);
			
			break;
		}
		case ALARM_TYPE_GPRS_UPLOAD_PIC:
		{
			net_upload_file(mail->user);
			
			break;
		}
		case ALARM_TYPE_RFID_KEY_ERROR:
		{
			msg_mail_alarm(2,motor_status(),mail->time);
			
			break;
		}
		case ALARM_TYPE_FPRINT_KEY_RIGHT:
		{
			rt_uint16_t *KeyPos;
			
			RT_ASSERT(mail->user != RT_NULL);

			KeyPos = (rt_uint16_t *)mail->user;
			msg_mail_opendoor(1,*KeyPos,mail->time);

			break;
		}
		case ALARM_TYPE_FPRINT_KEY_ADD:
		{
			FPrintData *data = RT_NULL;
			
			data = mail->user;
			RT_ASSERT(RT_NULL != RT_NULL);
			key_upload_process(KEY_TYPE_FPRINT,data->KeyMapPos);
			
			break;
		}
		case ALARM_TYPE_CODE_KEY_ADD:
		{
			KEYBOARD_USER_P data = RT_NULL;

			RT_ASSERT(mail != RT_NULL);
			data = mail->user;
			key_upload_process(KEY_TYPE_KBOARD,data->KeyPos);
			
			break;
		}
		case ALARM_TYPE_GPRS_SYS_TIME_UPDATE:
		{
			msg_mail_adjust_time();
		}
		default:
		{
			break;
		}
	}
}

/** 
@brief  Delete mail the memory resources
@param  mail :gprs thread mail
@retval void
*/
void gprs_mail_delete(GPRS_MAIL_TYPEDEF *mail)
{
	RT_ASSERT(mail != RT_NULL);
	if(mail->user != RT_NULL)
	{
		rt_free(mail->user);
	}
}

/** 
@brief  gprs thread entry
@param  *arg
@retval void
*/
void gprs_mail_manage_entry(void* arg)
{
	rt_err_t mq_result;
	rt_uint8_t flag = 0;
	GPRS_MAIL_TYPEDEF mail;
	rt_uint32_t count = UPDATE_KEY_CNT - 1;
	
	rt_kprintf("mail manage thread run\n");
	while(1)
	{
		//检测是否在线
		if(net_event_process(1,NET_ENVET_ONLINE) == 1)
		{
			rt_thread_delay(10);
			flag = 0;
			
			continue;
		}
		else
		{
			if(flag == 0)
			{
        send_gprs_mail(ALARM_TYPE_GPRS_SYS_TIME_UPDATE,0,RT_NULL);
        flag = 1;
			}
		}

		//更新钥匙 手机号 
		if(count++ > UPDATE_KEY_CNT)
		{
			count = 0;
      update_key_lib_remote();
		}

		//处理邮件
		mq_result = rt_mq_recv(gprs_mq,&mail,sizeof(GPRS_MAIL_TYPEDEF),100);
		if(mq_result == RT_EOK)
		{
			rt_kprintf("receive gprs mail < time: %d alarm_type: %s >\n",\
					   		mail.time, alarm_help_map[mail.alarm_type]);
			gprs_mail_process(&mail);
			gprs_mail_delete(&mail);
		}
	}
}

/** 
@brief  gprs thread mail init
@param  void 
@retval 0 :ok 1:error
*/
int gprs_mail_manage_init(void)
{
	rt_thread_t id;

	gprs_mq = rt_mq_create("GPRS",sizeof(GPRS_MAIL_TYPEDEF),10,RT_IPC_FLAG_FIFO);
	RT_ASSERT(gprs_mq != RT_NULL);
	
	id = rt_thread_create("gprs",
                         gprs_mail_manage_entry, RT_NULL,
                         1024, 107, 20);
  if(id == RT_NULL)
  {
    rt_kprintf("gprs thread init fail !\n");

    return 1;
  }

  rt_thread_startup(id);

  return 0;
}
INIT_APP_EXPORT(gprs_mail_manage_init);



void send_gprs_mail(ALARM_TYPEDEF AlarmType,time_t time,void *user)
{
	GPRS_MAIL_TYPEDEF mail;
	rt_err_t result;

	mail.alarm_type = AlarmType;
	mail.time = time;
	mail.user = user;
	
	result = rt_mq_send(gprs_mq,&mail,sizeof(GPRS_MAIL_TYPEDEF));
	//send mail fail
	if(result != RT_EOK)
	{
		if(user != RT_NULL)
		{
			rt_free(user);
		}
	}
}


#ifdef RT_USING_FINSH
#include <finsh.h>

FINSH_FUNCTION_EXPORT(send_gprs_mail,"(type,time,user)send gprs thread mail");

#endif


