#ifndef __NET_FILE_H__
#define __NET_FILE_H__
#include "netprotocol.h"

#define USEING_FILE_API

typedef enum 
{
	NET_FILE_BIN = 0,
	NET_FILE_PIC = 1,
}NET_FILE_TYPE;

typedef struct 
{
	char name[RT_NAME_MAX];
	rt_uint8_t AlarmType;
	rt_uint8_t FileType;
	rt_uint32_t time;
}UploadFile,*UploadFile_p;

rt_uint8_t net_recv_filerq_process(net_recvmsg_p mail);
rt_uint8_t net_file_packdata_process(net_recvmsg_p mail);
void net_file_timer_process(void);

//upload file
rt_err_t net_upload_file(UploadFile_p File);

void net_upload_complete_Callback(void (*Callback)(void *user));


#endif


