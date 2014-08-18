/*********************************************************************
 * Filename:      oled.h
 *
 * Description:
 *
 * Author:        wangzw <wangzw@yuettak.com>
 * Created at:    2013-04-22
 *
 * Modify:
 *
 * 2013-04-25 Bright Pan <loststriker@gmail.com>
 *
 * Copyright (C) 2013 Yuettak Co.,Ltd
 ********************************************************************/

#ifndef _OLED_H_
#define _OLED_H_

#include <rtthread.h>
#include <rtdevice.h>
#include <rthw.h>

#define RT_DEVICE_CTRL_CONFIG           0x03    /* configure device */
#define RT_DEVICE_CTRL_SET_INT          0x10    /* enable receive irq */
#define RT_DEVICE_CTRL_CLR_INT          0x11    /* disable receive irq */

struct oled_device;

/*
 *	oled user data
 */
struct rt_oled_ops
{
	rt_err_t (*configure)(struct oled_device *oled);
	rt_err_t (*control)(struct oled_device *oled, rt_uint8_t cmd, void *arg);
	void  (*out)(struct oled_device *oled, rt_uint8_t c);
	rt_uint8_t (*intput)(struct oled_device *oled);
};

/*
 *	struct oled_device data stuct
 */
struct oled_device
{
	struct rt_device parent;
	const struct rt_oled_ops *ops;
	rt_uint8_t pin_value;
};

rt_err_t rt_hw_oled_register(struct oled_device *oled,
                             const char *name,
                             rt_uint32_t flag,
                             void *data);
void rt_hw_oled_isr(struct oled_device *oled);

#endif
