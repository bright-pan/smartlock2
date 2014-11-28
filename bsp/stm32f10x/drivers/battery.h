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



typedef	struct 
{
	rt_uint16_t adc_value;  //adc��ֵ
	rt_uint8_t	DumpEnergy;	//ʣ�����
	float       voltage;    //��⵽�ĵ�ѹֵ
}Battery_Data;

void battery_get_data(Battery_Data* data);

#endif
