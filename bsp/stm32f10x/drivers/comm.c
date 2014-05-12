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
#include "usart.h"

#define BUF_SIZE 768

rt_mq_t comm_tx_mq = RT_NULL;
rt_mutex_t comm_tx_mutex = RT_NULL;

extern char smsc[20];
extern char phone_call[20];

//receive data
static struct rt_ringbuffer gprsringbuffer;
static rt_uint8_t buffer[BUF_SIZE];
static rt_mutex_t gprs_mutex = RT_NULL;

//receive event
static comm_call_back sub_event_api = RT_NULL;

typedef enum {

	FRAME_STATUS_INVALID = 0,
	FRAME_STATUS_VALID = 1,
	FRAME_STATUS_OK = 2,

}FRAME_STATUS;

void sub_event_callback(comm_call_back fun)
{
	if(fun != RT_NULL)
	{	
		sub_event_api = fun;
	}
}

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

__STATIC_INLINE CTW_STATUS
process_response(uint8_t cmd, uint8_t *rep_frame, uint16_t length)
{
	CTW_STATUS result = CTW_STATUS_ERROR;

	switch (cmd)// process response
	{
		case COMM_TYPE_SMS:
		case COMM_TYPE_GPRS:
		case COMM_TYPE_GSM_CTRL_OPEN:
		case COMM_TYPE_GSM_CTRL_CLOSE:
		case COMM_TYPE_GSM_CTRL_RESET:
		case COMM_TYPE_GSM_CTRL_SWITCH_TO_CMD:
		case COMM_TYPE_GSM_CTRL_SWITCH_TO_GPRS:
		case COMM_TYPE_GSM_CTRL_DIALING:
		case COMM_TYPE_GSM_CTRL_PHONE_CALL_ANSWER:
		case COMM_TYPE_GSM_CTRL_PHONE_CALL_HANG_UP:
		case COMM_TYPE_VOICE_AMP:
			{
				result = *rep_frame;
				break;
			}
        case COMM_TYPE_ADC:
            {
#if (defined RT_USING_FINSH) && (defined COMM_DEBUG)
				rt_kprintf("the response cmd %02x, adc value is %02X!\n", cmd, *((uint16_t *)rep_frame)));
#endif // RT_USING_FINSH
                break;
            }
		default :
			{
#if (defined RT_USING_FINSH) && (defined COMM_DEBUG)
				rt_kprintf("the response cmd %02x is invalid!\n", cmd);
#endif // RT_USING_FINSH
				break;
			}
	}

	return result;
}

rt_size_t comm_recv_gprs_data(rt_uint8_t *buffer,rt_size_t size)
{
	rt_uint8_t *ptr;

  ptr = buffer;

	/* interrupt mode Rx */
	while (size)
	{
    rt_uint8_t ch;
    rt_size_t readsize;

		rt_mutex_take(gprs_mutex,RT_WAITING_FOREVER);
		readsize = rt_ringbuffer_getchar(&gprsringbuffer,&ch);
		rt_mutex_release(gprs_mutex);
		
    if(readsize == 0)
    {
    	break;
    }
    *ptr = ch;
    ptr ++;
    size --;
	}
	size = (rt_uint32_t)ptr - (rt_uint32_t)buffer;
	/* set error code */
	if (size == 0)
	{
	    rt_set_errno(-RT_EEMPTY);
	}

	return size;
}

