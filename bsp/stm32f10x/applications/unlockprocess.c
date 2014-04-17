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
#include "camera.h"
#include "sms.h"

#define MOTOR_PWM_COUNT     50

#define PRINTF_FPRINT_INFO  1
#define READ_FPRINT_COUNT   20
#define RF_ERR_OUTTIME_T    6000
#define FP_ERR_ALARM_CNT    3

static rt_mq_t fprint_mq = RT_NULL;

typedef struct 
{
	rt_timer_t timer;
	rt_uint8_t ErrCnt;
}FPError;

static FPError fp_error = {RT_NULL,0};
static volatile rt_bool_t LockStatus;

/** 
@brief  fprint ok API
@param  *user  fprint data
@retval RT_EOK Successful operation
*/
rt_err_t fprint_ok_cb(void *user)
{
	FPINTF_USER *temp = user;
	FPrintData data;
	rt_kprintf("temp = %d\n",temp->KeyPos);
	
	data.KeyMapPos = temp->KeyPos;	
	send_fprint_dat_mail(&data);

	if(system_event_process(1,SYS_FPRINT_REGISTER) == 1)
	{
    send_alarm_mail(ALARM_TYPE_FPRINT_INPUT,
	                  ALARM_PROCESS_FLAG_LOCAL,
	                  0,0);
	}
	
	return RT_EOK;
}

/** 
@brief  fprint error API
@param  *user  fprint data
@retval RT_EOK Successful operation
*/
rt_err_t fprint_error_cb(void *user)
{
	FPINTF_USER *temp = user;
	FPrintData data;
	
	rt_kprintf("temp = %d\n",temp->KeyPos);

	data.KeyMapPos = temp->KeyPos;	
	send_fprint_dat_mail(&data);
	
	if(system_event_process(1,SYS_FPRINT_REGISTER) == 1)
	{
    send_alarm_mail(ALARM_TYPE_FPRINT_INPUT,
	                  ALARM_PROCESS_FLAG_LOCAL,
	                  0,0);
	}
	return RT_EOK;
}

/** 
@brief  fprint module init
@param  void
@retval 0 :succeed 1:fail
*/
void fprint_module_init(void)
{
	rt_uint8_t run = 10;
	
	while(--run)
	{
		if(send_fp_mail(FPRINT_CMD_INIT,0,1) != FPRINT_EOK)
		{
		  RT_DEBUG_LOG(PRINTF_FPRINT_INFO,("FPrint Init Fail !!!\n"));
		  rt_thread_delay(100);
		}
		else
		{
			break;
		}
	}
}

