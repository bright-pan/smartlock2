/*********************************************************************
 * Filename:			sync_phone.h
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
#ifndef _SYNC_phone_H_
#define _SYNC_phone_H_

#include <rtthread.h>
#include <rthw.h>
#include <stm32f10x.h>

struct phone_map_check_req {
    struct phone_valid_map *valid_map;
};
struct phone_map_check_ack {
    u32 result;
};
struct phone_map_req {
    struct phone_valid_map *valid_map;
};
struct phone_map_ack {
    u32 result;
};

struct phone_check_req {
    struct phone_head *ph;
    u16 id;
};
struct phone_check_ack {
    u32 result;
};

void sync_phone_map_check_req(struct phone_map_check_req *data);
void sync_phone_map_check_ack(struct phone_map_check_ack *data);

void sync_phone_map_req(struct phone_map_req *data);
void sync_phone_map_ack(struct phone_map_ack *data);


void sync_phone_check_request(struct phone_check_req *data);
void sync_phone_check_ack(struct phone_check_ack *data);

void sync_phone_map_recv_req(struct phone_map_req *data);
void sync_phone_map_recv_ack(struct phone_map_ack *data);

rt_err_t sync_phone(void);

#endif /* _CONFIG_H_ */
