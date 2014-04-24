/*********************************************************************
 * Filename:			keyboard.c
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

#include "kb_dev.h"
#include "i2c.h"
#include "i2cbus.h"
#include "untils.h"

struct kb_device
{
	struct rt_device               parent;      /**< RT-Thread device struct */
	struct rt_i2c_device           i2c_device;  /**< I2C interface */
	struct rt_mutex				   lock;		 /* key board mutex */
};

#define KB_DEV(x) ((struct kb_device *)x)

struct kb_device kb_device;

void rt_kb_lock(struct kb_device* kb_dev)
{
	rt_mutex_take(&(kb_dev->lock),RT_WAITING_FOREVER);
}


void rt_kb_unlock(struct kb_device* kb_dev)
{
	rt_mutex_release(&(kb_dev->lock));
}


static rt_err_t rt_kb_init(rt_device_t dev)
{
//	struct kb_device* kb = KB_DEV(dev);

//	if(kb->i2c_device.bus->owner != &kb->i2c_device)
//	{
//		/*	current configuration spi bus */
//		kb->i2c_device.bus->ops->configure(&kb->i2c_device,&kb->i2c_device.config);
//	}

	return RT_EOK;
}

static rt_err_t rt_kb_open(rt_device_t dev, rt_uint16_t oflag)
{
	return RT_EOK;
}


static rt_err_t rt_kb_close(rt_device_t dev)
{
	return RT_EOK;
}


static rt_size_t rt_kb_read(rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size)
{
	struct kb_device* kb = KB_DEV(dev);
	struct rt_i2c_message message;
    rt_uint8_t *buf = (rt_uint8_t *)buffer;
    rt_err_t result = size;
	rt_int32_t cnts;

	RT_ASSERT(kb != RT_NULL);
	if (size == 2) {

		rt_kb_lock(kb);
		rt_i2c_take_bus(&kb->i2c_device);
		message.cmd = RT_I2C_CMD_START;
		cnts = kb->i2c_device.bus->ops->xfer(&kb->i2c_device, &message);
        if (cnts < 0) {
            result = 0;
            goto __exit;
        }
		message.cmd = RT_I2C_CMD_ADDR_R;
		cnts = kb->i2c_device.bus->ops->xfer(&kb->i2c_device, &message);
        if (cnts < 0) {
            result = 0;
            goto __exit;
        }
		/* While there is data to be read */
		while(size)
		{
			if(size == 1)
			{
				/* Disable Acknowledgement */
				message.cmd = RT_I2C_CMD_NACK;
				cnts = kb->i2c_device.bus->ops->xfer(&kb->i2c_device, &message);
                if (cnts < 0) {
                    result = 0;
                    goto __exit;
                }
				/* Send STOP Condition */
				message.cmd = RT_I2C_CMD_STOP;
				cnts = kb->i2c_device.bus->ops->xfer(&kb->i2c_device, &message);
                if (cnts < 0) {
                    result = 0;
                    goto __exit;
                }
			}

			message.cmd = RT_I2C_CMD_READ;
			message.buf = buf++;
			cnts = kb->i2c_device.bus->ops->xfer(&kb->i2c_device, &message);
            if (cnts < 0) {
                result = 0;
                goto __exit;
            }
            size--;
		}
		/* Enable Acknowledgement to be ready for another reception */
		message.cmd = RT_I2C_CMD_ACK;
		cnts = kb->i2c_device.bus->ops->xfer(&kb->i2c_device, &message);
        if (cnts < 0) {
            result = 0;
            goto __exit;
        }
		rt_i2c_release_bus(&kb->i2c_device);
		rt_kb_unlock(kb);

	} else {
		result = 0;
	}
__exit:
    message.cmd = RT_I2C_CMD_STOP;
    cnts = kb->i2c_device.bus->ops->xfer(&kb->i2c_device, &message);
    rt_i2c_release_bus(&kb->i2c_device);
    rt_kb_unlock(kb);
	return result;
}


static rt_size_t rt_kb_write(rt_device_t dev, rt_off_t pos,const void* buffer, rt_size_t size)
{
	struct kb_device* kb = KB_DEV(dev);

	RT_ASSERT(kb != RT_NULL);

	rt_kb_lock(kb);

//	spi_kb_buffer_write(kb->spi_device,buffer,pos*4096,size*4096);

	rt_kb_unlock(kb);

	return size;
}


static rt_err_t rt_kb_control(rt_device_t dev, rt_uint8_t cmd, void *args)
{
    struct kb_device* kb = (struct kb_device*)dev;
    rt_err_t result;
    RT_ASSERT(kb != RT_NULL);
    switch (cmd)
    {
		case RT_DEVICE_CTRL_CONFIGURE:
			/* disable rx irq */
            if (kb->i2c_device.bus != RT_NULL)
            {
                result = rt_mutex_take(&(kb->i2c_device.bus->lock), RT_WAITING_FOREVER);
                if (result == RT_EOK)
                {
                    kb->i2c_device.bus->ops->configure(&kb->i2c_device, &kb->i2c_device.config);

                    /* release lock */
                    rt_mutex_release(&(kb->i2c_device.bus->lock));
                }
            }
			break;
    }
    return RT_EOK;
}

int
rt_hw_kb_init(void)
{
    rt_err_t result;
	struct rt_i2c_configuration i2c_configure = {
		0x13,
		RT_I2C_ADDR_7,
		20000,
	};

	rt_memset(&kb_device, 0, sizeof(kb_device));

	result = rt_mutex_init(&kb_device.lock, "m_kb", RT_IPC_FLAG_FIFO);
	if (result != RT_EOK)
	{
#ifdef RT_USING_FINSH
		rt_kprintf("mutex initial failure...\n");
#endif // RT_USING_FINSH
		return -RT_ENOSYS;
	}

	rt_i2c_bus_attach_device(&kb_device.i2c_device, "i2c1_kb", STM32_I2C_BUS_0_NAME, RT_NULL);
	kb_device.i2c_device.config = i2c_configure;

	kb_device.parent.type = RT_Device_Class_Char;
	kb_device.parent.init = rt_kb_init;
	kb_device.parent.open = rt_kb_open;
	kb_device.parent.close = rt_kb_close;
	kb_device.parent.read = rt_kb_read;
	kb_device.parent.write = rt_kb_write;
	kb_device.parent.control = rt_kb_control;

	kb_device.parent.user_data = RT_NULL;
	kb_device.parent.rx_indicate = RT_NULL;
	kb_device.parent.tx_complete = RT_NULL;

	// initial i2c bus device
	result = rt_device_register(&kb_device.parent, DEVICE_NAME_KEYBOARD, RT_DEVICE_FLAG_RDWR);
	return result;
}

INIT_DEVICE_EXPORT(rt_hw_kb_init);

#ifdef RT_USING_FINSH
#include <finsh.h>

void kb_test(void)
{
	rt_device_t dev;
	rt_size_t size;
	rt_uint8_t temp[2] = {0,};
	dev = device_enable(DEVICE_NAME_KEYBOARD);
	size = rt_device_read(dev, 0, temp, 2);
	if (size == 2){
		rt_kprintf("key board: %02X%02X\n", temp[0], temp[1]);
	} else {
		rt_kprintf("read key board failure!!!\n");
        rt_kprintf("key board: %02X%02X\n", temp[0], temp[1]);
	}

}

FINSH_FUNCTION_EXPORT(kb_test, key board test.);
#endif // RT_USING_FINSH