__STATIC_INLINE CTW_STATUS
process_request(uint8_t cmd, uint8_t order, uint8_t *rep_frame, uint16_t length)
{
	CTW_STATUS result = CTW_STATUS_ERROR;
	uint8_t rep_cmd = cmd | 0x80;

	switch (cmd)// process request
	{
		case COMM_TYPE_GSM_SMSC:
			{
				rt_memcpy(smsc, rep_frame, length);
				result = CTW_STATUS_OK;
				send_ctx_mail(rep_cmd, order, 0, &result, 1);
				break;
			}
		case COMM_TYPE_GSM_PHONE_CALL:
			{
				rt_memcpy(phone_call, rep_frame, length);
				result = CTW_STATUS_OK;
				send_ctx_mail(rep_cmd, order, 0, &result, 1);
				break;
			}
		case COMM_TYPE_GPRS:
			{
				rt_uint8_t *ptr = rep_frame+1;
				//printf_data(rep_frame,length);
				//rt_kprintf("COMM_TYPE_GPRS\n");
				while (length-1)
        {
        	rt_size_t readsize;
        	
	    		rt_mutex_take(gprs_mutex,RT_WAITING_FOREVER);
	    		readsize = rt_ringbuffer_putchar(&gprsringbuffer,*ptr);
	    		rt_mutex_release(gprs_mutex);
	        if (readsize != 0)
	        {
	            ptr ++;
	            length --;
	        }
	        else
	        {
	          break;
	        } 
        }
        
				result = CTW_STATUS_OK;
				send_ctx_mail(rep_cmd, order, 0, &result, 1);
				/* process data */
				break;
			}
		case COMM_TYPE_SWITCH:
			{
        result = CTW_STATUS_OK;
        send_ctx_mail(rep_cmd, order, 0, &result, 1);

					//gpio_pin_output(DEVICE_NAME_VOICE_AMP, 1);
#if (defined RT_USING_FINSH) && (defined COMM_DEBUG)
				rt_kprintf("the comm swtich push %d\n", *rep_frame);
#endif // RT_USING_FINSH
                switch (*rep_frame) {
                    case 1: {
                        // key1
                        if(sub_event_api != RT_NULL)
                        {
                        	COMM_SUB_USER data;

                        	data.event = SUB_ENT_KEY1;
													sub_event_api((void *)&data);
                        }
                        break;
                    }
                    case 2: {
                        // key2
                        if(sub_event_api != RT_NULL)
                        {
                        	COMM_SUB_USER data;

                        	data.event = SUB_ENT_KEY2;
													sub_event_api((void *)&data);
                        }
                        break;
                    }
                    case 3: {
                        // key3
                        break;
                    }
                    case 4: {
                        //VIN
                        break;
                    }
                    default :{
                        break;
                    }
                }
				break;
			}
		default :
			{
#if (defined RT_USING_FINSH) && (defined COMM_DEBUG)
				rt_kprintf("the request cmd %02x is invalid!\n", cmd);
#endif // RT_USING_FINSH
				break;
			}
	}

	return result;
}

static int8_t
process_frame(uint8_t *frame, uint16_t frame_size)
{
	uint8_t cmd, order;
	uint16_t length;
	int8_t result = 0;
	COMM_TWINDOW_NODE *tmp;
	COMM_TWINDOW_LIST *ctw_list_bk = &comm_twindow_list;
	struct list_head *pos, *q;
	RT_ASSERT(frame!=RT_NULL);
	RT_ASSERT(frame_size<=BUF_SIZE);
	RT_ASSERT(ctw_list_bk!=RT_NULL);

	cmd = *(frame + 2);
	order = *(frame + 3);
	length = frame_size - 6;

	frame += 4;

	if (cmd & 0x80)
	{
#if (defined RT_USING_FINSH) && (defined COMM_DEBUG)
		rt_kprintf("recv response frame \ncmd: 0x%02X, order: 0x%02X, length: %d\n", cmd, order, length);
		print_hex(frame, length);
#endif // RT_USING_FINSH
		cmd &= 0x7f;
		rt_mutex_take(ctw_list_bk->mutex, RT_WAITING_FOREVER);
		list_for_each_safe(pos, q, &ctw_list_bk->list)
		{
			tmp= list_entry(pos, COMM_TWINDOW_NODE, list);

            if ((tmp->data.mail).comm_type == cmd &&
                tmp->data.order == order)
            {
#if (defined RT_USING_FINSH) && (defined COMM_DEBUG)
                rt_kprintf("recv response and delete cw node\n");
#endif // RT_USING_FINSH
                *((tmp->data.mail).result) = process_response(cmd, frame, length);
                rt_sem_release((tmp->data.mail).result_sem);
                rt_free((tmp->data.mail).buf);
                list_del(pos);
                ctw_list_bk->size--;
                rt_free(tmp);
            }
		}
		rt_mutex_release(ctw_list_bk->mutex);
	}
	else
	{
#if (defined RT_USING_FINSH) && (defined COMM_DEBUG)
		rt_kprintf("recv request frame \ncmd: 0x%02X, order: 0x%02X, length: %d\n", cmd, order, length);
		print_hex(frame, length);
#endif // RT_USING_FINSH
		process_request(cmd, order, frame, length);
	}
	return result;
}

