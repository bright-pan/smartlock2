/*********************************************************************
 * Filename:			comm.c
 *
 * Description:
 *
 * Author:				Bright Pan
 * Email:				bright_pan@yuettak.com
 * Date:				2014-03-14
 *
 * Modify:
 *
 * Copyright (C) 2014 Yuettak Co.,Ltd
 ********************************************************************/
#include "comm.h"
#include "comm_window.h"

#define BUF_SIZE 768

rt_mq_t comm_tx_mq = RT_NULL;
rt_mutex_t comm_tx_mutex = RT_NULL;

typedef enum {

	FRAME_STATUS_INVALID = 0,
	FRAME_STATUS_VALID = 1,
	FRAME_STATUS_OK = 2,

}FRAME_STATUS;

FRAME_STATUS
check_frame(uint8_t *str)
{
	static FRAME_STATUS flag;

	if (*str == '\r')
	{
		flag = FRAME_STATUS_VALID;
	}
	else if(*str == '\n')
	{
		if (flag == FRAME_STATUS_VALID)
		{
			flag = FRAME_STATUS_OK;
		}
		else
		{
			flag = FRAME_STATUS_INVALID;
		}
	}
	else
	{
		flag = FRAME_STATUS_INVALID;
	}

	return flag;
}
__STATIC_INLINE CW_STATUS
process_response(uint8_t cmd, uint8_t *rep_frame, uint16_t length)
{
	CW_STATUS result;

	switch (cmd)// process response
	{
		case COMM_TYPE_SMS:
			{
				if (*rep_frame == 1)
					result = CW_STATUS_OK;
				break;
			}
		default :
			{
#ifdef RT_USING_FINSH
				rt_kprintf("this comm cmd is invalid!\n");
#endif // RT_USING_FINSH
				break;
			}
	}

	return result;
}

__STATIC_INLINE CW_STATUS
process_request(uint8_t cmd, uint8_t *rep_frame, uint16_t length)
{
	CW_STATUS result;

	switch (cmd)// process request
	{
		default :
			{
#ifdef RT_USING_FINSH
				rt_kprintf("this comm cmd is invalid!\n");
#endif // RT_USING_FINSH
				break;
			}
	}

	return result;
}

int8_t
process_frame(uint8_t *frame, uint16_t frame_size)
{
	uint8_t cmd, order;
	uint16_t length;
	int8_t result;
	COMM_WINDOW_NODE *tmp;
	COMM_WINDOW_LIST *cw_list_bk = &cw_list;
	struct list_head *pos, *q;
	RT_ASSERT(frame!=RT_NULL);
	RT_ASSERT(frame_size<=BUF_SIZE);
	RT_ASSERT(cw_list_bk!=RT_NULL);

	cmd = *(frame + 2);
	order = *(frame + 3);
	length = frame_size - 6;

	frame += 4;
#ifdef RT_USING_FINSH
	rt_kprintf("cmd: 0x%02X, order: 0x%02X, length: %d\n", cmd, order, length);
	print_hex(frame, length);
#endif // RT_USING_FINSH

	if (cmd & 0x80)
	{
		cmd &= 0x7f;
		rt_mutex_take(cw_list_bk->mutex, RT_WAITING_FOREVER);
		list_for_each_safe(pos, q, &cw_list_bk->list)
		{
			tmp= list_entry(pos, COMM_WINDOW_NODE, list);
			if (tmp->flag == CW_FLAG_REQUEST) // request
			{
				if ((tmp->mail).comm_type == cmd &&
					tmp->order == order)
				{
#ifdef RT_USING_FINSH
					rt_kprintf("recv response and delete cw node\n");
					rt_kprintf("comm_type: 0x%02X, order: 0x%02X, length: %d\n", (tmp->mail).comm_type, tmp->order, (tmp->mail).len);
					print_hex((tmp->mail).buf, (tmp->mail).len);
#endif // RT_USING_FINSH
					*((tmp->mail).result) = process_response(cmd, frame, length);
					rt_sem_release((tmp->mail).result_sem);
					rt_free((tmp->mail).buf);
					list_del(pos);
					rt_free(tmp);
				}
			}
		}
		rt_mutex_release(cw_list_bk->mutex);
	}
	else
	{
		process_request(cmd, frame, length);
	}
	return result;
}

void
comm_rx_thread_entry(void *parameters)
{
	rt_device_t device_comm;
	uint16_t length;
	uint16_t recv_counts;
	uint8_t flag;
    uint8_t *process_buf_bk;
	uint8_t process_buf[BUF_SIZE];

	device_comm = device_enable(DEVICE_NAME_COMM);

	while (1) {
		process_buf_bk = process_buf;
		recv_counts = 0;
		while (1)
		{
			if (rt_device_read(device_comm, 0, process_buf_bk, 1))
			{
				flag = 0;
				recv_counts++;
				if (check_frame(process_buf_bk) == FRAME_STATUS_OK)
				{
					length = *((uint16_t *)process_buf);
					if (length > BUF_SIZE -4 || length + 4 < recv_counts)
						break;
					if (length + 4 > recv_counts)
						goto continue_check;
					if (length + 4 == recv_counts)
					{
						process_frame(process_buf, recv_counts);
						break;
					}
				}
		  continue_check:
				if (recv_counts >= BUF_SIZE)
					break;
				process_buf_bk++;
			}
			else
			{
				if (flag++ < 10)
				{
					rt_thread_delay(1);
				}
				else
				{
					flag = 0;
					break;
				}
			}
		}
	}
}

