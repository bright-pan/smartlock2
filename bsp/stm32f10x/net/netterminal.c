/**
  ******************************************************************************
  * @file    netphone.c
  * @author  wangzw <wangzw@yuettak.com>
  * @version v0.1
  * @date    2014-4-2
  * @brief   This file provides net message phone process functions.
  ******************************************************************************
  * @attention
  *
	*
  ******************************************************************************
  */
#include "netterminal.h"
#include "netmailclass.h"
#include "dataSYNC.h"

//#include "unlockprocess.h"
//#include "camera.h"
//#include "apppubulic.h"


#define SHOW_TM_DEBUG_IFNO     1
#define RTC_DEVICE_NAME        "rtc"

/** 
@brief modify SmartLock alarm arg
@param mail: receive net message mail
@retval RT_EOK	 :Successful operation
@retval RT_ERROR :operation failure
*/
rt_err_t net_modify_alarm_arg(net_recvmsg_p mail)
{
	return RT_EOK;
}

/** 
@brief net motor control 
@param mail: receive net message mail
@retval RT_EOK	 :Successful operation
@retval RT_ERROR :operation failure
*/
rt_err_t net_motor_Control(net_recvmsg_p mail)
{
	if(local_event_process(1,LOCAL_EVT_SYSTEM_FREEZE) == 0)
	{
		rt_kprintf("system is freeze\n");
		return RT_ERROR;
	}
	if(mail->data.motor.motor.operation == 0)
	{
		union alarm_data KeyData;

		KeyData.lock.key_id = 0;
	  KeyData.lock.operation = LOCK_OPERATION_OPEN;
	  KeyData.lock.CheckMode = LOCK_NONE_AUTH_CHECK;
	  send_local_mail(ALARM_TYPE_LOCK_PROCESS,(time_t)net_get_date(),&KeyData);
	}
	
	return RT_EOK;
}

/** 
@brief set system time
@param mail: receive net message mail
@retval RT_EOK	 :Successful operation
@retval RT_ERROR :operation failure
*/
rt_err_t net_set_system_time(net_recvmsg_p mail)
{
	rt_uint32_t CurrentTime;
	rt_device_t dev = RT_NULL;
	
	RT_ASSERT(mail != RT_NULL);

	if(mail->data.timing.result == 1)
	{
		net_string_copy_uint32(&CurrentTime,mail->data.timing.time);
		RT_DEBUG_LOG(SHOW_TM_DEBUG_IFNO,("Timing:\n%s\n",ctime((time_t *)&CurrentTime)));
		
		dev = rt_device_find(RTC_DEVICE_NAME);
		if(dev == RT_NULL)
		{
			RT_DEBUG_LOG(SHOW_TM_DEBUG_IFNO,("rtc device not find\n"));
			return RT_ERROR;
		}
		rt_device_open(dev,RT_DEVICE_OFLAG_OPEN);
		rt_device_control(dev, RT_DEVICE_CTRL_RTC_SET_TIME, &CurrentTime);
		rt_device_close(dev);
	}
	else
	{
		RT_DEBUG_LOG(SHOW_TM_DEBUG_IFNO,("Timing Result :%d\n",mail->data.timing.result));
		
		return RT_ERROR;
	}
	
	return RT_EOK;
}

/** 
@brief Remote control camera
@param mail: receive net message mail
@retval RT_EOK	 :Successful operation
@retval RT_ERROR :operation failure
*/
rt_err_t net_photograph(net_recvmsg_p mail)
{
	/*RT_ASSERT(mail != RT_NULL);
	if(mail->data.camera.dat.operation == 0)
	{
		camera_send_mail(ALARM_TYPE_GPRS_CAMERA_OP,sys_cur_date());
	}
	*/
	return RT_EOK;
}

rt_err_t net_set_key0(net_recvmsg_p mail)
{
	/*rt_uint8_t i;
	
	RT_ASSERT(mail != RT_NULL);
	RT_DEBUG_LOG(SHOW_TM_DEBUG_IFNO,("New Key0:"));
	for(i = 0 ;i < 8;i++)
	{
		RT_DEBUG_LOG(SHOW_TM_DEBUG_IFNO,("%02X",mail->data.setk0.data.key0[i]));
	}
	RT_DEBUG_LOG(SHOW_TM_DEBUG_IFNO,("\n"));
	
	config_file_mutex_op(RT_TRUE);
	rt_memcpy(device_config.param.key0,mail->data.setk0.data.key0,8);
	config_file_mutex_op(RT_FALSE);
	
	//保存文件
	device_config_file_operate(&device_config,1);
	*/
	return RT_EOK;
}


