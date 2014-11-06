#ifndef __NETTERMINAL_H__
#define __NETTERMINAL_H__
#include "rtthread.h"
#include "netprotocol.h"


rt_err_t net_modify_alarm_arg(net_recvmsg_p mail);

rt_err_t net_motor_Control(net_recvmsg_p mail);

rt_err_t net_set_system_time(net_recvmsg_p mail);

rt_err_t net_photograph(net_recvmsg_p mail);

rt_err_t net_data_sync(net_recvmsg_p mail);

rt_err_t net_accmapadd_result(net_recvmsg_p mail);

rt_err_t net_accdatcks_result(net_recvmsg_p mail);

rt_err_t net_keymapadd_result(net_recvmsg_p mail);

rt_err_t net_keydatcks_result(net_recvmsg_p mail);

rt_err_t net_phmapadd_result(net_recvmsg_p mail);

rt_err_t net_phdatcks_result(net_recvmsg_p mail);

rt_err_t net_recmapadd_result(net_recvmsg_p mail);

rt_err_t net_recdatcks_result(net_recvmsg_p mail);
#endif
