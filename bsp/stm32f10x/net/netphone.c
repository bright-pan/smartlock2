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
	struct phone_head *ph = RT_NULL;
	rt_uint16_t				PhoneID;
	rt_uint8_t 				i;
	rt_int32_t				OpResult;
	
	RT_ASSERT(mail != RT_NULL);
	
	remote = &(mail->data.phoneadd);
	
	ph = rt_calloc(1,sizeof(struct phone_head));
	RT_ASSERT(ph != RT_NULL);

	//解析数据
	net_string_copy_uint16(&PhoneID,remote->pos);
	net_string_copy_uint16(&ph->auth,remote->permission);
	rt_memcpy(ph->address,remote->data,11);
	net_string_copy_uint32((rt_uint32_t *)&ph->updated_time,remote->date);

  RT_DEBUG_LOG(SHOW_NETPHONE_INFO,("Phone Add Data Info:>>>>>>\n"));
  RT_DEBUG_LOG(SHOW_NETPHONE_INFO,("PhoneID          = %x\n",PhoneID));
  RT_DEBUG_LOG(SHOW_NETPHONE_INFO,("ph->auth         = %X\n",ph->auth));
  RT_DEBUG_LOG(SHOW_NETPHONE_INFO,("ph->address      = %s\n",ph->address));
  RT_DEBUG_LOG(SHOW_NETPHONE_INFO,("ph->updated_time = %X\n",ph->updated_time));
  RT_DEBUG_LOG(SHOW_NETPHONE_INFO,("Phone hex show>>>>>>\n"));
  for(i = 0;i < 12;i++)
  {
    RT_DEBUG_LOG(SHOW_NETPHONE_INFO,("%02X",ph->address[i]));
  }
  RT_DEBUG_LOG(SHOW_NETPHONE_INFO,("\n"));
	//保存数据
	OpResult = device_config_phone_set(PhoneID,(u8 *)ph->address,11,ph->auth,ph->updated_time);
	
	

	rt_free(ph);
	if(OpResult < 0)
	{
		RT_DEBUG_LOG(SHOW_NETPHONE_INFO,("Phone add Fail !!!\n"));
		return RT_ERROR;
	}
	
	RT_DEBUG_LOG(SHOW_NETPHONE_INFO,("Phone add Finish^_^\n"));
	
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
	rt_uint16_t 			PhID;
	rt_uint32_t				OpTime;
	rt_int32_t				OpResult;
	
	RT_ASSERT(mail != RT_NULL);
	
	remote = &(mail->data.phonedel);

	net_string_copy_uint16(&PhID,remote->pos);
	net_string_copy_uint32(&OpTime,remote->date);

	
	RT_DEBUG_LOG(SHOW_NETPHONE_INFO,("Phone Date Delete Info:>>>>>>\n"));
	RT_DEBUG_LOG(SHOW_NETPHONE_INFO,("PhID   = %d\n",PhID));
	RT_DEBUG_LOG(SHOW_NETPHONE_INFO,("OpTime = %x\n",OpTime));
	
	OpResult = device_config_phone_delete(PhID,OpTime,1);
	
	if(OpResult < 0)
	{
    RT_DEBUG_LOG(SHOW_NETPHONE_INFO,("Phone delete Fail !!!\n"));
		return RT_ERROR;
	}

	RT_DEBUG_LOG(SHOW_NETPHONE_INFO,("Phone delete Finish^_^\n"));
	return RT_EOK;
}


