/*********************************************************************
 * Filename:			sync_account.c
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
#include <sync_account.h>
#include <config.h>

#define SYNC_RESULT_OLD 2
#define SYNC_RESULT_NEW 1
#define SYNC_RESULT_NOT 0

static rt_mq_t mq_account_map_check_ack;
static rt_mq_t mq_account_map_ack;
static rt_mq_t mq_account_check_ack;

void sync_account_map_check_req(struct account_map_check_req *data)
{
    
}
void sync_account_map_check_ack(struct account_map_check_ack *data)
{
    rt_mq_send(mq_account_map_check_ack, data, sizeof(*data));
}

void sync_account_map_req(struct account_map_req *data)
{
    
}
void sync_account_map_ack(struct account_map_ack *data)
{
    rt_mq_send(mq_account_map_ack, data, sizeof(*data));
}


void sync_account_check_request(struct account_check_req *data)
{
    
}
void sync_account_check_ack(struct account_check_ack *data)
{
    rt_mq_send(mq_account_check_ack, data, sizeof(*data));
}

void sync_account_map_recv_req(struct account_map_req *data)
{
    device_config_av_operate(data->valid_map,1);
}
void sync_account_map_recv_ack(struct account_map_ack *data)
{
    
}

int sync_account_check_callback(struct account_head *ah, int account_id, void *args)
{
    rt_err_t result;
    struct account_check_req ac_req;
    struct account_check_ack ac_ack;
    ac_req.ah = ah;
    ac_req.id = account_id;
    sync_account_check_request(&ac_req);
	result = rt_mq_recv(mq_account_check_ack,&ac_ack, sizeof(ac_ack), RT_WAITING_FOREVER);
	if(result == RT_EOK)
	{
        if (ac_ack.result == 2)
        {
            //发送账户数据包。
        }
    }
    return result;
}

//文件上传 给提供给更上层的API
rt_err_t sync_account(void)
{
    struct account_valid_map av_map;
    struct account_map_check_req amc_req;
    struct account_map_req am_req;
    struct account_map_check_ack amc_ack;
    struct account_map_ack am_ack;
    
	rt_err_t result;

    mq_account_map_check_ack = rt_mq_create("amc", sizeof(struct account_map_check_ack), 1, RT_IPC_FLAG_FIFO);
    mq_account_map_ack = rt_mq_create("am", sizeof(struct account_map_ack), 1, RT_IPC_FLAG_FIFO);
    mq_account_check_ack = rt_mq_create("ac", sizeof(struct account_check_ack), 1, RT_IPC_FLAG_FIFO);
    
    //发送有效域校验请求
    device_config_av_operate(&av_map, 0);
    amc_req.valid_map = &av_map;
	sync_account_map_check_req(&amc_req);
	
	//等待后台的应答
	result = rt_mq_recv(mq_account_map_check_ack,&amc_ack, sizeof(amc_ack), RT_WAITING_FOREVER);
	if(result == RT_EOK)
	{
		//收到后台的应答
		if(amc_ack.result == 2)
		{
            am_req.valid_map = &av_map;
            sync_account_map_req(&am_req);
            result = rt_mq_recv(mq_account_map_ack,&am_ack, sizeof(am_ack), RT_WAITING_FOREVER);
			
            if (result == RT_EOK)
            {
                if (am_ack.result == 1)
                {
                    device_config_account_index(sync_account_check_callback, RT_NULL);
                }
            }
		}
	}
    
    rt_mq_delete(mq_account_map_check_ack);
    rt_mq_delete(mq_account_map_ack);
    rt_mq_delete(mq_account_check_ack);
    return result;
}
