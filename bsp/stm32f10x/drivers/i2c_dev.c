/*
 * File      : i2c_dev.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2012, RT-Thread Development Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Change Logs:
 * Date           Author       Notes
 */

#include <rtthread.h>
#include "i2c.h"

/* I2C bus device interface, compatible with RT-Thread 0.3.x/1.0.x */
static rt_err_t _i2c_bus_device_init(rt_device_t dev)
{
    struct rt_i2c_bus *bus;

    bus = (struct rt_i2c_bus *)dev;
    RT_ASSERT(bus != RT_NULL);

    return RT_EOK;
}

static rt_size_t _i2c_bus_device_read(rt_device_t dev,
                                      rt_off_t    pos,
                                      void       *buffer,
                                      rt_size_t   size)
{
    struct rt_i2c_bus *bus;

    bus = (struct rt_i2c_bus *)dev;
    RT_ASSERT(bus != RT_NULL);
    RT_ASSERT(bus->owner != RT_NULL);

    return size;
}

static rt_size_t _i2c_bus_device_write(rt_device_t dev,
                                       rt_off_t    pos,
                                       const void *buffer,
                                       rt_size_t   size)
{
    struct rt_i2c_bus *bus;

    bus = (struct rt_i2c_bus *)dev;
    RT_ASSERT(bus != RT_NULL);
    RT_ASSERT(bus->owner != RT_NULL);

    return size;
}

static rt_err_t _i2c_bus_device_control(rt_device_t dev,
                                        rt_uint8_t  cmd,
                                        void       *args)
{
    struct rt_i2c_bus *bus;

    bus = (struct rt_i2c_bus *)dev;
    RT_ASSERT(bus != RT_NULL);

    switch (cmd)
    {
		case 0: /* set device */
			break;
		case 1:
			break;
    }

    return RT_EOK;
}

rt_err_t rt_i2c_bus_device_init(struct rt_i2c_bus *bus, const char *name)
{
    struct rt_device *device;
    RT_ASSERT(bus != RT_NULL);

    device = &bus->parent;

    /* set device type */
    device->type    = RT_Device_Class_I2CBUS;
    /* initialize device interface */
    device->init    = _i2c_bus_device_init;
    device->open    = RT_NULL;
    device->close   = RT_NULL;
    device->read    = _i2c_bus_device_read;
    device->write   = _i2c_bus_device_write;
    device->control = _i2c_bus_device_control;

    /* register to device manager */
    return rt_device_register(device, name, RT_DEVICE_FLAG_RDWR);
}

/* I2C Dev device interface, compatible with RT-Thread 0.3.x/1.0.x */
static rt_err_t _i2cdev_device_init(rt_device_t dev)
{
    struct rt_i2c_device *device;

    device = (struct rt_i2c_device *)dev;
    RT_ASSERT(device != RT_NULL);

    return RT_EOK;
}

static rt_size_t _i2cdev_device_read(rt_device_t dev,
									  rt_off_t    pos,
									  void       *buffer,
									  rt_size_t   size)
{
    struct rt_i2c_device *device;

    device = (struct rt_i2c_device *)dev;
    RT_ASSERT(device != RT_NULL);
    RT_ASSERT(device->bus != RT_NULL);

    return size;
}

static rt_size_t _i2cdev_device_write(rt_device_t dev,
									   rt_off_t    pos,
									   const void *buffer,
									   rt_size_t   size)
{
    struct rt_i2c_device *device;

    device = (struct rt_i2c_device *)dev;
    RT_ASSERT(device != RT_NULL);
    RT_ASSERT(device->bus != RT_NULL);

    return size;
}

static rt_err_t _i2cdev_device_control(rt_device_t dev,
										rt_uint8_t  cmd,
										void       *args)
{
    struct rt_i2c_device *device;

    device = (struct rt_i2c_device *)dev;
    RT_ASSERT(device != RT_NULL);

    switch (cmd)
    {
		case 0: /* set device */
			break;
		case 1:
			break;
    }

    return RT_EOK;
}

rt_err_t rt_i2cdev_device_init(struct rt_i2c_device *dev, const char *name)
{
    struct rt_device *device;
    RT_ASSERT(dev != RT_NULL);

    device = &(dev->parent);

    /* set device type */
    device->type    = RT_Device_Class_I2CDevice;
    device->init    = _i2cdev_device_init;
    device->open    = RT_NULL;
    device->close   = RT_NULL;
    device->read    = _i2cdev_device_read;
    device->write   = _i2cdev_device_write;
    device->control = _i2cdev_device_control;

    /* register to device manager */
    return rt_device_register(device, name, RT_DEVICE_FLAG_RDWR);
}
