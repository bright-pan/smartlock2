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

#define FPRINT_MAIL_MAX_MSGS 10

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

}FPRINT_ERROR_TYPEDEF;

typedef struct {

	FPRINT_CMD_TYPEDEF cmd;
	uint16_t key_id;
	rt_sem_t result_sem;
	FPRINT_ERROR_TYPEDEF *result;

}FPRINT_MAIL_TYPEDEF;

extern rt_mq_t fprint_mq;

void
fprint_thread_entry(void *parameters);

FPRINT_ERROR_TYPEDEF
send_fp_mail(FPRINT_CMD_TYPEDEF cmd, uint16_t key_id);

#endif /* _FPRINT_H_ */
