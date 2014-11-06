/*********************************************************************
 * Filename:			sync_account.h
 *
 * Description:
 *
 * Author:
 * Email:				lenovo@BRIGHT
 * Date:				2014-08-27
 *
 * Modify:
 *
 * Copyright (C) 2014 Yuettak Co.,Ltd
 ********************************************************************/
#ifndef _SYNC_ACCOUNT_H_
#define _SYNC_ACCOUNT_H_

#include <rtthread.h>
#include <rthw.h>
#include <stm32f10x.h>

struct account_map_check_req {
    struct account_valid_map *valid_map;
};
struct account_map_check_ack {
    u32 result;
};
struct account_map_req {
    struct account_valid_map *valid_map;
};
struct account_map_ack {
    u32 result;
};

struct account_check_req {
    struct account_head *ah;
    u16 id;
};
struct account_check_ack {
    u32 result;
};

void sync_account_map_check_req(struct account_map_check_req *data);
void sync_account_map_check_ack(struct account_map_check_ack *data);

void sync_account_map_req(struct account_map_req *data);
void sync_account_map_ack(struct account_map_ack *data);


void sync_account_check_request(struct account_check_req *data);
void sync_account_check_ack(struct account_check_ack *data);

void sync_account_map_recv_req(struct account_map_req *data);
void sync_account_map_recv_ack(struct account_map_ack *data);

rt_err_t sync_account(void);

#endif /* _CONFIG_H_ */
