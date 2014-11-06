#include "dataSYNC.h"
#include "gprs.h"
#include "netmailclass.h"

//Զ������ͬ�����Ĵ��� 
//������Ϊ��
rt_err_t remote_data_sync_process(void)
{
	//�������и��±�־λ

	set_all_update_flag(1);
	return RT_EOK;
}

#ifdef USEING_NEW_DATA_SYNC

//�˻�ӳ����������
void sync_account_map_req(struct account_map_req *data)
{
	rt_uint8_t *mapaddr;
	rt_size_t  size;

	mapaddr = (rt_uint8_t *)data->valid_map.data;
	size  = ACCOUNT_MAP_SIZE*4;
	
	msg_mail_accmapadd(mapaddr,size,data->valid_map.updated_time);
}

//�˻�����У�鱨�ķ���
void sync_account_check_request(struct account_check_req *data)
{
	msg_mail_accdatcks(data->id,data->ah.updated_time);
}

//Կ��ӳ����������
void sync_key_map_req(struct key_map_req *data)
{
	rt_uint8_t *mapaddr;
	rt_size_t  size;

	mapaddr = (rt_uint8_t *)data->valid_map.data;
	size  = KEY_MAP_SIZE*4;
	
	msg_mail_keymapadd(mapaddr,size,data->valid_map.updated_time);
}


//Կ������У�鱨�ķ���
void sync_key_check_request(struct key_check_req *data)
{
  msg_mail_keydatcks(data->id,data->k.head.updated_time);
}

//�ֻ�ӳ����������
void sync_phone_map_req(struct phone_map_req *data)
{
	rt_uint8_t *mapaddr;
	rt_size_t  size;

	mapaddr = (rt_uint8_t *)data->valid_map.data;
	size  = PHONE_MAP_SIZE*4;
	
	msg_mail_phmapadd(mapaddr,size,data->valid_map.updated_time);
}

//�ֻ�����У�鱨�ķ���
void sync_phone_check_request(struct phone_check_req *data)
{
	msg_mail_phdatcks(data->id,data->ph.updated_time);
}

//��¼ӳ���������ͱ���
void sync_event_map_req(struct event_map_req *data)
{
	rt_uint8_t *mapaddr;
	rt_size_t  size;

	mapaddr = (rt_uint8_t *)data->valid_map.data;
	size  = EVENT_MAP_SIZE*4;
	
	msg_mail_recmapadd(mapaddr,size,data->valid_map.updated_time);
}

//��¼����У�鱨�ķ���
void sync_event_check_request(struct event_check_req *data)
{
	msg_mail_recdatcks(data->id,data->e.head.updated_time);
}

#endif

