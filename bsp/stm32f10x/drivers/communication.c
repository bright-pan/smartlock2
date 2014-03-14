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

rt_mq_t comm_mq = RT_NULL;

void
comm_thread_entry(void *parameters)
{
	rt_err_t result;
	rt_device_t device_comm;
	COMM_MAIL_TYPEDEF comm_mail_buf;

	device_comm = rt_device_find(DEVICE_NAME_COMM);
	if (device_comm != RT_NULL)
	{
		if (device_comm->open_flag == RT_DEVICE_OFLAG_CLOSE)
		{
			rt_device_open(device_comm, RT_DEVICE_OFLAG_RDWR);
		}
	}

	while (1)
	{
		// receive mail
		rt_memset(&comm_mail_buf, 0, sizeof(comm_mail_buf));
		result = rt_mq_recv(comm_mq, &comm_mail_buf,
							sizeof(comm_mail_buf),
							100);
		if (result == RT_EOK)
		{
			// process mail
			rt_kprintf("process mail:\n");
			rt_kprintf("comm_type: %d, length: %d\n", comm_mail_buf.comm_type, comm_mail_buf.len);
			print_hex(comm_mail_buf.buf, comm_mail_buf.len);
			// send length data
			comm_mail_buf.len += 4;
			rt_device_write(device_comm, 0, (uint8_t *)&(comm_mail_buf.len), 2);
			// send comm_type data
			rt_device_write(device_comm, 0, (uint8_t *)&comm_mail_buf.comm_type, 1);// comm_type
			// send buf data
			rt_device_write(device_comm, 0, comm_mail_buf.buf, comm_mail_buf.len);
			// send "\r\n"
			rt_device_write(device_comm, 0, "\r\n", 2);
			rt_sem_release(comm_mail_buf.result_sem);
		}
		else // time out
		{

		}
	}
}

void
send_comm_mail(COMM_TYPE_TYPEDEF comm_type, uint8_t *buf, uint16_t len)
{
	rt_err_t result;
	COMM_MAIL_TYPEDEF comm_mail_buf;

	if (comm_mq != RT_NULL)
	{
		rt_memset(&comm_mail_buf, 0, sizeof(comm_mail_buf));

		comm_mail_buf.result_sem = rt_sem_create("s_comm", 0, RT_IPC_FLAG_FIFO);
		comm_mail_buf.comm_type = comm_type;
		comm_mail_buf.buf = buf;
		comm_mail_buf.len = len;

		result = rt_mq_send(comm_mq, &comm_mail_buf, sizeof(comm_mail_buf));
		if (result == -RT_EFULL)
		{
			rt_kprintf("comm_mq is full!!!\n");
		}
		else
		{
			rt_sem_take(comm_mail_buf.result_sem, RT_WAITING_FOREVER);
		}

		rt_sem_delete(comm_mail_buf.result_sem);
	}
	else
	{
		rt_kprintf("comm_mq is RT_NULL!!!!\n");
	}
}

#ifdef RT_USING_FINSH
#include <finsh.h>

FINSH_FUNCTION_EXPORT(send_comm_mail, send_mail_buf[comm_type buf length]);

#endif // RT_USING_FINSH
