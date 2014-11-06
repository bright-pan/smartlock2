/*********************************************************************
 * Fileneme:			sync_event.c
 *
 * Description:
 *
 * Author:				Bright Pan
 * Email:				bright_pan@yuettak.com
 * Date:				2014-10-22
 *
 * Modify:
 *
 * Copyright (C) 2014 Yuettak Co.,Ltd
 ********************************************************************/
#include <rtthread.h>
#include <stm32f10x.h>
#include "sync_event.h"
#include <config.h>

static rt_mq_t mq_event_map_check_ack;
static rt_mq_t mq_event_map_ack;
static rt_mq_t mq_event_check_ack;

void sync_event_map_check_req(struct event_map_check_req *data)
{
    
}
void sync_event_map_check_ack(struct event_map_check_ack *data)
{
    rt_mq_send(mq_event_map_check_ack, data, sizeof(*data));
}

void sync_event_map_req(struct event_map_req *data)
{
    
}
void sync_event_map_ack(struct event_map_ack *data)
{
    rt_mq_send(mq_event_map_ack, data, sizeof(*data));
}

void sync_event_check_request(struct event_check_req *data)
{
    
}

void sync_event_check_ack(struct event_check_ack *data)
{
    rt_mq_send(mq_event_check_ack, data, sizeof(*data));
}

void sync_event_map_recv_req(struct event_map_req *data)
{
    device_config_ev_operate(data->valid_map,1);
}
void sync_event_map_recv_ack(struct event_map_ack *data)
{
    
}

static int sync_event_check_callback(struct event *e, int event_id, void *args)
{
    rt_err_t result;
    struct event_check_req ac_req;
    struct event_check_ack ac_ack;
    ac_req.e = e;
    ac_req.id = event_id;
    sync_event_check_request(&ac_req);
	result = rt_mq_recv(mq_event_check_ack,&ac_ack, sizeof(ac_ack), RT_WAITING_FOREVER);
	if(result == RT_EOK)
	{
        if (ac_ack.result == 2)
        {
            //发送事件包。
        }
    }
    return result;
}

//文件上传 给提供给更上层的API
rt_err_t sync_event(void)
{
    struct event_valid_map ev_map;
    struct event_map_check_req emc_req;
    struct event_map_req em_req;
    struct event_map_check_ack emc_ack;
    struct event_map_ack em_ack;
    
	rt_err_t result;

    mq_event_map_check_ack = rt_mq_create("emc", sizeof(struct event_map_check_ack), 1, RT_IPC_FLAG_FIFO);
    mq_event_map_ack = rt_mq_create("em", sizeof(struct event_map_ack), 1, RT_IPC_FLAG_FIFO);
    mq_event_check_ack = rt_mq_create("ac", sizeof(struct event_check_ack), 1, RT_IPC_FLAG_FIFO);
    
    //发送有效域校验请求
    device_config_ev_operate(&ev_map, 0);
    emc_req.valid_map = &ev_map;
	sync_event_map_check_req(&emc_req);
	
	//等待后台的应答
	result = rt_mq_recv(mq_event_map_check_ack,&emc_ack, sizeof(emc_ack), RT_WAITING_FOREVER);
	if(result == RT_EOK)
	{
		//收到后台的应答
		if(emc_ack.result == 2)
		{
            em_req.valid_map = &ev_map;
            sync_event_map_req(&em_req);
            result = rt_mq_recv(mq_event_map_ack,&em_ack, sizeof(em_ack), RT_WAITING_FOREVER);
			
            if (result == RT_EOK)
            {
                if (em_ack.result == 1)
                {
                    device_config_event_index(sync_event_check_callback, RT_NULL);
                }
            }
		}
	}
    
    rt_mq_delete(mq_event_map_check_ack);
    rt_mq_delete(mq_event_map_ack);
    rt_mq_delete(mq_event_check_ack);
    return result;
}
