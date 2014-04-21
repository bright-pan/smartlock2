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
	
	if(device_config.mutex == RT_NULL)
	{
		RT_DEBUG_LOG(SHOW_NETPHONE_INFO,("device_config.mutex is RT_NULL!!!\n"));
		return RT_ERROR;
	}
	rt_mutex_take(device_config.mutex,RT_WAITING_FOREVER);

	device_config.param.telephone_address[remote->pos].flag = 1;
	rt_memcpy(device_config.param.telephone_address[remote->pos].address,
						remote->data,11);

	device_config.param.telephone_address[remote->pos].address[11] = '\0';
	
	rt_mutex_release(device_config.mutex);

	device_config_file_operate(&device_config,1);  

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
	
	if(device_config.mutex == RT_NULL)
	{
		RT_DEBUG_LOG(SHOW_NETPHONE_INFO,("device_config.mutex is RT_NULL!!!\n"));
		return RT_ERROR;
	}
	rt_mutex_take(device_config.mutex,RT_WAITING_FOREVER);

	device_config.param.telephone_address[remote->pos].flag = 0;
	rt_memset(device_config.param.telephone_address[remote->pos].address,
						0,12);
	
	rt_mutex_release(device_config.mutex);

	device_config_file_operate(&device_config,1);  

	
	RT_DEBUG_LOG(SHOW_NETPHONE_INFO,("Delete the phone number OK!!!\n"));
	return RT_EOK;
}


