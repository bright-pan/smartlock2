#ifndef __GPRS_H__
#define __GPRS_H__
#include "rtthread.h"
#include <time.h>
#include "alarm.h"

typedef struct
{
	rt_uint8_t type;
}GPRS_KeyErrDef,*GPRS_KeyErrDef_p;

typedef struct
{
	rt_uint16_t  pos;
	rt_uint8_t   type;
}GPRS_KeyRightDef,*GPRS_KeyRightDef_p;

typedef struct 
{
	rt_uint16_t pos;
	rt_uint8_t  name[20];
	rt_uint32_t date;
}GPRS_AccountAddDef,*GPRS_AccountAddDef_p;

typedef struct 
{
	rt_uint16_t pos;
	rt_uint16_t	auth;
	rt_uint8_t  code[12];
	rt_uint32_t date;
}GPRS_PhoneAddDef,*GPRS_PhoneAddDef_p;

typedef union
{
	GPRS_KeyErrDef        keyerr;
	GPRS_KeyRightDef      keyright;
	GPRS_AccountAddDef    AccountAdd;
	GPRS_PhoneAddDef      PhoneAdd;
}GPRSUserDef,*GPRSUserDef_p;

typedef struct
{

  time_t time;
  ALARM_TYPEDEF alarm_type;
  void* user;
}GPRS_MAIL_TYPEDEF;


#define GPRS_EVT_ALLOW1_DATSYNC      	 (0X01<<0)//允许数据同步条件1  已连接
#define GPRS_EVT_ALLOW2_DATSYNC       (0X01<<1)//允许数据同步条件2   没有进行文件修改
#define GPRS_EVT_SYNS_ALLDAT           (0X01<<2)//进行同步所有数据
#define GPRS_EVT_SYNC_DATMODE1         (0X01<<3)//进行除映射域之外的数据同步

void send_gprs_mail(ALARM_TYPEDEF AlarmType,time_t time,void *user);

void set_all_update_flag(rt_uint8_t flag);

rt_uint8_t gprs_event_process(rt_uint8_t mode,rt_uint32_t type);

#endif