rt_err_t net_set_domain(net_recvmsg_p mail)
{
	/*
	RT_ASSERT(mail != RT_NULL);
	
	if(mail->lenmap.bit.data-2 >= TCP_DOMAIN_LENGTH)
	{
		return RT_ERROR;
	}

	config_file_mutex_op(RT_TRUE);
	rt_memset(device_config.param.tcp_domain[mail->data.domain.data,mail->data.domain.pos].domain,0,TCP_DOMAIN_LENGTH);
	rt_memcpy(device_config.param.tcp_domain[mail->data.domain.data,mail->data.domain.pos].domain,mail->data.domain.data,mail->lenmap.bit.data-3);
	rt_memcpy((rt_uint8_t *)&(device_config.param.tcp_domain[mail->data.domain.data,mail->data.domain.pos].port),mail->data.domain.data+mail->lenmap.bit.data-3,2);
	device_config.param.tcp_domain[mail->data.domain.data,mail->data.domain.pos].port = net_rev16(device_config.param.tcp_domain[mail->data.domain.data,mail->data.domain.pos].port);
	config_file_mutex_op(RT_FALSE);
	
	//保存文件
	device_config_file_operate(&device_config,1);
	
	RT_DEBUG_LOG(
	SHOW_TM_DEBUG_IFNO,("%dURL:%s \n",mail->lenmap.bit.data
	,device_config.param.tcp_domain[mail->data.domain.data,mail->data.domain.pos].domain));
	RT_DEBUG_LOG(
	SHOW_TM_DEBUG_IFNO,("Port:%d\n"
	,device_config.param.tcp_domain[mail->data.domain.data,mail->data.domain.pos].port));
	*/
	return RT_EOK;
}

rt_err_t net_set_doormode(net_recvmsg_p mail)
{
	RT_ASSERT(mail != RT_NULL);

	
	return RT_EOK;
}


rt_err_t net_data_sync(net_recvmsg_p mail)
{
	RT_ASSERT(mail != RT_NULL);
	
	
	return remote_data_sync_process();
}


#ifdef USEING_NEW_DATA_SYNC
//账户映射域上传应答处理
rt_err_t net_accmapadd_result(net_recvmsg_p mail)
{
	struct account_map_ack data;
	RT_ASSERT(mail != RT_NULL);

	data.result = mail->data.AccMapAddAck.result;
	
	sync_account_map_ack(&data);	

	return RT_EOK;
}


//账户数据校验应答处理
rt_err_t net_accdatcks_result(net_recvmsg_p mail)
{
	struct account_check_ack data;
	RT_ASSERT(mail != RT_NULL);

	data.result = mail->data.AccDatCksAck.result;
	
	sync_account_check_ack(&data);	

	return RT_EOK;
}

//钥匙映射域上传应答处理
rt_err_t net_keymapadd_result(net_recvmsg_p mail)
{
	struct key_map_ack data;
	RT_ASSERT(mail != RT_NULL);

	data.result = mail->data.KeyMapAddAck.result;
	
	sync_key_map_ack(&data);	

	return RT_EOK;
}


//钥匙数据校验应答处理
rt_err_t net_keydatcks_result(net_recvmsg_p mail)
{
	struct key_check_ack data;
	RT_ASSERT(mail != RT_NULL);

	data.result = mail->data.KeyDatCksAck.result;
	
	sync_key_check_ack(&data);	

	return RT_EOK;
}


//手机映射域上传应答处理
rt_err_t net_phmapadd_result(net_recvmsg_p mail)
{
	struct phone_map_ack data;
	RT_ASSERT(mail != RT_NULL);

	data.result = mail->data.PhMapAddAck.result;
	
	sync_phone_map_ack(&data);	

	return RT_EOK;
}


//手机数据校验应答处理
rt_err_t net_phdatcks_result(net_recvmsg_p mail)
{
	struct phone_check_ack data;
	RT_ASSERT(mail != RT_NULL);

	data.result = mail->data.PhDatCksAck.result;
	
	sync_phone_check_ack(&data);	

	return RT_EOK;
}


//记录映射域上传应答处理
rt_err_t net_recmapadd_result(net_recvmsg_p mail)
{
	struct event_map_ack data;
	RT_ASSERT(mail != RT_NULL);

	data.result = mail->data.RecMapAddAck.result;
	
	sync_event_map_ack(&data);	

	return RT_EOK;
}


//记录数据校验应答处理
rt_err_t net_recdatcks_result(net_recvmsg_p mail)
{
	struct event_check_ack data;
	RT_ASSERT(mail != RT_NULL);

	data.result = mail->data.RecDatCksAck.result;
	
	sync_event_check_ack(&data);	

	return RT_EOK;
}

#endif


