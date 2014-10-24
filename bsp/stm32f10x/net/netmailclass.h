#ifndef __NETMAILCLASS_H__
#define __NETMAILCLASS_H__
#include "netprotocol.h"

#define USEING_MOTOR_API 
#define USEING_KEY_API
//#define USEING_SYS_UPDATE
//#define USEING_SYSCONFIG_API
#define USEING_SYS_TIME_API
//#define USEING_CAMERA_API
//#define USEING_FILE_API
#define USEING_PHONE_API
#define USEING_ACCOUNT_API
#define USEING_BIND_API


#ifdef USEING_MOTOR_API
#include "netterminal.h"
#include "local.h"
#endif

#ifdef USEING_KEY_API
#include "netkey.h"
#endif

#ifdef USEING_PHONE_API
#include "netphone.h"
#endif

#ifdef USEING_ACCOUNT_API
#include "netaccount.h"
#endif

#ifdef USEING_BIND_API
#include "netbind.h"
#endif

void send_net_landed_mail(void);
void net_mail_heart(void);


rt_err_t msg_mail_alarm(rt_uint8_t alarm,rt_uint8_t LockStatus,rt_uint32_t time);
rt_err_t msg_mail_fault(rt_uint8_t fault,rt_uint32_t time);
rt_err_t msg_mail_opendoor(rt_uint8_t type,rt_uint16_t account,rt_uint16_t key,rt_uint32_t time);
rt_err_t msg_mail_battery(rt_uint8_t status,rt_uint8_t capacity,rt_uint32_t time);
rt_err_t msg_mail_adjust_time(void);
rt_err_t msg_mail_alarmarg(rt_uint8_t Type,rt_uint8_t arg);
rt_err_t msg_mail_keyadd(net_keyadd_user *KeyData);
rt_err_t msg_mail_keydelete(rt_uint16_t pos,rt_uint32_t date);
rt_err_t msg_mail_phoneadd(rt_uint16_t PhID,rt_uint16_t flag,rt_uint8_t buf[],rt_uint32_t date);
rt_err_t msg_mail_phondel(rt_uint16_t PhID,rt_uint32_t date);
rt_err_t msg_mail_account_add(rt_int16_t account_pos,rt_uint8_t *name,rt_uint32_t date);
rt_err_t msg_mail_account_del(rt_int16_t account_pos,rt_uint32_t date);
rt_err_t msg_mail_keybind(rt_uint16_t key_pos,rt_uint16_t account_pos,rt_uint32_t date);
rt_err_t msg_mail_phonebind(rt_uint16_t phone_pos,rt_uint16_t account_pos,rt_uint32_t date);


void msg_null_ack(message_type MSGType);


rt_uint8_t net_message_recv_process(net_recvmsg_p Mail,void *UserData);


#endif


