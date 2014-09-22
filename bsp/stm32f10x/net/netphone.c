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
#include "untils.h"

#define SHOW_NETPHONE_INFO   1

/** 
@brief add phone process
@param mail: receive net message mail
@retval RT_EOK	 :add phone ok
@retval RT_ERROR :add phone false
*/
rt_err_t net_phone_add_process(net_recvmsg_p mail)
{
	net_recv_phoneadd *remote;
	
	RT_ASSERT(mail != RT_NULL);
	
	remote = &(mail->data.phoneadd);
	
	
	RT_DEBUG_LOG(SHOW_NETPHONE_INFO,("Add the phone number OK!!!\n"));

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
	net_recv_phonedel *remote;
	
	RT_ASSERT(mail != RT_NULL);
	
	remote = &(mail->data.phonedel);

	
	
	RT_DEBUG_LOG(SHOW_NETPHONE_INFO,("Delete the phone number OK!!!\n"));
	return RT_EOK;
}


