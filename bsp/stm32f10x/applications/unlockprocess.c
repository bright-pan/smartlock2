/**
	******************************************************************************
	* @file    unlockprocess.c
	* @author  wangzw <wangzw@yuettak.com>
	* @version v0.1
	* @date    2014-4-14
	* @brief   This file provides all unlock process
	******************************************************************************
	* @attention
	*
	*
	******************************************************************************
	*/
#include "unlockprocess.h"
#include "netmailclass.h"
#include "gprs.h"
#include "untils.h"
#include "fprint.h"
#include "apppubulic.h"
#include "gpio_pwm.h"


#define MOTOR_PWM_COUNT     50

#define PRINTF_FPRINT_INFO  1

static rt_mq_t fprint_mq = RT_NULL;

/** 
@brief  initialization fingerprint data mail 
@param  void
@retval 0 :succeed 1:fail
*/
int fprint_mail_init(void)
{
	if(fprint_mq == RT_NULL)
	{
		fprint_mq = rt_mq_create("fprint",sizeof(FPrintData),3,RT_IPC_FLAG_FIFO);
		RT_ASSERT(fprint_mq != RT_NULL);
	}

	return 0;
}
INIT_APP_EXPORT(fprint_mail_init);

/** 
@brief  send fprint input data
@param  direction : RT_TRUE->unlock 
										RT_FALSE->lock
@retval RT_TRUE :unlcok RT_FALSE:lock
*/
void motor_pwm_operate(const char *DeviceName)
{
  rt_device_t dev = RT_NULL;
  rt_uint16_t value = MOTOR_PWM_COUNT;
  
  dev = rt_device_find(DeviceName);
  if(dev != RT_NULL)
  {
    if(!(dev->open_flag & RT_DEVICE_OFLAG_OPEN))
    {
      rt_device_open(dev,RT_DEVICE_FLAG_WRONLY);
    }
    rt_device_control(dev,RT_DEVICE_CTRL_SET_PULSE_COUNTS,(void *)&value);
    rt_device_control(dev,RT_DEVICE_CTRL_SEND_PULSE,RT_NULL);
  }
}

/** 
@brief  send fprint input data
@param  direction : RT_TRUE->unlock 
										RT_FALSE->lock
@retval RT_TRUE :unlcok RT_FALSE:lock
*/
rt_bool_t motor_rotate(rt_bool_t direction)
{
	rt_bool_t result;
	
	if(direction == RT_TRUE)
	{
		motor_pwm_operate(DEVICE_NAME_MOTOR1);
	}
	else
	{
		motor_pwm_operate(DEVICE_NAME_MOTOR2);
	}

	return result;
}

/** 
@brief  send fprint input data
@param  void
@retval void
*/
void send_fprint_dat_mail(FPrintData *data)
{
	rt_mq_send(fprint_mq,(void *)data,sizeof(*data));
}

/** 
@brief  send add fingerprint data to server
@param  void
@retval void
*/
void fprint_gprs_send(rt_uint16_t  keypos)
{
	net_keyadd_user *KeyData;
	rt_uint16_t     col;

	KeyData = rt_calloc(1,sizeof(net_keyadd_user));
	RT_ASSERT(KeyData != RT_NULL);

	col = net_rev16(keypos);
	rt_memcpy(KeyData->data.col,&col,2);
	KeyData->data.type = 0;
	rt_memcpy(KeyData->data.createt,"\x12\x34\x56\x78",4);
	KeyData->data.accredit = 1;
	rt_memcpy(KeyData->data.start_t,"\x00\x09\x00\xa",4);
	rt_memcpy(KeyData->data.stop_t, "\x00\x0a\x00\xa",4);
	KeyData->DataLen = 498;
	KeyData->data.data = rt_calloc(1,KeyData->DataLen);
	RT_ASSERT(KeyData->data.data != RT_NULL);

	msg_mail_keyadd(KeyData);

	rt_free(KeyData);
}

/** 
@brief   fingerprint unlocking process
@param  void
@retval void
*/
void fprint_unlock_process(LOCAL_MAIL_TYPEDEF *mail)
{
	rt_err_t result;
	FPrintData data;

	if(system_event_process(2,SYS_FPRINT_REGISTER) == 0)
	{
		return ;
	}
	result = rt_mq_recv(fprint_mq,(void*)&data,sizeof(FPrintData),10);
	if(result == RT_EOK)
	{
		if(data.KeyMapPos == 0XFFFF)
		{
			send_voice_mail(VOICE_TYPE_KEY1_ERRPR);
		}
		else
		{
			rt_uint16_t *keypos;

			keypos = (rt_uint16_t *)rt_calloc(1,2);
			RT_ASSERT(keypos != RT_NULL);

			*keypos = data.KeyMapPos;
			motor_rotate(RT_TRUE);
			rt_kprintf("语音邮件已经发送\n");
			send_voice_mail(VOICE_TYPE_KEY1_OK);
			send_gprs_mail(ALARM_TYPE_FPRINT_KEY_RIGHT,mail->time,(void *)keypos);
		}
	}
	else
	{
		RT_DEBUG_LOG(PRINTF_FPRINT_INFO,("system error !!!\nfingerprint data mq is none mail !!!\n"));
	}
}

