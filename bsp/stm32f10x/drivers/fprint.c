/*********************************************************************
 * Filename:			fprint.c
 *
 * Description:
 *
 * Author:				Bright Pan
 * Email:				bright_pan@yuettak.com
 * Date:				2014-03-25
 *
 * Modify:
 *
 * Copyright (C) 2014 Yuettak Co.,Ltd
 ********************************************************************/

#include "fprint.h"
#include "gpio_pin.h"
#include "gpio_pwm.h"
#include "comm.h"

#define FPRINT_MAIL_MAX_MSGS 10

//#define FPRINT_DEBUG
#define DEVICE_NAME_FPRINT "uart2"
#define FPRINT_TEMPLATE_OFFSET 1000 // 1 <= offset <= 2000
#define FPRINT_TEMPLATE_SIZE 2000 // 1 <= offset <= 2000
#define FPRINT_TEMPLATE_ID_START FPRINT_TEMPLATE_OFFSET
#define FPRINT_TEMPLATE_ID_END (FPRINT_TEMPLATE_OFFSET + KEY_NUMBERS - 1)
//cmd define
#define FPRINT_FRAME_CMD_TEST_CONNECTION 0x0001
#define FPRINT_FRAME_CMD_SET_PARAM 0x0002
#define FPRINT_FRAME_CMD_STORE_CHAR 0x0040
#define FPRINT_FRAME_CMD_LOAD_CHAR 0x0041
#define FPRINT_FRAME_CMD_UP_CHAR 0x0042
#define FPRINT_FRAME_CMD_DOWN_CHAR 0x0043
#define	FPRINT_FRAME_CMD_DEL_CHAR 0x0044
#define FPRINT_FRAME_CMD_GET_STATUS 0x0046
#define FPRINT_FRAME_CMD_GET_IMAGE 0x0020
#define FPRINT_FRAME_CMD_GENERATE 0x0060
#define FPRINT_FRAME_CMD_MERGE 0x0061
#define FPRINT_FRAME_CMD_SEARCH 0x0063
//prefix define
#define FPRINT_FRAME_PREFIX_REQUEST 0xAA55
#define FPRINT_FRAME_PREFIX_RESPONSE 0x55AA
#define FPRINT_FRAME_PREFIX_DATA_REQUEST 0xA55A
#define FPRINT_FRAME_PREFIX_DATA_RESPONSE 0x5AA5
//result define
#define FPRINT_FRAME_ERR_SUCCESS 0x00
#define FPRINT_FRAME_ERR_FAIL 0x01
#define FPRINT_FRAME_ERR_TMPL_EMPTY 0x22
#define FPRINT_FRAME_ERR_INVALID_PARAM 0x12
#define FPRINT_FRAME_ERR_FP_NOT_DETECTED 0x28
#define FPRINT_FRAME_ERR_MEMORY 0x1c
#define FPRINT_FRAME_ERR_BAD_QUALITY 0x19
#define FPRINT_FRAME_ERR_DUPLICATION_ID 0x18

typedef struct{

	uint16_t prefix;
	uint8_t sid;
	uint8_t did;
	uint16_t cmd;
	uint16_t len;

}FPRINT_FRAME_HEAD_TYPEDEF;

typedef struct {

	uint8_t reserve[16];

}FPRINT_FRAME_DATA_REQ_TEST_CONNECTION_TYPEDEF;

typedef struct {

	uint16_t result;
	uint8_t reserve[14];

}FPRINT_FRAME_DATA_REP_TEST_CONNECTION_TYPEDEF;
typedef struct {

	uint8_t type;
	uint32_t data;
	uint8_t reserve[11];

}FPRINT_FRAME_DATA_REQ_SET_PARAM_TYPEDEF;

typedef struct {

	uint16_t result;
	uint8_t reserve[14];

}FPRINT_FRAME_DATA_REP_SET_PARAM_TYPEDEF;

typedef struct {

	uint16_t start;
	uint16_t end;
	uint8_t reserve[12];

}FPRINT_FRAME_DATA_REQ_DEL_CHAR_TYPEDEF;

typedef struct {

	uint16_t result;
	uint8_t reserve[14];

}FPRINT_FRAME_DATA_REP_DEL_CHAR_TYPEDEF;

typedef struct {
	uint16_t template_id;
	uint16_t ram_buf_id;
	uint8_t reserve[12];
}FPRINT_FRAME_DATA_REQ_STORE_CHAR_TYPEDEF;

typedef struct {
	uint16_t result;
	uint16_t template_id;
	uint8_t reserve[12];
}FPRINT_FRAME_DATA_REP_STORE_CHAR_TYPEDEF;

typedef struct {
	uint16_t template_id;
	uint16_t ram_buf_id;
	uint8_t reserve[12];
}FPRINT_FRAME_DATA_REQ_LOAD_CHAR_TYPEDEF;

typedef struct {
	uint16_t result;
	uint8_t reserve[14];
}FPRINT_FRAME_DATA_REP_LOAD_CHAR_TYPEDEF;

typedef struct {
	uint16_t ram_buf_id;
	uint8_t reserve[14];
}FPRINT_FRAME_DATA_REQ_UP_CHAR_TYPEDEF;

typedef struct {
	uint16_t result;
	uint16_t len;
	uint8_t reserve[12];
}FPRINT_FRAME_DATA_REP_UP_CHAR_TYPEDEF;

typedef struct {
	uint16_t result;
	uint8_t template[498];
}FPRINT_FRAME_DATA_DREP_UP_CHAR_TYPEDEF;

typedef struct {
	uint16_t len;
	uint8_t reserve[14];
}FPRINT_FRAME_DATA_REQ_DOWN_CHAR_TYPEDEF;