void
comm_rx_thread_entry(void *parameters)
{
	rt_device_t device_comm;
	uint16_t length;
	uint16_t recv_counts;
	uint8_t flag = 0;
    uint8_t *process_buf_bk;
	uint8_t process_buf[BUF_SIZE];

    //RT_ASSERT(crw_list_init(&comm_rwindow_list) == CRW_STATUS_OK);
	device_comm = device_enable(DEVICE_NAME_COMM);
	rt_ringbuffer_init(&gprsringbuffer,buffer,BUF_SIZE);
	gprs_mutex = rt_mutex_create("comrecv",RT_IPC_FLAG_FIFO);
	RT_ASSERT(gprs_mutex != RT_NULL);
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
					if (length > BUF_SIZE -4 || length + 4 < recv_counts) {
#if (defined RT_USING_FINSH) && (defined COMM_DEBUG)
                        rt_kprintf("\ncomm recv error frame length: %d\n", recv_counts);
                        print_hex(process_buf, recv_counts);
#endif
                        break;
                    }
					if (length + 4 > recv_counts)
						goto continue_check;
					if (length + 4 == recv_counts)
					{
#if (defined RT_USING_FINSH) && (defined COMM_DEBUG)
						rt_kprintf("\ncomm recv frame length: %d\n", recv_counts);
						print_hex(process_buf, recv_counts);
#endif
                        process_frame(process_buf, recv_counts);
						break;
					}
				}
		  continue_check:
				if (recv_counts >= BUF_SIZE) {
#if (defined RT_USING_FINSH) && (defined COMM_DEBUG)
                    rt_kprintf("\ncomm recv error frame length: %d\n", recv_counts);
                    print_hex(process_buf, recv_counts);
#endif
					break;
                }
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
	COMM_TMAIL_TYPEDEF comm_tmail_buf;
    COMM_TWINDOW_NODE_DATA_TYPEDEF data;
	CTW_STATUS ctw_status;
	COMM_TWINDOW_NODE *ctw_node;
	uint8_t order = 0;
    rt_device_t device_comm = device_enable(DEVICE_NAME_COMM);

    RT_ASSERT(device_comm != RT_NULL);
	RT_ASSERT(ctw_list_init(&comm_twindow_list) == CTW_STATUS_OK);

	while (1)
	{
		// receive mail
		rt_memset(&comm_tmail_buf, 0, sizeof(comm_tmail_buf));
		result = rt_mq_recv(comm_tx_mq, &comm_tmail_buf,
							sizeof(comm_tmail_buf),
							100);
		if (result == RT_EOK)
		{
			// process mail
			//RT_ASSERT(comm_tmail_buf.result_sem != RT_NULL);
			RT_ASSERT(comm_tmail_buf.result != RT_NULL);
			RT_ASSERT(comm_tmail_buf.buf != RT_NULL);
            if (comm_tmail_buf.comm_type & 0x80) {
                if (comm_tmail_buf.buf != RT_NULL) {
                    send_frame(device_comm, &comm_tmail_buf, comm_tmail_buf.order);
#if (defined RT_USING_FINSH) && (defined COMM_DEBUG)
                    rt_kprintf("send response frame and delete cw node\n cmd: 0x%02X, order: 0x%02X, length: %d\n",
                               comm_tmail_buf.comm_type, comm_tmail_buf.order, comm_tmail_buf.len);
                    print_hex(comm_tmail_buf.buf, comm_tmail_buf.len);
#endif // RT_USING_FINSH 
                    rt_free(comm_tmail_buf.buf);
                }


            } else {
                rt_memset(&data, 0, sizeof(data));
                data.mail = comm_tmail_buf;
                if (comm_tmail_buf.order)
                {
                    data.order = comm_tmail_buf.order;
                }
                else
                {
                    if (order++)
                        data.order = order++;
                    else
                        data.order = 1;
                }
                data.delay = comm_tmail_buf.delay;

                ctw_status = ctw_list_new(&ctw_node, &comm_twindow_list, &data);
                if (ctw_status == CTW_STATUS_OK)
                {
                    send_frame(device_comm, &comm_tmail_buf, data.order);
#if (defined RT_USING_FINSH) && (defined COMM_DEBUG)
                    rt_kprintf("send request frame\n cmd: 0x%02X, order: 0x%02X, length: %d\n",
                               comm_tmail_buf.comm_type, data.order, comm_tmail_buf.len);
                    print_hex(comm_tmail_buf.buf, comm_tmail_buf.len);
#endif // RT_USING_FINSH
                }
                else
                {
                    /* tell error for mail sender */
                    if (!(comm_tmail_buf.comm_type & 0x80)) {
                        *comm_tmail_buf.result = ctw_status;
                        rt_sem_release(comm_tmail_buf.result_sem);
                    }
                    if (comm_tmail_buf.buf != RT_NULL) {
                        rt_free(comm_tmail_buf.buf);
                    }
                }
            }
		}
		else // time out
		{

		}
	}
}

void
send_frame(rt_device_t device, COMM_TMAIL_TYPEDEF *mail, uint8_t order)
{
    uint16_t length;
    struct rt_serial_device *dev = (struct rt_serial_device *)device;
    struct stm32_uart *uart = dev->parent.user_data;
    rt_mutex_take(uart->lock, RT_WAITING_FOREVER);
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
    rt_mutex_release(uart->lock);
}

