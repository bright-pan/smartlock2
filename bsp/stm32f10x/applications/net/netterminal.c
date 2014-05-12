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
#include "unlockprocess.h"
#include "camera.h"
#include "apppubulic.h"


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
	if(mail->data.motor.motor.operation == 0)
	{
    motor_rotate(RT_TRUE);
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
	RT_ASSERT(mail != RT_NULL);
	if(mail->data.camera.dat.operation == 0)
	{
		camera_send_mail(ALARM_TYPE_GPRS_CAMERA_OP,sys_cur_date());
	}
	
	return RT_EOK;
}