typedef struct {
	uint16_t result;
	uint8_t reserve[14];
}FPRINT_FRAME_DATA_REP_DOWN_CHAR_TYPEDEF;

typedef struct {
	uint16_t ram_buf_id;
	uint8_t template[498];
}FPRINT_FRAME_DATA_DREQ_DOWN_CHAR_TYPEDEF;

typedef struct {
	uint16_t result;
}FPRINT_FRAME_DATA_DREP_DOWN_CHAR_TYPEDEF;

typedef struct {
	uint16_t template_id;
	uint8_t reserve[14];
}FPRINT_FRAME_DATA_REQ_GET_STATUS_TYPEDEF;

typedef struct {
	uint16_t result;
	uint8_t status;
	uint8_t reserve[13];
}FPRINT_FRAME_DATA_REP_GET_STATUS_TYPEDEF;

typedef struct {
	uint8_t reserve[16];
}FPRINT_FRAME_DATA_REQ_GET_IMAGE_TYPEDEF;

typedef struct {
	uint16_t result;
	uint8_t reserve[14];
}FPRINT_FRAME_DATA_REP_GET_IMAGE_TYPEDEF;

typedef struct {
	uint16_t ram_buf_id;
	uint8_t reserve[14];
}FPRINT_FRAME_DATA_REQ_GENERATE_TYPEDEF;

typedef struct {
	uint16_t result;
	uint8_t reserve[14];
}FPRINT_FRAME_DATA_REP_GENERATE_TYPEDEF;

typedef struct {
	uint16_t ram_buf_id;
	uint8_t ram_bufs;
	uint8_t reserve[13];
}FPRINT_FRAME_DATA_REQ_MERGE_TYPEDEF;

typedef struct {
	uint16_t result;
	uint8_t reserve[14];
}FPRINT_FREAM_DATA_REP_MERGE_TYPEDEF;

typedef struct {
	uint16_t ram_buf_id;
	uint16_t template_id_start;
	uint16_t template_id_end;
	uint8_t reserve[10];
}FPRINT_FRAME_DATA_REQ_SEARCH_TYPEDEF;

typedef struct {
	uint16_t result;
	uint16_t template_id;
	uint8_t status;
	uint8_t reserve[11];
}FPRINT_FRAME_DATA_REP_SEARCH_TYPEDEF;

typedef union {

	FPRINT_FRAME_DATA_REQ_TEST_CONNECTION_TYPEDEF req_test_connection;
	FPRINT_FRAME_DATA_REP_TEST_CONNECTION_TYPEDEF rep_test_connection;

	FPRINT_FRAME_DATA_REQ_SET_PARAM_TYPEDEF req_set_param;
	FPRINT_FRAME_DATA_REP_SET_PARAM_TYPEDEF rep_set_param;

	FPRINT_FRAME_DATA_REQ_DEL_CHAR_TYPEDEF req_del_char;
	FPRINT_FRAME_DATA_REP_DEL_CHAR_TYPEDEF rep_del_char;

	FPRINT_FRAME_DATA_REQ_STORE_CHAR_TYPEDEF req_store_char;
	FPRINT_FRAME_DATA_REP_STORE_CHAR_TYPEDEF rep_store_char;

	FPRINT_FRAME_DATA_REQ_LOAD_CHAR_TYPEDEF req_load_char;
	FPRINT_FRAME_DATA_REP_LOAD_CHAR_TYPEDEF rep_load_char;

	FPRINT_FRAME_DATA_REQ_UP_CHAR_TYPEDEF req_up_char;
	FPRINT_FRAME_DATA_REP_UP_CHAR_TYPEDEF rep_up_char;
	FPRINT_FRAME_DATA_DREP_UP_CHAR_TYPEDEF drep_up_char;

	FPRINT_FRAME_DATA_REQ_DOWN_CHAR_TYPEDEF req_down_char;
	FPRINT_FRAME_DATA_REP_DOWN_CHAR_TYPEDEF rep_down_char;
	FPRINT_FRAME_DATA_DREQ_DOWN_CHAR_TYPEDEF dreq_down_char;
	FPRINT_FRAME_DATA_DREP_DOWN_CHAR_TYPEDEF drep_down_char;

	FPRINT_FRAME_DATA_REQ_GET_STATUS_TYPEDEF req_get_status;
	FPRINT_FRAME_DATA_REP_GET_STATUS_TYPEDEF rep_get_status;

	FPRINT_FRAME_DATA_REQ_GET_IMAGE_TYPEDEF req_get_image;
	FPRINT_FRAME_DATA_REP_GET_IMAGE_TYPEDEF rep_get_image;

	FPRINT_FRAME_DATA_REQ_GENERATE_TYPEDEF req_generate;
	FPRINT_FRAME_DATA_REP_GENERATE_TYPEDEF rep_generate;

	FPRINT_FRAME_DATA_REQ_MERGE_TYPEDEF req_merge;
	FPRINT_FREAM_DATA_REP_MERGE_TYPEDEF rep_merge;

	FPRINT_FRAME_DATA_REQ_SEARCH_TYPEDEF req_search;
	FPRINT_FRAME_DATA_REP_SEARCH_TYPEDEF rep_search;

}FPRINT_FRAME_DATA_TYPEDEF;

static rt_mq_t fprint_mq;

//fprint output data API
static fprint_call_back 	fprintf_ok_fun = RT_NULL;
static fprint_call_back 	fprintf_error_fun = RT_NULL;
static fprint_call_back 	fprintf_null_fun = RT_NULL;

void fp_ok_callback(fprint_call_back fun)
{
	if(fun != RT_NULL)
	{
		fprintf_ok_fun = fun;
	}
}

void fp_error_callback(fprint_call_back fun)
{
	if(fun != RT_NULL)
	{
		fprintf_error_fun = fun;
	}
}

