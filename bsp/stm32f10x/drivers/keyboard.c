/*********************************************************************
 * Filename:			keyboard.c
 *
 * Description:
 *
 * Author:				BRIGHT PAN
 * Email:				bright_pan@yuettak.com
 * Date:				2014-04-18
 *
 * Modify:
 *
 * Copyright (C) 2014 Yuettak Co.,Ltd
 ********************************************************************/

#include "keyboard.h"
#include "kb_dev.h"
#include "untils.h"
#include "gpio_exti.h"

#define KB_DEBUG

static rt_sem_t kb_sem;

__STATIC_INLINE uint8_t bit_to_index(uint16_t data)
{
    uint8_t result = 0;
    while (data)
    {
        result++;
        data >>= 1;
    }
    return result;
}

static const uint8_t char_remap[16] = {
    '?','1', '2', '3', '4',
    '5', '6', '7', '8',
    '9', '0', '*', '#',
    '?', '?', '?', 
};

void
kb_thread_entry(void *parameters)
{
	rt_err_t result;
    rt_size_t size;
    uint16_t data;
    uint8_t c;
    rt_device_t device = RT_NULL;

    device_enable(DEVICE_NAME_KEY);
    
    device = device_enable(DEVICE_NAME_KEYBOARD);
	while (1) {

		result = rt_sem_take(kb_sem, 10*RT_TICK_PER_SECOND);
		if (result == RT_EOK) {
            data = 0;
            size = rt_device_read(device, 0, &data, 2);
            if (size == 2) {
				// filter keyboard input
				if (data != 0x0100) {
					data &= 0xfeff;
                }
				__REV16(data);
                c = char_remap[bit_to_index(data&0x0fff)];
#if (defined RT_USING_FINSH) && (defined KB_DEBUG)
                rt_kprintf("keyboard value is %04X, %c\n", data, c);
                
#endif
			} else {
				rt_kprintf("read key board failure!!!\n");
			}

		} else { //time out

		}
	}
}

void kb_detect(void)
{
    rt_sem_release(kb_sem);
}

int
rt_keyboard_init(void)
{
	rt_thread_t kb_thread;

	//initial keyboard sem
    kb_sem = rt_sem_create("kboard", 0, RT_IPC_FLAG_FIFO);
	if (kb_sem == RT_NULL)
		return -1;

	//key board thread
	kb_thread = rt_thread_create("kboard", kb_thread_entry,
									   RT_NULL, 512, 99, 5);
	if (kb_thread == RT_NULL)
		return -1;

	rt_thread_startup(kb_thread);
	return 0;
}

INIT_APP_EXPORT(rt_keyboard_init);

#ifdef RT_USING_FINSH
#include <finsh.h>

FINSH_FUNCTION_EXPORT(kb_detect, kb_detect[]);
#endif // RT_USING_FINSH
