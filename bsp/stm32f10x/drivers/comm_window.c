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
#include "comm.h"

#define COMM_TWINDOW_SIZE 5
#define CTW_TIMER_TIMEOUT_TICKS 10
#define CTW_TIMER_SEND_DELAY 5
#define CTW_TIMER_RESEND_COUNTS 5

#define COMM_RWINDOW_SIZE 5
#define CRW_TIMER_TIMEOUT_TICKS 10

COMM_TWINDOW_LIST comm_twindow_list;
COMM_RWINDOW_LIST comm_rwindow_list;

static void
ctw_timer_out(void *parameters)
{
	COMM_TWINDOW_NODE *tmp;
	COMM_TWINDOW_LIST *ctw_list = (COMM_TWINDOW_LIST *)parameters;
	struct list_head *pos, *q;

	rt_mutex_take(ctw_list->mutex, RT_WAITING_FOREVER);
	list_for_each_safe(pos, q, &ctw_list->list)
	{
		tmp= list_entry(pos, COMM_TWINDOW_NODE, list);
        if (tmp->data.cnts++ > tmp->data.delay)
        {
#ifdef RT_USING_FINSH
            rt_kprintf("resend request frame %d \ncmd: 0x%02X, order: 0x%02X, length: %d\n",
                       tmp->data.r_cnts + 1,(tmp->data.mail).comm_type, tmp->data.order, (tmp->data.mail).len);
            print_hex((tmp->data.mail).buf, (tmp->data.mail).len);
#endif // RT_USING_FINSH
            send_frame(ctw_list->device, &((tmp->data).mail), tmp->data.order);

            tmp->data.cnts = 0;
            tmp->data.r_cnts++;
        }
        if (tmp->data.r_cnts > CTW_TIMER_RESEND_COUNTS)
        {
#ifdef RT_USING_FINSH
            rt_kprintf("send failure and delete cw node\ncomm_type: 0x%02X, order: 0x%02X, length: %d\n",
                       (tmp->data.mail).comm_type, tmp->data.order, (tmp->data.mail).len);
            print_hex((tmp->data.mail).buf, (tmp->data.mail).len);
#endif // RT_USING_FINSH
            *((tmp->data.mail).result) = CTW_STATUS_SEND_ERROR;
            rt_sem_release((tmp->data.mail).result_sem);
            rt_free((tmp->data.mail).buf);
            list_del(pos);
            ctw_list->size--;
            rt_free(tmp);
        }
    }
	rt_mutex_release(ctw_list->mutex);
}

CTW_STATUS
ctw_list_init(COMM_TWINDOW_LIST *ctw_list)
{
	INIT_LIST_HEAD(&ctw_list->list);
	ctw_list->size = 0;
	ctw_list->timer = rt_timer_create("t_ctw", ctw_timer_out, ctw_list, CTW_TIMER_TIMEOUT_TICKS, RT_TIMER_FLAG_SOFT_TIMER|RT_TIMER_FLAG_PERIODIC);
	ctw_list->device = device_enable(DEVICE_NAME_COMM);
	ctw_list->mutex = rt_mutex_create("m_ctw", RT_IPC_FLAG_FIFO);
	if ((ctw_list->timer == RT_NULL) ||
		(ctw_list->device == RT_NULL) ||
		(ctw_list->mutex ==RT_NULL))
	{
		if (ctw_list->timer != RT_NULL)
		{
			rt_timer_delete(ctw_list->timer);
		}
		if (ctw_list->mutex != RT_NULL)
		{
			rt_mutex_delete(ctw_list->mutex);
		}

		return CTW_STATUS_INIT_ERROR;
	}
	rt_timer_start(ctw_list->timer);
	return CTW_STATUS_OK;
}

