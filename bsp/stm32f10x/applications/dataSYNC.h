#ifndef __DATASYNC_H__
#define __DATASYNC_H__
#include "rtthread.h"
#include "sync_account.h"
#include "sync_event.h"
#include "sync_key.h"
#include "sync_phone.h"

rt_err_t remote_data_sync_process(void);

//账户映射域请求发送
void sync_account_map_req_api(struct account_map_req *data);

//账户数据校验报文发送
void sync_account_check_request_api(struct account_check_req *data);

//账户数据校验报文发送
void sync_account_check_request_api(struct account_check_req *data);

//钥匙映射域请求发送
void sync_key_map_req_api(struct key_map_req *data);

//钥匙数据校验报文发送
void sync_key_check_request_api(struct key_check_req *data);

//手机映射域请求发送
void sync_phone_map_req_api(struct phone_map_req *data);

//手机数据校验报文发送
void sync_phone_check_request_api(struct phone_check_req *data);

//记录映射域请求发送报文
void sync_event_map_req_api(struct event_map_req *data);

//记录数据校验报文发送
void sync_event_check_request_api(struct event_check_req *data);


#endif

