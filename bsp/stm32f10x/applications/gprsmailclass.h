#ifndef __GPRSMAILCLASS_H__
#define __GPRSMAILCLASS_H__
#include "gprs.h"
#include "config.h"
#include "netprotocol.h"
#include "netmailclass.h"

// Կ����ӱ���
void gprs_Key_add_mail(rt_uint16_t pos);

// Կ����ȷ����
void gprs_key_right_mail(rt_uint16_t pos);

// Կ�״�����
void gprs_key_error_mail(rt_uint8_t type);

// �˻���ӱ���
void gprs_account_add_mail(rt_uint16_t pos);

// ӳ�����ϴ�����
void gprs_datamap_upload_mail(rt_uint8_t flag);

#endif



