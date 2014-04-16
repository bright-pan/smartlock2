#ifndef __GPRS_H__
#define __GPRS_H__
#include "rtthread.h"
#include <time.h>
#include "alarm.h"


typedef struct
{

  time_t time;
  ALARM_TYPEDEF alarm_type;
  void* user;
}GPRS_MAIL_TYPEDEF;

void send_gprs_mail(ALARM_TYPEDEF AlarmType,time_t time,void *user);

#endif