void fp_null_callback(fprint_call_back fun)
{
	if(fun != RT_NULL)
	{
		fprintf_null_fun = fun;
	}
}


static uint16_t
check_sum(uint8_t *frame, uint16_t len)
{
	uint16_t i;
	uint16_t cs = 0;

	for (i = 0; i < len; i++) {
		cs += *(frame + i);
	}

	return cs;
}

void
fprint_reset(void)
{
	gpio_pin_output(DEVICE_NAME_FPRINT_RESET, 0);
	rt_thread_delay(20);
	gpio_pin_output(DEVICE_NAME_FPRINT_RESET, 1);
	rt_thread_delay(100);
}


__STATIC_INLINE FPRINT_ERROR_TYPEDEF
fprint_frame_send(FPRINT_FRAME_HEAD_TYPEDEF *frame_head,
				  FPRINT_FRAME_DATA_TYPEDEF *frame_data)
{
	uint16_t cs;
	rt_device_t fprint_device;
	uint16_t data_length;
	FPRINT_ERROR_TYPEDEF error = FPRINT_EERROR;

	fprint_device = device_enable(DEVICE_NAME_FPRINT);
	if (fprint_device == RT_NULL)
		return error;

	rt_device_control(fprint_device, RT_DEVICE_CTRL_CLR_RB, RT_NULL);
	if (frame_head->prefix == FPRINT_FRAME_PREFIX_REQUEST)
	{
		switch (frame_head->cmd)
		{
			case FPRINT_FRAME_CMD_TEST_CONNECTION:
				{
					data_length = sizeof(frame_data->req_test_connection);
					break;
				}
			case FPRINT_FRAME_CMD_DEL_CHAR:
				{
					data_length = sizeof(frame_data->req_del_char);
					break;
				}
			case FPRINT_FRAME_CMD_GET_IMAGE:
				{
					data_length = sizeof(frame_data->req_get_image);
					break;
				}
			case FPRINT_FRAME_CMD_GENERATE:
				{
					data_length = sizeof(frame_data->req_generate);
					break;
				}
			case FPRINT_FRAME_CMD_MERGE:
				{
					data_length = sizeof(frame_data->req_merge);
					break;
				}
			case FPRINT_FRAME_CMD_STORE_CHAR:
				{
					data_length = sizeof(frame_data->req_store_char);
					break;
				}
			case FPRINT_FRAME_CMD_UP_CHAR:
				{
					data_length = sizeof(frame_data->req_up_char);
					break;
				}
			case FPRINT_FRAME_CMD_DOWN_CHAR:
				{
					data_length = sizeof(frame_data->req_down_char);
					break;
				}
			case FPRINT_FRAME_CMD_SEARCH:
				{
					data_length = sizeof(frame_data->req_search);
					break;
				}
			case FPRINT_FRAME_CMD_SET_PARAM:
				{
					//data_length = sizeof(frame_data->req_set_param.type) + sizeof(frame_data->req_set_param.data) + sizeof(frame_data->req_set_param.reserve);
					cs = check_sum((uint8_t *)frame_head, sizeof(*frame_head));
					cs += check_sum((uint8_t *)&(frame_data->req_set_param.type), sizeof(frame_data->req_set_param.type));
					cs += check_sum((uint8_t *)&(frame_data->req_set_param.data), sizeof(frame_data->req_set_param.data));
					cs += check_sum(frame_data->req_set_param.reserve, sizeof(frame_data->req_set_param.reserve));
					error = FPRINT_EOK;
					rt_device_write(fprint_device, 0, frame_head, sizeof(*frame_head));
					rt_device_write(fprint_device, 0, &(frame_data->req_set_param.type), sizeof(frame_data->req_set_param.type));
					rt_device_write(fprint_device, 0, &(frame_data->req_set_param.data), sizeof(frame_data->req_set_param.data));
					rt_device_write(fprint_device, 0, frame_data->req_set_param.reserve, sizeof(frame_data->req_set_param.reserve));
					rt_device_write(fprint_device, 0, &cs, 2);

#if (defined RT_USING_FINSH) && (defined FPRINT_DEBUG)
					rt_kprintf("\nfprint send :---------------------\n");
					print_hex((uint8_t *)frame_head, sizeof(*frame_head));
					print_hex((uint8_t *)&(frame_data->req_set_param.type), sizeof(frame_data->req_set_param.type));
					print_hex((uint8_t *)&(frame_data->req_set_param.data), sizeof(frame_data->req_set_param.data));
					print_hex(frame_data->req_set_param.reserve, sizeof(frame_data->req_set_param.reserve));
					print_hex((uint8_t *)&cs, sizeof(cs));
					rt_kprintf("------------------------------------\n");
#endif // RT_USING_FINSH

					return error;
				}
			default :
				{
					data_length = 0;
#if (defined RT_USING_FINSH) && (defined FPRINT_DEBUG)
					rt_kprintf("the finger print frame cmd 0x%04X is invalid!\n", frame_head->cmd);
#endif // RT_USING_FINSH
					return error;
				}
		}

	}
	else if (frame_head->prefix == FPRINT_FRAME_PREFIX_DATA_REQUEST)
	{
		switch (frame_head->cmd)
		{
			case FPRINT_FRAME_CMD_DOWN_CHAR:
				{
					data_length = sizeof(frame_data->dreq_down_char);
					break;
				}
			default :
				{
					data_length = 0;
#if (defined RT_USING_FINSH) && (defined FPRINT_DEBUG)
					rt_kprintf("the finger print frame cmd 0x%04X is invalid!\n", frame_head->cmd);
#endif // RT_USING_FINSH
					return error;
				}
		}
	}

	cs = check_sum((uint8_t *)frame_head, sizeof(*frame_head)) + check_sum((uint8_t *)frame_data, data_length);
	error = FPRINT_EOK;
	rt_device_write(fprint_device, 0, frame_head, sizeof(*frame_head));
	rt_device_write(fprint_device, 0, frame_data, data_length);
	rt_device_write(fprint_device, 0, &cs, 2);

#if (defined RT_USING_FINSH) && (defined FPRINT_DEBUG)
	rt_kprintf("\nfprint send :---------------------\n");
	print_hex((uint8_t *)frame_head, sizeof(*frame_head));
	print_hex((uint8_t *)frame_data, data_length);
	print_hex((uint8_t *)&cs, sizeof(cs));
	rt_kprintf("------------------------------------\n");
#endif // RT_USING_FINSH

	return error;
}

