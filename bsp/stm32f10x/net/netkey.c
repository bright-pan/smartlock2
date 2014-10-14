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
//#include "apppubulic.h"

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
	net_recv_keyadd *remote;
  struct key			*keydat;
	rt_uint16_t     KeyID;
	rt_int32_t			KeyOpResult;
	
	RT_ASSERT(remote != RT_NULL);
  remote = &(mail->data.keyadd);

	keydat = rt_calloc(1,sizeof(*keydat));

	net_string_copy_uint16(&KeyID,remote->col);
	keydat->head.key_type = remote->type+1;//1//1.Ö¸ÎÆ 2.RFID 3.ÃÜÂë
	keydat->head.operation_type = remote->accredit+1;
	net_string_copy_uint32((rt_uint32_t *)&keydat->head.updated_time,remote->createt);
	net_string_copy_uint32((rt_uint32_t *)&keydat->head.start_time,remote->start_t);
	net_string_copy_uint32((rt_uint32_t *)&keydat->head.end_time,remote->stop_t);
	
	RT_DEBUG_LOG(SHOW_NETKEY_INFO,("Key add data info:>>>>>>\n"));
	RT_DEBUG_LOG(SHOW_NETKEY_INFO,("KeyID                       = %x\n",KeyID));
	RT_DEBUG_LOG(SHOW_NETKEY_INFO,("keydat->head.key_type       = %x\n",keydat->head.key_type));
	RT_DEBUG_LOG(SHOW_NETKEY_INFO,("keydat->head.operation_type = %x\n",keydat->head.operation_type));
	RT_DEBUG_LOG(SHOW_NETKEY_INFO,("keydat->head.is_updated     = %x\n",keydat->head.is_updated));
	RT_DEBUG_LOG(SHOW_NETKEY_INFO,("keydat->head.updated_time   = %x\n",keydat->head.updated_time));
	RT_DEBUG_LOG(SHOW_NETKEY_INFO,("keydat->head.start_time     = %x\n",keydat->head.start_time));
	RT_DEBUG_LOG(SHOW_NETKEY_INFO,("keydat->head.end_time       = %x\n",keydat->head.end_time));
	if(keydat->head.key_type == KEY_TYPE_KBOARD)
	{
    rt_memcpy((void *)keydat->data.kboard.code,(const void *)remote->data,6);
    RT_DEBUG_LOG(SHOW_NETKEY_INFO,("keydat->data.kboard       = %s\n",keydat->data.kboard));
	}
	else if(keydat->head.key_type == KEY_TYPE_KBOARD)
	{
		rt_memcpy((void *)keydat->data.rfid.code,(const void *)remote->data,6);
    RT_DEBUG_LOG(SHOW_NETKEY_INFO,("keydat->data.rfid         = %s\n",keydat->data.rfid));
	}
	else if(keydat->head.key_type == KEY_TYPE_FPRINT)
	{
    rt_memcpy((void *)keydat->data.fprint.code,(const void *)remote->data,512);
    RT_DEBUG_LOG(SHOW_NETKEY_INFO,("Fprintf type\n"));
	}
	else
	{
    RT_DEBUG_LOG(SHOW_NETKEY_INFO,("error type key\n"));
	}
	
	KeyOpResult = device_config_key_set(KeyID,keydat,keydat->head.updated_time);
	
	rt_free(keydat);

	if(KeyOpResult < 0)
	{
		return RT_ERROR;
	}
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
	rt_uint16_t 	keypos;
	rt_uint32_t 	date;
	rt_int32_t		KeyOpResult;
	
	remote = &(mail->data.keydel.key);

	net_string_copy_uint16(&keypos,remote->pos);
  net_string_copy_uint32(&date,remote->date);

	/*if(keypos >= KEY_NUMBERS)
	{
    RT_DEBUG_LOG(SHOW_NETKEY_INFO,("Remote key positional fault!!!\n"));
		return RT_ERROR;
	}*/
	RT_DEBUG_LOG(SHOW_NETKEY_INFO,("key delete data info:>>>>>>\n"));
	RT_DEBUG_LOG(SHOW_NETKEY_INFO,("keypos   = %d\n",keypos));
	RT_DEBUG_LOG(SHOW_NETKEY_INFO,("date     = %x\n",date));
	KeyOpResult = device_config_key_delete(keypos,date,1);
	
  RT_DEBUG_LOG(SHOW_NETKEY_INFO,("Remote delete keys to success!!!\n"));
  
	if(KeyOpResult < 0)
	{
		return RT_ERROR;
	}
	return RT_EOK;
}

