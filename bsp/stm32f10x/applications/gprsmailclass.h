#ifndef __GPRSMAILCLASS_H__
#define __GPRSMAILCLASS_H__
#include "gprs.h"
#include "config.h"

void gprs_Key_add_mail(rt_uint16_t pos);

void gprs_key_right_mail(rt_uint16_t pos);

void gprs_key_error_mail(rt_uint8_t type);

void gprs_account_add_mail(rt_uint16_t pos);


#endif