__STATIC_INLINE FPRINT_ERROR_TYPEDEF
fprint_frame_recv(FPRINT_FRAME_HEAD_TYPEDEF *frame_head,
				  FPRINT_FRAME_DATA_TYPEDEF *frame_data)
{
	uint16_t cs;
	rt_device_t fprint_device;
	uint16_t data_length;
	rt_size_t recv_cnts;
	FPRINT_ERROR_TYPEDEF error;
	FPRINT_FRAME_HEAD_TYPEDEF head;
	uint8_t cnts;

	error = FPRINT_EERROR;

	head = *frame_head;

	fprint_device = device_enable(DEVICE_NAME_FPRINT);
	if (fprint_device == RT_NULL)
		goto error_process;
	for (cnts = 0; cnts < 20; cnts++) {
		rt_thread_delay(20);
		recv_cnts = rt_device_read(fprint_device, 0, frame_head, sizeof(*frame_head));
		if (recv_cnts != sizeof(*frame_head)) {
			if (recv_cnts == 0) {
				error = FPRINT_ERESPONSE;
				continue;
			} else {
				error = FPRINT_EERROR;
				goto error_process;
			}
		} else {
			error = FPRINT_EERROR;
			break;
		}
	}

	if (error == FPRINT_ERESPONSE)
		goto error_process;

	if (head.prefix == FPRINT_FRAME_PREFIX_REQUEST)
	{
		if (frame_head->prefix != FPRINT_FRAME_PREFIX_RESPONSE)
			goto error_process;
		switch (head.cmd)
		{
			case FPRINT_FRAME_CMD_TEST_CONNECTION:
				{
					data_length = sizeof(frame_data->rep_test_connection);
					break;
				}
			case FPRINT_FRAME_CMD_DEL_CHAR:
				{
					data_length = sizeof(frame_data->rep_del_char);
					break;
				}
			case FPRINT_FRAME_CMD_GET_IMAGE:
				{
					data_length = sizeof(frame_data->rep_get_image);
					break;
				}
			case FPRINT_FRAME_CMD_GENERATE:
				{
					data_length = sizeof(frame_data->rep_generate);
					break;
				}
			case FPRINT_FRAME_CMD_MERGE:
				{
					data_length = sizeof(frame_data->rep_merge);
					break;
				}
			case FPRINT_FRAME_CMD_STORE_CHAR:
				{
					data_length = sizeof(frame_data->rep_store_char);
					break;
				}
			case FPRINT_FRAME_CMD_UP_CHAR:
				{
					data_length = sizeof(frame_data->rep_up_char);
					break;
				}
			case FPRINT_FRAME_CMD_DOWN_CHAR:
				{
					data_length = sizeof(frame_data->rep_down_char);
					break;
				}
			case FPRINT_FRAME_CMD_SEARCH:
				{
					data_length = sizeof(frame_data->rep_search);
					break;
				}
			case FPRINT_FRAME_CMD_SET_PARAM:
				{
					data_length = sizeof(frame_data->rep_set_param);
					break;
				}
			default :
				{
					data_length = 0;
#if (defined RT_USING_FINSH) && (defined FPRINT_DEBUG)
					rt_kprintf("the finger print frame request cmd 0x%04X is invalid!\n", head.cmd);
#endif // RT_USING_FINSH
					goto error_process;
				}
		}
	}
	else if (head.prefix == FPRINT_FRAME_PREFIX_DATA_REQUEST)
	{
		if (frame_head->prefix != FPRINT_FRAME_PREFIX_DATA_RESPONSE)
			goto error_process;
		switch (head.cmd)
		{
			case FPRINT_FRAME_CMD_DOWN_CHAR:
				{
					data_length = sizeof(frame_data->drep_down_char);
					break;
				}
			default :
				{
					data_length = 0;
#if (defined RT_USING_FINSH) && (defined FPRINT_DEBUG)
					rt_kprintf("the finger print frame data request cmd 0x%04X is invalid!\n", head.cmd);
#endif // RT_USING_FINSH
					goto error_process;
				}
		}
	}
	else if (head.prefix == FPRINT_FRAME_PREFIX_RESPONSE)
	{
		if (frame_head->prefix != FPRINT_FRAME_PREFIX_DATA_RESPONSE)
			goto error_process;
		switch (head.cmd)
		{
			case FPRINT_FRAME_CMD_UP_CHAR:
				{
					data_length = sizeof(frame_data->drep_up_char);
					break;
				}
			default :
				{
					data_length = 0;
#if (defined RT_USING_FINSH) && (defined FPRINT_DEBUG)
					rt_kprintf("the finger print frame data response cmd 0x%04X is invalid!\n", head.cmd);
#endif // RT_USING_FINSH
					goto error_process;
				}
		}
	}
	else
	{
		goto error_process;
	}

	if (head.cmd != frame_head->cmd)
		goto error_process;

	recv_cnts = rt_device_read(fprint_device, 0, frame_data, data_length);
	if (recv_cnts < data_length)
		goto error_process;
	recv_cnts = rt_device_read(fprint_device, 0, &cs, sizeof(cs));
	if (recv_cnts < sizeof(cs))
		goto error_process;
	if (cs == (uint16_t)(check_sum((uint8_t *)frame_head, sizeof(head)) + check_sum((uint8_t *)frame_data, data_length)))
	{
		error = FPRINT_EOK;
	}
#if (defined RT_USING_FINSH) && (defined FPRINT_DEBUG)
	rt_kprintf("\nfprint recv :---------------------\n");
	print_hex((uint8_t *)frame_head, sizeof(*frame_head));
	print_hex((uint8_t *)frame_data, data_length);
	print_hex((uint8_t *)&cs, sizeof(cs));
	rt_kprintf("------------------------------------\n");
#endif // RT_USING_FINSH

error_process:

	return error;
}

