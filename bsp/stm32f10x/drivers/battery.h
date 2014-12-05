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
	BAT_MAILTYPE_SAMPLE0,			//电池电量
	BAT_MAILTYPE_LowEnergy,		//电量过低检测
}BatTypeDef;

typedef	struct 
{
	rt_uint16_t adc_value;  //adc的值
	rt_uint8_t	DumpEnergy;	//剩余电量
	float       voltage;    //检测到的电压值
}Battery_Data;

typedef struct 
{
	BatTypeDef Type;	//邮件类型
	void (*result)(void *arg);//邮件执行结果
}BatteryMailDef,*BatteryMailDef_p;

void battery_get_data(Battery_Data* data);

// 给电池管理线程发送邮件
rt_err_t battery_send_mail(BatteryMailDef_p mail);

void battery_low_energy_check(void);

#endif
