/*
 * File      : i2c.h
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
 * 2012-11-23     Bernard      Add extern "C"
 */

#ifndef __I2C_H__
#define __I2C_H__

#include <stdlib.h>
#include <rtthread.h>

#ifdef __cplusplus
extern "C"{
#endif

#define RT_I2C_ADDR_7       (0)                             /* 7 BITS ADDRESS */
#define RT_I2C_ADDR_10      (1)                             /* 10 BITS ADDRESS */

#define RT_I2C_MASTER   (0)                             /* I2C master device */
#define RT_I2C_SLAVE    (1)                             /* I2C slave device */

#define RT_I2C_CMD_START (0)
#define RT_I2C_CMD_STOP (1)
#define RT_I2C_CMD_WRITE (2)
#define RT_I2C_CMD_READ (3)
#define RT_I2C_CMD_BUS_TEST (4)
#define RT_I2C_CMD_ACK (5)
#define RT_I2C_CMD_NACK (6)
#define RT_I2C_CMD_ADDR_W (7)
#define RT_I2C_CMD_ADDR_R (8)

	/* I2C message structure */

	struct rt_i2c_message
	{
		rt_uint8_t cmd;
		void *buf;
		rt_size_t length;
	};

	/**
	 * I2C configuration structure
	 */
	struct rt_i2c_configuration
	{
        rt_uint16_t addr;
		rt_uint8_t addr_width;
		rt_uint32_t speed;
	};



	struct rt_i2c_bus
	{
		struct rt_device parent;
		const struct rt_i2c_ops *ops;

		struct rt_mutex lock;
		struct rt_i2c_device *owner;
	};


	/**
	 * I2C Virtual BUS, one device must connected to a virtual BUS
	 */
	struct rt_i2c_device
	{
		struct rt_device parent;
		struct rt_i2c_bus *bus;

		struct rt_i2c_configuration config;
	};
#define I2C_DEVICE(dev) ((struct rt_i2c_device *)(dev))

    	/**
	 * I2C operators
	 */
	struct rt_i2c_ops
	{
		rt_err_t (*configure)(struct rt_i2c_device *device, struct rt_i2c_configuration *configuration);
		rt_int32_t (*xfer)(struct rt_i2c_device *device, struct rt_i2c_message *message);
        rt_err_t (*reset)(struct rt_i2c_device *device);
	};
	/* register a I2C bus */
	rt_err_t rt_i2c_bus_register(struct rt_i2c_bus       *bus,
								 const char              *name,
								 const struct rt_i2c_ops *ops);

	/* attach a device on I2C bus */
	rt_err_t rt_i2c_bus_attach_device(struct rt_i2c_device *device,
									  const char           *name,
									  const char           *bus_name,
									  void                 *user_data);
    rt_err_t rt_i2c_take_bus(struct rt_i2c_device *device);
    rt_err_t rt_i2c_release_bus(struct rt_i2c_device *device);
	/* set configuration on I2C device */
	rt_err_t rt_i2c_configure(struct rt_i2c_device        *device,
							  struct rt_i2c_configuration *cfg);


#ifdef __cplusplus
}
#endif

#endif
