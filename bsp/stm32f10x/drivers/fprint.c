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

#include <stm32f10x.h>
#include "untils.h"
#include "config.h"
#include "board.h"
#include "fprint.h"
#include "gpio_pin.h"
#include "gpio_pwm.h"
#include "local.h"

#define FPRINT_MAIL_MAX_MSGS 10

#define FPRINT_DEBUG 1
#define DEVICE_NAME_FPRINT "uart2"
#define FPRINT_TEMPLATE_OFFSET 0 // 0 <= offset <= 999
#define FPRINT_TEMPLATE_SIZE (1000 - 1) // 1 <= offset <= 2000
#define FPRINT_TEMPLATE_ID_START FPRINT_TEMPLATE_OFFSET
#define FPRINT_TEMPLATE_ID_END (FPRINT_TEMPLATE_OFFSET + KEY_NUMBERS - 1)
#define FPRINT_PARAM_DATA_SIZE 3

static const u16 fprint_param_data_size_map[] = {
    32, 64, 128, 256,
};

//head define
#define FPRINT_FRAME_START 0xef01
//#define FPRINT_FRAME_ADDR 0x06030024
#define FPRINT_FRAME_ADDR 0xffffffff
#define FPRINT_FRAME_PW 0x00000000

#define FPRINT_FRAME_PID_CMD 0x01
#define FPRINT_FRAME_PID_DATA 0x02
#define FPRINT_FRAME_PID_ACK 0x07
#define FPRINT_FRAME_PID_END 0x08
//cmd define
#define FPRINT_FRAME_CMD_VERIFY 0x13
#define FPRINT_FRAME_CMD_EMPTY 0x0d
#define FPRINT_FRAME_CMD_GET_ECHO 0x53
#define FPRINT_FRAME_CMD_SET_PARAM 0x0e
#define FPRINT_FRAME_CMD_SET_ADDR 0x15
#define FPRINT_FRAME_CMD_STORE_CHAR 0x06
#define FPRINT_FRAME_CMD_LOAD_CHAR 0x07
#define FPRINT_FRAME_CMD_UP_CHAR 0x08
#define FPRINT_FRAME_CMD_DOWN_CHAR 0x09
#define	FPRINT_FRAME_CMD_DEL_CHAR 0x0c
#define FPRINT_FRAME_CMD_GET_STATUS 0x46
#define FPRINT_FRAME_CMD_GET_IMAGE 0x01
#define FPRINT_FRAME_CMD_GENERATE 0x02
#define FPRINT_FRAME_CMD_MERGE 0x05
#define FPRINT_FRAME_CMD_SEARCH 0x04
//result define
#define FPRINT_FRAME_ERR_SUCCESS 0x00
#define FPRINT_FRAME_ERR_FAIL 0x01
#define FPRINT_FRAME_ERR_FP_NOT_DETECTED 0x02

typedef enum {

	FPRINT_EOK,
	FPRINT_EERROR,
	FPRINT_ERESPONSE,
	FPRINT_EEXIST,
    FPRINT_ENO_DETECTED,
	FPRINT_EINVALID,
    FPRINT_EEXCEPTION,

}FPRINT_ERROR_TYPEDEF;

typedef enum {

	FPRINT_CMD_INIT,
	FPRINT_CMD_ENROLL,
	FPRINT_CMD_DELETE,
    FPRINT_CMD_VERIFY,
	FPRINT_CMD_RESET,
    FPRINT_CMD_UP_CHAR,
    FPRINT_CMD_STORE_TEMPLATE,
    FPRINT_CMD_GET_TEMPLATE,

}FPRINT_CMD_TYPEDEF;

typedef struct {

	FPRINT_CMD_TYPEDEF cmd;
	uint16_t *key_id;
    uint8_t *buf;
    uint16_t size;
	rt_sem_t result_sem;
	FPRINT_ERROR_TYPEDEF *result;

}FPRINT_MAIL_TYPEDEF;

typedef struct{

    uint8_t start[2];
	uint8_t addr[4];
	uint8_t pid;
	uint8_t len[2];

}FPRINT_FRAME_HEAD_TYPEDEF;

typedef struct {
    uint8_t cmd;
    uint8_t pw[4];
}FPRINT_FRAME_REQ_VERIFY_TYPEDEF;

typedef struct {
    uint8_t cmd;
}FPRINT_FRAME_REQ_EMPTY_TYPEDEF;

typedef struct {
    uint8_t cmd;
    uint8_t type;
    uint8_t data;
}FPRINT_FRAME_REQ_SET_PARAM_TYPEDEF;
typedef struct {
    uint8_t cmd;
    uint8_t addr[4];
}FPRINT_FRAME_REQ_SET_ADDR_TYPEDEF;
typedef struct {
    uint8_t cmd;
}FPRINT_FRAME_REQ_GET_IMAGE_TYPEDEF;
typedef struct {
    uint8_t cmd;
    uint8_t buf_id;
}FPRINT_FRAME_REQ_GENERATE_TYPEDEF;
typedef struct {
    uint8_t cmd;
}FPRINT_FRAME_REQ_MERGE_TYPEDEF;

typedef struct {

    uint8_t cmd;
    uint8_t buf_id;

}FPRINT_FRAME_REQ_UP_CHAR_TYPEDEF;
typedef struct {

    uint8_t cmd;
    uint8_t buf_id;

}FPRINT_FRAME_REQ_DOWN_CHAR_TYPEDEF;
typedef struct {

    uint8_t cmd;
    uint8_t buf_id;
    uint8_t template_id[2];

}FPRINT_FRAME_REQ_LOAD_CHAR_TYPEDEF;

typedef struct {

    uint8_t cmd;
    uint8_t buf_id;
    uint8_t template_id[2];

}FPRINT_FRAME_REQ_STORE_CHAR_TYPEDEF;

typedef struct {

    uint8_t cmd;
    uint8_t template_id[2];
    uint8_t size[2];

}FPRINT_FRAME_REQ_DEL_CHAR_TYPEDEF;

typedef struct {

    uint8_t cmd;
    uint8_t buf_id;
    uint8_t template_id[2];
    uint8_t size[2];

}FPRINT_FRAME_REQ_SEARCH_TYPEDEF;

