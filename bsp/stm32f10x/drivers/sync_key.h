/*********************************************************************
 * Filename:			sync_key.h
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
#ifndef _SYNC_key_H_
#define _SYNC_key_H_

#include <rtthread.h>
#include <rthw.h>
#include <stm32f10x.h>

struct key_map_check_req {
    struct key_valid_map *valid_map;
};
struct key_map_check_ack {
    u32 result;
};
struct key_map_req {
    struct key_valid_map *valid_map;
};
struct key_map_ack {
    u32 result;
};

struct key_check_req {
    struct key *k;
    u16 id;
};
struct key_check_ack {
    u32 result;
};

void sync_key_map_check_req(struct key_map_check_req *data);
void sync_key_map_check_ack(struct key_map_check_ack *data);

void sync_key_map_req(struct key_map_req *data);
void sync_key_map_ack(struct key_map_ack *data);


void sync_key_check_request(struct key_check_req *data);
void sync_key_check_ack(struct key_check_ack *data);

void sync_key_map_recv_req(struct key_map_req *data);
void sync_key_map_recv_ack(struct key_map_ack *data);

rt_err_t sync_key(void);

#endif /* _CONFIG_H_ */
