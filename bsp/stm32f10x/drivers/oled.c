/*********************************************************************
 * Filename:      oled.c
 *
 * Description:
 *
 * Author:        wangzw <wangzw@yuettak.com>
 * Created at:    2013-04-25 15:21:03
 *
 * Modify:
 *
 * 2013-04-25 Bright Pan <loststriker@gmail.com>
 *
 * Copyright (C) 2013 Yuettak Co.,Ltd
 ********************************************************************/

#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>
#include "oled.h"

static rt_err_t 
rt_oled_init(struct rt_device *dev)
{

	rt_err_t result = RT_EOK;
	struct oled_device *oled = RT_NULL;

	RT_ASSERT(dev != RT_NULL);
	oled = (struct oled_device *)dev;

	if (!(dev->flag & RT_DEVICE_FLAG_ACTIVATED))//uninitiated
	{
		/* apply configuration */
		if (oled->ops->configure)
		{
			result = oled->ops->configure(oled);
		}
		if (result != RT_EOK)
		{
			return result;
		}
		/* set activated */
		dev->flag |= RT_DEVICE_FLAG_ACTIVATED;
	}
	return result;
}
/*
 * oled open
 */
static rt_err_t 
rt_oled_open(struct rt_device *dev,rt_uint16_t oflag)
{
	rt_err_t result = RT_EOK;
	rt_uint32_t int_flags = 0;
	struct oled_device *oled;

	RT_ASSERT(dev != RT_NULL);
	oled = (struct oled_device *)dev;

	if(dev->flag & RT_DEVICE_FLAG_INT_RX) //interrupt intpur
	{
		int_flags |= RT_DEVICE_FLAG_INT_RX;
	}
	if (int_flags)
	{
		oled->ops->control(oled, RT_DEVICE_CTRL_SET_INT, (void *)int_flags);
	}

	return result;
}
/*
 * oled close
 */
static rt_err_t 
rt_oled_close(struct rt_device *dev)
{
	struct oled_device *oled = RT_NULL;
	rt_uint32_t int_flags = 0;

	RT_ASSERT(dev != RT_NULL);
	oled = (struct oled_device *)dev;

	if (dev->flag & RT_DEVICE_FLAG_INT_RX)
	{
		int_flags |= RT_DEVICE_FLAG_INT_RX;
	}
	if (int_flags)
	{
		oled->ops->control(oled, RT_DEVICE_CTRL_CLR_INT, (void *)int_flags);
	}

	return RT_EOK;
}
/*
 * oled read
 */
static rt_size_t rt_oled_read(struct rt_device *dev,
                              rt_off_t          pos,
                              void             *buffer,
                              rt_size_t         size )
{
	struct oled_device *oled = RT_NULL;
	RT_ASSERT(dev != RT_NULL);

	oled = (struct oled_device *)dev;
	*(rt_uint8_t *)buffer = oled->ops->intput(oled);

	return RT_EOK;
}
/*
 * oled write
 */
static rt_size_t rt_oled_write(struct rt_device *dev,
							   rt_off_t pos,
							   const void *buffer,
							   rt_size_t size)
{
	struct oled_device *oled = RT_NULL;

	RT_ASSERT(dev != RT_NULL);
	oled = (struct oled_device *)dev;
	oled->ops->out(oled,*(rt_uint8_t *)buffer);

	return RT_EOK;
}
/*
 * oled coltrol
 */
static rt_err_t rt_oled_control(struct rt_device *dev,
								rt_uint8_t cmd,
								void *args)
{
	struct oled_device *oled = RT_NULL;

	RT_ASSERT(dev != RT_NULL);
	oled = (struct oled_device *)dev;

	switch (cmd)
	{
		case RT_DEVICE_CTRL_SUSPEND:
			{
				/* suspend device */
				dev->flag |= RT_DEVICE_FLAG_SUSPENDED;
				break;
			}
		case RT_DEVICE_CTRL_RESUME:
			{
				/* resume device */
				dev->flag &= ~RT_DEVICE_FLAG_SUSPENDED;
				break;
			}
		case RT_DEVICE_CTRL_CONFIG:
			{
				/* configure device */
				oled->ops->configure(oled);
				break;
			}
		default:
			{
				/* configure device */
				oled->ops->control(oled, cmd, args);
				break;
			}
	}
	return RT_EOK;
}

/*
 * serial register
 */
rt_err_t rt_hw_oled_register(struct oled_device *oled,
                             const char *name,
                             rt_uint32_t flag,
                             void *data)
{
    struct rt_device *device = RT_NULL;
    RT_ASSERT(oled != RT_NULL);

    device = &(oled->parent);

    device->type        = RT_Device_Class_Char;
    device->rx_indicate = RT_NULL;
    device->tx_complete = RT_NULL;

    device->init 		= rt_oled_init;
    device->open   = rt_oled_open;
    device->close 	= rt_oled_close;
    device->read    = rt_oled_read;
    device->write    = rt_oled_write;
    device->control    	= rt_oled_control;
    device->user_data   = data;
	device->init(device);
    /* register a character device */
    return rt_device_register(device, name, flag);
}

/*
 * oled interrupt input deal
 */
void rt_hw_oled_isr(struct oled_device *oled)
{
    /* interrupt mode receive */
    RT_ASSERT(oled->parent.flag & RT_DEVICE_FLAG_INT_RX);

    oled->pin_value = oled->ops->intput(oled);
    if(oled->parent.rx_indicate != RT_NULL)
    {
        oled->parent.rx_indicate(&(oled->parent), 1);
    }
}


