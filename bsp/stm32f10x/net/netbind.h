#ifndef __NETBIND_H__
#define __NETBIND_H_
#include "rtthread.h"
#include "netprotocol.h"
#include "config.h"

//�����Կ�״���
rt_err_t net_bind_key_process(net_recvmsg_p mail);

//������ֻ�����
rt_err_t net_bind_phone_process(net_recvmsg_p mail);


#endif