CTW_STATUS
send_ctx_mail(COMM_TYPE_TYPEDEF comm_type, uint8_t order, uint16_t delay, uint8_t *buf, uint16_t len)
{
	rt_err_t result = -RT_EFULL;
	uint8_t *buf_bk = RT_NULL;
	COMM_TMAIL_TYPEDEF comm_tmail_buf;
	CTW_STATUS ctw_status;

    rt_memset(&comm_tmail_buf, 0, sizeof(comm_tmail_buf));
    comm_tmail_buf.result_sem = RT_NULL;
	if (comm_tx_mq != RT_NULL)
	{
		if (len) {
			buf_bk = (uint8_t *)rt_malloc(len);
			if (buf_bk == RT_NULL)
				goto __free_process;
			rt_memcpy(buf_bk, buf, len);
		}
		else
			buf_bk = buf;

		if (comm_type & 0x80)
			comm_tmail_buf.result_sem = RT_NULL;
		else {
			comm_tmail_buf.result_sem = rt_sem_create("s_ctx", 0, RT_IPC_FLAG_FIFO);
            if (comm_tmail_buf.result_sem == RT_NULL)
                goto __free_process;
        }
        comm_tmail_buf.result = &ctw_status;
		comm_tmail_buf.comm_type = comm_type;
		comm_tmail_buf.order = order;
		comm_tmail_buf.buf = buf_bk;
		comm_tmail_buf.len = len;
		comm_tmail_buf.delay = delay;

		if (!(comm_type & 0x80))
			result = rt_mq_send(comm_tx_mq, &comm_tmail_buf, sizeof(comm_tmail_buf));
		else
			result = rt_mq_urgent(comm_tx_mq, &comm_tmail_buf, sizeof(comm_tmail_buf));

		if (result == -RT_EFULL)
		{
#if (defined RT_USING_FINSH) && (defined COMM_DEBUG)
			rt_kprintf("comm_mq is full!!!\n");
#endif
            goto __free_process;
		}
		else
		{
			if (!(comm_type & 0x80)) {
				rt_sem_take(comm_tmail_buf.result_sem, RT_WAITING_FOREVER);
#if (defined RT_USING_FINSH) && (defined COMM_DEBUG)
				rt_kprintf("send result is %d\n", *comm_tmail_buf.result);
#endif
			}
		}
		if (!(comm_type & 0x80))
			rt_sem_delete(comm_tmail_buf.result_sem);
	}
	else
	{
#if (defined RT_USING_FINSH) && (defined COMM_DEBUG)
		rt_kprintf("comm_mq is RT_NULL!!!!\n");
#endif
	}
    return CTW_STATUS_OK;

__free_process:
    if (buf_bk != RT_NULL)
		rt_free(buf_bk);
    if (comm_tmail_buf.result_sem != RT_NULL)
		rt_sem_delete(comm_tmail_buf.result_sem);
	return CTW_STATUS_ERROR;
}

int
rt_comm_init(void)
{
	rt_thread_t comm_rx_thread;
	rt_thread_t comm_tx_thread;
	// initial comm thread
	comm_rx_thread = rt_thread_create("comm_rx",
									  comm_rx_thread_entry, RT_NULL,
									  2048,101,5);
	if (comm_rx_thread == RT_NULL)
        return -1;

	rt_thread_startup(comm_rx_thread);

	// initial comm msg queue
	comm_tx_mq = rt_mq_create("comm_tx", sizeof(COMM_TMAIL_TYPEDEF),
							  COMM_TMAIL_MAX_MSGS, RT_IPC_FLAG_FIFO);
    if (comm_tx_mq == RT_NULL)
        return -1;
	// initial comm thread
	comm_tx_thread = rt_thread_create("comm_tx",
							   comm_tx_thread_entry, RT_NULL,
							   512, 102, 5);
	if (comm_tx_thread == RT_NULL)
        return -1;

    rt_thread_startup(comm_tx_thread);
    return 0;
}

INIT_APP_EXPORT(rt_comm_init);

#ifdef RT_USING_FINSH
#include <finsh.h>

void send_dialing(void)
{
	uint8_t *buf = rt_malloc(512);
	*buf = 0;
	rt_memcpy(buf+1, &(device_config.param.tcp_domain[0]), sizeof(device_config.param.tcp_domain[0]));
	send_ctx_mail(COMM_TYPE_GSM_CTRL_DIALING, 0, 0, buf, sizeof(device_config.param.tcp_domain[0])+1);
	rt_free(buf);
}

FINSH_FUNCTION_EXPORT(send_ctx_mail, send_mail_buf[comm_type buf length]);
FINSH_FUNCTION_EXPORT(send_dialing, send_gsm_dialing[]);
#endif // RT_USING_FINSH
