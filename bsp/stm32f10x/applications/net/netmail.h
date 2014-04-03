#ifndef __NETMAIL_H__
#define __NETMALL_H__
#include "netprotocol.h"
#include "netmailclass.h"

/**
�����ʼ�����
*/
typedef enum
{
	MSGTYPE_WORK_ALARM,
	MSGTYPE_FAULT_ALARM,
	MSGTYPE_NULL_ACK
}ALARM_TYPEDEF;



//��������
typedef struct 
{
	rt_uint8_t AlarmType;
	rt_uint8_t LockStatus;
}NetMsg_Alarm;

//����
typedef struct 
{
	rt_uint16_t KeyCode;
}NetMsg_OpenDoor;

//���
typedef struct 
{
	rt_uint8_t status;
	rt_uint8_t capacity;
}NetMsg_BAT;

//������Ӧ��
typedef struct 
{
	message_type MSGType;
}NetMsg_NullACK;

//���ֱ�������
typedef union 
{
	NetMsg_Alarm     alarm;
	NetMsg_OpenDoor  opendoor;
	NetMsg_BAT       Battery;
	NetMsg_NullACK   NullACK;
}NetMsg_UserData;

//gprs�ʼ�
typedef struct 
{
	rt_uint8_t    MsgType;//�����ʼ�����
	rt_uint32_t   time;
	NetMsg_UserData data;
}NetMsg_Mail,*NetMsg_Mail_p;


void NetMsg_Mail_Send(NetMsg_Mail_p data);


#endif


