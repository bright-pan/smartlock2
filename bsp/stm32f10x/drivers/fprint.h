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
#include "untils.h"
#include "board.h"

typedef enum {

	FPRINT_CMD_INIT,
	FPRINT_CMD_ENROLL,
	FPRINT_CMD_DELETE,
	FPRINT_CMD_RESET,

}FPRINT_CMD_TYPEDEF;

typedef enum {

	FPRINT_EOK,
	FPRINT_EERROR,
	FPRINT_ERESPONSE,
	FPRINT_EEXIST,
    FPRINT_ENO_DETECTED,

}FPRINT_ERROR_TYPEDEF;

typedef struct {

	FPRINT_CMD_TYPEDEF cmd;
	uint16_t key_id;
	rt_sem_t result_sem;
	FPRINT_ERROR_TYPEDEF *result;

}FPRINT_MAIL_TYPEDEF;

typedef  rt_err_t (*fprint_call_back)(void *user);

typedef struct
{
	rt_uint16_t KeyPos;
}FPINTF_USER;

void
fprint_thread_entry(void *parameters);

FPRINT_ERROR_TYPEDEF
send_fp_mail(FPRINT_CMD_TYPEDEF cmd, uint16_t key_id, uint8_t flag);

void 
fp_ok_callback(fprint_call_back fun);

void 
fp_error_callback(fprint_call_back fun);

void 
fp_null_callback(fprint_call_back fun);



#endif /* _FPRINT_H_ */
