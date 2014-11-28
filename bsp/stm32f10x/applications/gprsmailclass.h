#ifndef __GPRSMAILCLASS_H__
#define __GPRSMAILCLASS_H__
#include "gprs.h"
#include "config.h"
#include "netprotocol.h"
#include "netmailclass.h"

// 钥匙添加报文
void gprs_Key_add_mail(rt_uint16_t pos);

// 钥匙正确报文
void gprs_key_right_mail(rt_uint16_t pos);

// 钥匙错误报文
void gprs_key_error_mail(rt_uint8_t type);

// 账户添加报文
void gprs_account_add_mail(rt_uint16_t pos);

// 映射域上传报文
void gprs_datamap_upload_mail(rt_uint8_t flag);

#endif



