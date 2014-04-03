/**
  ******************************************************************************
  * @file    netkey.c
  * @author  wangzw <wangzw@yuettak.com>
  * @version v0.1
  * @date    2014-4-2
  * @brief   This file provides net message key process functions.
  ******************************************************************************
  * @attention
  *
	*
  ******************************************************************************
  */

#include "netkey.h"


/** 
@brief add key process
@param receive net message mail
@retval RT_EOK	 :add ok
@retval RT_ERROR :add false
*/
rt_err_t net_key_add_process(net_recvmsg_p mail)
{
	return RT_EOK;
}

/** 
@brief delete key process
@param receive net message mail
@retval RT_EOK	 :add ok
@retval RT_ERROR :add false
*/
rt_err_t net_key_del_process(net_recvmsg_p mail)
{
	return RT_EOK;
}
