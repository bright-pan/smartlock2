#ifndef __NETACCOUNT_H__
#define __NETACCOUNT_H__
#include "rtthread.h"
#include "netprotocol.h"
#include "config.h"

//��������˻�����
rt_err_t net_account_add_process(net_recvmsg_p mail);

//����ɾ���˻�����

rt_err_t net_account_del_process(net_recvmsg_p mail)
;


#endif


