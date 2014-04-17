/*
 * File      : i2c_core.c
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
 * 2012-01-08     bernard      first version.
 * 2012-02-03     bernard      add const attribute to the ops.
 * 2012-05-15     dzzxzz       fixed the return value in attach_device.
 * 2012-05-18     bernard      Changed I2C message to message list.
 *                             Added take/release I2C device/bus interface.
 * 2012-09-28     aozima       fixed rt_i2c_release_bus assert error.
 */

#include "i2c.h"

extern rt_err_t rt_i2c_bus_device_init(struct rt_i2c_bus *bus, const char *name);
extern rt_err_t rt_i2cdev_device_init(struct rt_i2c_device *dev, const char *name);

rt_err_t rt_i2c_bus_register(struct rt_i2c_bus       *bus,
                             const char              *name,
                             const struct rt_i2c_ops *ops)
{
    rt_err_t result;

    result = rt_i2c_bus_device_init(bus, name);
    if (result != RT_EOK)
        return result;

    /* initialize mutex lock */
    rt_mutex_init(&(bus->lock), name, RT_IPC_FLAG_FIFO);
    /* set ops */
    bus->ops = ops;
    /* initialize owner */
    bus->owner = RT_NULL;

    return RT_EOK;
}

rt_err_t rt_i2c_bus_attach_device(struct rt_i2c_device *device,
                                  const char           *name,
                                  const char           *bus_name,
                                  void                 *user_data)
{
    rt_err_t result;
    rt_device_t bus;

    /* get physical i2c bus */
    bus = rt_device_find(bus_name);
    if (bus != RT_NULL && bus->type == RT_Device_Class_I2CBUS)
    {
        device->bus = (struct rt_i2c_bus *)bus;

        /* initialize i2cdev device */
        result = rt_i2cdev_device_init(device, name);
        if (result != RT_EOK)
            return result;

        rt_memset(&device->config, 0, sizeof(device->config));
        device->parent.user_data = user_data;

        return RT_EOK;
    }

    /* not found the host bus */
    return -RT_ERROR;
}

rt_err_t rt_i2c_configure(struct rt_i2c_device        *device,
                          struct rt_i2c_configuration *cfg)
{
    rt_err_t result;

    RT_ASSERT(device != RT_NULL);

    /* set configuration */
    device->config.addr_width = cfg->addr_width;
    device->config.speed       = cfg->speed;

    if (device->bus != RT_NULL)
    {
        result = rt_mutex_take(&(device->bus->lock), RT_WAITING_FOREVER);
        if (result == RT_EOK)
        {
            if (device->bus->owner == device)
            {
                device->bus->ops->configure(device, &device->config);
            }

            /* release lock */
            rt_mutex_release(&(device->bus->lock));
        }
    }

    return RT_EOK;
}

//rt_size_t rt_i2c_transfer(struct rt_i2c_device *device,
//                          const void           *send_buf,
//                          void                 *recv_buf,
//                          rt_size_t             length)
//{
//    rt_err_t result;
//    struct rt_i2c_message message;

//    RT_ASSERT(device != RT_NULL);
//    RT_ASSERT(device->bus != RT_NULL);

//    result = rt_mutex_take(&(device->bus->lock), RT_WAITING_FOREVER);
//    if (result == RT_EOK)
//    {
//        if (device->bus->owner != device)
//        {
//            /* not the same owner as current, re-configure I2C bus */
//            result = device->bus->ops->configure(device, &device->config);
//            if (result == RT_EOK)
//            {
//                /* set I2C bus owner */
//                device->bus->owner = device;
//            }
//            else
//            {
//                /* configure I2C bus failed */
//                rt_set_errno(-RT_EIO);
//                result = 0;
//                goto __exit;
//            }
//        }

//        /* initial message */
//        message.send_buf   = send_buf;
//        message.recv_buf   = recv_buf;
//        message.length     = length;
//        message.cs_take    = 1;
//        message.cs_release = 1;
//        message.next       = RT_NULL;

//        /* transfer message */
//        result = device->bus->ops->xfer(device, &message);
//        if (result == 0)
//        {
//            rt_set_errno(-RT_EIO);
//            goto __exit;
//        }
//    }
//    else
//    {
//        rt_set_errno(-RT_EIO);

//        return 0;
//    }

//__exit:
//    rt_mutex_release(&(device->bus->lock));

//    return result;
//}

//struct rt_i2c_message *rt_i2c_transfer_message(struct rt_i2c_device  *device,
//                                               struct rt_i2c_message *message)
//{
//    rt_err_t result;
//    struct rt_i2c_message *index;

//    RT_ASSERT(device != RT_NULL);

//    /* get first message */
//    index = message;
//    if (index == RT_NULL)
//        return index;

//    result = rt_mutex_take(&(device->bus->lock), RT_WAITING_FOREVER);
//    if (result != RT_EOK)
//    {
//        rt_set_errno(-RT_EBUSY);

//        return index;
//    }

//    /* reset errno */
//    rt_set_errno(RT_EOK);

//    /* configure I2C bus */
//    if (device->bus->owner != device)
//    {
//        /* not the same owner as current, re-configure I2C bus */
//        result = device->bus->ops->configure(device, &device->config);
//        if (result == RT_EOK)
//        {
//            /* set I2C bus owner */
//            device->bus->owner = device;
//        }
//        else
//        {
//            /* configure I2C bus failed */
//            rt_set_errno(-RT_EIO);
//            result = 0;
//            goto __exit;
//        }
//    }

//    /* transmit each I2C message */
//    while (index != RT_NULL)
//    {
//        /* transmit I2C message */
//        result = device->bus->ops->xfer(device, index);
//        if (result == 0)
//        {
//            rt_set_errno(-RT_EIO);
//            break;
//        }

//        index = index->next;
//    }

//__exit:
//    /* release bus lock */
//    rt_mutex_release(&(device->bus->lock));

//    return index;
//}

rt_err_t rt_i2c_take_bus(struct rt_i2c_device *device)
{
    rt_err_t result = RT_EOK;

    RT_ASSERT(device != RT_NULL);
    RT_ASSERT(device->bus != RT_NULL);

    result = rt_mutex_take(&(device->bus->lock), RT_WAITING_FOREVER);
    if (result != RT_EOK)
    {
        rt_set_errno(-RT_EBUSY);

        return -RT_EBUSY;
    }

    /* reset errno */
    rt_set_errno(RT_EOK);

    /* configure I2C bus */
    if (device->bus->owner != device)
    {
        /* not the same owner as current, re-configure I2C bus */
        result = device->bus->ops->configure(device, &device->config);
        if (result == RT_EOK)
        {
            /* set I2C bus owner */
            device->bus->owner = device;
        }
        else
        {
            /* configure I2C bus failed */
            rt_set_errno(-RT_EIO);
            /* release lock */
            rt_mutex_release(&(device->bus->lock));

            return -RT_EIO;
        }
    }

    return result;
}

rt_err_t rt_i2c_release_bus(struct rt_i2c_device *device)
{
    RT_ASSERT(device != RT_NULL);
    RT_ASSERT(device->bus != RT_NULL);
    RT_ASSERT(device->bus->owner == device);

    /* release lock */
    rt_mutex_release(&(device->bus->lock));

    return RT_EOK;
}
