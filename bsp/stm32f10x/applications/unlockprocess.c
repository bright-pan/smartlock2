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
#include "keyboard.h"

#define PRINTF_FPRINT_INFO  1

#define AUTO_LOCK_TIME			10 		//�Զ�����ʱ��

#define MOTOR_PWM_COUNT     50		//���pwm����
#define READ_FPRINT_COUNT   20		//ָ�Ʋɼ�����
#define RF_ERR_OUTTIME_T    6000  //ָ�ƴ����������ʱ��
#define FP_ERR_ALARM_CNT    3     //�������ָ�ƴ���Ĵ���

//fingerprint data transfer mail
static rt_mq_t FPdata_mq = RT_NULL;
static rt_mutex_t keymanage_mutex = RT_NULL;

typedef struct 
{
	rt_timer_t timer;
	rt_uint8_t ErrCnt;
}FPError;

typedef struct 
{
	volatile rt_bool_t Status;
	rt_uint16_t        cnt;
}MotorDef;

static FPError fp_error = {RT_NULL,0};
static MotorDef LockStatus;

/** 
@brief  fingerprint ok API
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
@brief  fingerprint error API
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
@brief  fingerprint module init
@param  void
@retval 0 :succeed 1:fail
*/
rt_err_t fprint_module_init(void)
{
	rt_uint8_t run = 10;
	
	while(--run)
	{
		if(send_fp_mail(FPRINT_CMD_INIT,0,1) != FPRINT_EOK)
		{
		  RT_DEBUG_LOG(PRINTF_FPRINT_INFO,("FPrint Init Fail !!!\n"));
		}
		else
		{
			break;
		}
	}

	return (run==0)?RT_ERROR:RT_EOK;
}

/** 
@brief  fingerprint API init
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
@brief  keyboard API process function
@param  user:keyboard process data
@retval 0 :succeed 1:fail
*/
rt_err_t keyboard_event_process(void *user)
{
	rt_uint32_t AlarmTime;
	KEYBOARD_USER_P data = (KEYBOARD_USER_P)user;

	RT_ASSERT(data != RT_NULL);
	AlarmTime = sys_cur_date();
	/*
	KEY_NOTIFYSOUND,		
	KEY_UNLOCK_OK,			//�����ɹ�
	KEY_CODE_ERROR,			//�������
	KEY_UNLOCK_FAIL,		//����ʧ��
	KEY_SET_MODE,				//��������ģʽ
	KEY_NORMAL_MODE,    //��ͨģʽ
	KEY_INPUT_NEW_CODE, //��ʾ����������
	*/
	switch(data->event)
	{
		case KEY_NOTIFYSOUND:
		{
			//��ʾ����
			RT_DEBUG_LOG(PRINTF_FPRINT_INFO,("��\n"));

			send_voice_mail(VOICE_TYPE_KEY2_HINT);
			break;
		}
		case KEY_UNLOCK_OK:
		{
			//�����ɹ�
			RT_DEBUG_LOG(PRINTF_FPRINT_INFO,("�����ɹ�\n"));

			send_voice_mail(VOICE_TYPE_KEY1_OK);
			break;
		}
		case KEY_CODE_ERROR:
		{
			//�������
			RT_DEBUG_LOG(PRINTF_FPRINT_INFO,("�������\n"));
			
			send_voice_mail(VOICE_TYPE_KEY1_ERRPR);
			break;
		}
		case KEY_UNLOCK_FAIL:
		{
			//����ʧ��
			RT_DEBUG_LOG(PRINTF_FPRINT_INFO,("����ʧ��\n"));

			send_voice_mail(VOICE_TYPE_ALARM);
			send_gprs_mail(ALARM_TYPE_RFID_KEY_ERROR,AlarmTime,RT_NULL);
			break;
		}
		case KEY_SET_MODE:
		{
			//��������ģʽ
			RT_DEBUG_LOG(PRINTF_FPRINT_INFO,("��������ģʽ\n"));

			send_voice_mail(VOICE_TYPE_KEY2_SET_MODE);
			break;
		}
		case KEY_NORMAL_MODE:
		{
			//��ͨģʽ
			RT_DEBUG_LOG(PRINTF_FPRINT_INFO,("��ͨģʽ\n"));

			send_voice_mail(VOICE_TYPE_KEY2_NORMAL_MODE);
			break;
		}
		case KEY_INPUT_NEW_CODE:
		{
			//��ʾ����������
			RT_DEBUG_LOG(PRINTF_FPRINT_INFO,("��ʾ����������\n"));

			send_voice_mail(VOICE_TYPE_KEY2_INPUT);
			break;
		}
		case KEY_REINPUT_NEW_CODE:
		{
			//��ʾ������������
			RT_DEBUG_LOG(PRINTF_FPRINT_INFO,("��ʾ������������\n"));

			send_voice_mail(VOICE_TYPE_KEY2_RRINPUT);
			break;
		}
		case KEY_CHOOSE_MODE:
		{
			//��ѡ������ģʽ
			RT_DEBUG_LOG(PRINTF_FPRINT_INFO,("��ѡ������ģʽ\n"));

			send_voice_mail(VOICE_TYPE_KEY2_CHOOSE_MODE);
			break;
		}
		case KEY_MODE_INPUT_ERROR:
		{
			//ģʽ����
			RT_DEBUG_LOG(PRINTF_FPRINT_INFO,("ģʽ����\n"));

			send_voice_mail(VOICE_TYPE_KEY2_MODE_ERROR);
			break;
		}
		case KEY_REGISTER_OK:
		{
			//ע��ɹ� 
			KEYBOARD_USER_P KeyData;

			KeyData = rt_calloc(1,sizeof(KEYBOARD_USER));
			RT_ASSERT(KeyData != RT_NULL);
			KeyData->KeyPos = data->KeyPos;
			
			RT_DEBUG_LOG(PRINTF_FPRINT_INFO,("ע��ɹ�\n"));
			send_voice_mail(VOICE_TYPE_REGISTER_OK);
			send_gprs_mail(ALARM_TYPE_CODE_KEY_ADD,AlarmTime,KeyData);
			break;
		}
		case KEY_REGISTER_FAIL:
		{
			//ע��ʧ��
			RT_DEBUG_LOG(PRINTF_FPRINT_INFO,("ע��ʧ��\n"));
			send_voice_mail(VOICE_TYPE_REGISTER_FIAL);
			break;
		}
		case KEY_LIB_FULL:
		{
			//Կ�׿�����
			RT_DEBUG_LOG(PRINTF_FPRINT_INFO,("Կ�׿�����\n"));
			send_voice_mail(VOICE_TYPE_KEY_FULL);
			break;
		}
		default:
		{
			break;
		}
	}

	return RT_EOK;
}

