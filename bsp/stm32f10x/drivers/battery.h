/*********************************************************************
 * Filename:      battery.h
 * 
 *
 * Author:        Bright Pan <loststriker@gmail.com>
 * Created at:    2013-06-08 11:18:50
 *
 *
 *
 * Change Log:    
 *
 * Copyright (C) 2013 Yuettak Co.,Ltd
 ********************************************************************/

#ifndef _BATTERY_H_
#define _BATTERY_H_

#include <rtthread.h>
#include <rthw.h>
#include <stm32f10x.h>

typedef enum
{
	BAT_MAILTYPE_SAMPLE0,			//��ص���
	BAT_MAILTYPE_LowEnergy,		//�������ͼ��
}BatTypeDef;

typedef	struct 
{
	rt_uint16_t adc_value;  //adc��ֵ
	rt_uint8_t	DumpEnergy;	//ʣ�����
	float       voltage;    //��⵽�ĵ�ѹֵ
}Battery_Data;

typedef struct 
{
	BatTypeDef Type;	//�ʼ�����
	void (*result)(void *arg);//�ʼ�ִ�н��
}BatteryMailDef,*BatteryMailDef_p;

void battery_get_data(Battery_Data* data);

// ����ع����̷߳����ʼ�
rt_err_t battery_send_mail(BatteryMailDef_p mail);

void battery_low_energy_check(void);

#endif
