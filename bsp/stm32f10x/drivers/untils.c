/*********************************************************************
 * Filename:      untils.c
 *
 *
 * Author:        Bright Pan <loststriker@gmail.com>
 * Created at:    2013-05-22 09:25:39
 *
 *
 * Change Log:
 *
 * Copyright (C) 2013 Yuettak Co.,Ltd
 ********************************************************************/

#include "untils.h"
#include <dfs_init.h>
#include <dfs_elm.h>
#include <dfs_fs.h>
#include "dfs_posix.h"
#include <time.h>
#include "gpio.h"
#include "gpio_pin.h"
#define CONFIG_DEBUG 1

rt_device_t rtc_device;

/*
  功能:获得当前时间
*/
time_t
sys_cur_date(void)
{
	rt_device_t device;
	time_t time=0;

	device = rt_device_find("rtc");
	if (device != RT_NULL)
	{
		rt_device_control(device, RT_DEVICE_CTRL_RTC_GET_TIME, &time);
	}
	RT_DEBUG_LOG(CONFIG_DEBUG,("System Time: %s",ctime((const time_t *)&time)));
	return time;
}

void
delay_us(uint32_t time)
{
	uint8_t nCount;
	while(time--)
	{
		for(nCount = 6 ; nCount != 0; nCount--);
	}
}

void
print_hex(void *buf, u16 length)
{
	u32 i;
    u8 *bk = buf;
	RT_ASSERT(buf != RT_NULL);

	for (i = 0; i < length; i++) {
		rt_kprintf("%02X ", bk[i]);
	}
	rt_kprintf("\n");
}

void
print_char(void *buf, u16 length)
{
	u32 i;
    u8 *bk = buf;
	RT_ASSERT(buf != RT_NULL);

	for (i = 0; i < length; i++) {
		rt_kprintf("%c", bk[i]);
	}
	rt_kprintf("\n");
}


#ifndef __GNUC__
void *
memmem(const void *haystack, rt_size_t haystack_len,
	   const void *needle, rt_size_t needle_len)
{
	const char *begin = haystack;
	const char *last_possible = begin + haystack_len - needle_len;
	const char *tail = needle;
	char point;

	/*
	 * The first occurrence of the empty string is deemed to occur at
	 * the beginning of the string.
	 */
	if (needle_len == 0)
		return (void *)begin;

	/*
	 * Sanity check, otherwise the loop might search through the whole
	 * memory.
	 */
	if (haystack_len < needle_len)
		return NULL;

	point = *tail++;
	for (; begin <= last_possible; begin++) {
		if (*begin == point && !memcmp(begin + 1, tail, needle_len - 1))
			return (void *)begin;
	}

	return NULL;
}

#endif

rt_device_t
device_enable(const char *name)
{
    rt_device_t device = RT_NULL;
    device = rt_device_find(name);
    if (device != RT_NULL)
    {
        if (device->open_flag == RT_DEVICE_OFLAG_CLOSE)
        {
            rt_device_open(device, RT_DEVICE_OFLAG_RDWR);
        }
    }
    else
    {
#ifdef RT_USING_FINSH
        rt_kprintf("the exti device %s is not found!\n", name);
#endif
    }
	return device;
}



#ifdef RT_USING_FINSH
#include <finsh.h>
int
print_rcc(void)
{
	RCC_ClocksTypeDef RCC_ClockFreq;
	RCC_GetClocksFreq(&RCC_ClockFreq);
	rt_kprintf("\n******************* CLOCK *********************");
	rt_kprintf("\nSYSCLK = %ld", RCC_ClockFreq.SYSCLK_Frequency);
	rt_kprintf("\nHCLK = %ld", RCC_ClockFreq.HCLK_Frequency);
	rt_kprintf("\nPCLK1 = %ld", RCC_ClockFreq.PCLK1_Frequency);
	rt_kprintf("\nPCLK2 = %ld", RCC_ClockFreq.PCLK2_Frequency);
	rt_kprintf("\nADCCLK = %ld", RCC_ClockFreq.ADCCLK_Frequency);
    rt_kprintf("\n******************* CLOCK *********************");
    return 0;
}

FINSH_FUNCTION_EXPORT(device_enable, device_enable[name]);
INIT_BOARD_EXPORT(print_rcc);

void sysreset()
{
  NVIC_SystemReset();
}
FINSH_FUNCTION_EXPORT(sysreset,sysreset() -- reset stm32);

void sysinit(void)
{
	unlink("/config");
	NVIC_SystemReset();
}
FINSH_FUNCTION_EXPORT(sysinit,-- init system);

void delay_us_test(char *name, int cnts, int delay1, int delay2)
{
    u8 dat = 1;
    int i;
    rt_device_t device = device_enable(name);
	gpio_device *gpio = (gpio_device *)device;
    struct gpio_pin_user_data *user = (struct gpio_pin_user_data*)gpio->parent.user_data;
      
    
    //rt_base_t level = rt_hw_interrupt_disable();
    for (i = 0; i < cnts; ++i) {
        GPIO_SetBits(user->gpiox,user->gpio_pinx);
        delay_us(delay1);
        GPIO_ResetBits(user->gpiox,user->gpio_pinx);
        delay_us(delay2);
    }
    //rt_hw_interrupt_enable(level);
}
FINSH_FUNCTION_EXPORT(delay_us_test,delay_us_test());
#endif
