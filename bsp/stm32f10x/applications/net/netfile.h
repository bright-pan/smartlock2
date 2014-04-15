#ifndef __NET_FILE_H__
#define __NET_FILE_H__
#include "netprotocol.h"

rt_uint8_t net_recv_filerq_process(net_recvmsg_p mail);
rt_uint8_t net_file_packdata_process(net_recvmsg_p mail);
void net_file_timer_process(void);

//upload file
rt_err_t net_upload_file(char *FileName);

void net_upload_complete_Callback(void (*Callback)(void));


#endif