__STATIC_INLINE   FPRINT_ERROR_TYPEDEF
fprint_frame_process(uint16_t cmd, uint16_t prefix, uint8_t sid,
					 uint8_t did, FPRINT_FRAME_DATA_TYPEDEF *frame_data)
{
	FPRINT_ERROR_TYPEDEF error;
	FPRINT_FRAME_HEAD_TYPEDEF *head;

	error = FPRINT_EERROR;
	head = rt_malloc(sizeof(*head));
	if (head == RT_NULL)
		return error;

	rt_memset(head, 0, sizeof(*head));

	head->prefix = prefix;
    head->cmd = cmd;
	head->sid = sid;
	head->did = did;

	if (prefix == FPRINT_FRAME_PREFIX_REQUEST)
	{
		switch (cmd)
		{
			case FPRINT_FRAME_CMD_TEST_CONNECTION:
				{
					head->len = sizeof(frame_data->req_test_connection) - sizeof(frame_data->req_test_connection.reserve);
					break;
				}
			case FPRINT_FRAME_CMD_DEL_CHAR:
				{
					head->len = sizeof(frame_data->req_del_char) - sizeof(frame_data->req_del_char.reserve);
					break;
				}
			case FPRINT_FRAME_CMD_GET_IMAGE:
				{
					head->len = sizeof(frame_data->req_get_image) - sizeof(frame_data->req_get_image.reserve);
					break;
				}
			case FPRINT_FRAME_CMD_GENERATE:
				{
					head->len = sizeof(frame_data->req_generate) - sizeof(frame_data->req_generate.reserve);
					break;
				}
			case FPRINT_FRAME_CMD_MERGE:
				{
					head->len = sizeof(frame_data->req_merge) - sizeof(frame_data->req_merge.reserve);
					break;
				}
			case FPRINT_FRAME_CMD_STORE_CHAR:
				{
					head->len = sizeof(frame_data->req_store_char) - sizeof(frame_data->req_store_char.reserve);
					break;
				}
			case FPRINT_FRAME_CMD_UP_CHAR:
				{
					head->len = sizeof(frame_data->req_up_char) - sizeof(frame_data->req_up_char.reserve);
					break;
				}
			case FPRINT_FRAME_CMD_DOWN_CHAR:
				{
					head->len = sizeof(frame_data->req_down_char) - sizeof(frame_data->req_down_char.reserve);
					break;
				}
			case FPRINT_FRAME_CMD_SEARCH:
				{
					head->len = sizeof(frame_data->req_search) - sizeof(frame_data->req_search.reserve);
					break;
				}
			case FPRINT_FRAME_CMD_SET_PARAM:
				{
					head->len = sizeof(frame_data->req_set_param.type) + sizeof(frame_data->req_set_param.data);
					break;
				}
			default :
				{
					goto process_error;
				}
		}
	}
	else if(prefix == FPRINT_FRAME_PREFIX_DATA_REQUEST)
	{
		switch (cmd)
		{
			case FPRINT_FRAME_CMD_DOWN_CHAR:
				{
					head->len = sizeof(frame_data->dreq_down_char);
					break;
				}
			default :
				{
					goto process_error;
				}
		}
	}
	else
	{
		goto process_error;
	}

	error = fprint_frame_send(head, frame_data);
	if (error != FPRINT_EOK)
		goto process_error;
    //rt_thread_delay(50);
	error = fprint_frame_recv(head, frame_data);
	if (error != FPRINT_EOK)
		goto process_error;

    error = FPRINT_EERROR;
	if (head->prefix == FPRINT_FRAME_PREFIX_RESPONSE)
	{
		switch (head->cmd)
		{
			case FPRINT_FRAME_CMD_TEST_CONNECTION:
				{
					if (frame_data->rep_test_connection.result == FPRINT_FRAME_ERR_SUCCESS)
						error  = FPRINT_EOK;
					break;
				}
			case FPRINT_FRAME_CMD_DEL_CHAR:
				{
					if (frame_data->rep_del_char.result != FPRINT_FRAME_ERR_FAIL)
						error  = FPRINT_EOK;
					break;
				}
			case FPRINT_FRAME_CMD_GET_IMAGE:
				{
                    if (frame_data->rep_get_image.result == FPRINT_FRAME_ERR_FP_NOT_DETECTED)
                        error = FPRINT_ENO_DETECTED;
					if (frame_data->rep_get_image.result == FPRINT_FRAME_ERR_SUCCESS)
						error = FPRINT_EOK;
					break;
				}
			case FPRINT_FRAME_CMD_GENERATE:
				{
					if (frame_data->rep_generate.result == FPRINT_FRAME_ERR_SUCCESS)
						error = FPRINT_EOK;
                    break;
				}
			case FPRINT_FRAME_CMD_MERGE:
				{
					if (frame_data->rep_merge.result == FPRINT_FRAME_ERR_SUCCESS)
						error = FPRINT_EOK;
					break;
				}
			case FPRINT_FRAME_CMD_STORE_CHAR:
				{
					if (frame_data->rep_store_char.result == FPRINT_FRAME_ERR_SUCCESS)
						error = FPRINT_EOK;
					if (frame_data->rep_store_char.result == FPRINT_FRAME_ERR_DUPLICATION_ID)
						error = FPRINT_EEXIST;
					break;
				}
			case FPRINT_FRAME_CMD_DOWN_CHAR:
				{
					if (frame_data->rep_down_char.result == FPRINT_FRAME_ERR_SUCCESS)
						error = FPRINT_EOK;
					break;
				}
			case FPRINT_FRAME_CMD_SEARCH:
				{
					if (frame_data->rep_search.result == FPRINT_FRAME_ERR_SUCCESS)
						error = FPRINT_EOK;
					break;
				}
			case FPRINT_FRAME_CMD_UP_CHAR:
				{
					if (frame_data->rep_up_char.result == FPRINT_FRAME_ERR_SUCCESS) {
						error = fprint_frame_recv(head, frame_data);
						if (error != FPRINT_EOK)
							goto process_error;
						error = FPRINT_EERROR;
						if (frame_data->drep_up_char.result == FPRINT_FRAME_ERR_SUCCESS) {
							error = FPRINT_EOK;
						}
					}

					break;
				}
			case FPRINT_FRAME_CMD_SET_PARAM:
				{
					if (frame_data->rep_set_param.result == FPRINT_FRAME_ERR_SUCCESS)
						error = FPRINT_EOK;
					break;
				}
			default :
				{
					goto process_error;
				}
		}
	}
	else if(head->prefix == FPRINT_FRAME_PREFIX_DATA_RESPONSE)
	{
		switch (cmd)
		{
			case FPRINT_FRAME_CMD_DOWN_CHAR:
				{
					if (frame_data->drep_down_char.result == FPRINT_FRAME_ERR_SUCCESS)
						error = FPRINT_EOK;
					break;
				}
			default :
				{
					goto process_error;
				}
		}
	}

process_error:
	rt_free(head);

	return error;
}

