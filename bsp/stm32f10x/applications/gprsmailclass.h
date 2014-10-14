#ifndef __GPRSMAILCLASS_H__
#define __GPRSMAILCLASS_H__
#include "gprs.h"
#include "config.h"

//钥匙正确私有数据
typedef struct
{
	rt_uint16_t  keypos;
	rt_uint8_t   keytype;
}GPRS_KeyRightUser,*GPRS_KeyRightUser_p;

//钥匙错误私有数据
typedef struct
{
	rt_uint8_t   type;
}GPRS_KeyErrorUser,*GPRS_KeyErrorUser_p;
#endif

void gprs_key_right_mail(rt_uint16_t pos);

void gprs_key_error_mail(rt_uint8_t type);

