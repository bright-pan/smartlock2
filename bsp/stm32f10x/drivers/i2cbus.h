/*********************************************************************
 * Filename:			i2cbus.h
 *
 * Description:
 *
 * Author:				Bright Pan
 * Email:				bright_pan@yuettka.com
 * Date:				2014-04-03
 *
 * Modify:
 *
 * Copyright (C) 2014 Yuettak Co.,Ltd
 ********************************************************************/
#ifndef _I2CBUS_H_
#define _I2CBUS_H_

#include <rtthread.h>
#include "stm32f10x.h"
#include "i2c.h"

#define STM32_I2C_BUS_0_NAME "i2c1"

struct stm32_i2c_bus
{
	struct rt_i2c_bus parent;
	I2C_TypeDef *I2C;
};

rt_err_t
stm32_i2c_resister(I2C_TypeDef *I2C,
				   struct stm32_i2c_bus *i2c_bus,
				   const char * bus_name);

#endif /* _I2CBUS_H_ */
