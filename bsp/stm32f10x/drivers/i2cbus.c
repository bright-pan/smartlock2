/*********************************************************************
 * Filename:			i2cbus.c
 *
 * Description:
 *
 * Author:				Bright Pan
 * Email:				bright_pan@yuettak.com
 * Date:				2014-04-03
 *
 * Modify:
 *
 * Copyright (C) 2014 Yuettak Co.,Ltd
 ********************************************************************/

#include "i2cbus.h"
#include "untils.h"

#define STM32_I2C_BUS_0 I2C2
#define STM32_I2C_BUS_0_CLK RCC_APB1Periph_I2C2

#define STM32_I2C_BUS_0_SCL_CLK RCC_APB2Periph_GPIOB
#define STM32_I2C_BUS_0_SCL_PIN GPIO_Pin_10                  /* PB.10 */
#define STM32_I2C_BUS_0_SCL_GPIO GPIOB

#define STM32_I2C_BUS_0_SDA_CLK RCC_APB2Periph_GPIOB
#define STM32_I2C_BUS_0_SDA_PIN GPIO_Pin_11                  /* PB.11 */
#define STM32_I2C_BUS_0_SDA_GPIO GPIOB

static struct stm32_i2c_bus stm32_i2c_bus_0;

static rt_err_t
configure(struct rt_i2c_device *device, struct rt_i2c_configuration* configuration);

static rt_int32_t
xfer(struct rt_i2c_device* device, struct rt_i2c_message* message);

static rt_err_t
reset(struct rt_i2c_device *device);

static struct rt_i2c_ops stm32_i2c_ops = {

	configure,
	xfer,
    reset,
};

static rt_err_t
configure(struct rt_i2c_device *device, struct rt_i2c_configuration* configuration)
{
	I2C_InitTypeDef I2C_InitStructure;
	struct stm32_i2c_bus *i2c_bus = (struct stm32_i2c_bus *)device->bus;

	I2C_Cmd(i2c_bus->I2C, DISABLE);
    I2C_DeInit(i2c_bus->I2C);
	I2C_StructInit(&I2C_InitStructure);
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	if (configuration->addr_width == RT_I2C_ADDR_7)
		I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	if (configuration->addr_width == RT_I2C_ADDR_10)
		I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_10bit;
	I2C_InitStructure.I2C_ClockSpeed = configuration->speed;

    I2C_Cmd(i2c_bus->I2C, ENABLE);
	I2C_Init(i2c_bus->I2C, &I2C_InitStructure);

    return 0;
}

static rt_err_t
reset(struct rt_i2c_device *device)
{
	I2C_InitTypeDef I2C_InitStructure;
	struct stm32_i2c_bus *i2c_bus = (struct stm32_i2c_bus *)device->bus;

	I2C_SoftwareResetCmd(i2c_bus->I2C, ENABLE);
    delay_us(200);
    I2C_SoftwareResetCmd(i2c_bus->I2C, DISABLE);

    return 0;
}

static rt_int32_t
xfer(struct rt_i2c_device *device, struct rt_i2c_message *message)
{
	struct stm32_i2c_bus *i2c_bus = (struct stm32_i2c_bus *)device->bus;
	rt_uint32_t timeout;
	rt_int32_t result = 0;
	switch(message->cmd)
	{
		case RT_I2C_CMD_BUS_TEST:
			{
				timeout = 100000;
				while(I2C_GetFlagStatus(i2c_bus->I2C, I2C_FLAG_BUSY))
				{
					delay_us(5);
					if((timeout--) == 0) {
						result = -1;
						break;
					}
				}
				break;
			}
		case RT_I2C_CMD_START:
			{
				I2C_GenerateSTART(i2c_bus->I2C, ENABLE);
				timeout = 100000;
				while(!I2C_CheckEvent(i2c_bus->I2C, I2C_EVENT_MASTER_MODE_SELECT))
				{
					delay_us(5);
					if((timeout--) == 0) {
						result = -2;
						break;
					}
				}
				break;
			}
		case RT_I2C_CMD_STOP:
			{
				I2C_GenerateSTOP(i2c_bus->I2C, ENABLE);
				break;
			}
		case RT_I2C_CMD_ACK:
			{
				I2C_AcknowledgeConfig(i2c_bus->I2C, ENABLE);
				break;
			}
		case RT_I2C_CMD_NACK:
			{
				I2C_AcknowledgeConfig(i2c_bus->I2C, DISABLE);
				break;
			}

		case RT_I2C_CMD_READ:
			{
                timeout = 100000;
				while(!I2C_CheckEvent(i2c_bus->I2C, I2C_EVENT_MASTER_BYTE_RECEIVED)) {
					delay_us(1);
					if((timeout--) == 0) {
						result = -2;
						break;
					}
                }
                *(rt_uint8_t *)message->buf = I2C_ReceiveData(i2c_bus->I2C);
				break;
			}
		case RT_I2C_CMD_WRITE:
			{

				break;
			}
		case RT_I2C_CMD_ADDR_W:
			{
				I2C_Send7bitAddress(i2c_bus->I2C, device->config.addr, I2C_Direction_Transmitter);
				timeout = 30000;
				while(!I2C_CheckEvent(i2c_bus->I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
				{
					delay_us(1);
					if((timeout--) == 0) {
						result = -2;
						break;
					}
				}
				break;
			}
		case RT_I2C_CMD_ADDR_R:
			{
				I2C_Send7bitAddress(i2c_bus->I2C, device->config.addr, I2C_Direction_Receiver);
				timeout = 30000;
				while(!I2C_CheckEvent(i2c_bus->I2C, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
				{
					delay_us(2);
					if((timeout--) == 0) {
						result = -2;
						break;
					}
				}
				break;
			}
		default :
			{
				break;
			}
	}
	return result;
}

void
stm32_i2c_bus_0_gpio_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(STM32_I2C_BUS_0_SCL_CLK | STM32_I2C_BUS_0_SDA_CLK, ENABLE);
	RCC_APB1PeriphClockCmd(STM32_I2C_BUS_0_CLK, ENABLE);

	/* Configure I2C1 pins: SCL and SDA */
	GPIO_InitStructure.GPIO_Pin = STM32_I2C_BUS_0_SCL_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_Init(STM32_I2C_BUS_0_SCL_GPIO, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = STM32_I2C_BUS_0_SDA_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_Init(STM32_I2C_BUS_0_SDA_GPIO, &GPIO_InitStructure);
}

int
stm32_i2c_bus_register(void)
{
	stm32_i2c_bus_0_gpio_init();
	stm32_i2c_bus_0.I2C = STM32_I2C_BUS_0;
	rt_i2c_bus_register(&stm32_i2c_bus_0.parent, STM32_I2C_BUS_0_NAME, &stm32_i2c_ops);
    return 0;
}

INIT_BOARD_EXPORT(stm32_i2c_bus_register);
