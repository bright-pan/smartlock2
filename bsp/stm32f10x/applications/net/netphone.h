#ifndef __NETPHTONE_H__
#define __NETPHTONE_H__
#include "rtthread.h"
#include "netprotocol.h"


rt_err_t net_phone_add_process(net_recvmsg_p mail);

rt_err_t net_phone_del_process(net_recvmsg_p mail);


#endif
