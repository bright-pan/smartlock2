#ifndef __BDCOM_H__
#define __BDCOM_H__
#include "rtthread.h"

typedef enum 
{
	GSM_MAIL_SMS,
	GSM_MAIL_MMS,
	GSM_MAIL_GPRS,
	GSM_MAIL_LINK
}GSM_MailType;

typedef struct 
{
	GSM_MailType type;
	rt_uint8_t   *buf;
	rt_size_t    BufSize;
	rt_sem_t     ResultSem;
	rt_uint8_t   SendMode;
}GSM_Mail,*GSM_Mail_p;

void gsm_set_link(rt_uint8_t status);

rt_bool_t gsm_is_setup(void);

rt_bool_t gsm_is_link(void);

void gsm_mail_send(GSM_Mail_p mail);


#endif

