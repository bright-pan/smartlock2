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
	return RT_EOK;
}