/** 
@brief  fingerprint key add
@param  void
@retval void
*/
void fprint_key_add(LOCAL_MAIL_TYPEDEF *mail)
{
	rt_err_t result;
	FPrintData data;
  FPRINT_ERROR_TYPEDEF fprint_result;
  rt_uint16_t fprintnum;

	system_event_process(0,SYS_FPRINT_REGISTER);//entry register
	send_voice_mail(VOICE_TYPE_MANAGE1KEY1);

	rt_kprintf("\n\n 输入管理员指纹\n");
	fprintnum = get_fprint_key_num();
	rt_kprintf("fprintnum = %d\n",fprintnum);
	if(fprintnum == 0)
	{
		rt_uint8_t Run = 10;

		RT_DEBUG_LOG(PRINTF_FPRINT_INFO,("Please enter the administrator fingerprints:\n"));
		while(Run--)
		{
      fprint_result = send_fp_mail(FPRINT_CMD_ENROLL,fprintnum);
      if(fprint_result == FPRINT_EOK)
      {
				break;
      }
      rt_thread_delay(10);
		}
    if(fprint_result != FPRINT_EOK)
    {
    	RT_DEBUG_LOG(PRINTF_FPRINT_INFO,("The fingerprint registration results:%d\n",fprint_result));
			send_voice_mail(VOICE_TYPE_REGISTER_FIAL);
			return ;
    }
	}
	result = rt_mq_recv(fprint_mq,(void*)&data,sizeof(FPrintData),1000);
	if(result == RT_EOK)
	{
		rt_kprintf("data.KeyMapPos = %d\n",data.KeyMapPos);
		if(check_fprint_pos_inof(data.KeyMapPos) == RT_TRUE)
		{
			rt_uint16_t keypos;
			rt_kprintf("是管理员\n");

			keypos = get_new_key_pos();
			if(keypos == KEY_NUMBERS)
			{
				//key position error
				RT_DEBUG_LOG(PRINTF_FPRINT_INFO,("key number is max!!!\n"));
	      send_voice_mail(VOICE_TYPE_REGISTER_FIAL);
			}
			else
			{
	      send_voice_mail(VOICE_TYPE_KEY1_INPUT);
	      RT_DEBUG_LOG(PRINTF_FPRINT_INFO,("To collect new fingerprint!!!\n"));
	      rt_thread_delay(300);
					
	      fprint_result = send_fp_mail(FPRINT_CMD_ENROLL,keypos);
	      if(fprint_result == FPRINT_EOK)
	      {
	        FPrintData *key;
	      
	        key = rt_calloc(1,sizeof(*key));
	        RT_ASSERT(key != RT_NULL);
	        key->KeyMapPos = data.KeyMapPos;

	        set_key_using_status(key->KeyMapPos,KEY_TYPE_FPRINT,1);
	        send_voice_mail(VOICE_TYPE_REGISTER_OK);
	        send_gprs_mail(mail->alarm_type,mail->time,(void *)key);
	      } 
	      else
	      {
	      	RT_DEBUG_LOG(PRINTF_FPRINT_INFO,("FPrint enroll error!!!\n"));
          send_voice_mail(VOICE_TYPE_REGISTER_FIAL);
	      }
	      
			}
		}	
		else
		{
			fprintnum  = get_fprint_key_num();
			if(fprintnum == 0)
			{
				//第一个指纹录入
				FPrintData *key;
				
				key = rt_calloc(1,sizeof(*key));
		    RT_ASSERT(key != RT_NULL);
		    key->KeyMapPos = data.KeyMapPos;

        set_key_using_status(key->KeyMapPos,KEY_TYPE_FPRINT,1);
		    send_voice_mail(VOICE_TYPE_REGISTER_OK);
		    send_gprs_mail(mail->alarm_type,mail->time,(void *)key);
			}
		}
	}
	else
	{
   	send_voice_mail(VOICE_TYPE_KEY1_OUTIME);
	}
}

#ifdef RT_USING_FINSH
#include <finsh.h>

void regfp(void)
{
	send_alarm_mail(ALARM_TYPE_FPRINT_KEY_ADD,ALARM_PROCESS_FLAG_LOCAL,0,0);
}

FINSH_FUNCTION_EXPORT(regfp,"test Computer Regietration Availability");

FINSH_FUNCTION_EXPORT(motor_rotate,"(direction) motor operated");

#endif

