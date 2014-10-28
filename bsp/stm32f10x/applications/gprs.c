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

#define UPDATE_KEY_CNT    					60 //钥匙同步周期
#define UPDATE_FLAG_VALUE 					1

static rt_mq_t gprs_mq = RT_NULL;//gprs data mail 


//手机上传处理
static void phone_upload_process(rt_uint16_t pos)
{
	struct phone_head *data = RT_NULL;
	rt_int8_t 				op_result;
	rt_err_t					result;

	if(pos > PHONE_NUMBERS)
	{
		rt_kprintf("Pos error %s",__FUNCTION__);
		return ;
	}
	data = rt_calloc(1,sizeof(*data));
	RT_ASSERT(data != RT_NULL);
	
	op_result = device_config_phone_operate(pos,data,0);
	if(op_result < 0)
	{
  	rt_kprintf("GPRS mail phone operate fail >>%s",__FUNCTION__);
	}
	result = msg_mail_phoneadd(pos,data->auth,(rt_uint8_t *)data->address,data->updated_time);
	if(result == RT_EOK)
	{
		//手机数据上传成功
    rt_kprintf("Phone Upload data succeed\n");
		result = msg_mail_phonebind(pos,data->account,data->updated_time);
		if(result == RT_EOK)
		{
			//手机绑定成功	
			data->is_update = 1-UPDATE_FLAG_VALUE;
			device_config_phone_operate(pos,data,1);
			rt_kprintf("Phone bind succeed\n");
		}
	}
	else
	{
		rt_kprintf("Phone Upload Fail\n");		
	}

	rt_free(data);
}

