/*********************************************************************
 * Filename:			sync_event.h
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
#ifndef _SYNC_event_H_
#define _SYNC_event_H_

#include <rtthread.h>
#include <rthw.h>
#include <stm32f10x.h>

struct event_map_check_req {
    struct event_valid_map *valid_map;
};
struct event_map_check_ack {
    u32 result;
};
struct event_map_req {
    struct event_valid_map *valid_map;
};
struct event_map_ack {
    u32 result;
};

struct event_check_req {
    struct event *e;
    u16 id;
};
struct event_check_ack {
    u32 result;
};

void sync_event_map_check_req(struct event_map_check_req *data);
void sync_event_map_check_ack(struct event_map_check_ack *data);

void sync_event_map_req(struct event_map_req *data);
void sync_event_map_ack(struct event_map_ack *data);


void sync_event_check_request(struct event_check_req *data);
void sync_event_check_ack(struct event_check_ack *data);

void sync_event_map_recv_req(struct event_map_req *data);
void sync_event_map_recv_ack(struct event_map_ack *data);

rt_err_t sync_event(void);

#endif /* _CONFIG_H_ */
