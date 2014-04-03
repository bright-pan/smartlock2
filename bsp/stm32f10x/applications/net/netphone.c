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

#include "netphone.h"

/** 
@brief add phone process
@param mail: receive net message mail
@retval RT_EOK	 :add phone ok
@retval RT_ERROR :add phone false
*/
rt_err_t net_phone_add_process(net_recvmsg_p mail)
{
	return RT_EOK;
}

/** 
@brief delete phone process
@param mail: receive net message mail
@retval RT_EOK: add phone ok
@retval RT_ERROR:add phone false
*/
rt_err_t net_phone_del_process(net_recvmsg_p mail)
{
	return RT_EOK;
}


