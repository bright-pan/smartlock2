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
#include "gprsmailclass.h"
#include "netmailclass.h"

#define UPDATE_KEY_CNT    60 //钥匙同步周期

static rt_mq_t gprs_mq = RT_NULL;//gprs data mail 

//手机上传处理
static void phone_upload_process(rt_uint16_t pos)
{

}

//账户上传 
static void account_upload_process(rt_uint16_t pos)
{
	struct account_head *data;

	data = rt_calloc(1,sizeof(*data));
	device_config_account_operate(pos,data,0);
	
	rt_free(data);
}
/** 
@brief  key upload process
@param  mail :gprs thread mail
@param  keytype :key of type
@retval void
*/
static void key_upload_process(rt_uint16_t UpdatePos)
{
	net_keyadd_user *data = RT_NULL;
	rt_uint16_t 		keypos;
	rt_bool_t 			result;
	struct key 			*KeyData = RT_NULL;

	data = rt_calloc(1,sizeof(*data));
	RT_ASSERT(data != RT_NULL);

	//获得钥钥匙数据
	KeyData = rt_calloc(1,sizeof(*KeyData));
	RT_ASSERT(KeyData != RT_NULL);

	device_config_key_operate(UpdatePos,KeyData,0);
	
	keypos = UpdatePos;
  keypos = net_rev16(keypos);
  rt_memcpy(data->data.col,&keypos,2);

	data->data.type = KeyData->head.key_type;
  net_uint32_copy_string(data->data.createt,KeyData->head.updated_time);
	data->data.accredit = 0;

  net_uint32_copy_string(data->data.start_t,KeyData->head.start_time);
  
  net_uint32_copy_string(data->data.stop_t,KeyData->head.end_time);

	switch(KeyData->head.key_type)
	{
		case KEY_TYPE_FPRINT:
		{
			data->DataLen = KEY_FPRINT_CODE_SIZE;
			data->data.data = rt_calloc(1,data->DataLen);
			RT_ASSERT(data->data.data != RT_NULL);
			rt_memcpy(data->data.data,KeyData->data.fprint.code,data->DataLen);
			break;
		}
		case KEY_TYPE_KBOARD:
		{
			data->DataLen = KEY_KBOARD_CODE_SIZE;
			data->data.data = rt_calloc(1,data->DataLen);
			RT_ASSERT(data->data.data != RT_NULL);
			rt_memcpy(data->data.data,KeyData->data.kboard.code,data->DataLen);
			break;
		}
		case KEY_TYPE_RFID:
		{
			data->DataLen = KEY_RFID_CODE_SIZE;
			data->data.data = rt_calloc(1,data->DataLen);
			RT_ASSERT(data->data.data != RT_NULL);
			rt_memcpy(data->data.data,KeyData->data.rfid.code,data->DataLen);
			break;
		}
		default:
		{
			
			rt_kprintf("Upload key type is error !!!\nfunction:%s line:%d\n",__FUNCTION__, __LINE__);
			rt_free(data);
			rt_free(KeyData);
			return ;
		}
	}
	
	result = msg_mail_keyadd(data);
	if(result == RT_TRUE)
	{
		//标记
		KeyData->head.is_updated = 1;
		device_config_key_operate(UpdatePos,KeyData,1);
	}

	rt_free(data->data.data);
	rt_free(data);
  rt_free(KeyData);
}

//获得要更新的钥匙位置
rt_uint16_t get_key_update_pos(void)
{
	rt_uint16_t 	i;
	struct key 		*keydat;

	keydat = rt_calloc(1,sizeof(*keydat));
	for(i = 0;i < ACCOUNT_NUMBERS;i++)
	{
		device_config_key_operate(i,keydat,0);
		if(keydat->head.is_updated == 0)
		{
			return i;
		}
	}
	rt_free(keydat);

	return ACCOUNT_NUMBERS;
}

/** 
@brief  update key lib remote
@param  mail :gprs thread mail
@retval void
*/
void update_key_lib_remote(void)
{   
	rt_uint16_t run = KEY_NUMBERS;
	
	while(run-- < KEY_NUMBERS);
	{
		rt_uint16_t pos;
		
		pos = get_key_update_pos();
		if(pos < KEY_NUMBERS)
		{  		
      key_upload_process(pos);
 		}
	}
}

