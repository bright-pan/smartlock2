/*********************************************************************
 * Filename:			fprint.h
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

#ifndef _FPRINT_H_
#define _FPRINT_H_

#include <rthw.h>
#include <rtthread.h>
#include <stm32f10x.h>

typedef enum {

	FPRINT_EOK,
	FPRINT_EERROR,
	FPRINT_ERESPONSE,
	FPRINT_EEXIST,
    FPRINT_ENO_DETECTED,
	FPRINT_EINVALID,
    FPRINT_EEXCEPTION,

}FPRINT_ERROR_TYPEDEF;

typedef  rt_err_t (*fprint_call_back)(void *user);

typedef struct
{
	rt_uint16_t KeyPos;
}FPINTF_USER;

void
fp_ok_callback(fprint_call_back fun);

void
fp_error_callback(fprint_call_back fun);

void
fp_null_callback(fprint_call_back fun);

int 
fp_init(void);

int
fp_enroll(uint16_t *, uint8_t *, uint32_t);

int
fp_delete(const uint16_t, const uint16_t);

void
fp_inform(void);
int
fp_get_template(uint8_t *buf, uint32_t timeout);

int
fp_store_template(const uint16_t key_id, uint8_t *buf, uint32_t timeout);

#endif /* _FPRINT_H_ */