//账户上传 
static void account_upload_process(rt_uint16_t pos)
{
	struct account_head *data;
	rt_err_t 						result;
	rt_int8_t           ah_result;
	
	data = rt_calloc(1,sizeof(*data));
	
	ah_result = device_config_account_operate(pos,data,0);
	if(ah_result < 0)
	{
		rt_kprintf("GPRS mail account operate fail >>%s",__FUNCTION__);
	}

	result = msg_mail_account_add(pos,(rt_uint8_t *)data->name,data->updated_time);
	if(result == RT_EOK)
	{
		//成功上传
		data->is_updated = 1-UPDATE_FLAG_VALUE;
    device_config_account_operate(pos,data,1);
		rt_kprintf("Account Upload succeed\n");
	}
	else
	{
		rt_kprintf("Account Upload Fail\n");
	}
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
	rt_err_t        result;
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

	data->data.type = KeyData->head.key_type-1;//注意由于本地数据定义与协议不匹配所以-1
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
		case KEY_TYPE_RF433:
		{
			break;
		};
		default:
		{
			
			rt_kprintf("pos %d Upload key type is error !!!\nfunction:%s line:%d\n",UpdatePos,__FUNCTION__, __LINE__);
			rt_free(data);
			rt_free(KeyData);
			return ;
		}
	}
	
	result = msg_mail_keyadd(data);
	if(result == RT_EOK)
	{
		//数据上传成功
 		result = msg_mail_keybind(UpdatePos,KeyData->head.account,KeyData->head.updated_time);
		if(result == RT_EOK)
		{
      KeyData->head.is_updated = 1-UPDATE_FLAG_VALUE;
      device_config_key_operate(UpdatePos,KeyData,1);
		}
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
	rt_int16_t    result;
	rt_int32_t   maxnum = device_config_key_counts();

	keydat = rt_calloc(1,sizeof(*keydat));
	for(i = 0;i < maxnum;i++)
	{
		result = device_config_get_key_valid(i);
		if(result == 1)
		{
			device_config_key_operate(i,keydat,0);
			if(keydat->head.is_updated == UPDATE_FLAG_VALUE)
			{
				//433钥匙不需要上传
				if(keydat->head.key_type != KEY_TYPE_RF433)
				{
          rt_free(keydat);
          return i;
				}
			}		
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
	rt_uint16_t run = device_config_key_counts();
	
	while(run--)
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
	rt_int32_t          result;
	rt_int32_t         maxnum = device_config_phone_counts();
	phdata = rt_calloc(1,sizeof(*phdata));
	for(i = 0;i < maxnum;i++)
	{
		result = device_config_get_phone_valid(i);
		
		if(result == 1)
		{
			device_config_phone_operate(i,phdata,0);
      if(phdata->is_update == UPDATE_FLAG_VALUE)
		  {
		  	rt_free(phdata);
		    return i;
		  }
		}
	}
	rt_free(phdata);

	return PHONE_NUMBERS;
}

//手机号码库远程更新
void update_phone_lib_remote(void)
{
	rt_uint16_t run = device_config_phone_counts();
	
	while(run--)
	{
		rt_uint16_t pos;
		
		pos = get_phone_update_pos();
		if(pos < PHONE_NUMBERS)
		{  		
      phone_upload_process(pos);
 		}
	}
}

//获取要更新的账户位置
rt_uint16_t get_account_update_pos(void)
{
	rt_uint16_t 				  i;
	rt_int16_t            result;
	struct account_head 	*data;
	rt_int32_t           maxnum = device_config_account_counts();

	data = rt_calloc(1,sizeof(*data));
	for(i = 0;i < maxnum;i++)
	{
		result = device_config_get_account_valid(i);
		if(result == 1)
		{
			device_config_account_operate(i,data,0);
      if(data->is_updated == UPDATE_FLAG_VALUE)
      {
      	rt_free(data);
        return i;
      }
		}
	}
	rt_free(data);
	return ACCOUNT_NUMBERS;
}

//账户数据远程更新
void update_account_lib_remote(void)
{
	rt_uint16_t run = device_config_account_counts();

	rt_kprintf("Account num %d\n",run);
	while(run--) 
	{
		rt_uint16_t pos;
		
		pos = get_account_update_pos();
		if(pos < ACCOUNT_NUMBERS)
		{  		
			rt_kprintf("update account %d\n",pos);
      account_upload_process(pos);
 		}
	}
}

//更新智能锁数据库
static void update_smartlock_database(void)
{
  rt_kprintf("update start\n");
  rt_kprintf("update account data...\n");
  update_account_lib_remote();
  
  rt_kprintf("update key data...\n");
  update_key_lib_remote();
  
  rt_kprintf("update phone data...\n");
  update_phone_lib_remote();
  rt_kprintf("update end\n");
}

//设置数据库中数据更新标志位
static void set_database_all_update(rt_uint8_t flag)
{
  rt_uint16_t           i;
	rt_int16_t            result;
	struct account_head   *data;
	struct phone_head 	  *phdata;
	struct key 		        *keydat;
	rt_int32_t           	maxnum;

	data = rt_calloc(1,sizeof(*data));
	maxnum = device_config_account_counts();
	for(i = 0;i < maxnum;i++)
	{
	  result = device_config_get_account_valid(i);
	  if(result == 1)
	  {
	    device_config_account_operate(i,data,0);
			data->is_updated = flag;		
	    device_config_account_operate(i,data,1);
	  }
	}
	rt_free(data);

	maxnum = device_config_phone_counts();
	phdata = rt_calloc(1,sizeof(*phdata));
	for(i = 0;i < maxnum;i++)
	{
	  result = device_config_get_phone_valid(i);
	  
	  if(result == 1)
	  {
	    device_config_phone_operate(i,phdata,0);
			phdata->is_update = flag;
	    device_config_phone_operate(i,phdata,1);
	  }
	}
	rt_free(phdata);

  maxnum = device_config_key_counts();
	keydat = rt_calloc(1,sizeof(*keydat));
	for(i = 0;i < maxnum;i++)
	{
		result = device_config_get_key_valid(i);
		if(result == 1)
		{
			device_config_key_operate(i,keydat,0);
			keydat->head.is_updated = flag;
			device_config_key_operate(i,keydat,1);
		}
	}
	rt_free(keydat);
}

//处理钥匙开门正确邮件
static void gprs_key_right_mail_process(GPRS_MAIL_TYPEDEF *mail)
{
  GPRSUserDef_p user = RT_NULL;
	struct key    *keydat;
	
	RT_ASSERT(mail != RT_NULL);
	user = mail->user;
	keydat = rt_calloc(1,sizeof(*keydat));
	RT_ASSERT(keydat != RT_NULL);
	
	device_config_key_operate(user->keyright.pos,keydat,0);
	
	switch(user->keyright.type)
	{
	  case KEY_TYPE_FPRINT:
	  {
	    msg_mail_opendoor(1,keydat->head.account,user->keyright.pos,mail->time);
	    break;
	  }
	  case KEY_TYPE_KBOARD:
	  {
	    msg_mail_opendoor(2,keydat->head.account,user->keyright.pos,mail->time);
	    break;
	  }
	  case KEY_TYPE_RFID:
	  {
	    msg_mail_opendoor(4,keydat->head.account,user->keyright.pos,mail->time);
	    break;
	  }
	  case KEY_TYPE_RF433:
	  {
			msg_mail_opendoor(5,keydat->head.account,user->keyright.pos,mail->time);
	    break;
	  }
	  default:
	  {
	  	rt_kprintf("Key right process type error!!!\n");
	    break;
	  }
	}

	rt_free(keydat);
}

//处理钥匙开门错误邮件
static void gprs_key_error_mail_process(GPRS_MAIL_TYPEDEF *mail)
{
  GPRSUserDef_p user = RT_NULL;

	RT_ASSERT(mail != RT_NULL);
	user = mail->user;

	switch(user->keyerr.type)
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

//账户添加处理
static void gprs_account_add_mail_process(GPRS_MAIL_TYPEDEF *mail)
{
	GPRSUserDef_p  user = RT_NULL;
	
	RT_ASSERT(mail != RT_NULL);
	RT_ASSERT(mail->user != RT_NULL);

	user = mail->user;

	msg_mail_account_add(user->AccountAdd.pos,user->AccountAdd.name,user->AccountAdd.date);
}

//手机添加处理
static void gprs_phone_add_mail_process(GPRS_MAIL_TYPEDEF *mail)
{
	GPRSUserDef_p  user = RT_NULL;
	
	RT_ASSERT(mail != RT_NULL);
	RT_ASSERT(mail->user != RT_NULL);

	user = mail->user;

	msg_mail_phoneadd(user->PhoneAdd.pos,user->PhoneAdd.auth,user->PhoneAdd.code,user->PhoneAdd.date);
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
			break;
		}
		case ALARM_TYPE_GPRS_ADD_ACCOUNT:
		{
			gprs_account_add_mail_process(mail);
			break;
		}
		case ALARM_TYPE_GPRS_ADD_PHONE:
		{
			gprs_phone_add_mail_process(mail);
			break;
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
		rt_kprintf("Delete GPRS Mail User");
		rt_free(mail->user);
	}
}

//保存没有上传的记录
void gprs_local_mail_save(GPRS_MAIL_TYPEDEF *mail)
{
	union event_data *data = RT_NULL;
	GPRSUserDef_p    user = RT_NULL;
	
	data = rt_calloc(1,sizeof(*data));
	RT_ASSERT(data != RT_NULL);

	switch(mail->alarm_type)
	{
		case ALARM_TYPE_KEY_ERROR:
		{	
			RT_ASSERT(mail->user != RT_NULL);
			user = mail->user;

			switch(user->keyerr.type)
			{
				case KEY_TYPE_FPRINT:
				{
					data->alarm.flag = 7;
					break;
				}
				case KEY_TYPE_KBOARD:
				{
					data->alarm.flag = 8;
					break;
				}
				default:
				{
					break;
				}
			}
			data->alarm.status = motor_status_get();
			data->alarm.time  = mail->time;
			device_config_event_create(EVENT_ID_INVALID,EVENT_TYPE_ALARM,1,data);		
			break;
		}
		case ALARM_TYPE_KEY_RIGHT:
		{
			struct key *keydat = RT_NULL;
			
			keydat = rt_calloc(1,sizeof(*keydat));
			RT_ASSERT(keydat != RT_NULL);
			
			device_config_key_operate(user->keyright.pos,keydat,0);
			data->unlock.account_id = keydat->head.account;
			data->unlock.key_id = user->keyright.pos;
			data->unlock.time = mail->time;
			data->unlock.type = keydat->head.key_type-1;
			device_config_event_create(EVENT_ID_INVALID,EVENT_TYPE_UNLOCK,1,data);	

			rt_free(keydat);
			break;
		}
		default:
		{
			break;
		}
	}
	rt_free(data);
}

//重发记录
int gprs_local_mail_resend(struct event *data, void *user)
{
	switch(data->head.event_type)
	{
		case EVENT_TYPE_ALARM:
		{
			if(data->head.is_updated == 1)
			{
        msg_mail_alarm(data->data.alarm.flag,data->data.alarm.status,data->data.alarm.time);
			}
			
			break;
		}
		case EVENT_TYPE_UNLOCK:
		{
			if(data->head.is_updated == 1)
			{
        msg_mail_opendoor(data->data.unlock.type,data->data.unlock.account_id,data->data.unlock.key_id,data->data.unlock.time);
			}
			break;
		}
		default:
		{
			rt_kprintf("Read resend type is error\n");
			break;
		}
	}
	return 0;
}

/** 
@brief  gprs thread entry
@param  *arg
@retval void
*/
void gprs_mail_manage_entry(void* arg)
{
	rt_err_t 					mq_result;
	rt_uint8_t 				login_flag = 0;
	GPRS_MAIL_TYPEDEF mail;
	rt_uint32_t 			count;
	
	while(1)
	{
		//检测是否在线
		if(net_event_process(1,NET_ENVET_ONLINE) == 1)
		{
			mq_result =rt_mq_recv(gprs_mq,&mail,sizeof(GPRS_MAIL_TYPEDEF),100);
			if(mq_result == RT_EOK)
			{
				gprs_local_mail_save(&mail);
				gprs_mail_delete(&mail);//释放资源
			}
			login_flag = 0;

			continue;
		}
		else
		{
			if(login_flag == 0)
			{
				send_gprs_mail(ALARM_TYPE_GPRS_SYS_TIME_UPDATE,0,RT_NULL);
				set_database_all_update(1);
				count = UPDATE_KEY_CNT;
				device_config_event_index(gprs_local_mail_resend,RT_NULL);
        login_flag = 1;
			}
		}
		//更新钥匙 手机号 
		if(count++ > UPDATE_KEY_CNT)
		{
			count = 0;

			update_smartlock_database();
		}
		
		//处理邮件
		mq_result = rt_mq_recv(gprs_mq,&mail,sizeof(GPRS_MAIL_TYPEDEF),100);
		if(mq_result == RT_EOK)
		{
			rt_kprintf("receive gprs mail < time: %d alarm_type: %s >\n",\
					   		mail.time, alarm_help_map[mail.alarm_type]);
			gprs_local_mail_save(&mail);
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

void test_int(void)
{
	rt_int8_t cmd1;
	rt_uint8_t cmd2;

	cmd1 = 0x8b;
	cmd2 = 0x8b;

	rt_kprintf("int cmd1 = 0x8b; uint cmd2 = 0x8b;\n");
	
	rt_kprintf("int  %d = %x\n",cmd1,cmd1);
	
	rt_kprintf("uint %d = %x\n",cmd2,cmd2);

	
	rt_kprintf("uint %d = %d = %x\n",cmd1,cmd2,cmd2);
}
FINSH_FUNCTION_EXPORT(test_int,"int test");

#endif


