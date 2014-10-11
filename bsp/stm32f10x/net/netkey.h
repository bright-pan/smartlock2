#ifndef __NETKEY_H__
#define __NETKEY_H__
#include "rtthread.h"
#include "netprotocol.h"
#include "config.h"

rt_err_t net_key_add_process(net_recvmsg_p mail);

rt_err_t net_key_del_process(net_recvmsg_p mail);

#endif

