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
#define CONFIG_DEBUG 0

rt_device_t rtc_device;


/**
  * @brief  创建一个映射域
  * @param  BitMaxNum 最大映射数量
  * @retval 一个映射域对象
  */
MapByteDef_p map_byte_create(rt_size_t BitMaxNum)
{
	MapByteDef_p map;

	map = rt_calloc(1,sizeof(*map));
	RT_ASSERT(map != RT_NULL);
	if(BitMaxNum % 8 == 0)
	{
		map->ByteSize = BitMaxNum / 8;
	}
	else
	{
		map->ByteSize = BitMaxNum / 8;
		map->ByteSize++;
	}
	map->BitMaxNum = BitMaxNum;
	
	map->data = rt_calloc(1,map->ByteSize);
	RT_ASSERT(map->data != RT_NULL);
	
	return map;
}

/**
  * @brief  删除一个映射域
  * @param  map 一个映射域对象
  * @retval none
  */
void map_byte_delete(MapByteDef_p map)
{
	RT_ASSERT(map->data != RT_NULL);
	rt_free(map->data);
	
	RT_ASSERT(map != RT_NULL);
	rt_free(map);
}

/**
  * @brief  设置映射域中的某个位
  * @param  map 一个映射域对象
  * @param  Bit 位的位置
  * @param  data RT_TRUE设置该位为1，RT_FALSE设置该位置为0
  * @retval none
  */
void map_byte_bit_set(MapByteDef_p Map,rt_size_t Bit,rt_bool_t data)
{
	rt_size_t BytePos;
	rt_uint8_t BitPos;

	if(Bit >= Map->BitMaxNum)
	{
		rt_kprintf("bit max is %d\n",Map->BitMaxNum);
		return ;
	}
	BytePos = Bit / 8;
	BitPos = Bit % 8;
	if(data == RT_TRUE)
	{
    Map->data[BytePos] |= 1<<(7-BitPos);
	}
	else
	{
		Map->data[BytePos] &= ~(0x01<<(7-BitPos));
	}

}

/**
  * @brief  获得映射域中的某个位
  * @param  map 一个映射域对象
  * @param  Bit 位的位置
  * @retval RT_TRUE 该位置为1
  * @retval RT_FALSE 该位置为0
  */
rt_bool_t map_byte_bit_get(MapByteDef_p Map,rt_size_t Bit)
{
  rt_size_t BytePos;
	rt_uint8_t BitPos;

	if(Bit >= Map->BitMaxNum)
	{
	  rt_kprintf("bit max is %d\n",Map->BitMaxNum);
	  return RT_FALSE;
	}
	BytePos = Bit / 8;
	BitPos = Bit % 8;

	if((Map->data[BytePos])&(1<<(7-BitPos)))
	{
		return RT_TRUE;
	}

	return RT_FALSE;
}

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

static MapByteDef_p maptest = RT_NULL;

void MapBitOp(rt_uint8_t cmd,rt_size_t data)
{
	switch(cmd)
	{
		case 0:
		{
			rt_kprintf("cmd : 0 help  data:none\n");
			rt_kprintf("cmd : 1 clear bit data: bit pos\n");
			rt_kprintf("cmd : 2 set bit  data: bit pos\n");
			rt_kprintf("cmd : 3 read bit data: bit pos\n");
			rt_kprintf("cmd : 4 create map data: bit max num\n");
			rt_kprintf("cmd : 5 delete map data: none\n");
			rt_kprintf("cmd : 6 show map data: Max bit\n");

			if(maptest == RT_NULL)
			{
				maptest = map_byte_create(32);
				rt_kprintf("Auto Create Map Byte BitMax:32\n");
			}
			break;
		}
		case 1:
		{
			map_byte_bit_set(maptest,data,RT_FALSE);
			break;
		}		
		case 2:
		{
			map_byte_bit_set(maptest,data,RT_TRUE);
			break;
		}
		case 3:
		{
			map_byte_bit_get(maptest,data);
			break;
		}
		case 4:
		{
			if(maptest != RT_NULL)
			{
				rt_kprintf("Map is of substance\n");
			}
			else
			{
        maptest = map_byte_create(data);
			}
	
      break;
		}
		case 5:
		{
			if(maptest == RT_NULL)
			{
				rt_kprintf("Map Not is Null!!!\n");
			}
			else
			{
        map_byte_delete(maptest);
        maptest = RT_NULL;
			}
			
      break;
		}
		case 6:
		{
			rt_size_t i;

			if(maptest == RT_NULL)
			{
				rt_kprintf("Map none create\n");

				break;
			}
			for(i = 0; i < data;i++)
			{
				if(i%10 == 0)
				{
					rt_kprintf("\n");
				}
				rt_kprintf("%01d ",map_byte_bit_get(maptest,i));
			}
			rt_kprintf("\n");

			break;
		}
		default:
		{
			rt_kprintf("Cmd is error!!!\n");
			break;
		}
	}
}
FINSH_FUNCTION_EXPORT(MapBitOp,(cmd data) test map bype);

#endif
