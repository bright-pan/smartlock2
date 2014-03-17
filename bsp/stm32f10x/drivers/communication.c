/*********************************************************************
 * Filename:			communication.c
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
#include "communication.h"

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

int8_t
process_frame(uint8_t *frame, uint16_t frame_size)
{
	uint8_t cmd;
	uint16_t length;
	int8_t result;
	RT_ASSERT(frame!=RT_NULL);
	RT_ASSERT(frame_size<=BUF_SIZE);


	cmd = *(frame + 2);
	length = frame_size -5;
	switch (cmd)
	{
		case COMM_TYPE_SMS:
			{
				break;
			}
		default :
			{
				rt_kprintf("this comm cmd is invalid!\n");
				break;
			}
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
						rt_kprintf("\ncomm recv frame length: %d\n", recv_counts);
						print_hex(process_buf, recv_counts);
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
	rt_device_t device_comm;
	COMM_MAIL_TYPEDEF comm_mail_buf;
    uint16_t length;
	device_comm = device_enable(DEVICE_NAME_COMM);

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
			rt_kprintf("process comm tx mail:\n");
			rt_kprintf("comm_type: %d, length: %d\n", comm_mail_buf.comm_type, comm_mail_buf.len);
			print_hex(comm_mail_buf.buf, comm_mail_buf.len);
			// send length data
            length = comm_mail_buf.len + 1;
			rt_device_write(device_comm, 0, (uint8_t *)&length, 2);
			// send comm_type data
			rt_device_write(device_comm, 0, (uint8_t *)&comm_mail_buf.comm_type, 1);// comm_type
			// send buf data
			rt_device_write(device_comm, 0, comm_mail_buf.buf, comm_mail_buf.len);
			// send "\r\n"
			rt_device_write(device_comm, 0, "\r\n", 2);
			// free mail buf memory
			rt_free(comm_mail_buf.buf);
		}
		else // time out
		{

		}
	}
}

rt_err_t
send_ctx_mail(COMM_TYPE_TYPEDEF comm_type, uint8_t *buf, uint16_t len)
{
	rt_err_t result = -RT_EFULL;
	uint8_t *buf_bk = RT_NULL;
	COMM_MAIL_TYPEDEF comm_mail_buf;

	if (comm_tx_mq != RT_NULL)
	{
		if ((buf_bk = (uint8_t *)rt_malloc(len)) != RT_NULL)
		{

			rt_memcpy(buf_bk, buf, len);

			rt_memset(&comm_mail_buf, 0, sizeof(comm_mail_buf));
			comm_mail_buf.result_sem = rt_sem_create("s_comm", 0, RT_IPC_FLAG_FIFO);
			comm_mail_buf.comm_type = comm_type;
			comm_mail_buf.buf = buf_bk;
			comm_mail_buf.len = len;

			result = rt_mq_send(comm_tx_mq, &comm_mail_buf, sizeof(comm_mail_buf));
			if (result == -RT_EFULL)
			{
				rt_kprintf("comm_mq is full!!!\n");
			}
			/*
			  else
			  {
			  rt_sem_take(comm_mail_buf.result_sem, RT_WAITING_FOREVER);
			  }
			  rt_sem_delete(comm_mail_buf.result_sem);
			*/
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
