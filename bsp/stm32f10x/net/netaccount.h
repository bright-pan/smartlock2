#ifndef __NETACCOUNT_H__
#define __NETACCOUNT_H__
#include "rtthread.h"
#include "netprotocol.h"
#include "config.h"

//网络添加账户处理
rt_err_t net_account_add_process(net_recvmsg_p mail);

//网络删除账户处理

rt_err_t net_account_del_process(net_recvmsg_p mail)
;


#endif


