/*********************************************************************
 * Filename:			comm_window.c
 *
 * Description:
 *
 * Author:              Bright Pan
 * Email:				bright_pan@yuettak.com
 * Date:				2014-03-17
 *
 * Modify:
 *
 * Copyright (C) 2014 Yuettak Co.,Ltd
 ********************************************************************/

#include "comm_window.h"

#define COMM_WINDOW_SIZE 5
#define CW_TIMER_TIMEOUT_TICKS 10
#define CW_TIMER_SEND_DELAY 5
#define CW_TIMER_RESEND_COUNTS 5

COMM_WINDOW_LIST cw_list;

void
cw_timer_out(void *parameters)
{
	COMM_WINDOW_NODE *tmp;
	COMM_WINDOW_LIST *cw_list = (COMM_WINDOW_LIST *)parameters;
	struct list_head *pos, *q;

	rt_mutex_take(cw_list->mutex, RT_WAITING_FOREVER);
	list_for_each_safe(pos, q, &cw_list->list)
	{
		tmp= list_entry(pos, COMM_WINDOW_NODE, list);

		if (tmp->flag == CW_FLAG_REQUEST) // request
		{
			if (tmp->cnts == 1)
			{
				send_frame(cw_list->device, &tmp->mail, tmp->order);
			}
			if (tmp->cnts++ > tmp->delay)
			{
				tmp->cnts = 1;
				tmp->r_cnts++;
			}
			if (tmp->r_cnts >= CW_TIMER_RESEND_COUNTS)
			{
#ifdef RT_USING_FINSH
				rt_kprintf("send failure and delete cw node\n");
				rt_kprintf("comm_type: 0x%02X, order: 0x%02X, length: %d\n", (tmp->mail).comm_type, tmp->order, (tmp->mail).len);
				print_hex((tmp->mail).buf, (tmp->mail).len);
#endif // RT_USING_FINSH
				*((tmp->mail).result) = CW_STATUS_SEND_ERROR;
				rt_sem_release((tmp->mail).result_sem);
				rt_free((tmp->mail).buf);
				list_del(pos);
				cw_list->size--;
				rt_free(tmp);
			}
		}
		else // response
		{
			send_frame(cw_list->device, &tmp->mail, tmp->order);
#ifdef RT_USING_FINSH
			rt_kprintf("send response and delete cw node\n");
			rt_kprintf("comm_type: 0x%02X, order: 0x%02X, length: %d\n", (tmp->mail).comm_type, tmp->order, (tmp->mail).len);
			print_hex((tmp->mail).buf, (tmp->mail).len);
#endif // RT_USING_FINSH
			*((tmp->mail).result) = CW_STATUS_OK;
			rt_sem_release((tmp->mail).result_sem);
			rt_free((tmp->mail).buf);
			list_del(pos);
			cw_list->size--;
			rt_free(tmp);
		}
    }
	rt_mutex_release(cw_list->mutex);
}

CW_STATUS
cw_list_init(COMM_WINDOW_LIST *cw_list)
{
	INIT_LIST_HEAD(&cw_list->list);
	cw_list->size = 0;
	cw_list->timer = rt_timer_create("t_cw", cw_timer_out, cw_list, CW_TIMER_TIMEOUT_TICKS, RT_TIMER_FLAG_SOFT_TIMER|RT_TIMER_FLAG_PERIODIC);
	cw_list->device = device_enable(DEVICE_NAME_COMM);
	cw_list->mutex = rt_mutex_create("m_cw", RT_IPC_FLAG_FIFO);
	if ((cw_list->timer == RT_NULL) ||
		(cw_list->device == RT_NULL) ||
		(cw_list->mutex ==RT_NULL))
	{
		if (cw_list->timer != RT_NULL)
		{
			rt_timer_delete(cw_list->timer);
		}
		if (cw_list->mutex != RT_NULL)
		{
			rt_mutex_delete(cw_list->mutex);
		}

		return CW_STATUS_INIT_ERROR;
	}
	rt_timer_start(cw_list->timer);
	return CW_STATUS_OK;
}

CW_STATUS
cw_list_new(COMM_WINDOW_NODE **node, COMM_WINDOW_LIST *cw_list)
{
	if (cw_list->size <= COMM_WINDOW_SIZE)
	{
		*node = rt_malloc(sizeof(**node));
		if (*node != RT_NULL)
		{
			(*node)->r_cnts = 1;
			(*node)->cnts = 1;
			(*node)->delay = CW_TIMER_SEND_DELAY;
			rt_mutex_take(cw_list->mutex, RT_WAITING_FOREVER);
			list_add(&(*node)->list, &cw_list->list);
			rt_mutex_release(cw_list->mutex);

			cw_list->size++;
		}
		else
			return CW_STATUS_NEW_ERROR;
	}
	else
		return CW_STATUS_FULL;

	return CW_STATUS_OK;
}