FPRINT_ERROR_TYPEDEF
fprint_init(FPRINT_FRAME_DATA_TYPEDEF *frame_data)
{
	uint16_t i;
	FPRINT_ERROR_TYPEDEF error = FPRINT_EERROR;
	KEY_TYPEDEF key;
	// test connection
	rt_memset(frame_data, 0, sizeof(*frame_data));
	error = fprint_frame_process(FPRINT_FRAME_CMD_TEST_CONNECTION, FPRINT_FRAME_PREFIX_REQUEST, 0, 0, frame_data);
	if (error != FPRINT_EOK)
		return error;
	// delete all template
	rt_memset(frame_data, 0, sizeof(*frame_data));
	frame_data->req_del_char.start = 1;
	frame_data->req_del_char.end = 2000;
	error = fprint_frame_process(FPRINT_FRAME_CMD_DEL_CHAR, FPRINT_FRAME_PREFIX_REQUEST, 0, 0, frame_data);
	if (error != FPRINT_EOK)
		return error;
	// set dup 0
	rt_memset(frame_data, 0, sizeof(*frame_data));
	frame_data->req_set_param.type = 2;
	frame_data->req_set_param.data = 1;
	error = fprint_frame_process(FPRINT_FRAME_CMD_SET_PARAM, FPRINT_FRAME_PREFIX_REQUEST, 0, 0, frame_data);
	if (error != FPRINT_EOK)
		return error;
	/* TODO:  write fprint template from key file */
	for (i = 0; i < KEY_NUMBERS; i++) {
		key = device_config.param.key[i];
		if (key.flag && key.key_type == KEY_TYPE_FPRINT) {
			rt_memset(frame_data, 0, sizeof(*frame_data));
			frame_data->req_down_char.len = sizeof(frame_data->dreq_down_char);
			error = fprint_frame_process(FPRINT_FRAME_CMD_DOWN_CHAR, FPRINT_FRAME_PREFIX_REQUEST, 0, 0, frame_data);
			if (error != FPRINT_EOK)
				return error;
			rt_memset(frame_data, 0, sizeof(*frame_data));
			frame_data->dreq_down_char.ram_buf_id = 0;
			device_config_key_operate(i, KEY_TYPE_FPRINT, frame_data->dreq_down_char.template, 0);
			error = fprint_frame_process(FPRINT_FRAME_CMD_DOWN_CHAR, FPRINT_FRAME_PREFIX_DATA_REQUEST, 0, 0, frame_data);
			if (error != FPRINT_EOK)
				return error;
			// store fprint template to template_id
			rt_memset(frame_data, 0, sizeof(*frame_data));
			frame_data->req_store_char.template_id = i + FPRINT_TEMPLATE_OFFSET;
			frame_data->req_store_char.ram_buf_id = 0;
			error = fprint_frame_process(FPRINT_FRAME_CMD_STORE_CHAR,
										 FPRINT_FRAME_PREFIX_REQUEST,
										 0, 0, frame_data);
			if (error != FPRINT_EOK)
				return error;

		}
	}

	return error;
}