typedef union {

    FPRINT_FRAME_REQ_VERIFY_TYPEDEF req_verify;
    FPRINT_FRAME_REQ_EMPTY_TYPEDEF req_empty;
    FPRINT_FRAME_REQ_SET_PARAM_TYPEDEF req_set_param;
    FPRINT_FRAME_REQ_SET_ADDR_TYPEDEF req_set_addr;
    FPRINT_FRAME_REQ_GET_IMAGE_TYPEDEF req_get_image;
    FPRINT_FRAME_REQ_GENERATE_TYPEDEF req_generate;
    FPRINT_FRAME_REQ_MERGE_TYPEDEF req_merge;
    FPRINT_FRAME_REQ_UP_CHAR_TYPEDEF req_up_char;
    FPRINT_FRAME_REQ_DOWN_CHAR_TYPEDEF req_down_char;
    FPRINT_FRAME_REQ_LOAD_CHAR_TYPEDEF req_load_char;
    FPRINT_FRAME_REQ_STORE_CHAR_TYPEDEF req_store_char;
    FPRINT_FRAME_REQ_DEL_CHAR_TYPEDEF req_del_char;
    FPRINT_FRAME_REQ_SEARCH_TYPEDEF req_search;

}FPRINT_FRAME_REQ_DATA_TYPEDEF;

typedef struct {

    uint8_t result;
    uint8_t cs[2];

}FPRINT_FRAME_REP_VERIFY_TYPEDEF;

typedef struct {

    uint8_t result;
    uint8_t cs[2];

}FPRINT_FRAME_REP_EMPTY_TYPEDEF;

typedef struct {

    uint8_t result;
    uint8_t cs[2];

}FPRINT_FRAME_REP_SET_PARAM_TYPEDEF;

typedef struct {

    uint8_t result;
    uint8_t cs[2];

}FPRINT_FRAME_REP_SET_ADDR_TYPEDEF;
typedef struct {

    uint8_t result;
    uint8_t cs[2];

}FPRINT_FRAME_REP_GET_IMAGE_TYPEDEF;
typedef struct {

    uint8_t result;
    uint8_t cs[2];

}FPRINT_FRAME_REP_GENERATE_TYPEDEF;
typedef struct {

    uint8_t result;
    uint8_t cs[2];

}FPRINT_FRAME_REP_MERGE_TYPEDEF;
typedef struct {

    uint8_t result;
    uint8_t cs[2];

}FPRINT_FRAME_REP_UP_CHAR_TYPEDEF;

typedef struct {

    uint8_t result;
    uint8_t cs[2];

}FPRINT_FRAME_REP_DOWN_CHAR_TYPEDEF;


typedef struct {

    uint8_t result;
    uint8_t cs[2];

}FPRINT_FRAME_REP_LOAD_CHAR_TYPEDEF;

typedef struct {

    uint8_t result;
    uint8_t cs[2];

}FPRINT_FRAME_REP_STORE_CHAR_TYPEDEF;

typedef struct {

    uint8_t result;
    uint8_t cs[2];

}FPRINT_FRAME_REP_DEL_CHAR_TYPEDEF;

typedef struct {

    uint8_t result;
    uint8_t template_id[2];
    uint8_t score[2];
    uint8_t cs[2];

}FPRINT_FRAME_REP_SEARCH_TYPEDEF;

typedef union {

    FPRINT_FRAME_REP_VERIFY_TYPEDEF rep_verify;
    FPRINT_FRAME_REP_EMPTY_TYPEDEF rep_empty;
    FPRINT_FRAME_REP_SET_PARAM_TYPEDEF rep_set_param;
    FPRINT_FRAME_REP_SET_ADDR_TYPEDEF rep_set_addr;
    FPRINT_FRAME_REP_GET_IMAGE_TYPEDEF rep_get_image;
    FPRINT_FRAME_REP_GENERATE_TYPEDEF rep_generate;
    FPRINT_FRAME_REP_MERGE_TYPEDEF rep_merge;
    FPRINT_FRAME_REP_UP_CHAR_TYPEDEF rep_up_char;
    FPRINT_FRAME_REP_DOWN_CHAR_TYPEDEF rep_down_char;
    FPRINT_FRAME_REP_LOAD_CHAR_TYPEDEF rep_load_char;
    FPRINT_FRAME_REP_STORE_CHAR_TYPEDEF rep_store_char;
    FPRINT_FRAME_REP_DEL_CHAR_TYPEDEF rep_del_char;
    FPRINT_FRAME_REP_SEARCH_TYPEDEF rep_search;

}FPRINT_FRAME_REP_DATA_TYPEDEF;


static rt_mq_t fprint_mq;
static rt_sem_t s_fprint;

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

__STATIC_INLINE void
reverse(uint8_t *dst, uint8_t *src, uint8_t len)
{
    if (len)
        src += len - 1;
    else
        return;

    while (len-- > 0)
    {
        *dst++ = *src--;
    }
}

static void
fprint_reset(void)
{
}


