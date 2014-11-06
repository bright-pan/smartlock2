/*********************************************************************
 * Filename:			sync_phone.c
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
#include "sync_phone.h"
#include <config.h>

static rt_mq_t mq_phone_map_check_ack;
static rt_mq_t mq_phone_map_ack;
static rt_mq_t mq_phone_check_ack;

void sync_phone_map_check_req(struct phone_map_check_req *data)
{
    
}
void sync_phone_map_check_ack(struct phone_map_check_ack *data)
{
    rt_mq_send(mq_phone_map_check_ack, data, sizeof(*data));
}

void sync_phone_map_req(struct phone_map_req *data)
{
    
}
void sync_phone_map_ack(struct phone_map_ack *data)
{
    rt_mq_send(mq_phone_map_ack, data, sizeof(*data));
}

void sync_phone_check_request(struct phone_check_req *data)
{
    
}
void sync_phone_check_ack(struct phone_check_ack *data)
{
    rt_mq_send(mq_phone_check_ack, data, sizeof(*data));
}

void sync_phone_map_recv_req(struct phone_map_req *data)
{
    device_config_pv_operate(data->valid_map,1);
}
void sync_phone_map_recv_ack(struct phone_map_ack *data)
{
    
}

static int sync_phone_check_callback(struct phone_head *ph, int phone_id, void *args)
{
    rt_err_t result;
    struct phone_check_req pc_req;
    struct phone_check_ack pc_ack;
    pc_req.ph = ph;
    pc_req.id = phone_id;
    sync_phone_check_request(&pc_req);
	result = rt_mq_recv(mq_phone_check_ack,&pc_ack, sizeof(pc_ack), RT_WAITING_FOREVER);
	if(result == RT_EOK)
	{
        if (pc_ack.result == 2)
        {
            //发送电话数据包。
        }
            
    }
    return result;
}

//文件上传 给提供给更上层的API
rt_err_t sync_phone(void)
{
    struct phone_valid_map pv_map;
    struct phone_map_check_req pmc_req;
    struct phone_map_req pm_req;
    struct phone_map_check_ack pmc_ack;
    struct phone_map_ack pm_ack;
    
	rt_err_t result;

    mq_phone_map_check_ack = rt_mq_create("pmc", sizeof(struct phone_map_check_ack), 1, RT_IPC_FLAG_FIFO);
    mq_phone_map_ack = rt_mq_create("pm", sizeof(struct phone_map_ack), 1, RT_IPC_FLAG_FIFO);
    mq_phone_check_ack = rt_mq_create("pc", sizeof(struct phone_check_ack), 1, RT_IPC_FLAG_FIFO);
    
    //发送有效域校验请求
    device_config_pv_operate(&pv_map, 0);
    pmc_req.valid_map = &pv_map;
	sync_phone_map_check_req(&pmc_req);
	
	//等待后台的应答
	result = rt_mq_recv(mq_phone_map_check_ack,&pmc_ack, sizeof(pmc_ack), RT_WAITING_FOREVER);
	if(result == RT_EOK)
	{
		//收到后台的应答
		if(pmc_ack.result == 2)
		{
            pm_req.valid_map = &pv_map;
            sync_phone_map_req(&pm_req);
            result = rt_mq_recv(mq_phone_map_ack,&pm_ack, sizeof(pm_ack), RT_WAITING_FOREVER);
			
            if (result == RT_EOK)
            {
                if (pm_ack.result == 1)
                {
                    device_config_phone_index(sync_phone_check_callback, RT_NULL);
                }
            }
		}
	}
    
    rt_mq_delete(mq_phone_map_check_ack);
    rt_mq_delete(mq_phone_map_ack);
    rt_mq_delete(mq_phone_check_ack);
    return result;
}