FPRINT_ERROR_TYPEDEF
fprint_enroll(uint16_t template_id, FPRINT_FRAME_DATA_TYPEDEF *frame_data)
{
	FPRINT_ERROR_TYPEDEF error = FPRINT_EERROR;

	// get fprint image
	rt_memset(frame_data, 0, sizeof(*frame_data));
	error = fprint_frame_process(FPRINT_FRAME_CMD_GET_IMAGE,
								 FPRINT_FRAME_PREFIX_REQUEST,
								 0, 0, frame_data);
	if (error != FPRINT_EOK)
		return error;

	// generate fprint template to rambuf0
	rt_memset(frame_data, 0, sizeof(*frame_data));
	frame_data->req_generate.ram_buf_id = 0;
	error = fprint_frame_process(FPRINT_FRAME_CMD_GENERATE,
								 FPRINT_FRAME_PREFIX_REQUEST,
								 0, 0, frame_data);
	if (error != FPRINT_EOK)
		return error;
	// get fprint image
	rt_memset(frame_data, 0, sizeof(*frame_data));
	error = fprint_frame_process(FPRINT_FRAME_CMD_GET_IMAGE,
								 FPRINT_FRAME_PREFIX_REQUEST,
								 0, 0, frame_data);
	if (error != FPRINT_EOK)
		return error;

	// generate fprint template to rambuf1
	rt_memset(frame_data, 0, sizeof(*frame_data));
	frame_data->req_generate.ram_buf_id = 1;
	error = fprint_frame_process(FPRINT_FRAME_CMD_GENERATE,
								 FPRINT_FRAME_PREFIX_REQUEST,
								 0, 0, frame_data);
	if (error != FPRINT_EOK)
		return error;

	// merge rambuf0 rambuf1 to rambuf0
	rt_memset(frame_data, 0, sizeof(*frame_data));
	frame_data->req_merge.ram_buf_id = 1;
	frame_data->req_merge.ram_bufs = 2;
	error = fprint_frame_process(FPRINT_FRAME_CMD_MERGE,
								 FPRINT_FRAME_PREFIX_REQUEST,
								 0, 0, frame_data);
	if (error != FPRINT_EOK)
		return error;

	// store fprint template to template_id
	rt_memset(frame_data, 0, sizeof(*frame_data));
	frame_data->req_store_char.template_id = template_id;
	frame_data->req_store_char.ram_buf_id = 0;
	error = fprint_frame_process(FPRINT_FRAME_CMD_STORE_CHAR,
								 FPRINT_FRAME_PREFIX_REQUEST,
								 0, 0, frame_data);
	if (error != FPRINT_EOK)
		return error;

	// get fprint template from rambuf0
	rt_memset(frame_data, 0, sizeof(*frame_data));
	frame_data->req_up_char.ram_buf_id = 0;
	error = fprint_frame_process(FPRINT_FRAME_CMD_UP_CHAR,
								 FPRINT_FRAME_PREFIX_REQUEST,
								 0, 0, frame_data);
	return error;
}

FPRINT_ERROR_TYPEDEF
fprint_verify(FPRINT_FRAME_DATA_TYPEDEF *frame_data)
{
	FPRINT_ERROR_TYPEDEF error = FPRINT_EERROR;

	// get fprint image
	rt_memset(frame_data, 0, sizeof(*frame_data));
	error = fprint_frame_process(FPRINT_FRAME_CMD_GET_IMAGE,
								 FPRINT_FRAME_PREFIX_REQUEST,
								 0, 0, frame_data);
	if (error != FPRINT_EOK)
    {
        if (error != FPRINT_ENO_DETECTED) {
            rt_kprintf("get image : %d\n", error);
            error = FPRINT_EEXCEPTION;
        }
		return error;
    }
	// generate fprint template to rambuf0
	rt_memset(frame_data, 0, sizeof(*frame_data));
	frame_data->req_generate.ram_buf_id = 0;
	error = fprint_frame_process(FPRINT_FRAME_CMD_GENERATE,
								 FPRINT_FRAME_PREFIX_REQUEST,
								 0, 0, frame_data);
	if (error != FPRINT_EOK) {
        rt_kprintf("generate : %d\n", error);
        error = FPRINT_EEXCEPTION;
        return error;
    }

	// search fprint template to rambuf0
	rt_memset(frame_data, 0, sizeof(*frame_data));
	frame_data->req_search.ram_buf_id = 0;
	frame_data->req_search.template_id_start = FPRINT_TEMPLATE_ID_START;
	frame_data->req_search.template_id_end = FPRINT_TEMPLATE_ID_END;
	error = fprint_frame_process(FPRINT_FRAME_CMD_SEARCH,
								 FPRINT_FRAME_PREFIX_REQUEST,
								 0, 0, frame_data);

	return error;
}

