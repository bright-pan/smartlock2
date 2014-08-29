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

#define bits_mask(x) (1<<(x))

u32
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

#endif
