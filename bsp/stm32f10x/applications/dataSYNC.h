#ifndef __DATASYNC_H__
#define __DATASYNC_H__
#include "rtthread.h"
#include "sync_account.h"
#include "sync_event.h"
#include "sync_key.h"
#include "sync_phone.h"

rt_err_t remote_data_sync_process(void);

//�˻�ӳ����������
void sync_account_map_req_api(struct account_map_req *data);

//�˻�����У�鱨�ķ���
void sync_account_check_request_api(struct account_check_req *data);

//�˻�����У�鱨�ķ���
void sync_account_check_request_api(struct account_check_req *data);

//Կ��ӳ����������
void sync_key_map_req_api(struct key_map_req *data);

//Կ������У�鱨�ķ���
void sync_key_check_request_api(struct key_check_req *data);

//�ֻ�ӳ����������
void sync_phone_map_req_api(struct phone_map_req *data);

//�ֻ�����У�鱨�ķ���
void sync_phone_check_request_api(struct phone_check_req *data);

//��¼ӳ���������ͱ���
void sync_event_map_req_api(struct event_map_req *data);

//��¼����У�鱨�ķ���
void sync_event_check_request_api(struct event_check_req *data);


#endif