void
comm_tx_thread_entry(void *parameters)
{
	rt_err_t result;
	COMM_MAIL_TYPEDEF comm_mail_buf;
	CW_STATUS cw_status;
	COMM_WINDOW_NODE *cw_node;
	uint8_t order = 0;

	RT_ASSERT(cw_list_init(&cw_list) == CW_STATUS_OK);

	while (1)
	{
		// receive mail
		rt_memset(&comm_mail_buf, 0, sizeof(comm_mail_buf));
		result = rt_mq_recv(comm_tx_mq, &comm_mail_buf,
							sizeof(comm_mail_buf),
							100);
		if (result == RT_EOK)
		{
			// process mail
			RT_ASSERT(comm_mail_buf.result_sem != RT_NULL);
			RT_ASSERT(comm_mail_buf.result != RT_NULL);
			RT_ASSERT(comm_mail_buf.buf != RT_NULL);

			cw_status = cw_list_new(&cw_node, &cw_list);
			if (cw_status == CW_STATUS_OK)
			{
				cw_node->mail = comm_mail_buf;
				if (comm_mail_buf.order)
				{
					cw_node->order = comm_mail_buf.order;
				}
				else
				{
					cw_node->order = order++;
				}
				cw_node->flag = (comm_mail_buf.comm_type & 0x80) ? CW_FLAG_RESPONSE : CW_FLAG_REQUEST;
#ifdef RT_USING_FINSH
				rt_kprintf("process comm tx mail:\n");
				rt_kprintf("comm_type: %d, length: %d\n", comm_mail_buf.comm_type, comm_mail_buf.len);
				print_hex(comm_mail_buf.buf, comm_mail_buf.len);
#endif // RT_USING_FINSH
			}
			else
			{
				/* tell error for mail sender */
				*comm_mail_buf.result = cw_status;
				rt_sem_release(comm_mail_buf.result_sem);
			}
		}
		else // time out
		{

		}
	}
}

void
send_frame(rt_device_t device, COMM_MAIL_TYPEDEF *mail, uint8_t order)
{
	uint16_t length;
	// send length data
	length = mail->len + 2;
	rt_device_write(device, 0, (uint8_t *)&length, 2);
	// send comm_type data
	rt_device_write(device, 0, (uint8_t *)&mail->comm_type, 1);// comm_type
	rt_device_write(device, 0, &order, 1);// order
	// send buf data
	rt_device_write(device, 0, mail->buf, mail->len);
	// send "\r\n"
	rt_device_write(device, 0, "\r\n", 2);
	// free mail buf memory
}

rt_err_t
send_ctx_mail(COMM_TYPE_TYPEDEF comm_type, uint8_t *buf, uint16_t len, uint8_t order)
{
	rt_err_t result = -RT_EFULL;
	uint8_t *buf_bk = RT_NULL;
	COMM_MAIL_TYPEDEF comm_mail_buf;
	CW_STATUS cw_status;

	if (comm_tx_mq != RT_NULL)
	{
		if ((buf_bk = (uint8_t *)rt_malloc(len)) != RT_NULL)
		{
			rt_memcpy(buf_bk, buf, len);

			rt_memset(&comm_mail_buf, 0, sizeof(comm_mail_buf));
			comm_mail_buf.result_sem = rt_sem_create("s_comm", 0, RT_IPC_FLAG_FIFO);
			comm_mail_buf.result = &cw_status;
			comm_mail_buf.comm_type = comm_type;
			comm_mail_buf.order = order;
			comm_mail_buf.buf = buf_bk;
			comm_mail_buf.len = len;

			result = rt_mq_send(comm_tx_mq, &comm_mail_buf, sizeof(comm_mail_buf));
			if (result == -RT_EFULL)
			{
				rt_kprintf("comm_mq is full!!!\n");
			}
			else
			{
				rt_sem_take(comm_mail_buf.result_sem, RT_WAITING_FOREVER);
				rt_kprintf("send result is %d\n", *comm_mail_buf.result);
			}
			rt_sem_delete(comm_mail_buf.result_sem);
		}
	}
	else
	{
		rt_kprintf("comm_mq is RT_NULL!!!!\n");
	}
	return result;
}

#ifdef RT_USING_FINSH
#include <finsh.h>

FINSH_FUNCTION_EXPORT(send_ctx_mail, send_mail_buf[comm_type buf length]);

#endif // RT_USING_FINSH
