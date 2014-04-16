#include "gprs.h"
#include "unlockprocess.h"
#include "netmailclass.h"
#include "untils.h"
#include "fprint.h"
#include "apppubulic.h"

static rt_mq_t gprs_mq = RT_NULL;

/** 
@brief  fingerprint upload process
@param  mail :gprs thread mail
@retval void
*/
static void fprint_upload_process(GPRS_MAIL_TYPEDEF *mail)
{
	FPrintData *key; 
	net_keyadd_user *data;
	rt_uint16_t keypos;
	rt_uint8_t result;

	RT_ASSERT(mail != RT_NULL);
	key = (FPrintData *)mail->user;

	data = rt_calloc(1,sizeof(*data));
	RT_ASSERT(data != RT_NULL);

	keypos = key->KeyMapPos;
  keypos = net_rev16(keypos);
  rt_memcpy(data->data.col,&keypos,2);

	data->data.type = 0;
  net_uint32_copy_string(data->data.createt,mail->time);
	data->data.accredit = 1;
	rt_memcpy(data->data.start_t,"\x00\x09\x00\xa",4);
	rt_memcpy(data->data.stop_t, "\x00\x0a\x00\xa",4);
	data->DataLen = KEY_FPRINT_CODE_SIZE;
	data->data.data = rt_calloc(1,data->DataLen);
	RT_ASSERT(data->data.data != RT_NULL);
	
	device_config_key_operate(keypos,data->data.data,0);
	
	result = msg_mail_keyadd(data);
	if(result == 0)
	{
		send_fp_mail(FPRINT_CMD_DELETE,keypos);
	}

	rt_free(data);

}

static void gprs_mail_process(GPRS_MAIL_TYPEDEF *mail)
{
	switch(mail->alarm_type)
	{
		case ALARM_TYPE_CAMERA_IRDASENSOR:
		{
			msg_mail_alarm(0,1,mail->time);
			
			break;
		}
		case ALARM_TYPE_GPRS_UPLOAD_PIC:
		{
			net_upload_file(mail->user);
			
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
			fprint_upload_process(mail);
			break;
		}
		default:
		{
			break;
		}
	}
}

void gprs_mail_delete(GPRS_MAIL_TYPEDEF *mail)
{
	RT_ASSERT(mail != RT_NULL);
	if(mail->user != RT_NULL)
	{
		rt_free(mail->user);
	}
}
/*
功能:线程入口
*/
void gprs_mail_manage_entry(void* arg)
{
	rt_err_t mq_result;
	GPRS_MAIL_TYPEDEF mail;
	
	rt_kprintf("mail manage thread run\n");
	while(1)
	{
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

/*
功能:创建邮件管理线程
*/
int gprs_mail_manage_init(void)
{
	rt_thread_t id;

	gprs_mq = rt_mq_create("GPRS",sizeof(GPRS_MAIL_TYPEDEF),10,RT_IPC_FLAG_FIFO);
	RT_ASSERT(gprs_mq != RT_NULL);
	
  //设置接收处理函数
	//Net_Set_MsgRecv_Callback(net_message_recv_process);
	
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


