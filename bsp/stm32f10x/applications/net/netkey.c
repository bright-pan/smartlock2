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
#include "untils.h"
#include "unlockprocess.h"
#include "apppubulic.h"

#define SHOW_NETKEY_INFO      1

/** 
@brief Check whether the key data received legally
@param receive net message mail
@retval RT_EOK	 :add ok
@retval RT_ERROR :add false
*/
static rt_bool_t check_net_key_data(rt_uint16_t keypos,rt_uint8_t KeyType)
{
	if(keypos >= KEY_NUMBERS)
	{
		return RT_FALSE;
	}
	if(KeyType > KEY_TYPE_KBOARD)
	{
		return RT_FALSE;
	}

	return RT_TRUE;
}

/** 
@brief add key process
@param receive net message mail
@retval RT_EOK	 :add ok
@retval RT_ERROR :add false
*/
rt_err_t net_key_add_process(net_recvmsg_p mail)
{
	KEY_TYPEDEF *key = RT_NULL;
	net_recv_keyadd *remote;
	rt_uint16_t keypos;
	
	key = (KEY_TYPEDEF*)rt_calloc(1,sizeof(KEY_TYPEDEF));
	RT_ASSERT(key != RT_NULL);
	
	remote = &(mail->data.keyadd);

	net_string_copy_uint16(&keypos,remote->col);

	if(check_net_key_data(keypos,remote->type) == RT_FALSE)
	{
		RT_DEBUG_LOG(SHOW_NETKEY_INFO,("Remote key information error!!!\n"));
		rt_free(key);
		return RT_ERROR;
	}
	
	if(device_config.param.key[keypos].is_updated != 0)
	{
		RT_DEBUG_LOG(SHOW_NETKEY_INFO,("Remote key position in Terminal is need updated!!!\n"));
		rt_free(key);
		return RT_ERROR;
	}

	key->key_type = (KEY_TYPE)remote->type;
	key->operation_type = (OPERATION_TYPE)remote->accredit;
	net_string_copy_uint32((rt_uint32_t *)&key->created_time,remote->createt);
	net_string_copy_uint32((rt_uint32_t *)&key->start_time,remote->start_t);
	net_string_copy_uint32((rt_uint32_t *)&key->end_time,remote->stop_t);

	if(device_config.param.key[keypos].created_time < key->created_time)
	{
	  key->flag = 1;
	  key->is_updated = 1; 	
	  
		RT_ASSERT(device_config.mutex != RT_NULL);
		rt_mutex_take(device_config.mutex,RT_WAITING_FOREVER);
	
	  device_config.param.key[keypos] = *key;

	  rt_mutex_release(device_config.mutex);
	  device_config_key_operate(keypos, key->key_type,remote->data,1);
		device_config_file_operate(&device_config,1);   
		fprint_module_init();
		RT_DEBUG_LOG(SHOW_NETKEY_INFO,("Remote add keys to success!!!\n"));
	}
	else
	{
		RT_DEBUG_LOG(SHOW_NETKEY_INFO,("Remote key information is too old!!!\n"));
	}
	
	rt_free(key);
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
	net_keydelete *remote;
	rt_uint16_t keypos;
	
	remote = &(mail->data.keydel.key);

	net_string_copy_uint16(&keypos,remote->pos);

	if(keypos >= KEY_NUMBERS)
	{
    RT_DEBUG_LOG(SHOW_NETKEY_INFO,("Remote key positional fault!!!\n"));
		return RT_ERROR;
	}
	rt_memset(&device_config.param.key[keypos],0,sizeof(KEY_TYPEDEF));

	fprint_module_init();
	
  RT_DEBUG_LOG(SHOW_NETKEY_INFO,("Remote delete keys to success!!!\n"));
  
	return RT_EOK;
}

