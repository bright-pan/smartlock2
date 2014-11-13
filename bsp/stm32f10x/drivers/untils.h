/*********************************************************************
 * Filename:      untils.h
 *
 *
 * Author:        Bright Pan <loststriker@gmail.com>
 * Created at:    2013-05-22 09:26:03
 *
 *
 *
 * Change Log:
 *
 * Copyright (C) 2013 Yuettak Co.,Ltd
 ********************************************************************/

#ifndef _UNTILS_H_
#define _UNTILS_H_

#include <rtthread.h>
#include <rthw.h>
#include <stm32f10x.h>
#include <time.h>
#include "debug_manage.h"

#define bits_mask(x) (1<<(x))

//映射字节数据定义
typedef struct 
{
	rt_uint8_t *data;  			//映射域数据
	rt_size_t  ByteSize;   	//数组大小
	rt_size_t  BitMaxNum; 	//位数的最大值
}MapByteDef,*MapByteDef_p;


MapByteDef_p map_byte_create(rt_size_t BitMaxNum);

void map_byte_delete(MapByteDef_p map);

void map_byte_bit_set(MapByteDef_p Map,rt_size_t Bit,rt_bool_t data);

rt_bool_t map_byte_bit_get(MapByteDef_p Map,rt_size_t Bit);

time_t
sys_cur_date(void);

void
print_hex(void *, u16);
void
print_char(void *, u16);
void
delay_us(u32);

rt_device_t
device_enable(const char *);

#ifndef __GNUC__
void *
memmem(const void *haystack,
	   rt_size_t haystack_len,
	   const void *needle,
	   rt_size_t needle_len);
#endif

void sysinit(void);

#endif
