/*********************************************************************
 * Filename:			gsm.h
 *
 * Description:
 *
 * Author:				Bright Pan
 * Email:				bright_pan@yuettak.com
 * Date:				2014-03-15
 *
 * Modify:
 *
 * Copyright (C) 2014 Yuettak Co.,Ltd
 ********************************************************************/

#ifndef _GSM_H_
#define _GSM_H_

#include <rtthread.h>
#include <rthw.h>
#include <stm32f10x.h>
#include <string.h>
#include <stdio.h>

#include "board.h"
#include "untils.h"
#include "gpio_pin.h"
#include "gpio_exti.h"

#define GSM_MAIL_MAX_MSGS 5

typedef enum {
	GSM_SETUP_ENABLE_SUCCESS,
	GSM_SETUP_ENABLE_FAILURE,
	GSM_SETUP_DISABLE_SUCCESS,
	GSM_SETUP_DISABLE_FAILURE,
	GSM_RESET_SUCCESS,
	GSM_RESET_FAILURE,
	GSM_SMS_SEND_SUCCESS,
	GSM_SMS_SEND_FAILURE,
}GsmStatus;

typedef enum
{
	AT = 0,//index=0
	AT_CNMI,
	AT_CSCA,
	AT_CMGF_0,
	AT_CMGF_1,
	AT_CMGD,//index=5
	AT_CPIN,
	AT_CSQ,
	AT_CGREG,
	AT_CGATT,
	AT_CIPMODE,//index=10
	AT_CSTT,
	AT_CIICR,
	AT_CIFSR,
	AT_CIPSHUT,
	AT_CIPSTATUS,//index=15
	AT_CIPSTART,
	AT_CMGS,
	AT_CMGS_SUFFIX,
	ATO,
	PLUS3,//index=20
	AT_CMMSINIT,
	AT_CMMSTERM,
	AT_CMMSCURL,
	AT_CMMSCID,
	AT_CMMSPROTO,//index=25
	AT_CMMSSENDCFG,
	AT_SAPBR_CONTYPE,
	AT_SAPBR_APN_CMWAP,
	AT_SAPBR_OPEN,
	AT_SAPBR_CLOSE,//index=30
	AT_SAPBR_REQUEST,
	AT_CMMSEDIT_OPEN,
	AT_CMMSEDIT_CLOSE,
	AT_CMMSDOWN_PIC,
	AT_CMMSDOWN_TITLE,//index=35
	AT_CMMSDOWN_TEXT,
	AT_CMMSDOWN_DATA,
	AT_CMMSRECP,
	AT_CMMSSEND,
	AT_CLCC,//index=40
	ATA,
	ATH5,
	AT_RING,
	AT_RECV_SMS,
	ATI,//index=45
	AT_GSV,
	AT_V,
	AT_D1,
	AT_W,
	AT_HTTPINIT,//index=50
	AT_HTTPTERM,
	AT_HTTPPARA_CID,
	AT_HTTPPARA_URL,
	AT_HTTPACTION_POST,
	AT_HTTPACTION_GET,//index=55
	AT_HTTPACTION_HEAD,
	AT_HTTPREAD,
	AT_SAPBR_APN_CMNET,
	AT_HTTPPARA_BREAK,
	AT_HTTPPARA_BREAKEND,//index=60
	AT_SIDET,
	AT_CMIC,
	AT_CLVL,
	AT_CHFA,
	AT_ECHO,//index=65
	AT_CPAS,
	AT_IFC,
	AT_IFC1,

}AT_COMMAND_INDEX_TYPEDEF;

typedef enum
{

	EVENT_GSM_MODE_GPRS = 0x01,// AT
	EVENT_GSM_MODE_CMD = 0x02,
	EVENT_GSM_MODE_GPRS_CMD = 0x04,
	EVENT_GSM_MODE_SETUP = 0x08,

}EVENT_GSM_MODE_TYPEDEF;


typedef enum {

	GSM_MODE_CMD,
	GSM_MODE_GPRS,
	GSM_MODE_CONTROL,

}GSM_MODE_TYPEDEF;



typedef struct
{
	uint8_t *request;
	uint16_t request_length;
}GSM_MAIL_GPRS;

typedef struct
{
	uint16_t length;
	uint16_t cmgs_length;
	uint8_t *buf;
}GSM_MAIL_CMD_CMGS;

typedef struct
{
	uint8_t *buf;
}GSM_MAIL_CMD_CMMSRECP;

typedef struct
{
	uint32_t length;
}GSM_MAIL_CMD_CMMSDOWN_PIC;

typedef struct
{
	uint32_t length;
}GSM_MAIL_CMD_CMMSDOWN_TITLE;

typedef struct
{
	uint32_t length;
}GSM_MAIL_CMD_CMMSDOWN_TEXT;

typedef struct
{
	uint32_t length;
	uint8_t *buf;
	uint8_t has_complete;
}GSM_MAIL_CMD_CMMSDOWN_DATA;

typedef struct
{
	uint32_t length;
	uint8_t *buf;
}GSM_MAIL_CMD_HTTPPARA_URL;
typedef struct
{
	uint32_t start;
	uint32_t *recv_counts;
	uint8_t *buf;
	uint32_t size_of_process;
}GSM_MAIL_CMD_HTTPREAD;

typedef struct
{
	uint32_t start;
}GSM_MAIL_CMD_HTTPPARA_BREAK;