CTW_STATUS
ctw_list_new(COMM_TWINDOW_NODE **node, COMM_TWINDOW_LIST *ctw_list, COMM_TWINDOW_NODE_DATA_TYPEDEF *data)
{
	if (ctw_list->size <= COMM_TWINDOW_SIZE)
	{
		*node = rt_malloc(sizeof(**node));
		if (*node != RT_NULL)
		{
			(*node)->data = *data;
			rt_kprintf("add cmd: 0x%02X, order: 0x%02X, length: %d\n",
					   ((*node)->data.mail).comm_type, (*node)->data.order, ((*node)->data.mail).len);
			rt_mutex_take(ctw_list->mutex, RT_WAITING_FOREVER);
			list_add(&(*node)->list, &ctw_list->list);
			ctw_list->size++;
			rt_mutex_release(ctw_list->mutex);

		}
		else
			return CTW_STATUS_NEW_ERROR;
	}
	else
		return CTW_STATUS_FULL;

	return CTW_STATUS_OK;
}

static void
crw_timer_out(void *parameters)
{
    rt_err_t error;
	COMM_RWINDOW_NODE *tmp;
	COMM_RWINDOW_LIST *crw_list = (COMM_RWINDOW_LIST *)parameters;
	struct list_head *pos, *q;

	rt_mutex_take(crw_list->mutex, RT_WAITING_FOREVER);
	list_for_each_safe(pos, q, &crw_list->list)
	{
		tmp= list_entry(pos, COMM_RWINDOW_NODE, list);
		if (tmp->data.sem != RT_NULL)
		{
			error = rt_sem_take(tmp->data.sem, RT_WAITING_NO);
			if (error == RT_EOK) {
				send_ctx_mail(tmp->data.comm_type, tmp->data.order, 0, (uint8_t *)(tmp->data.result), 1);
				if (tmp->data.buf != RT_NULL)
					rt_free(tmp->data.buf);
				if (tmp->data.result != RT_NULL)
					rt_free(tmp->data.result);
				if (tmp->data.sem != RT_NULL)
					rt_sem_delete(tmp->data.sem);
				list_del(pos);
				crw_list->size--;
				rt_free(tmp);
			}
		}
		else
		{
			list_del(pos);
			crw_list->size--;
			rt_free(tmp);
		}
    }
	rt_mutex_release(crw_list->mutex);
}

CRW_STATUS
crw_list_init(COMM_RWINDOW_LIST *crw_list)
{
	INIT_LIST_HEAD(&crw_list->list);
	crw_list->size = 0;
	crw_list->timer = rt_timer_create("t_crw", crw_timer_out, crw_list, CRW_TIMER_TIMEOUT_TICKS, RT_TIMER_FLAG_SOFT_TIMER|RT_TIMER_FLAG_PERIODIC);
	crw_list->mutex = rt_mutex_create("m_crw", RT_IPC_FLAG_FIFO);
	if ((crw_list->timer == RT_NULL) ||
		(crw_list->mutex ==RT_NULL))
	{
		if (crw_list->timer != RT_NULL)
		{
			rt_timer_delete(crw_list->timer);
		}
		if (crw_list->mutex != RT_NULL)
		{
			rt_mutex_delete(crw_list->mutex);
		}

		return CRW_STATUS_INIT_ERROR;
	}
	rt_timer_start(crw_list->timer);
	return CRW_STATUS_OK;
}

CRW_STATUS
crw_list_new(COMM_RWINDOW_NODE **node, COMM_RWINDOW_LIST *crw_list, COMM_RWINDOW_NODE_DATA_TYPEDEF *data)
{
	if (crw_list->size <= COMM_RWINDOW_SIZE)
	{
		*node = rt_malloc(sizeof(**node));
		if (*node != RT_NULL)
		{
			rt_mutex_take(crw_list->mutex, RT_WAITING_FOREVER);
			(*node)->data = *data;
			list_add(&(*node)->list, &crw_list->list);
			crw_list->size++;
			rt_mutex_release(crw_list->mutex);

		}
		else
			return CRW_STATUS_NEW_ERROR;
	}
	else
		return CRW_STATUS_FULL;

	return CRW_STATUS_OK;
}


#ifdef RT_USING_FINSH
#include <finsh.h>

void
cw_print(void)
{
	rt_kprintf("cw_list size : %d", comm_twindow_list.size);
}

FINSH_FUNCTION_EXPORT(cw_print, debug cw_list);

#endif // RT_USING_FINSH