__STATIC_INLINE FPRINT_ERROR_TYPEDEF
fprint_frame_send(uint8_t cmd, FPRINT_FRAME_HEAD_TYPEDEF *frame_head,
                    FPRINT_FRAME_REQ_DATA_TYPEDEF *req_data)
{
	uint16_t cs;
	rt_device_t fprint_device;
	uint16_t data_length, data_offset;
	FPRINT_ERROR_TYPEDEF error = FPRINT_EERROR;

	fprint_device = device_enable(DEVICE_NAME_FPRINT);
	if (fprint_device == RT_NULL)
		return error;

	rt_device_control(fprint_device, RT_DEVICE_CTRL_CLR_RB, RT_NULL);

    data_offset = 0;
    data_length = 0;


    switch (cmd)
    {
        case FPRINT_FRAME_CMD_VERIFY:
            {
                req_data->req_verify.cmd = cmd;
                data_offset = 0;
                data_length = sizeof(req_data->req_verify) + 2;
                break;
            }
        case FPRINT_FRAME_CMD_EMPTY:
            {
                req_data->req_empty.cmd = cmd;
                data_offset = 0;
                data_length = sizeof(req_data->req_empty) + 2;
                break;
            }
        case FPRINT_FRAME_CMD_SET_PARAM:
            {
                req_data->req_set_param.cmd = cmd;
                data_offset = 0;
                data_length = sizeof(req_data->req_set_param) + 2;
                break;
            }
        case FPRINT_FRAME_CMD_SET_ADDR:
            {
                req_data->req_set_addr.cmd = cmd;
                data_offset = 0;
                data_length = sizeof(req_data->req_set_addr) + 2;
                break;
            }
        case FPRINT_FRAME_CMD_GET_IMAGE:
            {
                req_data->req_get_image.cmd = cmd;
                data_offset = 0;
                data_length = sizeof(req_data->req_get_image) + 2;
                break;
            }
        case FPRINT_FRAME_CMD_GENERATE:
            {
                req_data->req_generate.cmd = cmd;
                data_offset = 0;
                data_length = sizeof(req_data->req_generate) + 2;
                break;
            }
        case FPRINT_FRAME_CMD_MERGE:
            {
                req_data->req_merge.cmd = cmd;
                data_offset = 0;
                data_length = sizeof(req_data->req_merge) + 2;
                break;
            }
        case FPRINT_FRAME_CMD_UP_CHAR:
            {
                req_data->req_up_char.cmd = cmd;
                data_offset = 0;
                data_length = sizeof(req_data->req_up_char) + 2;
                break;
            }
        case FPRINT_FRAME_CMD_DOWN_CHAR:
            {
                req_data->req_up_char.cmd = cmd;
                data_offset = 0;
                data_length = sizeof(req_data->req_down_char) + 2;
                break;
            }
        case FPRINT_FRAME_CMD_LOAD_CHAR:
            {
                req_data->req_load_char.cmd = cmd;
                data_offset = 0;
                data_length = sizeof(req_data->req_load_char) + 2;
                break;
            }
        case FPRINT_FRAME_CMD_STORE_CHAR:
            {
                req_data->req_store_char.cmd = cmd;
                data_offset = 0;
                data_length = sizeof(req_data->req_store_char) + 2;
                break;
            }
        case FPRINT_FRAME_CMD_DEL_CHAR:
            {
                req_data->req_del_char.cmd = cmd;
                data_offset = 0;
                data_length = sizeof(req_data->req_del_char) + 2;
                break;
            }
        case FPRINT_FRAME_CMD_SEARCH:
            {
                req_data->req_search.cmd = cmd;
                data_offset = 0;
                data_length = sizeof(req_data->req_search) + 2;
                break;
            }
        default :
            {
                goto error_process;
            }
    }

    reverse(frame_head->len, (uint8_t *)&data_length, 2);
    data_length -= 2;
	cs = check_sum((uint8_t *)frame_head+6, sizeof(*frame_head)-6) + check_sum((uint8_t *)req_data + data_offset, data_length);
	error = FPRINT_EOK;
	rt_device_write(fprint_device, 0, frame_head, sizeof(*frame_head));
	rt_device_write(fprint_device, 0, req_data + data_offset, data_length);
    cs = __REV16(cs);
	rt_device_write(fprint_device, 0, &cs, 2);

#if (defined RT_USING_FINSH) && (defined FPRINT_DEBUG)
	rt_kprintf("\nfprint send :---------------------\n");
	print_hex((uint8_t *)frame_head, sizeof(*frame_head));
	print_hex((uint8_t *)req_data + data_offset, data_length);
	print_hex((uint8_t *)&cs, sizeof(cs));
	rt_kprintf("------------------------------------\n");
#endif // RT_USING_FINSH

error_process:
	return error;
}

#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

