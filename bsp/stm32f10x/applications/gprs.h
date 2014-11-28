#ifndef __GPRS_H__
#define __GPRS_H__
#include "rtthread.h"
#include <time.h>
#include "alarm.h"

/* ����Կ�״������� */
typedef struct
{
	rt_uint8_t type;
}GPRS_KeyErrDef,*GPRS_KeyErrDef_p;

/* ����Կ����ȷ���� */
typedef struct
{
	rt_uint16_t  pos;
	rt_uint8_t   type;
}GPRS_KeyRightDef,*GPRS_KeyRightDef_p;

/* �����˻�������� */
typedef struct 
{
	rt_uint16_t pos;
	rt_uint8_t  name[20];
	rt_uint32_t date;
}GPRS_AccountAddDef,*GPRS_AccountAddDef_p;

/* ����Կ��������� */
typedef struct 
{
	rt_uint16_t pos;
	rt_uint16_t	auth;
	rt_uint8_t  code[12];
	rt_uint32_t date;
}GPRS_PhoneAddDef,*GPRS_PhoneAddDef_p;

/* ����ӳ�����ϴ� */
typedef struct
{
	rt_uint32_t MapType;
}GPRS_DataMapDef,*GPRS_DataMapDef_p;

/* gprs Ӧ�ò��û�˽�����ݼ��ϣ���gprs�߳�ʹ�� */
typedef union
{
	GPRS_KeyErrDef        keyerr;				//Կ�״���
	GPRS_KeyRightDef      keyright;     //Կ�׿�����ȷ
	GPRS_AccountAddDef    AccountAdd;   //Կ�����
	GPRS_PhoneAddDef      PhoneAdd;     //�ֻ����
	GPRS_DataMapDef       MapUpload;    //ӳ�����ϴ�
}GPRSUserDef,*GPRSUserDef_p;

/* gprs Ӧ�ò����ݳ���ӿڣ��ṩ��Ӧ�ò�ʹ�� */
typedef struct
{
  time_t time;
  ALARM_TYPEDEF alarm_type;
  void* user;
}GPRS_MAIL_TYPEDEF;


#define GPRS_EVT_ALLOW1_DATSYNC      	 (0X01<<0)//��������ͬ������1  ������
#define GPRS_EVT_ALLOW2_DATSYNC       (0X01<<1)//��������ͬ������2   û�н����ļ��޸�
#define GPRS_EVT_SYNS_ALLDAT           (0X01<<2)//����ͬ����������
#define GPRS_EVT_SYNC_DATMODE1         (0X01<<3)//���г�ӳ����֮�������ͬ��

void send_gprs_mail(ALARM_TYPEDEF AlarmType,time_t time,void *user);

void set_all_update_flag(rt_uint8_t flag);

rt_uint8_t gprs_event_process(rt_uint8_t mode,rt_uint32_t type);

#endif

