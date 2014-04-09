#ifndef __NETTERMINAL_H__
#define __NETTERMINAL_H__
#include "rtthread.h"
#include "netprotocol.h"

rt_err_t net_modify_alarm_arg(net_recvmsg_p mail);

rt_err_t net_motor_Control(net_recvmsg_p mail);

#endif
