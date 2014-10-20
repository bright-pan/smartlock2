#include "netaccount.h"
#define SHOW_NETPHONE_INFO 1

rt_err_t net_account_add_process(net_recvmsg_p mail)
{
	net_recv_accountadd *remote = RT_NULL;
  struct account_head *ah = RT_NULL;
	rt_uint16_t 				AhID;
	rt_int32_t 					OpResult;
	
	RT_ASSERT(mail != RT_NULL)
	remote = &(mail->data.AccountAdd);
	ah = rt_calloc(1,sizeof(struct account_head));
	//解析网络数据
	net_string_copy_uint16(&AhID,remote->pos);
	rt_memcpy(ah->name,remote->name,20);
	net_string_copy_uint32((rt_uint32_t *)&ah->updated_time,remote->date);

  RT_DEBUG_LOG(SHOW_NETPHONE_INFO,("Account Add Data Info:>>>>>>\n"));
  RT_DEBUG_LOG(SHOW_NETPHONE_INFO,("AhID             = %x\n",AhID));
  RT_DEBUG_LOG(SHOW_NETPHONE_INFO,("ah->updated_time = %x\n",ah->updated_time));
  RT_DEBUG_LOG(SHOW_NETPHONE_INFO,("ah->name         = %s\n",ah->name));
	//处理数据
	OpResult = device_config_account_set(AhID,(char *)ah->name,rt_strlen((const char *)ah->name),ah->updated_time);

	rt_free(ah);

	if(OpResult < 0)
	{
    RT_DEBUG_LOG(SHOW_NETPHONE_INFO,("Account Add Fail !!!!\n"));
		return RT_ERROR;
	}
	
  RT_DEBUG_LOG(SHOW_NETPHONE_INFO,("Account Add Finish^_^\n"));
	return RT_EOK;
}

rt_err_t net_account_del_process(net_recvmsg_p mail)
{
	net_recv_accountdel *remote = RT_NULL;
	rt_uint16_t 				AccountID;
	rt_uint32_t					OpTime;
	rt_int32_t					OpResult;
	
	RT_ASSERT(mail != RT_NULL)
	remote = &(mail->data.AccountDel);
	//解析网络数据
	net_string_copy_uint16(&AccountID,remote->pos);
	net_string_copy_uint32(&OpTime,remote->date);
	
  RT_DEBUG_LOG(SHOW_NETPHONE_INFO,("Account Del Data Info:>>>>>>\n"));
  RT_DEBUG_LOG(SHOW_NETPHONE_INFO,("AccountID = %x\n",AccountID));
  RT_DEBUG_LOG(SHOW_NETPHONE_INFO,("OpTime    = %x\n",OpTime));
	//处理数据
	OpResult = device_config_account_delete(AccountID,OpTime,1);
	if(OpResult < 0)
	{
    RT_DEBUG_LOG(SHOW_NETPHONE_INFO,("Account Del Fail !!!!\n"));
		return RT_ERROR;
	}
	
  RT_DEBUG_LOG(SHOW_NETPHONE_INFO,("Account Delete Finish^_^\n"));
	return RT_EOK;
}
