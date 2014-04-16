#ifndef __NETMAILCLASS_H__
#define __NETMAILCLASS_H__
#include "netprotocol.h"

void send_net_landed_mail(void);

rt_uint8_t msg_mail_alarm(rt_uint8_t alarm,rt_uint8_t LockStatus,rt_uint32_t time);
rt_uint8_t msg_mail_fault(rt_uint8_t fault,rt_uint32_t time);
rt_uint8_t msg_mail_opendoor(rt_uint8_t type,rt_uint16_t key,rt_uint32_t time);
rt_uint8_t msg_mail_battery(rt_uint8_t status,rt_uint8_t capacity,rt_uint32_t time);
rt_uint8_t msg_mail_adjust_time(void);
rt_uint8_t msg_mail_alarmarg(rt_uint8_t Type,rt_uint8_t arg);
rt_uint8_t msg_mail_keyadd(net_keyadd_user *KeyData);

void msg_null_ack(message_type MSGType);


rt_uint8_t net_message_recv_process(net_recvmsg_p Mail,void *UserData);

#endif


