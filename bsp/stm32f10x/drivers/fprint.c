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

#define FPRINT_DEBUG
#define DEVICE_NAME_FPRINT "uart2"
#define FPRINT_TEMPLATE_OFFSET 1000 // 1 <= offset <= 2000
#define FPRINT_TEMPLATE_SIZE 2000 // 1 <= offset <= 2000
#define FPRINT_TEMPLATE_ID_START FPRINT_TEMPLATE_OFFSET
#define FPRINT_TEMPLATE_ID_END (FPRINT_TEMPLATE_OFFSET + KEY_NUMBERS - 1)

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
#define FPRINT_FRAME_CMD_SEARCH 0x55
//result define
#define FPRINT_FRAME_ERR_SUCCESS 0x00
#define FPRINT_FRAME_ERR_FAIL 0x01
#define FPRINT_FRAME_ERR_FP_NOT_DETECTED 0x02

typedef struct{

    uint8_t start[2];
	uint8_t addr[4];
	uint8_t pid;
	uint8_t len[2];

}FPRINT_FRAME_HEAD_TYPEDEF;

typedef struct {
    uint8_t data[512];
}FPRINT_FRAME_DATA_TYPEDEF;

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

}FPRINT_FRAME_REQ_STORE_CHAR_TYPEDEF;
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
    FPRINT_FRAME_REQ_STORE_CHAR_TYPEDEF req_store_char;

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

}FPRINT_FRAME_REP_STORE_CHAR_TYPEDEF;

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
    FPRINT_FRAME_REP_STORE_CHAR_TYPEDEF rep_store_char;

}FPRINT_FRAME_REP_DATA_TYPEDEF;


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

void
fprint_reset(void)
{
//	gpio_pin_output(DEVICE_NAME_FPRINT_RESET, 0);
	rt_thread_delay(20);
//	gpio_pin_output(DEVICE_NAME_FPRINT_RESET, 1);
	rt_thread_delay(100);
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
        case FPRINT_FRAME_CMD_STORE_CHAR:
            {
                req_data->req_store_char.cmd = cmd;
                data_offset = 0;
                data_length = sizeof(req_data->req_store_char) + 2;
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
fprint_frame_send_data(uint8_t *buf, uint16_t size)
{
	uint16_t cs;
	rt_device_t fprint_device;
	uint16_t data_length, data_offset;
    uint16_t check = 0;
	rt_size_t recv_cnts;
	FPRINT_ERROR_TYPEDEF error;
	uint8_t cnts;
    FPRINT_FRAME_HEAD_TYPEDEF head;
    uint16_t start = FPRINT_FRAME_START;
    uint32_t addr = FPRINT_FRAME_ADDR;
    data_length = 0;
    data_offset = 0;
    
	error = FPRINT_EERROR;


	fprint_device = device_enable(DEVICE_NAME_FPRINT);
	if (fprint_device == RT_NULL)
		goto error_process;
    while (size > 0)
    {
        rt_memset(&head, 0, sizeof(head));

        reverse(head.start, (uint8_t *)&start, sizeof(start));
        reverse(head.addr, (uint8_t *)&addr, sizeof(addr));

        if (size > 256) {
            data_length = 256;
            head.pid = FPRINT_FRAME_PID_DATA;
        } else {
            data_length = size;
            head.pid = FPRINT_FRAME_PID_END;
        }
        size -= data_length;
        
        //reverse((uint8_t *)&data_length, head.len, sizeof(data_length));

        if (data_length <= size)
            goto error_process;
        data_length += 2;
        reverse(head.len, (uint8_t *)&data_length, sizeof(data_length));
        data_length -= 2;
        cs = check_sum((uint8_t *)&head+6, sizeof(head)-6) + check_sum(buf + data_offset, data_length);
        error = FPRINT_EOK;
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
	uint16_t data_length, data_offset;
	rt_size_t recv_cnts;
	FPRINT_ERROR_TYPEDEF error;
	uint8_t cnts;

    data_length = 0;
    data_offset = 0;

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
    uint16_t length = 0;
    
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
        case FPRINT_FRAME_CMD_STORE_CHAR:
            {
                break;
            }
        default :
            {
#if (defined RT_USING_FINSH) && (defined FPRINT_DEBUG)
					rt_kprintf("the finger print frame request cmd 0x%02X is invalid!\n", cmd);
#endif // RT_USING_FINSH
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
        case FPRINT_FRAME_CMD_STORE_CHAR:
            {
                if (rep_data->rep_store_char.result == FPRINT_FRAME_ERR_SUCCESS) {
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

FPRINT_ERROR_TYPEDEF
fprint_init(FPRINT_FRAME_REQ_DATA_TYPEDEF *req_data, FPRINT_FRAME_REP_DATA_TYPEDEF *rep_data)
{
	uint16_t i;
	FPRINT_ERROR_TYPEDEF error = FPRINT_EERROR;
    uint32_t pw = FPRINT_FRAME_PW;
    uint32_t addr = FPRINT_FRAME_ADDR;
	KEY_TYPEDEF key;
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
	req_data->req_set_param.data = 3;
	error = fprint_frame_process(FPRINT_FRAME_CMD_SET_PARAM, req_data, rep_data);
	if (error != FPRINT_EOK)
		return error;
    /*
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
    */
	return error;
}

FPRINT_ERROR_TYPEDEF
fprint_enroll(uint16_t template_id, uint8_t *buf, FPRINT_FRAME_REQ_DATA_TYPEDEF *req_data, FPRINT_FRAME_REP_DATA_TYPEDEF *rep_data)
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

	// store fprint template to template_id
	rt_memset(req_data, 0, sizeof(*req_data));
    rt_memset(rep_data, 0, sizeof(*rep_data));
    reverse(req_data->req_store_char.template_id, (uint8_t *)&template_id, sizeof(template_id));
	req_data->req_store_char.buf_id = 1;
	error = fprint_frame_process(FPRINT_FRAME_CMD_STORE_CHAR,
                                    req_data, rep_data);
	if (error != FPRINT_EOK)
		return error;

	// get fprint template from rambuf0
	rt_memset(req_data, 0, sizeof(*req_data));
    rt_memset(rep_data, 0, sizeof(*rep_data));
    req_data->req_up_char.buf_id = 1;
	error = fprint_frame_process(FPRINT_FRAME_CMD_UP_CHAR,
                                    req_data, rep_data);
    fprint_frame_recv_data(buf, 512);
	return error;
}
/*
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
*/
void
fprint_thread_entry(void *parameters)
{
	rt_err_t result;
	//rt_device_t fprint_device;
	FPRINT_MAIL_TYPEDEF fprint_mail;
	FPRINT_FRAME_REQ_DATA_TYPEDEF req_data;
    FPRINT_FRAME_REP_DATA_TYPEDEF rep_data;
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
						error = fprint_init(&req_data, &rep_data);
						break;
					}
				case FPRINT_CMD_ENROLL:
					{
                        uint8_t buf[512];
                        fprint_enroll(1, buf,&req_data, &rep_data);
                        /*
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
                        */
						break;
					}
				case FPRINT_CMD_DELETE:
					{
                        /*
						rt_memset(&frame_data, 0, sizeof(frame_data));
						frame_data.req_del_char.start = fprint_mail.key_id + FPRINT_TEMPLATE_OFFSET;
						frame_data.req_del_char.end = fprint_mail.key_id + FPRINT_TEMPLATE_OFFSET;
						error = fprint_frame_process(FPRINT_FRAME_CMD_DEL_CHAR, FPRINT_FRAME_PREFIX_REQUEST, 0, 0, &frame_data);
                        */
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
            /*
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
        */
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
