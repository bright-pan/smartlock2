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

MapByteDef_p SystemPrintMap = RT_NULL;

/**
  * @brief  ����һ��ӳ����
  * @param  BitMaxNum ���ӳ������
  * @retval һ��ӳ�������
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
  * @brief  ɾ��һ��ӳ����
  * @param  map һ��ӳ�������
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
  * @brief  ����ӳ�����е�ĳ��λ
  * @param  map һ��ӳ�������
  * @param  Bit λ��λ��
  * @param  data RT_TRUE���ø�λΪ1��RT_FALSE���ø�λ��Ϊ0
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
  * @brief  ���ӳ�����е�ĳ��λ
  * @param  map һ��ӳ�������
  * @param  Bit λ��λ��
  * @retval RT_TRUE ��λ��Ϊ1
  * @retval RT_FALSE ��λ��Ϊ0
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
  ����:��õ�ǰʱ��
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


#ifdef USEING_CAN_SET_DEBUG
//����ϵͳ��ӡӳ����
int system_printf_map_byte(void)
{
	SystemPrintMap = map_byte_create(32);

	rt_kprintf("sizeof(MapByteDef_p) = %d\n",sizeof(*SystemPrintMap));
	RT_ASSERT(SystemPrintMap != RT_NULL);

	return 0;
}
INIT_APP_EXPORT(system_printf_map_byte);

rt_bool_t debug_check(rt_uint32_t flag)
{
	return map_byte_bit_get(SystemPrintMap,flag);
}
#endif


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


#ifdef USEING_CAN_SET_DEBUG
void sys_printf(rt_uint8_t cmd,rt_uint8_t data)
{
	switch(cmd)
	{
		case 0:
		{
			rt_kprintf("--help\n");
			rt_kprintf("cmd:1 Set Printf Debug Type\n");
			rt_kprintf("cmd:2 Clear Printf Debug Type\n");
			rt_kprintf("cmd:3 Set 0~data Printf output");
			rt_kprintf("cmd:4 Set 0~data Printf close");
			
			rt_kprintf("data:0  USEING_GPRS_DEBUG\n");
			rt_kprintf("data:1  SHOW_MSG_THREAD\n");
			rt_kprintf("data:2  SHOW_RECV_GSM_RST\n");
			rt_kprintf("data:3  SHOW_RECV_MSG_INFO\n");
			rt_kprintf("data:4  SHOW_SEND_MSG_INFO\n");
			rt_kprintf("data:5  SHOW_LENMAP_INFO\n");
			rt_kprintf("data:6  SHOW_SEND_MODE_INFO\n");
			rt_kprintf("data:7  SHOW_MEM_INFO\n");
			rt_kprintf("data:8  SHOW_WND_INFO\n");
			rt_kprintf("data:9  SHOW_SET_MSG_INOF\n");
			rt_kprintf("data:10 SHOW_RECV_MAIL_ADDR\n");
			rt_kprintf("data:11 SHOW_NONE_ENC_DATA\n");
			rt_kprintf("data:12 SHOW_NFILE_CRC32\n");
			rt_kprintf("data:13 SHOW_NFILE_SEND\n");
			rt_kprintf("data:14 SHOW_NFILE_SRESULT\n");
			rt_kprintf("data:15 SHOW_CRC16_INIF\n");
			rt_kprintf("data:16 LOCAL_DEBUG_THREAD\n");
			rt_kprintf("data:17 LOCAL_DEBUG_MAIL\n");
			rt_kprintf("data:18 BT_DEBUG_THREAD\n");
			rt_kprintf("data:19 BT_DEBUG_RCVDAT\n");
			rt_kprintf("data:20 BT_DEBUG_SENDDAT\n");
			rt_kprintf("data:21 NETPY_DEBUG_THREAD\n");
			rt_kprintf("data:22 NETMAILCLASS_DEBUG\n");
			break;
		}
		case 1:
		{
			//����
			map_byte_bit_set(SystemPrintMap,data,RT_TRUE);
			break;
		}
		case 2:
		{
			//���
			map_byte_bit_set(SystemPrintMap,data,RT_FALSE);
			break;
		}
		case 3:
		{
			//����0��data������Ϣ���
			rt_uint8_t i;

			for(i = 0;i < data;i++)
			{
				map_byte_bit_set(SystemPrintMap,i,RT_TRUE);
			}
			rt_kprintf("Set 0~%d Printf output\n",data);
			break;
		}
		case 4:
		{
			//����0��data�ĵ�����Ϣ�ر�
			rt_uint8_t i;

			for(i = 0;i < data;i++)
			{
				map_byte_bit_set(SystemPrintMap,i,RT_FALSE);
			}
			rt_kprintf("Set 0~%d  Printf close\n",data);
			break;
		}
		case 5:
		{
			//��ʾ�����Ѿ����õĵ���ѡ��
			break;
		}
		default:
		{
			break;
		}
	}
}
FINSH_FUNCTION_EXPORT(sys_printf,(cmd data) system debug printf );

#endif
#endif