/** 
@brief  initialization keyboard API port
@param  void
@retval 0 :succeed 1:fail
*/
int keyboard_cb_init(void)
{
	key_api_port_callback(keyboard_event_process);
	
	return 0;
}
INIT_APP_EXPORT(keyboard_cb_init);

/** 
@brief  initialization fingerprint data mail 
@param  void
@retval 0 :succeed 1:fail
*/
int fprint_mail_init(void)
{
	if(FPdata_mq == RT_NULL)
	{
		FPdata_mq = rt_mq_create("fprint",sizeof(FPrintData),3,RT_IPC_FLAG_FIFO);
		RT_ASSERT(FPdata_mq != RT_NULL);
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
	return LockStatus.Status;
}

/** 
@brief  set motor new status
@param  vonew_statusid
@retval void
*/

static void set_motor_status(rt_bool_t new_status)
{
	rt_base_t leave;

	leave = rt_hw_interrupt_disable();

	LockStatus.Status = new_status;
	LockStatus.cnt = 0;
	
	rt_hw_interrupt_enable(leave);
}

/** 
@brief  send fprint input data
@param  direction : RT_TRUE->unlock 
										RT_FALSE->lock
@retval RT_TRUE :unlcok RT_FALSE:lock
*/
rt_bool_t motor_rotate(rt_bool_t direction)
{
	rt_bool_t result = 0;
	
	if((direction == RT_TRUE) && (motor_status() != RT_TRUE))
	{
		motor_pwm_operate(DEVICE_NAME_MOTOR2);
		set_motor_status(RT_TRUE);
	}
	else if((direction == RT_FALSE) && (motor_status() != RT_FALSE))
	{
    motor_pwm_operate(DEVICE_NAME_MOTOR1);
		set_motor_status(RT_FALSE);
	}

	return result;
}

/** 
@brief  motor auto lock
@param  flag: RT_TRUE clear counter
							RT_FALSE summary counter 
@retval void
*/
void motor_auto_lock(rt_bool_t clear)
{
	rt_uint8_t flag = 0;
  rt_base_t leave;
  
  leave = rt_hw_interrupt_disable();

	if(clear == RT_TRUE)
	{
		LockStatus.cnt = 0;
	}
	LockStatus.cnt++;
	//rt_kprintf("%d",LockStatus.cnt);
	if(LockStatus.cnt > AUTO_LOCK_TIME)
	{
		LockStatus.cnt = 0;
		flag = 1;
	}
	rt_hw_interrupt_enable(leave);
	if(flag == 1)
	{
    motor_rotate(RT_FALSE);
	}
}

/** 
@brief  motor default state
@param  void
@retval 0:ok 1:file
*/
int motor_status_init(void)
{
	motor_pwm_operate(DEVICE_NAME_MOTOR2);
	set_motor_status(RT_TRUE);

	return 0;
}
INIT_APP_EXPORT(motor_status_init);


/** 
@brief  send fingerprint input data
@param  void
@retval void
*/
void send_fprint_dat_mail(FPrintData *data)
{
	if(data != RT_NULL)
	{
    rt_mq_send(FPdata_mq,(void *)data,sizeof(*data));
	}
	else
	{
		rt_kprintf("%s:%d Fingerprint Mail data is RT_NULL!!!\n", __FUNCTION__, __LINE__);
	}
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
	motor_rotate(RT_FALSE);
	if (fp_error.timer ->parent.flag & RT_TIMER_FLAG_ACTIVATED)
	{
		fp_error.ErrCnt++;
		RT_DEBUG_LOG(PRINTF_FPRINT_INFO,("The number of input errors:%d\n",fp_error.ErrCnt));
		if(fp_error.ErrCnt > FP_ERR_ALARM_CNT)
		{
			//���ձ���
			send_voice_mail(VOICE_TYPE_ALARM);
			
			camera_send_mail(ALARM_TYPE_RFID_KEY_ERROR,mail->time);
			
     	send_sms_mail(ALARM_TYPE_RFID_KEY_ERROR,mail->time);
     	send_gprs_mail(ALARM_TYPE_RFID_KEY_ERROR,mail->time,RT_NULL);
     	fprint_error_clear();
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
	result = rt_mq_recv(FPdata_mq,(void*)&data,sizeof(FPrintData),10);
	if(result == RT_EOK)
	{
		if(data.KeyMapPos == 0XFFFF)
		{
			fprint_error_process(mail);
		}
		else
		{
			rt_uint16_t *keypos;

			result = check_open_access(data.KeyMapPos);
			if(result == RT_TRUE)
			{
        motor_auto_lock(RT_TRUE);
	      fprint_error_clear();
	      keypos = (rt_uint16_t *)rt_calloc(1,2);
	      RT_ASSERT(keypos != RT_NULL);
	      *keypos = data.KeyMapPos;
	      motor_rotate(RT_TRUE);
	      send_voice_mail(VOICE_TYPE_KEY1_OK);
	      send_gprs_mail(ALARM_TYPE_FPRINT_KEY_RIGHT,mail->time,(void *)keypos);
			}
			else
			{
				//û��Ȩ��
        fprint_error_process(mail);
			}
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
static void fprint_key_add(LOCAL_MAIL_TYPEDEF *mail)
{
	rt_err_t result;
	FPrintData data;
  rt_uint16_t fprintnum = 0;
  FPRINT_ERROR_TYPEDEF fprint_result = FPRINT_EOK;

	system_event_process(0,SYS_FPRINT_REGISTER);//entry register
	send_voice_mail(VOICE_TYPE_MANAGE1KEY1);

	//�ж��Ƿ��ǵ�һ��ָ��
	fprintnum = get_fprint_key_num();
	if(fprintnum == 0)
	{
		rt_uint8_t Run = READ_FPRINT_COUNT;

		RT_DEBUG_LOG(PRINTF_FPRINT_INFO,("Please enter the administrator fingerprints:\n"));
		while(--Run)
		{
			rt_uint16_t keypos;
			
			keypos = get_new_key_pos();
		fprint_result = send_fp_mail(FPRINT_CMD_ENROLL,keypos,1);
		if(fprint_result == FPRINT_EOK)
		{
				break;
      }
      else
      {
      	result = rt_mq_recv(FPdata_mq,(void*)&data,sizeof(FPrintData),10);
      }
		}
    if(fprint_result != FPRINT_EOK)
    {
    	//ע��ʧ��
    	RT_DEBUG_LOG(PRINTF_FPRINT_INFO,("The fingerprint registration results:%d\n",fprint_result));
    	
    	if(Run == 0)
	  	{
	    	send_voice_mail(VOICE_TYPE_REGISTER_FIAL);
	  	}
	  	else
	  	{
	    	send_voice_mail(VOICE_TYPE_REGISTER_FIAL);
	  	}
	  	//���ļ�������
			//send_fp_mail(FPRINT_CMD_DELETE,fprintnum,1);
			//fprint_module_init();
			
			return ;
    }
	}
	
	//������Ѿ�ע���ָ��
	result = rt_mq_recv(FPdata_mq,(void*)&data,sizeof(FPrintData),1000);
	if(result == RT_EOK)
	{
		//�Ƿ�Ϊ����Աָ��
		if(check_fprint_pos_inof(data.KeyMapPos) == RT_TRUE)
		{
			rt_uint16_t keypos;

			//���Կ�׿�����
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

				//�ɼ���ָ��
	      while(--run)
	      {
          fprint_result = send_fp_mail(FPRINT_CMD_ENROLL,keypos,1);
					if(fprint_result == FPRINT_EOK)
					{
						result = rt_mq_recv(FPdata_mq,(void*)&data,sizeof(FPrintData),50);
						if(result == RT_EOK)
						{
							if(check_fprint_pos_inof(data.KeyMapPos) == RT_TRUE)
							{
								//���λ���Ѿ���ָ����
								RT_DEBUG_LOG(PRINTF_FPRINT_INFO,("The fingerprint has been!!!\n"));
								fprint_result = FPRINT_EERROR;								
                //send_fp_mail(FPRINT_CMD_DELETE,fprintnum,1);
							}
              break;
						}
					}
					else
					{
						result = rt_mq_recv(FPdata_mq,(void*)&data,sizeof(FPrintData),2);
					}
	      }
					
	      if((fprint_result == FPRINT_EOK) && (data.KeyMapPos < KEY_NUMBERS))
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
			//����ǵ�һ��¼���ָ��
			fprintnum  = get_fprint_key_num();
			if((fprintnum == 0) && ((data.KeyMapPos < KEY_NUMBERS)))
			{
				//��һ��ָ��¼��
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
				//�Ȳ�����ע���ָ��Ҳ���ǵ�һע���ָ��
				RT_DEBUG_LOG(PRINTF_FPRINT_INFO,("FPrint enroll error!!!\n"));
        send_voice_mail(VOICE_TYPE_REGISTER_FIAL);
			}
		}
	}
	else
	{
   	send_voice_mail(VOICE_TYPE_KEY1_OUTIME);
	}
  //fprint_module_init();
	system_event_process(2,SYS_FPRINT_REGISTER);
}


void fprint_key_add_porcess(LOCAL_MAIL_TYPEDEF *mail)
{
	rt_err_t result;
	
  result = keylib_mutex_op(RT_TRUE,RT_WAITING_NO);
	if(result == RT_EOK)
	{
	  fprint_key_add(mail);
	  keylib_mutex_op(RT_FALSE,RT_WAITING_NO);
	}
	else
	{
 		RT_DEBUG_LOG(PRINTF_FPRINT_INFO,("Can't add keys at the same time!!!\n"));
    send_voice_mail(VOICE_TYPE_REGISTER_FIAL);
	}
}

/** 
@brief  init key library mutex
@param  void
@retval 1:ok 
@retval 0:fail
*/
int init_keylib_mutex(void)
{
	keymanage_mutex = rt_mutex_create("keylib",RT_IPC_FLAG_FIFO);
	RT_ASSERT(keymanage_mutex != RT_NULL);

	return 0;
}
INIT_APP_EXPORT(init_keylib_mutex);


/** 
@brief  init key library mutex
@param  way:RT_TRUE take;RT_FALSE release
@param  wait mutex wait time
@retval 1:ok 
@retval 0:fail
*/
rt_err_t keylib_mutex_op(rt_bool_t way,rt_int32_t wait)
{
	rt_err_t result = RT_EOK;
	
	if(way == RT_TRUE)
	{
		result = rt_mutex_take(keymanage_mutex,wait);
	}
	else
	{
		rt_mutex_release(keymanage_mutex);
	}

	return result;
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

