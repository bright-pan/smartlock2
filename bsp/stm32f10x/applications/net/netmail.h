#ifndef __NETMAIL_H__
#define __NETMALL_H__
#include "netprotocol.h"
#include "netmailclass.h"

/**
报文邮件类型
*/
typedef enum
{
	MSGTYPE_WORK_ALARM,
	MSGTYPE_FAULT_ALARM,
	MSGTYPE_NULL_ACK
}ALARM_TYPEDEF;



//工作报警
typedef struct 
{
	rt_uint8_t AlarmType;
	rt_uint8_t LockStatus;
}NetMsg_Alarm;

//开门
typedef struct 
{
	rt_uint16_t KeyCode;
}NetMsg_OpenDoor;

//电池
typedef struct 
{
	rt_uint8_t status;
	rt_uint8_t capacity;
}NetMsg_BAT;

//空数据应答
typedef struct 
{
	message_type MSGType;
}NetMsg_NullACK;

//各种报文数据
typedef union 
{
	NetMsg_Alarm     alarm;
	NetMsg_OpenDoor  opendoor;
	NetMsg_BAT       Battery;
	NetMsg_NullACK   NullACK;
}NetMsg_UserData;

//gprs邮件
typedef struct 
{
	rt_uint8_t    MsgType;//网络邮件类型
	rt_uint32_t   time;
	NetMsg_UserData data;
}NetMsg_Mail,*NetMsg_Mail_p;


void NetMsg_Mail_Send(NetMsg_Mail_p data);


#endif