__STATIC_INLINE uint16_t
fprint_frame_recv_data(uint8_t *buf, uint16_t size)
{
	uint16_t cs;
	rt_device_t fprint_device;
	uint16_t data_length, data_offset;
    uint16_t check = 0;
	rt_size_t recv_cnts;
	FPRINT_ERROR_TYPEDEF error;
	uint8_t cnts;
    FPRINT_FRAME_HEAD_TYPEDEF head;

    data_length = 0;
    data_offset = 0;

	error = FPRINT_EERROR;


	fprint_device = device_enable(DEVICE_NAME_FPRINT);
	if (fprint_device == RT_NULL)
		goto error_process;
    while (size > 0)
    {
        for (cnts = 0; cnts < 20; cnts++) {
            rt_thread_delay(20);
            recv_cnts = rt_device_read(fprint_device, 0, &head, sizeof(head));
            if (recv_cnts != sizeof(head)) {
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

        reverse((uint8_t *)&data_length, head.len, sizeof(data_length));
/*

        if (data_length > size)
            goto error_process;
*/
        data_length -= 2;

        if (data_length > size)
            goto error_process;

        switch (head.pid)
        {
            case FPRINT_FRAME_PID_DATA:
                {
                    size -= data_length;
                    break;
                }
            case FPRINT_FRAME_PID_END:
                {
                    size = 0;
                    break;
                }

            default :
                {
                    size = 0;
                    break;
                }
        }

        recv_cnts = rt_device_read(fprint_device, 0, (uint8_t *)buf + data_offset, data_length);
        if (recv_cnts < data_length)
            goto error_process;
        cs = 0;
        cs = check_sum((uint8_t *)&head + 6, sizeof(head) - 6);
        cs +=  check_sum((uint8_t *)buf + data_offset, data_length);
        check = 0;
        recv_cnts = rt_device_read(fprint_device, 0, (uint8_t *)&check, 2);
        if (recv_cnts < 2)
            goto error_process;
#if (defined RT_USING_FINSH) && (defined FPRINT_DEBUG)
        rt_kprintf("\nfprint recv :---------------------\n");
        print_hex((uint8_t *)&head, sizeof(head));
        print_hex((uint8_t *)buf+data_offset, data_length);
        print_hex((uint8_t *)&check, sizeof(check));
        rt_kprintf("------------------------------------\n");
#endif // RT_USING_FINSH
        //reverse((uint8_t *)&check, (uint8_t *)&check, sizeof(check));
        __REV16(check);
        if (cs==check)
        {
            error = FPRINT_EOK;
        }

        data_offset += data_length;
    }
error_process:

	return data_offset;

}

__STATIC_INLINE uint16_t
fprint_frame_send_data(void *buffer, uint16_t size)
{
	uint16_t cs;
    u8 *buf;
	rt_device_t fprint_device;
	uint16_t data_length, data_offset, frame_data_size;
    FPRINT_FRAME_HEAD_TYPEDEF head;
    uint16_t start = FPRINT_FRAME_START;
    uint32_t addr = FPRINT_FRAME_ADDR;
    data_length = 0;
    data_offset = 0;
    frame_data_size = fprint_param_data_size_map[FPRINT_PARAM_DATA_SIZE];
    buf = buffer;
	fprint_device = device_enable(DEVICE_NAME_FPRINT);
	if (fprint_device == RT_NULL)
		goto error_process;
    while (size > 0)
    {
        rt_memset(&head, 0, sizeof(head));

        reverse(head.start, (uint8_t *)&start, sizeof(start));
        reverse(head.addr, (uint8_t *)&addr, sizeof(addr));

        if (size > frame_data_size) {
            data_length = frame_data_size;
            head.pid = FPRINT_FRAME_PID_DATA;
        } else {
            data_length = size;
            head.pid = FPRINT_FRAME_PID_END;
        }
        size -= data_length;

        //reverse((uint8_t *)&data_length, head.len, sizeof(data_length));


        data_length += 2;
        reverse(head.len, (uint8_t *)&data_length, sizeof(data_length));
        data_length -= 2;
        cs = check_sum((uint8_t *)&head+6, sizeof(head)-6) + check_sum(buf + data_offset, data_length);
        rt_device_write(fprint_device, 0, &head, sizeof(head));
        rt_device_write(fprint_device, 0, buf + data_offset, data_length);
        cs = __REV16(cs);
        rt_device_write(fprint_device, 0, &cs, 2);

#if (defined RT_USING_FINSH) && (defined FPRINT_DEBUG)
        rt_kprintf("\nfprint send :---------------------\n");
        print_hex((uint8_t *)&head, sizeof(head));
        print_hex((uint8_t *)buf + data_offset, data_length);
        print_hex((uint8_t *)&cs, sizeof(cs));
        rt_kprintf("------------------------------------\n");
#endif
        data_offset += data_length;
    }
error_process:

	return data_offset;

}


__STATIC_INLINE FPRINT_ERROR_TYPEDEF
fprint_frame_recv(uint8_t cmd, FPRINT_FRAME_HEAD_TYPEDEF *frame_head,
				  FPRINT_FRAME_REP_DATA_TYPEDEF *rep_data)
{
	uint16_t cs;
	rt_device_t fprint_device;
	uint16_t data_length;
	rt_size_t recv_cnts;
	FPRINT_ERROR_TYPEDEF error;
	uint8_t cnts;

    data_length = 0;

	error = FPRINT_EERROR;

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

    reverse((uint8_t *)&data_length, frame_head->len, sizeof(data_length));

	recv_cnts = rt_device_read(fprint_device, 0, (uint8_t *)rep_data, data_length);
	if (recv_cnts < data_length)
		goto error_process;
    cs = check_sum((uint8_t *)frame_head + 6, sizeof(*frame_head) - 6);
    cs +=  check_sum((uint8_t *)rep_data, data_length-2);
	if (*((uint8_t *)rep_data + data_length -2) == ((uint8_t *)&cs)[1] && *((uint8_t *)rep_data + data_length -1) == ((uint8_t *)&cs)[0])
	{
		error = FPRINT_EOK;
	}
#if (defined RT_USING_FINSH) && (defined FPRINT_DEBUG)
	rt_kprintf("\nfprint recv :---------------------\n");
	print_hex((uint8_t *)frame_head, sizeof(*frame_head));
	print_hex((uint8_t *)rep_data, data_length);
	rt_kprintf("------------------------------------\n");
#endif // RT_USING_FINSH

error_process:

	return error;
}

__STATIC_INLINE FPRINT_ERROR_TYPEDEF
fprint_frame_process(uint8_t cmd, FPRINT_FRAME_REQ_DATA_TYPEDEF *req_data, FPRINT_FRAME_REP_DATA_TYPEDEF *rep_data)
{
	FPRINT_ERROR_TYPEDEF error;
	FPRINT_FRAME_HEAD_TYPEDEF head;

    uint16_t start = FPRINT_FRAME_START;
    uint32_t addr = FPRINT_FRAME_ADDR;
    error = FPRINT_EERROR;
    switch (cmd)
    {
        case FPRINT_FRAME_CMD_VERIFY:
        case FPRINT_FRAME_CMD_EMPTY:
        case FPRINT_FRAME_CMD_SET_PARAM:
        case FPRINT_FRAME_CMD_SET_ADDR:
        case FPRINT_FRAME_CMD_GET_IMAGE:
        case FPRINT_FRAME_CMD_GENERATE:
        case FPRINT_FRAME_CMD_MERGE:
        case FPRINT_FRAME_CMD_UP_CHAR:
        case FPRINT_FRAME_CMD_DOWN_CHAR:
        case FPRINT_FRAME_CMD_LOAD_CHAR:
        case FPRINT_FRAME_CMD_STORE_CHAR:
        case FPRINT_FRAME_CMD_DEL_CHAR:
        case FPRINT_FRAME_CMD_SEARCH:
            {
                break;
            }
        default :
            {
                RT_DEBUG_LOG(FPRINT_DEBUG, ("the finger print frame request cmd 0x%02X is invalid!\n", cmd));
                goto process_error;
            }
    }

	error = FPRINT_EERROR;


	rt_memset(&head, 0, sizeof(head));

	reverse(head.start, (uint8_t *)&start, sizeof(start));
    reverse(head.addr, (uint8_t *)&addr, sizeof(addr));

    head.pid = FPRINT_FRAME_PID_CMD;

	error = fprint_frame_send(cmd, &head, req_data);
	if (error != FPRINT_EOK)
		goto process_error;
    //rt_thread_delay(50);
	error = fprint_frame_recv(cmd, &head, rep_data);
	if (error != FPRINT_EOK)
		goto process_error;

    error = FPRINT_EERROR;
    switch (cmd)
    {
        case FPRINT_FRAME_CMD_VERIFY:
            {
                if (rep_data->rep_verify.result == FPRINT_FRAME_ERR_SUCCESS)
                    error  = FPRINT_EOK;
                break;
            }
        case FPRINT_FRAME_CMD_EMPTY:
            {
                if (rep_data->rep_empty.result == FPRINT_FRAME_ERR_SUCCESS)
                    error  = FPRINT_EOK;
                break;
            }
        case FPRINT_FRAME_CMD_SET_PARAM:
            {
                if (rep_data->rep_set_param.result == FPRINT_FRAME_ERR_SUCCESS)
                    error  = FPRINT_EOK;
                break;
            }
        case FPRINT_FRAME_CMD_SET_ADDR:
            {
                if (rep_data->rep_set_addr.result == FPRINT_FRAME_ERR_SUCCESS)
                    error  = FPRINT_EOK;
                break;
            }
        case FPRINT_FRAME_CMD_GET_IMAGE:
            {
                if (rep_data->rep_get_image.result == FPRINT_FRAME_ERR_SUCCESS)
                    error  = FPRINT_EOK;
                else if (rep_data->rep_get_image.result == FPRINT_FRAME_ERR_FP_NOT_DETECTED)
                    error = FPRINT_ENO_DETECTED;
                break;
            }
        case FPRINT_FRAME_CMD_GENERATE:
            {
                if (rep_data->rep_generate.result == FPRINT_FRAME_ERR_SUCCESS)
                    error  = FPRINT_EOK;
                break;
            }
        case FPRINT_FRAME_CMD_MERGE:
            {
                if (rep_data->rep_merge.result == FPRINT_FRAME_ERR_SUCCESS)
                    error  = FPRINT_EOK;
                break;
            }
        case FPRINT_FRAME_CMD_UP_CHAR:
            {
                if (rep_data->rep_up_char.result == FPRINT_FRAME_ERR_SUCCESS) {
                    error  = FPRINT_EOK;

                }
                break;
            }
        case FPRINT_FRAME_CMD_DOWN_CHAR:
            {
                if (rep_data->rep_down_char.result == FPRINT_FRAME_ERR_SUCCESS) {
                    error  = FPRINT_EOK;

                }
                break;
            }
        case FPRINT_FRAME_CMD_LOAD_CHAR:
            {
                if (rep_data->rep_load_char.result == FPRINT_FRAME_ERR_SUCCESS) {
                    error  = FPRINT_EOK;

                }
                break;
            }
        case FPRINT_FRAME_CMD_STORE_CHAR:
            {
                if (rep_data->rep_store_char.result == FPRINT_FRAME_ERR_SUCCESS) {
                    error  = FPRINT_EOK;

                }
                break;
            }
        case FPRINT_FRAME_CMD_DEL_CHAR:
            {
                if (rep_data->rep_del_char.result == FPRINT_FRAME_ERR_SUCCESS) {
                    error  = FPRINT_EOK;

                }
                break;
            }
        case FPRINT_FRAME_CMD_SEARCH:
            {
                if (rep_data->rep_search.result == FPRINT_FRAME_ERR_SUCCESS) {
                    error  = FPRINT_EOK;

                }
                break;
            }
        default :
            {
                goto process_error;
            }
    }

process_error:

	return error;
}

int fprint_key_init_callback(struct key *k, void *arg1, void *arg2, void *arg3)
{
    FPRINT_ERROR_TYPEDEF error = FPRINT_EERROR;
    FPRINT_FRAME_REQ_DATA_TYPEDEF *req_data = arg1;
    FPRINT_FRAME_REP_DATA_TYPEDEF *rep_data = arg2;
    u16 key_id = *(u16 *)arg3 + FPRINT_TEMPLATE_OFFSET;
    if (k->head.key_type == KEY_TYPE_FPRINT) {
        rt_memset(req_data, 0, sizeof(*req_data));
        rt_memset(rep_data, 0, sizeof(*rep_data));
        req_data->req_down_char.buf_id = 1;
        error = fprint_frame_process(FPRINT_FRAME_CMD_DOWN_CHAR, req_data, rep_data);
        if (error != FPRINT_EOK)
            return error;
		fprint_frame_send_data(&k->data,512);
        /*
        // get fprint template from rambuf0
        rt_memset(req_data, 0, sizeof(*req_data));
        rt_memset(rep_data, 0, sizeof(*rep_data));
        req_data->req_up_char.buf_id = 1;
        error = fprint_frame_process(FPRINT_FRAME_CMD_UP_CHAR,
                                        req_data, rep_data);
        if (error == FPRINT_EOK) {
            u8 buf[512];
            fprint_frame_recv_data(buf, 512);
            print_hex(buf, 512);
        }
        */
		// store fprint template to template_id
		rt_memset(req_data, 0, sizeof(*req_data));
		rt_memset(rep_data, 0, sizeof(*rep_data));
        req_data->req_store_char.buf_id = 1;
		reverse(req_data->req_store_char.template_id, (uint8_t *)&key_id, sizeof(key_id));
		error = fprint_frame_process(FPRINT_FRAME_CMD_STORE_CHAR, req_data, rep_data);

		//if (error != FPRINT_EOK)
		//    return error;
    }
    return error;
}

static FPRINT_ERROR_TYPEDEF
fprint_init(uint8_t *buf, FPRINT_FRAME_REQ_DATA_TYPEDEF *req_data, FPRINT_FRAME_REP_DATA_TYPEDEF *rep_data)
{
//	uint16_t i;
	FPRINT_ERROR_TYPEDEF error = FPRINT_EERROR;
    uint32_t pw = FPRINT_FRAME_PW;
    uint32_t addr = FPRINT_FRAME_ADDR;
//	KEY_TYPEDEF key;
	// test connection
	rt_memset(req_data, 0, sizeof(*req_data));
    rt_memset(rep_data, 0, sizeof(*rep_data));
    reverse(req_data->req_verify.pw, (uint8_t *)&pw, sizeof(pw));
	error = fprint_frame_process(FPRINT_FRAME_CMD_VERIFY, req_data, rep_data);
	if (error != FPRINT_EOK)
		return error;

	// delete all template
	rt_memset(req_data, 0, sizeof(*req_data));
    rt_memset(rep_data, 0, sizeof(*rep_data));
	error = fprint_frame_process(FPRINT_FRAME_CMD_EMPTY, req_data, rep_data);
	if (error != FPRINT_EOK)
		return error;
	// set ADDR
	rt_memset(req_data, 0, sizeof(*req_data));
    rt_memset(rep_data, 0, sizeof(*rep_data));
    reverse(req_data->req_set_addr.addr, (uint8_t *)&addr, sizeof(addr));
	error = fprint_frame_process(FPRINT_FRAME_CMD_SET_ADDR, req_data, rep_data);
	if (error != FPRINT_EOK)
		return error;
	// set package size
	rt_memset(req_data, 0, sizeof(*req_data));
    rt_memset(rep_data, 0, sizeof(*rep_data));
	req_data->req_set_param.type = 6;
	req_data->req_set_param.data = 2;
	error = fprint_frame_process(FPRINT_FRAME_CMD_SET_PARAM, req_data, rep_data);
	if (error != FPRINT_EOK)
		return error;
    error = (FPRINT_ERROR_TYPEDEF)device_config_key_index(fprint_key_init_callback, req_data, rep_data);

	return error;
}

static FPRINT_ERROR_TYPEDEF
fprint_enroll(uint8_t *buf, FPRINT_FRAME_REQ_DATA_TYPEDEF *req_data, FPRINT_FRAME_REP_DATA_TYPEDEF *rep_data)
{

	FPRINT_ERROR_TYPEDEF error = FPRINT_EERROR;

	// get fprint image
	rt_memset(req_data, 0, sizeof(*req_data));
    rt_memset(rep_data, 0, sizeof(*rep_data));
	error = fprint_frame_process(FPRINT_FRAME_CMD_GET_IMAGE,
                                    req_data, rep_data);
	if (error != FPRINT_EOK)
		return error;

	// generate fprint template to buf1
	rt_memset(req_data, 0, sizeof(*req_data));
    rt_memset(rep_data, 0, sizeof(*rep_data));
	req_data->req_generate.buf_id = 1;
	error = fprint_frame_process(FPRINT_FRAME_CMD_GENERATE,
                                    req_data, rep_data);

	if (error != FPRINT_EOK)
		return error;
	// get fprint image
	rt_memset(req_data, 0, sizeof(*req_data));
    rt_memset(rep_data, 0, sizeof(*rep_data));
	error = fprint_frame_process(FPRINT_FRAME_CMD_GET_IMAGE,
                                    req_data, rep_data);
	if (error != FPRINT_EOK)
		return error;

	// generate fprint template to buf2
	rt_memset(req_data, 0, sizeof(*req_data));
    rt_memset(rep_data, 0, sizeof(*rep_data));
	req_data->req_generate.buf_id = 2;
	error = fprint_frame_process(FPRINT_FRAME_CMD_GENERATE,
                                    req_data, rep_data);

	if (error != FPRINT_EOK)
		return error;

	// merge
	rt_memset(req_data, 0, sizeof(*req_data));
    rt_memset(rep_data, 0, sizeof(*rep_data));
	error = fprint_frame_process(FPRINT_FRAME_CMD_MERGE,
                                    req_data, rep_data);
	if (error != FPRINT_EOK)
		return error;

    // get fprint template from rambuf0
	rt_memset(req_data, 0, sizeof(*req_data));
    rt_memset(rep_data, 0, sizeof(*rep_data));
    req_data->req_up_char.buf_id = 1;
	error = fprint_frame_process(FPRINT_FRAME_CMD_UP_CHAR,
                                    req_data, rep_data);
    if (error != FPRINT_EOK)
		return error;
    fprint_frame_recv_data(buf, 512);

	return error;
}

static FPRINT_ERROR_TYPEDEF
fprint_verify(FPRINT_FRAME_REQ_DATA_TYPEDEF *req_data, FPRINT_FRAME_REP_DATA_TYPEDEF *rep_data)
{

	FPRINT_ERROR_TYPEDEF error = FPRINT_EERROR;
    uint16_t temp;
	// get fprint image
	rt_memset(req_data, 0, sizeof(*req_data));
    rt_memset(rep_data, 0, sizeof(*rep_data));
	error = fprint_frame_process(FPRINT_FRAME_CMD_GET_IMAGE,
                                    req_data, rep_data);
	if (error != FPRINT_EOK)
		return error;

	// generate fprint template to buf1
	rt_memset(req_data, 0, sizeof(*req_data));
    rt_memset(rep_data, 0, sizeof(*rep_data));
	req_data->req_generate.buf_id = 1;
	error = fprint_frame_process(FPRINT_FRAME_CMD_GENERATE,
                                    req_data, rep_data);

	if (error != FPRINT_EOK)
		return error;
	// search fprint template to rambuf0
	rt_memset(req_data, 0, sizeof(*req_data));
    rt_memset(rep_data, 0, sizeof(*rep_data));
    req_data->req_search.buf_id = 1;
    temp = FPRINT_TEMPLATE_ID_START;
    reverse(req_data->req_search.template_id, (uint8_t *)&temp, sizeof(temp));
    temp = FPRINT_TEMPLATE_ID_END - FPRINT_TEMPLATE_ID_START;
    reverse(req_data->req_search.size, (uint8_t *)&temp, sizeof(temp));

	error = fprint_frame_process(FPRINT_FRAME_CMD_SEARCH,
                                    req_data, rep_data);

	return error;
}

static void
fprint_thread_entry(void *parameters)
{
	rt_err_t result;
	//rt_device_t fprint_device;
	FPRINT_MAIL_TYPEDEF fprint_mail;
	FPRINT_FRAME_REQ_DATA_TYPEDEF req_data;
    FPRINT_FRAME_REP_DATA_TYPEDEF rep_data;
	FPRINT_ERROR_TYPEDEF error;
    int temp;
	//fprint_device = device_enable(DEVICE_NAME_FPRINT)
    uint8_t buf[512];

    error = FPRINT_EERROR;
    
    fprint_init(buf, &req_data, &rep_data);
	while (1)
	{
		result = rt_mq_recv(fprint_mq, &fprint_mail, sizeof(fprint_mail), 500);
		if (result == RT_EOK)
		{
			switch(fprint_mail.cmd)
			{
				case FPRINT_CMD_INIT:
					{
						fprint_reset();
						error = fprint_init(buf, &req_data, &rep_data);
						break;
					}
				case FPRINT_CMD_ENROLL:
					{	
                        static uint16_t template_id = 0;

                        rt_thread_delay(RT_TICK_PER_SECOND);
                        rt_memset(&req_data, 0, sizeof(req_data));
                        rt_memset(&rep_data, 0, sizeof(rep_data));
                        error = fprint_verify(&req_data, &rep_data);
                        if (error == FPRINT_EOK) {
                            reverse((uint8_t *)&template_id, rep_data.rep_search.template_id, 2);
                            RT_DEBUG_LOG(FPRINT_DEBUG, ("fprint verify is exist, %d\n", template_id - FPRINT_TEMPLATE_OFFSET));
                            error = FPRINT_EEXIST;
                        } else {
                            error = fprint_enroll(buf,&req_data, &rep_data);
                            if (error == FPRINT_EOK) {
                                temp = device_config_key_create(*fprint_mail.key_id, KEY_TYPE_FPRINT, buf, sizeof(buf));
                                if (temp < 0) {
                                    RT_DEBUG_LOG(FPRINT_DEBUG, ("the finger print key create failure! error: %d\n", temp));
                                    error = FPRINT_EERROR;
                                } else {
                                    // store fprint template to template_id
                                    rt_memset(&req_data, 0, sizeof(req_data));
                                    rt_memset(&rep_data, 0, sizeof(rep_data));
                                    temp += FPRINT_TEMPLATE_OFFSET;
                                    reverse(req_data.req_store_char.template_id, (uint8_t *)&temp, 2);
                                    req_data.req_store_char.buf_id = 1;
                                    error = fprint_frame_process(FPRINT_FRAME_CMD_STORE_CHAR,
                                                                    &req_data, &rep_data);
                                    if (error == FPRINT_EOK) {
                                        if (fprint_mail.buf != RT_NULL)
                                            rt_memcpy(fprint_mail.buf, buf, 512);
                                        if (fprint_mail.key_id != RT_NULL)
                                            *fprint_mail.key_id = (uint16_t)temp;
                                    }
                                }
                            }
                        }
						break;
					}
                case FPRINT_CMD_GET_TEMPLATE:
					{
                        static uint16_t template_id = 0;
                        
                        rt_memset(&req_data, 0, sizeof(req_data));
                        rt_memset(&rep_data, 0, sizeof(rep_data));
                        error = fprint_verify(&req_data, &rep_data);
                        if (error == FPRINT_EOK) {
                            reverse((uint8_t *)&template_id, rep_data.rep_search.template_id, 2);
                            RT_DEBUG_LOG(FPRINT_DEBUG, ("fprint verify is exist, %d\n", template_id - FPRINT_TEMPLATE_OFFSET));
                            error = FPRINT_EEXIST;
                        } else {
                            error = fprint_enroll(buf,&req_data, &rep_data);
                            if (error == FPRINT_EOK) {
                                if (fprint_mail.buf != RT_NULL)
                                    rt_memcpy(fprint_mail.buf, buf, 512);
                            }
                        }
						break;
					}
				case FPRINT_CMD_DELETE:
					{
                        rt_memset(&req_data, 0, sizeof(req_data));
                        rt_memset(&rep_data, 0, sizeof(rep_data));
						temp = *fprint_mail.key_id + FPRINT_TEMPLATE_OFFSET;
                        reverse(req_data.req_del_char.template_id, (uint8_t *)&(temp), 2);
						temp = fprint_mail.size + FPRINT_TEMPLATE_OFFSET;
                        reverse(req_data.req_del_char.size, (uint8_t *)&temp, 2);
                        error = fprint_frame_process(FPRINT_FRAME_CMD_DEL_CHAR,
                                                        &req_data, &rep_data);
                        break;
					}
				case FPRINT_CMD_STORE_TEMPLATE:
					{
                        rt_memset(&req_data, 0, sizeof(req_data));
                        rt_memset(&rep_data, 0, sizeof(rep_data));
                        req_data.req_down_char.buf_id = 1;
                        error = fprint_frame_process(FPRINT_FRAME_CMD_DOWN_CHAR, &req_data, &rep_data);
                        if (error != FPRINT_EOK)
                            fprint_frame_send_data(fprint_mail.buf,512);
                        rt_memset(&req_data, 0, sizeof(req_data));
                        rt_memset(&rep_data, 0, sizeof(rep_data));
                        temp = *fprint_mail.key_id + FPRINT_TEMPLATE_OFFSET;
                        reverse(req_data.req_store_char.template_id, (uint8_t *)&temp, 2);
                        req_data.req_store_char.buf_id = 1;
                        error = fprint_frame_process(FPRINT_FRAME_CMD_STORE_CHAR,
                                                        &req_data, &rep_data);

                        break;
					}
				case FPRINT_CMD_UP_CHAR:
					{
                        // store fprint template to template_id
                        rt_memset(&req_data, 0, sizeof(req_data));
                        rt_memset(&rep_data, 0, sizeof(rep_data));
                        req_data.req_load_char.buf_id = 1;
                        temp = *fprint_mail.key_id + FPRINT_TEMPLATE_OFFSET;
                        reverse(req_data.req_load_char.template_id, (uint8_t *)&(temp), 2);
                        error = fprint_frame_process(FPRINT_FRAME_CMD_LOAD_CHAR, &req_data, &rep_data);
                        if (error != FPRINT_EOK)
                            break;
                        // get fprint template from rambuf0
                        rt_memset(&req_data, 0, sizeof(req_data));
                        rt_memset(&rep_data, 0, sizeof(rep_data));
                        req_data.req_up_char.buf_id = 1;
                        error = fprint_frame_process(FPRINT_FRAME_CMD_UP_CHAR,
                                                        &req_data, &rep_data);
                        if (error == FPRINT_EOK) {
                            fprint_frame_recv_data(buf, 512);
                            print_hex(buf, 512);
                        }
                        break;
					}
				case FPRINT_CMD_VERIFY:
					{
                        static uint16_t f_detect = 0;
                        static uint16_t r_detect = 0;
                        static uint16_t template_id = 0;

                        rt_thread_delay(RT_TICK_PER_SECOND/2);
                        error = fprint_verify(&req_data, &rep_data);
                        if (error == FPRINT_EOK) {
                            //union alarm_data data;
                            reverse((uint8_t *)&template_id, rep_data.rep_search.template_id, 2);
                            RT_DEBUG_LOG(FPRINT_DEBUG, ("fprint verify is exist, %d\n", template_id - FPRINT_TEMPLATE_OFFSET));

                            //data.lock.key_id = template_id - FPRINT_TEMPLATE_OFFSET;
                            //data.lock.operation = GATE_UNLOCK;
                            //send_local_mail(ALARM_TYPE_LOCK_PROCESS, 0, &data);
                            if(fprintf_ok_fun != RT_NULL)
                            {
                                FPINTF_USER key;

                                key.KeyPos = template_id - FPRINT_TEMPLATE_OFFSET;
                                rt_kprintf("key pos : %d\n", key.KeyPos);
                                fprintf_ok_fun((void *)&key);
                            }

                        }
                        if (error == FPRINT_ENO_DETECTED) {
                            RT_DEBUG_LOG(FPRINT_DEBUG, ("fprint verify is no detected\n"));
                            if(fprintf_null_fun != RT_NULL)
                            {
                                fprintf_null_fun(RT_NULL);
                            }
                        }
                        if (error == FPRINT_EERROR) {

                            if (f_detect++) {

                            } else {

                                RT_DEBUG_LOG(FPRINT_DEBUG, ("fprint verify is error\n"));
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
                                RT_DEBUG_LOG(FPRINT_DEBUG, ("fprint has no response , may be fault!\n"));
                                fprint_init(buf, &req_data, &rep_data);
                                r_detect = 0;
                            }
                        } else {
                            r_detect = 0;
                        }
                        break;
					}
				default :
					{
                        RT_DEBUG_LOG(FPRINT_DEBUG, ("the finger print cmd is invalid!\n"));
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

        }

	}
}

static FPRINT_ERROR_TYPEDEF
send_fp_mail(FPRINT_CMD_TYPEDEF cmd, uint16_t *key_id, uint8_t *buf, uint16_t size, uint8_t flag)
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
        mail.size = size;
        mail.buf = buf;
		result = rt_mq_send(fprint_mq, &mail, sizeof(mail));
		if (result == -RT_EFULL)
		{
            RT_DEBUG_LOG(FPRINT_DEBUG, ("fprint_mq is full!!!\n"));
		}
		else
		{
            if (flag)
                rt_sem_take(mail.result_sem, RT_WAITING_FOREVER);
            RT_DEBUG_LOG(FPRINT_DEBUG, ("send result is %d\n", *mail.result));
		}
        if (flag)
            rt_sem_delete(mail.result_sem);
	}
	else
	{
        RT_DEBUG_LOG(FPRINT_DEBUG, ("fprint_mq is null!!!\n"));
	}
	return error;
}

static int
rt_fprint_init(void)
{
	rt_thread_t fprint_thread;

    //initial fprint msg queue
	fprint_mq = rt_mq_create("fprint", sizeof(FPRINT_MAIL_TYPEDEF),
							 FPRINT_MAIL_MAX_MSGS, RT_IPC_FLAG_FIFO);
    if (fprint_mq == RT_NULL)
        return -1;

    s_fprint = rt_sem_create("s_fprint", 0, RT_IPC_FLAG_FIFO);

    // finger print thread
	fprint_thread = rt_thread_create("fprint", fprint_thread_entry,
									 RT_NULL, 3000, 100, 5);
	if (fprint_thread == RT_NULL)
        return -1;

    rt_thread_startup(fprint_thread);
    return 0;
}

INIT_APP_EXPORT(rt_fprint_init);

int
fp_init(void)
{
    int result = -1;

    if (send_fp_mail(FPRINT_CMD_INIT, RT_NULL, RT_NULL, 0, 1) == FPRINT_EOK)
        result = 0;

    return result;
}

static char enroll_flag = 0;

int
fp_enroll(uint16_t *key_id, uint8_t *buf, uint32_t timeout)
{
    rt_err_t error;
    int result = -1;
    
    RT_ASSERT(key_id != RT_NULL);

    enroll_flag = 1;
    error = rt_sem_take(s_fprint, timeout);
    if (error == RT_EOK) {
        if (send_fp_mail(FPRINT_CMD_ENROLL, key_id, buf, 0, 1) == FPRINT_EOK)
            result = *key_id;
    }
    enroll_flag = 0;
    return result;
}

int
fp_get_template(uint8_t *buf, uint32_t timeout)
{
    rt_err_t error;
    int result = -1;

    enroll_flag = 1;
    error = rt_sem_take(s_fprint, timeout);
    if (error == RT_EOK) {
        if (send_fp_mail(FPRINT_CMD_GET_TEMPLATE, RT_NULL, buf, 0, 1) == FPRINT_EOK)
            result = 1;
    }
    enroll_flag = 0;
    return result;
}

int
fp_store_template(const uint16_t key_id, uint8_t *buf, uint32_t timeout)
{
    int result = -1;
    if (send_fp_mail(FPRINT_CMD_STORE_TEMPLATE, (uint16_t *)&key_id, buf, 0, 1) == FPRINT_EOK)
        result = key_id;
    return result;
}

int
fp_delete(const uint16_t key_id, const uint16_t size)
{
    int result = -1;

    if (send_fp_mail(FPRINT_CMD_DELETE, (uint16_t *)&key_id, RT_NULL, size, 1) == FPRINT_EOK)
        result = key_id;
    return result;
}

int
fp_verify(void)
{
    int result = -1;
    if (send_fp_mail(FPRINT_CMD_VERIFY, RT_NULL, RT_NULL, 0, 0) == FPRINT_EOK)
        result = 0;
    return result;
}

void
fp_inform(void)
{
    if (enroll_flag)
        rt_sem_release(s_fprint);
    else
        fp_verify();
}

int
fp_up_char(uint16_t key_id)
{

    int result = -1;
    if (send_fp_mail(FPRINT_CMD_UP_CHAR, (uint16_t *)&key_id, RT_NULL, 0, 1) == FPRINT_EOK)
        result = 0;
    return result;
}
#ifdef RT_USING_FINSH
#include <finsh.h>

void fp_enroll_test(u16 timeout)
{
    uint16_t key_id;
    uint8_t buf[512];
    if (fp_enroll(&key_id, buf, timeout) >= 0) {
        rt_kprintf("key_id = %d\n--------------------\n", key_id);
        print_hex(buf, 512);
    } else {
        rt_kprintf("fprint enroll timeout\n");
    }
}

FINSH_FUNCTION_EXPORT(fp_init, fp_init[void]);
FINSH_FUNCTION_EXPORT(fp_enroll_test, fp_enroll_test[timeout]);
FINSH_FUNCTION_EXPORT(fp_delete, fp_delete[uint16]);
FINSH_FUNCTION_EXPORT(fp_verify, fp_verify[void]);
FINSH_FUNCTION_EXPORT(fp_inform, fp_inform[void]);
FINSH_FUNCTION_EXPORT_ALIAS(fp_up_char, fp_uchar, fp_up_char[key_id]);
#endif // RT_USING_FINSH
