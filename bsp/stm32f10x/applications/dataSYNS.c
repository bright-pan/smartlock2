#include "dataSYNC.h"
#include "gprs.h"
#include "netmailclass.h"

//远程数据同步报文处理 
//数据域为空
rt_err_t remote_data_sync_process(void)
{
	//设置所有更新标志位

	set_all_update_flag(1);
	return RT_EOK;
}

#ifdef USEING_NEW_DATA_SYNC

//账户映射域请求发送
void sync_account_map_req(struct account_map_req *data)
{
	rt_uint8_t *mapaddr;
	rt_size_t  size;

	mapaddr = (rt_uint8_t *)data->valid_map.data;
	size  = ACCOUNT_MAP_SIZE*4;
	
	msg_mail_accmapadd(mapaddr,size,data->valid_map.updated_time);
}

//账户数据校验报文发送
void sync_account_check_request(struct account_check_req *data)
{
	msg_mail_accdatcks(data->id,data->ah.updated_time);
}

//钥匙映射域请求发送
void sync_key_map_req(struct key_map_req *data)
{
	rt_uint8_t *mapaddr;
	rt_size_t  size;

	mapaddr = (rt_uint8_t *)data->valid_map.data;
	size  = KEY_MAP_SIZE*4;
	
	msg_mail_keymapadd(mapaddr,size,data->valid_map.updated_time);
}


//钥匙数据校验报文发送
void sync_key_check_request(struct key_check_req *data)
{
  msg_mail_keydatcks(data->id,data->k.head.updated_time);
}

//手机映射域请求发送
void sync_phone_map_req(struct phone_map_req *data)
{
	rt_uint8_t *mapaddr;
	rt_size_t  size;

	mapaddr = (rt_uint8_t *)data->valid_map.data;
	size  = PHONE_MAP_SIZE*4;
	
	msg_mail_phmapadd(mapaddr,size,data->valid_map.updated_time);
}

//手机数据校验报文发送
void sync_phone_check_request(struct phone_check_req *data)
{
	msg_mail_phdatcks(data->id,data->ph.updated_time);
}

//记录映射域请求发送报文
void sync_event_map_req(struct event_map_req *data)
{
	rt_uint8_t *mapaddr;
	rt_size_t  size;

	mapaddr = (rt_uint8_t *)data->valid_map.data;
	size  = EVENT_MAP_SIZE*4;
	
	msg_mail_recmapadd(mapaddr,size,data->valid_map.updated_time);
}

//记录数据校验报文发送
void sync_event_check_request(struct event_check_req *data)
{
	msg_mail_recdatcks(data->id,data->e.head.updated_time);
}

#endif