typedef struct
{
	uint32_t end;
}GSM_MAIL_CMD_HTTPPARA_BREAKEND;

typedef struct
{
	rt_uint8_t	pos;
	rt_int8_t		*text_buf;
	rt_int8_t		*info_buf;
}GSM_MAIL_CMD_SMS_RCV;



typedef union
{
	GSM_MAIL_CMD_CMGS cmgs;
	GSM_MAIL_CMD_CMMSRECP cmmsrecp;
	GSM_MAIL_CMD_CMMSDOWN_PIC cmmsdown_pic;
	GSM_MAIL_CMD_CMMSDOWN_TITLE cmmsdown_title;
	GSM_MAIL_CMD_CMMSDOWN_TEXT cmmsdown_text;
	GSM_MAIL_CMD_CMMSDOWN_DATA cmmsdown_data;
	GSM_MAIL_CMD_HTTPPARA_URL httppara_url;
	GSM_MAIL_CMD_HTTPREAD httpread;
	GSM_MAIL_CMD_HTTPPARA_BREAK httppara_break;
	GSM_MAIL_CMD_HTTPPARA_BREAKEND httppara_breakend;
	GSM_MAIL_CMD_SMS_RCV sms_rcv;
}GSM_MAIL_CMD_DATA;

typedef struct
{
	AT_COMMAND_INDEX_TYPEDEF index;
	uint16_t delay;
	GSM_MAIL_CMD_DATA cmd_data;
}GSM_MAIL_CMD;

typedef enum
{
	GSM_CTRL_CLOSE = 0,
	GSM_CTRL_OPEN,
	GSM_CTRL_RESET,
	GSM_CTRL_DIALING,
	GSM_CTRL_SWITCH_TO_CMD,
	GSM_CTRL_SWITCH_TO_GPRS,
	GSM_CTRL_PHONE_CALL_ANSWER,
	GSM_CTRL_PHONE_CALL_HANG_UP,

}GSM_CONTROL_TYPEDEF;

typedef struct
{
    uint8_t *buf;
}GSM_MAIL_CTRL_DAILING;

typedef union
{
    GSM_MAIL_CTRL_DAILING dailing;
}GSM_MAIL_CONTROL_DATA;

typedef struct
{
	uint8_t cmd;
	GSM_MAIL_CONTROL_DATA data;
}GSM_MAIL_CONTROL;

typedef union
{
	GSM_MAIL_CMD cmd;
	GSM_MAIL_GPRS gprs;
	GSM_MAIL_CONTROL control;
}GSM_MAIL_DATA;

typedef enum
{
	GSM_EOK,
	GSM_EERROR,
}GSM_ERROR_TYPEDEF;

typedef enum {

	AT_RESPONSE_OK,
	AT_RESPONSE_ERROR,
	AT_RESPONSE_NO_CARRIER,
	AT_RESPONSE_TCP_CLOSED,
	AT_RESPONSE_CONNECT_OK,
	AT_RESPONSE_PDP_DEACT,
	AT_RESPONSE_PARTIAL_CONTENT,
	AT_RESPONSE_BAD_REQUEST,
	AT_RESPONSE_NO_MEMORY,
	AT_RESPONSE_READY,
	AT_RESPONSE_UNKNOW,
	AT_RESPONSE_RING,
	AT_RESPONSE_CALLING,
	AT_NO_RESPONSE,

}AT_RESPONSE_TYPEDEF;

typedef struct{

	GSM_MODE_TYPEDEF send_mode;
	GSM_MAIL_DATA mail_data;
	AT_RESPONSE_TYPEDEF *result;
	rt_sem_t result_sem;
	uint8_t flag;

}GSM_MAIL_TYPEDEF;

typedef enum
{
	GSM_STATUS_CLOSE,
	GSM_STATUS_OPEN,
	GSM_STATUS_GPRS,
}GSM_STATUS;

#define TCP_DOMAIN_LENGTH 20

typedef struct {

	char domain[TCP_DOMAIN_LENGTH];
	int32_t port;

}TCP_DOMAIN_TYPEDEF;

//typedef struct GSM_MAIL_TYPEDEF
extern char smsc[20];
extern char phone_call[20];
extern rt_mq_t gsm_mq;
extern rt_mutex_t mutex_gsm_mail_sequence;

GsmStatus gsm_reset(void);
GsmStatus gsm_setup(FunctionalState state);
void gsm_power(FunctionalState state);

void
gsm_thread_entry(void *parameters);

rt_uint32_t gsm_mode_get(void);
void gsm_mode_set(rt_uint32_t mode);
void gsm_put_char(const uint8_t *str, uint16_t length);
void gsm_put_hex(const uint8_t *str, uint16_t length);
void gsm_muntex_control(rt_uint8_t cmd,char *username);


AT_RESPONSE_TYPEDEF
gsm_phone_call_process(int type, uint8_t flag);
AT_RESPONSE_TYPEDEF
gsm_ring_process(uint8_t flag);

GSM_ERROR_TYPEDEF
send_gsm_sms_mail(uint8_t *buf, uint16_t length, uint8_t flag);

GSM_ERROR_TYPEDEF
send_gsm_ctrl_mail(u8 ctrl_cmd, uint8_t *buf, uint16_t length, uint8_t flag);
AT_RESPONSE_TYPEDEF
send_cmd_mail(AT_COMMAND_INDEX_TYPEDEF command_index, uint16_t delay, GSM_MAIL_CMD_DATA *cmd_data, u8 flag);
#endif