//获得要更新的手机位置
rt_uint16_t get_phone_update_pos(void)
{
	rt_uint16_t 				i;
	struct phone_head 	*phdata;

	phdata = rt_calloc(1,sizeof(*phdata));
	for(i = 0;i < PHONE_NUMBERS;i++)
	{
		device_config_phone_operate(i,phdata,0);
		if(phdata->is_update == 0)
		{
			return i;
		}
	}
	rt_free(phdata);

	return PHONE_NUMBERS;
}

//手机号码库远程更新
void update_phone_lib_remote(void)
{
	rt_uint16_t run = PHONE_NUMBERS;
	
	while(run-- < PHONE_NUMBERS);
	{
		rt_uint16_t pos;
		
		pos = get_phone_update_pos();
		if(pos < PHONE_NUMBERS)
		{  		
      //key_upload_process(pos);
 		}
	}
}

//获取要更新的账户位置
rt_uint16_t get_account_update_pos(void)
{
	rt_uint16_t 				  i;
	struct account_head 	*data;

	data = rt_calloc(1,sizeof(*data));
	for(i = 0;i < ACCOUNT_NUMBERS;i++)
	{
		device_config_account_operate(i,data,0);
		//if(data->is_update == 0)
		{
			return i;
		}
	}
	rt_free(data);

	return ACCOUNT_NUMBERS;
}

//账户数据远程更新
void update_account_lib_remote(void)
{
	rt_uint16_t run = ACCOUNT_NUMBERS;
	
	while(run-- < ACCOUNT_NUMBERS);
	{
		rt_uint16_t pos;
		
		pos = get_account_update_pos();
		if(pos < PHONE_NUMBERS)
		{  		
      //key_upload_process(pos);
 		}
	}
}

//处理钥匙开门正确邮件
static void gprs_key_right_mail_process(GPRS_MAIL_TYPEDEF *mail)
{
  GPRS_KeyRightUser_p user = RT_NULL;

	RT_ASSERT(mail != RT_NULL);
	user = mail->user;

	switch(user->keytype)
	{
	  case KEY_TYPE_FPRINT:
	  {
	    msg_mail_opendoor(1,user->keypos,mail->time);
	    break;
	  }
	  case KEY_TYPE_KBOARD:
	  {
	    msg_mail_opendoor(2,user->keypos,mail->time);
	    break;
	  }
	  case KEY_TYPE_RFID:
	  {
	    msg_mail_opendoor(4,user->keypos,mail->time);
	    break;
	  }
	  default:
	  {
	    break;
	  }
	}
}

//处理钥匙开门错误邮件
static void gprs_key_error_mail_process(GPRS_MAIL_TYPEDEF *mail)
{
  GPRS_KeyErrorUser_p user = RT_NULL;

	RT_ASSERT(mail != RT_NULL);
	user = mail->user;

	switch(user->type)
	{
	  case KEY_TYPE_FPRINT:
	  {
	    msg_mail_alarm(7,motor_status_get(),mail->time);
	    break;
	  }
	  case KEY_TYPE_KBOARD:
	  {
	    msg_mail_alarm(8,motor_status_get(),mail->time);
	    break;
	  }
	  /*协议中没有定义这种类型
	  case KEY_TYPE_RFID:
	  {
	    msg_mail_alarm(4,motor_status_get(),mail->time);
	    break;
	  }*/
	  default:
	  {
	  	msg_mail_alarm(2,motor_status_get(),mail->time);
	    break;
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
		case ALARM_TYPE_KEY_ADD:
		{
			rt_uint16_t *keypos;
			
			RT_ASSERT(mail != RT_NULL);
			keypos = mail->user;
			key_upload_process(*(rt_uint16_t *)keypos);
			break;
		}
		case ALARM_TYPE_KEY_RIGHT:
		{
			//钥匙正确邮件
			gprs_key_right_mail_process(mail);
			break;
		}
		case ALARM_TYPE_KEY_ERROR:
		{
			gprs_key_error_mail_process(mail);
			break;
		}
		case ALARM_TYPE_GPRS_SYS_TIME_UPDATE:
		{
			msg_mail_adjust_time();
		}
		/*case ALARM_TYPE_CAMERA_IRDASENSOR:
		{
			msg_mail_alarm(0,motor_status(),mail->time);
			
			break;
		}
		case ALARM_TYPE_GPRS_UPLOAD_PIC:
		{
			#if(PIC_UPLOAD_PIC == 1)
			rt_kprintf("System upload function is close\n");
			#else
			net_upload_file(mail->user);
			#endif
			
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
			RT_ASSERT(mail != RT_NULL);
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
		*/
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


