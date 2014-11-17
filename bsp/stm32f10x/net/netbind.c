#include "netbind.h"

#define SHOW_NETPHONE_INFO 		1

/** 
@brief  远程端钥匙绑定处理
@param  mail 绑定报文的数据
@retval RT_EOK	 :succeed
@retval RT_ERROR :Failure
*/
rt_err_t net_bind_key_process(net_recvmsg_p mail)
{
	net_recv_keybind *remote = RT_NULL;
	rt_uint16_t 		 KeyID;
	rt_uint16_t			 AccountID;
	rt_uint32_t			 OpTime;
	rt_int32_t			 OpResult;
	
	RT_ASSERT(mail != RT_NULL);
	remote = &(mail->data.KeyBind);

	//解析远程端发送过来的钥匙绑定数据
	net_string_copy_uint16(&KeyID,remote->KeyPos);
	net_string_copy_uint16(&AccountID,remote->AccountPos);
	net_string_copy_uint32(&OpTime,remote->date);
  RT_DEBUG_LOG(SHOW_NETPHONE_INFO,("Bind Key Data Info:>>>>>>\n"));
  RT_DEBUG_LOG(SHOW_NETPHONE_INFO,("AccountID  %04x\n",AccountID));
  RT_DEBUG_LOG(SHOW_NETPHONE_INFO,("KeyID			 %04x\n",KeyID));
  RT_DEBUG_LOG(SHOW_NETPHONE_INFO,("OpTime     %04x\n",OpTime));

	//根据解析得到的数据进行钥匙绑定操作
	OpResult = device_config_account_append_key(AccountID,KeyID,OpTime,1);
	if(OpResult < 0)
	{
    RT_DEBUG_LOG(SHOW_NETPHONE_INFO,("Bind Key Fail %d\n",OpResult));
    device_config_key_delete(KeyID,0,0);
		return RT_ERROR;
	}

  RT_DEBUG_LOG(SHOW_NETPHONE_INFO,("Bind Key Finish\n"));
	return RT_EOK;
}

/** 
@brief 远程端手机绑定处理
@param  mail 远程端的数据
@retval RT_EOK	 :succeed
@retval RT_ERROR :Failure
*/
rt_err_t net_bind_phone_process(net_recvmsg_p mail)
{
	net_recv_phonebind 	*remote = RT_NULL;
	rt_uint16_t 				PhoneID;
	rt_uint16_t 				AccountID;
	rt_uint32_t 				OpTime;
	rt_int32_t					OpResult;
	
	RT_ASSERT(mail != RT_NULL);
	remote = &(mail->data.PhoneBind);

	//解析网络数据
	net_string_copy_uint16(&PhoneID,remote->PhonePos);
	net_string_copy_uint16(&AccountID,remote->AccountPos);
	net_string_copy_uint32(&OpTime,remote->date);
  RT_DEBUG_LOG(SHOW_NETPHONE_INFO,("Bind Phone Data Info:>>>>>>\n"));
	RT_DEBUG_LOG(SHOW_NETPHONE_INFO,("AccountID  %04x\n",AccountID));
	RT_DEBUG_LOG(SHOW_NETPHONE_INFO,("KeyID      %04x\n",PhoneID));
	RT_DEBUG_LOG(SHOW_NETPHONE_INFO,("OpTime     %04x\n",OpTime));

	//根据收到的数据对手机进行绑定操作
	OpResult = device_config_account_append_phone(AccountID,PhoneID,OpTime,1);
	if(OpResult < 0)
	{
    RT_DEBUG_LOG(SHOW_NETPHONE_INFO,("Bind Phone Fail %d\n",OpResult));
    device_config_phone_delete(PhoneID,0,0);
		return RT_ERROR;
	}
	
  RT_DEBUG_LOG(SHOW_NETPHONE_INFO,("Bind Phone Finish\n"));
	return RT_EOK;
}

