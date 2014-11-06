/*********************************************************************
 * Filename:			sync_key.c
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
#include "sync_key.h"
#include <config.h>

static rt_mq_t mq_key_map_check_ack;
static rt_mq_t mq_key_map_ack;
static rt_mq_t mq_key_check_ack;

void sync_key_map_check_req(struct key_map_check_req *data)
{
    
}
void sync_key_map_check_ack(struct key_map_check_ack *data)
{
    rt_mq_send(mq_key_map_check_ack, data, sizeof(*data));
}

void sync_key_map_req(struct key_map_req *data)
{
    
}
void sync_key_map_ack(struct key_map_ack *data)
{
    rt_mq_send(mq_key_map_ack, data, sizeof(*data));
}

void sync_key_check_request(struct key_check_req *data)
{
    
}
void sync_key_check_ack(struct key_check_ack *data)
{
    rt_mq_send(mq_key_check_ack, data, sizeof(*data));
}

void sync_key_map_recv_req(struct key_map_req *data)
{
    device_config_kv_operate(data->valid_map,1);
}
void sync_key_map_recv_ack(struct key_map_ack *data)
{
    
}

static int sync_key_check_callback(struct key *k, int key_id, void *arg1, void *arg2)
{
    rt_err_t result;
    struct key_check_req kc_req;
    struct key_check_ack kc_ack;
    kc_req.k = k;
    kc_req.id = key_id;
    sync_key_check_request(&kc_req);
	result = rt_mq_recv(mq_key_check_ack,&kc_ack, sizeof(kc_ack), RT_WAITING_FOREVER);
	if(result == RT_EOK)
	{
        if (kc_ack.result == 2)
        {
            //发送钥匙数据。
        }
    }
    return result;
}

//文件上传 给提供给更上层的API
rt_err_t sync_key(void)
{
    struct key_valid_map kv_map;
    struct key_map_check_req kmc_req;
    struct key_map_req km_req;
    struct key_map_check_ack kmc_ack;
    struct key_map_ack km_ack;
    
	rt_err_t result;

    mq_key_map_check_ack = rt_mq_create("kmc", sizeof(struct key_map_check_ack), 1, RT_IPC_FLAG_FIFO);
    mq_key_map_ack = rt_mq_create("km", sizeof(struct key_map_ack), 1, RT_IPC_FLAG_FIFO);
    mq_key_check_ack = rt_mq_create("kc", sizeof(struct key_check_ack), 1, RT_IPC_FLAG_FIFO);
    
    //发送有效域校验请求
    device_config_kv_operate(&kv_map, 0);
    kmc_req.valid_map = &kv_map;
	sync_key_map_check_req(&kmc_req);
	
	//等待后台的应答
	result = rt_mq_recv(mq_key_map_check_ack,&kmc_ack, sizeof(kmc_ack), RT_WAITING_FOREVER);
	if(result == RT_EOK)
	{
		//收到后台的应答
		if(kmc_ack.result == 2)
		{
            km_req.valid_map = &kv_map;
            sync_key_map_req(&km_req);
            result = rt_mq_recv(mq_key_map_ack,&km_ack, sizeof(km_ack), RT_WAITING_FOREVER);
			
            if (result == RT_EOK)
            {
                if (km_ack.result == 1)
                {
                    device_config_key_index(sync_key_check_callback, RT_NULL, RT_NULL);
                }
            }
		}
	}
    
    rt_mq_delete(mq_key_map_check_ack);
    rt_mq_delete(mq_key_map_ack);
    rt_mq_delete(mq_key_check_ack);
    return result;
}