/** 
@brief  fprint API init
@param  void
@retval 0 :succeed 1:fail
*/
int fprint_cb_init(void)
{
	fp_ok_callback(fprint_ok_cb);
	fp_error_callback(fprint_error_cb);

	return 0;
}
INIT_APP_EXPORT(fprint_cb_init);

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
@brief  Acquiring a lock state
@param  void
@retval RT_TRUE :unlcok 
@retval RT_FALSE:lock
*/
rt_bool_t motor_status(void)
{
	return LockStatus;
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
		LockStatus = RT_TRUE;
	}
	else
	{
		motor_pwm_operate(DEVICE_NAME_MOTOR2);
		LockStatus = RT_FALSE;
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
@brief  fingerprint error timer 
@param  void
@retval void
*/
static void fprint_err_outtime(void *arg)
{
	fp_error.ErrCnt = 0;
}

/** 
@brief  fprint error Return to the initial state
@param  void
@retval void
*/
static void fprint_error_clear(void)
{
	if(fp_error.timer  == RT_NULL)
	{
		fp_error.timer  = rt_timer_create("FPerr",
																		fprint_err_outtime,
																		RT_NULL,
																		RF_ERR_OUTTIME_T,
																		RT_TIMER_FLAG_ONE_SHOT);
	RT_ASSERT(fp_error.timer  != RT_NULL)
	}
	rt_timer_stop(fp_error.timer);
	fp_error.ErrCnt = 0;
}

/** 
@brief  fingerprint input data error
@param  void
@retval void
*/
static void fprint_error_process(LOCAL_MAIL_TYPEDEF *mail)
{
	if(fp_error.timer  == RT_NULL)
	{
		fp_error.timer  = rt_timer_create("FPerr",
																		fprint_err_outtime,
																		RT_NULL,
																		RF_ERR_OUTTIME_T,
																		RT_TIMER_FLAG_ONE_SHOT);
	RT_ASSERT(fp_error.timer  != RT_NULL)
	}
	if (fp_error.timer ->parent.flag & RT_TIMER_FLAG_ACTIVATED)
	{
		fp_error.ErrCnt++;
		RT_DEBUG_LOG(PRINTF_FPRINT_INFO,("The number of input errors:%d\n",fp_error.ErrCnt));
		if(fp_error.ErrCnt == FP_ERR_ALARM_CNT)
		{
			//拍照报警
			motor_rotate(RT_FALSE);
			send_voice_mail(VOICE_TYPE_ALARM);
			
			camera_send_mail(ALARM_TYPE_RFID_KEY_ERROR,mail->time);
			
     	send_sms_mail(ALARM_TYPE_RFID_KEY_ERROR,mail->time);
     	send_gprs_mail(ALARM_TYPE_RFID_KEY_ERROR,mail->time,RT_NULL);
     	
		}
		else
		{
			send_voice_mail(VOICE_TYPE_KEY1_ERRPR);
		}
	}
	else
	{
		rt_timer_start(fp_error.timer);
		fp_error.ErrCnt++;
		send_voice_mail(VOICE_TYPE_KEY1_ERRPR);
	}
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
			fprint_error_process(mail);
		}
		else
		{
			rt_uint16_t *keypos;

			fprint_error_clear();
			keypos = (rt_uint16_t *)rt_calloc(1,2);
			RT_ASSERT(keypos != RT_NULL);

			*keypos = data.KeyMapPos;
			motor_rotate(RT_TRUE);
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

	//判断是否是第一个指纹
	fprintnum = get_fprint_key_num();
	if(fprintnum == 0)
	{
		rt_uint8_t Run = READ_FPRINT_COUNT;

		RT_DEBUG_LOG(PRINTF_FPRINT_INFO,("Please enter the administrator fingerprints:\n"));
		while(--Run)
		{
      fprint_result = send_fp_mail(FPRINT_CMD_ENROLL,fprintnum,1);
      if(fprint_result == FPRINT_EOK)
      {
				break;
      }
      else
      {
        rt_thread_delay(2);
      }
		}
    if(fprint_result != FPRINT_EOK)
    {
    	//注册失败
    	RT_DEBUG_LOG(PRINTF_FPRINT_INFO,("The fingerprint registration results:%d\n",fprint_result));
    	if(Run == 0)
	  	{
	    	send_voice_mail(VOICE_TYPE_REGISTER_FIAL);
	  	}
	  	else
	  	{
	    	send_voice_mail(VOICE_TYPE_REGISTER_FIAL);
	  	}
	  	//对文件做处理
			//send_fp_mail(FPRINT_CMD_DELETE,fprintnum,1);
			fprint_module_init();
			
			return ;
    }
	}
	
	//如果是已经注册的指纹
	result = rt_mq_recv(fprint_mq,(void*)&data,sizeof(FPrintData),1000);
	if(result == RT_EOK)
	{
		//是否为管理员指纹
		if(check_fprint_pos_inof(data.KeyMapPos) == RT_TRUE)
		{
			rt_uint16_t keypos;

			//如果钥匙库已满
			keypos = get_new_key_pos();
			if(keypos == KEY_NUMBERS)
			{
				RT_DEBUG_LOG(PRINTF_FPRINT_INFO,("Key library is full!!!\n"));
	      send_voice_mail(VOICE_TYPE_REGISTER_FIAL);
			}
			else
			{
				rt_uint8_t run = READ_FPRINT_COUNT;
				
	      send_voice_mail(VOICE_TYPE_KEY1_INPUT);
	      RT_DEBUG_LOG(PRINTF_FPRINT_INFO,("To collect new fingerprint!!!\n"));

				//采集新指纹
	      while(--run)
	      {
          fprint_result = send_fp_mail(FPRINT_CMD_ENROLL,keypos,1);
					if(fprint_result == FPRINT_EOK)
					{
						result = rt_mq_recv(fprint_mq,(void*)&data,sizeof(FPrintData),100);
						if(result == RT_EOK)
						{
							if(check_fprint_pos_inof(data.KeyMapPos) == RT_TRUE)
							{
								//这个位置已经有指纹了
								RT_DEBUG_LOG(PRINTF_FPRINT_INFO,("The fingerprint has been!!!\n"));
								fprint_result = FPRINT_EERROR;								
                //send_fp_mail(FPRINT_CMD_DELETE,fprintnum,1);
							}
              break;
						}
					}
					else
					{
						rt_thread_delay(2);
					}
	      }
					
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
	      	if(run == 0)
	      	{
            send_voice_mail(VOICE_TYPE_REGISTER_FIAL);
	      	}
	      	else
	      	{
            send_voice_mail(VOICE_TYPE_REGISTER_FIAL);
	      	}
	      }
	      
			}
		}	
		else
		{
			//如果是第一个录入的指纹
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
			else
			{
				//既不是已注册的指纹也不是第一注册的指纹
				RT_DEBUG_LOG(PRINTF_FPRINT_INFO,("FPrint enroll error!!!\n"));
        send_voice_mail(VOICE_TYPE_REGISTER_FIAL);
			}
		}
	}
	else
	{
   	send_voice_mail(VOICE_TYPE_KEY1_OUTIME);
	}
  fprint_module_init();
	system_event_process(2,SYS_FPRINT_REGISTER);
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