void
fprint_thread_entry(void *parameters)
{
	rt_err_t result;
	//rt_device_t fprint_device;
	FPRINT_MAIL_TYPEDEF fprint_mail;
	FPRINT_FRAME_DATA_TYPEDEF frame_data;
	FPRINT_ERROR_TYPEDEF error;

	//fprint_device = device_enable(DEVICE_NAME_FPRINT)

	while (1)
	{
		result = rt_mq_recv(fprint_mq, &fprint_mail, sizeof(fprint_mail), 20);
		if (result == RT_EOK)
		{
			switch(fprint_mail.cmd)
			{
				case FPRINT_CMD_INIT:
					{
						fprint_reset();
						error = fprint_init(&frame_data);
						break;
					}
				case FPRINT_CMD_ENROLL:
					{
						error = fprint_enroll(fprint_mail.key_id + FPRINT_TEMPLATE_OFFSET, &frame_data);
						if (error == FPRINT_EOK) {

							if (device_config_key_operate(fprint_mail.key_id, KEY_TYPE_FPRINT, frame_data.drep_up_char.template, 1) < 0) {
#if (defined RT_USING_FINSH)
								rt_kprintf("the finger print key save failure!\n");
#endif // RT_USING_FINSH
								error = FPRINT_EERROR;
							}
						}
						if (error == FPRINT_EEXIST) {
#if (defined RT_USING_FINSH)
							rt_kprintf("the finger print has been existed, could not enroll!\n");
#endif // RT_USING_FINSH
						}
						break;
					}
				case FPRINT_CMD_DELETE:
					{
						rt_memset(&frame_data, 0, sizeof(frame_data));
						frame_data.req_del_char.start = fprint_mail.key_id + FPRINT_TEMPLATE_OFFSET;
						frame_data.req_del_char.end = fprint_mail.key_id + FPRINT_TEMPLATE_OFFSET;
						error = fprint_frame_process(FPRINT_FRAME_CMD_DEL_CHAR, FPRINT_FRAME_PREFIX_REQUEST, 0, 0, &frame_data);
						break;
					}
				default :
					{
#if (defined RT_USING_FINSH)
						rt_kprintf("the finger print cmd is invalid!\n");
#endif // RT_USING_FINSH
						break;
					}
			}
            if (fprint_mail.result_sem != RT_NULL) {
                *fprint_mail.result = error;
                rt_sem_release(fprint_mail.result_sem);
            }
		}
		else // time out
		{
            static uint16_t s_detect = 0;
            static uint16_t f_detect = 0;
            static uint16_t r_detect = 0;
            static uint16_t template_id = 0;
            error = fprint_verify(&frame_data);
			if (error == FPRINT_EEXCEPTION) {
#if (defined RT_USING_FINSH)
				rt_kprintf("fprint verify is exception\n");
#endif // RT_USING_FINSH
			}
			if (error == FPRINT_EOK) {
                if (s_detect++) {
                    if (template_id != frame_data.rep_search.template_id) {
                        template_id = frame_data.rep_search.template_id;
                        s_detect = 0;
                    }
                } else {
                    template_id = frame_data.rep_search.template_id;
#if (defined RT_USING_FINSH)
                    rt_kprintf("fprint verify is exist, %d\n", frame_data.rep_search.template_id);
#endif // RT_USING_FINSH
					if(fprintf_ok_fun != RT_NULL)
					{
						FPINTF_USER key;

						key.KeyPos = frame_data.rep_search.template_id - FPRINT_TEMPLATE_OFFSET;
						rt_kprintf("key pos : %d\n", key.KeyPos);
						fprintf_ok_fun((void *)&key);
					}
                }

			} else {

                s_detect = 0;

            }
            if (error == FPRINT_ENO_DETECTED) {
#if (defined RT_USING_FINSH) && (defined FPRINT_DEBUG)
                rt_kprintf("fprint verify is no detected\n");
#endif // RT_USING_FINSH
				if(fprintf_null_fun != RT_NULL)
		        {
					fprintf_null_fun(RT_NULL);
		        }
            }
            if (error == FPRINT_EERROR) {

                if (f_detect++) {

                } else {


#if (defined RT_USING_FINSH)
                    rt_kprintf("fprint verify is error\n");
#endif // RT_USING_FINSH
					if(fprintf_error_fun != RT_NULL)
					{
						FPINTF_USER key;

						key.KeyPos = 0xffff;
						fprintf_error_fun((void *)&key);
					}
                }
            } else {
                f_detect = 0;
            }
            if (error == FPRINT_ERESPONSE) {

                if (r_detect++ > 1000) {

#if (defined RT_USING_FINSH)
                    rt_kprintf("fprint has no response , may be fault!\n");
#endif // RT_USING_FINSH
                    fprint_init(&frame_data);
					r_detect = 0;
                }
            } else {
                r_detect = 0;
            }
		}
	}
}

FPRINT_ERROR_TYPEDEF
send_fp_mail(FPRINT_CMD_TYPEDEF cmd, uint16_t key_id, uint8_t flag)
{
	FPRINT_MAIL_TYPEDEF mail;
	rt_err_t result;
	FPRINT_ERROR_TYPEDEF error;

	if (fprint_mq != RT_NULL)
	{
        if (flag) {
            mail.result_sem = rt_sem_create("s_fp", 0, RT_IPC_FLAG_FIFO);
            RT_ASSERT(mail.result_sem != RT_NULL);
        } else {
            mail.result_sem = RT_NULL;
        }
        mail.result = &error;
		mail.cmd = cmd;
		mail.key_id = key_id;
		result = rt_mq_send(fprint_mq, &mail, sizeof(mail));
		if (result == -RT_EFULL)
		{
#if (defined RT_USING_FINSH)
			rt_kprintf("fprint_mq is full!!!\n");
#endif // RT_USING_FINSH
		}
		else
		{
            if (flag)
                rt_sem_take(mail.result_sem, RT_WAITING_FOREVER);
#if (defined RT_USING_FINSH)
			rt_kprintf("send result is %d\n", *mail.result);
#endif // RT_USING_FINSH
		}
        if (flag)
            rt_sem_delete(mail.result_sem);
	}
	else
	{
#if (defined RT_USING_FINSH)
		rt_kprintf("fprint_mq is null!!!\n");
#endif // RT_USING_FINSH
	}
	return error;
}

int
rt_fprint_init(void)
{
	rt_thread_t fprint_thread;

    //initial fprint msg queue
	fprint_mq = rt_mq_create("fprint", sizeof(FPRINT_MAIL_TYPEDEF),
							 FPRINT_MAIL_MAX_MSGS, RT_IPC_FLAG_FIFO);
    if (fprint_mq == RT_NULL)
        return -1;

    // finger print thread
	fprint_thread = rt_thread_create("fprint", fprint_thread_entry,
									 RT_NULL, 1536, 100, 5);
	if (fprint_thread == RT_NULL)
        return -1;

    rt_thread_startup(fprint_thread);
    return 0;
}

INIT_APP_EXPORT(rt_fprint_init);

#ifdef RT_USING_FINSH
#include <finsh.h>

FINSH_FUNCTION_EXPORT(send_fp_mail, send_fprint_mail[cmd]);
#endif // RT_USING_FINSH
