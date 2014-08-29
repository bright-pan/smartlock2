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
#include "apppubulic.h"

//#include "eeprom.h"
//#include "funtable.h"
#define UNTILS_DEBUG 1

rt_device_t rtc_device;

#define DEVICE_CONFIG_FILE_NAME	"/config"

DEVICE_CONFIG_TYPEDEF device_config = {
	RT_NULL,
	{
		//alarm telephone
		{
			{
				0,
				"8613316975697"
			},
			{
				0,
				"8618675557185"
			},
		},
		//key
		{
			{
				0, 0,
				KEY_TYPE_KBOARD,
				OPERATION_TYPE_FOREVER,
				0, 0, 0,
			},
			{
				0,0,
				KEY_TYPE_RFID,
				OPERATION_TYPE_FOREVER,
				0, 0, 0,
			},
			{
				0, 0,
				KEY_TYPE_FPRINT,
				OPERATION_TYPE_FOREVER,
				0, 0, 0,
			},
		},
		{
			"iyuet.com",
			6800
			//"115.29.235.194",
			//6868
		},
		//lock gate alarm time
		30,
		//device id
		//{0x12,0x34,0x56,0x78,0x9a,0xbc,0xde,0xf0},
		{0x99,0x99,0x15,0x10,0x90,0x00,0x01,0x50},
		//CDKEY
		{0x9C,0x9E,0x11,0x36,0xD3,0x64,0xAF,0xA9},
		//key0
		//{0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa},
		{0x01,0x02,0x03,0x04,0x05,0x06,0x99,0x99},
		//key1
		//{0x00,0x00,0xCB,0x17,0x62,0x2F,0x7A,0xC5},
		{0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa},
		//
		0,
    "123456",
    //Four kinds of alarm parameters
  	{
			{1},
			{1},
			{1},
			{1},
  	},
	},
};

int
device_config_key_operate(uint16_t key_id, KEY_TYPE key_type, uint8_t *buf, uint8_t flag)
{
	int fd;
	int result = -1;
	uint16_t len;

    RT_ASSERT(key_id < KEY_NUMBERS);
	rt_mutex_take(device_config.mutex, RT_WAITING_FOREVER);

	switch (key_type)
	{
		case KEY_TYPE_FPRINT:
			{
				len = KEY_FPRINT_CODE_SIZE;
				break;
			}
		case KEY_TYPE_RFID:
			{
				len = KEY_RFID_CODE_SIZE;
				break;
			}
		case KEY_TYPE_KBOARD:
			{
				len = KEY_KBOARD_CODE_SIZE;
				break;
			}
		default :
			{
                RT_DEBUG_LOG(UNTILS_DEBUG, ("the key type is invalid\n"));
                goto __exit;
			}
	}

	fd = open(DEVICE_CONFIG_FILE_NAME, O_RDWR, 0x777);
	if (fd >= 0)
	{
		lseek(fd, DEVICE_CONFIG_FILE_KEY_OFFSET(key_id), SEEK_SET);
		if (flag) {
			if (write(fd, buf, len) == len)
				result = len;
		} else {
			if (read(fd, buf, len) == len)
				result = len;
		}
		close(fd);
	}
__exit:
	rt_mutex_release(device_config.mutex);
	return result;
}

int
device_config_key_verify(KEY_TYPE type, const uint8_t *buf)
{
    int result = -1;
    uint16_t i;
	KEY_TYPEDEF key;
#if (KEY_KBOARD_CODE_SIZE >= KEY_RFID_CODE_SIZE)
    uint8_t temp[KEY_KBOARD_CODE_SIZE];
#else
    uint8_t temp[KEY_RFID_CODE_SIZE];
#endif

    uint16_t len;
    rt_mutex_take(device_config.mutex, RT_WAITING_FOREVER);
    switch (type)
    {
        case KEY_TYPE_RFID:
            {
                len = KEY_RFID_CODE_SIZE;
                break;
            }
        case KEY_TYPE_KBOARD:
            {
                len = KEY_KBOARD_CODE_SIZE;
                break;
            }
        case KEY_TYPE_FPRINT:
            {
                len = KEY_FPRINT_CODE_SIZE;
                break;
            }
        default :
            {
                RT_DEBUG_LOG(UNTILS_DEBUG, ("the key type is invalid\n"));
                goto __exit;
            }
    }
	for (i = 0; i < KEY_NUMBERS; i++) {
		key = device_config.param.key[i];

		if (key.flag && key.key_type == type) {
			rt_memset(temp, 0, sizeof(temp));
			device_config_key_operate(i, type, temp, 0);
            if (!rt_memcmp(temp, buf, len)) {
                result = i;
				break;
            }
		}
	}
__exit:
	rt_mutex_release(device_config.mutex);
    return result;
}

int
device_config_superpwd_save(uint8_t *buf)
{
	int result = -1;

	rt_mutex_take(device_config.mutex, RT_WAITING_FOREVER);
	rt_memcpy(device_config.param.password, buf, 6);
	result = device_config_file_operate(&device_config, 1);
	rt_mutex_release(device_config.mutex);
	return result;
}

int
device_config_superpwd_verify(const uint8_t *buf)
{
    int result = -1;

    rt_mutex_take(device_config.mutex, RT_WAITING_FOREVER);
	if (!rt_memcmp(device_config.param.password, buf, 6)) {
		result = 0;
	}
	rt_mutex_release(device_config.mutex);
    return result;
}

int
device_config_key_create(KEY_TYPE type, uint8_t *buf)
{
    int result = -1;
    uint16_t i;
	KEY_TYPEDEF *key;

    rt_mutex_take(device_config.mutex, RT_WAITING_FOREVER);

	for (i = 0; i < KEY_NUMBERS; i++) {
		key = &device_config.param.key[i];

		if (!key->flag) {
			if (device_config_key_operate(i, type, buf, 1) < 0)
                goto __exit;
            rt_memset(key, 0, sizeof(*key));
			key->key_type = type;
			key->is_updated = 1;
			key->operation_type = OPERATION_TYPE_FOREVER;
			key->created_time = sys_cur_date();
			key->flag = 1;
			if (device_config_file_operate(&device_config, 1) < 0)
				goto __exit;
			result = i;
			break;
		}
	}
__exit:
	rt_mutex_release(device_config.mutex);
    return result;
}

int
device_config_key_delete(uint16_t key_id)
{
    int result = -1;
	KEY_TYPEDEF *key;
    RT_ASSERT(key_id < KEY_NUMBERS);
    rt_mutex_take(device_config.mutex, RT_WAITING_FOREVER);

    key = &device_config.param.key[key_id];

    if (key->flag) {
        rt_memset(key, 0, sizeof(*key));
        if (device_config_file_operate(&device_config, 1) < 0)
            goto __exit;
/*        
        if (key->key_type == KEY_TYPE_FPRINT)
            fp_delete(key_id);
*/
        result = key_id;

    } else {
        result = 0;
    }
__exit:
	rt_mutex_release(device_config.mutex);
    return result;
}

int
device_config_init(DEVICE_CONFIG_TYPEDEF *config)
{
	int result;

	config->mutex = rt_mutex_create("m_config", RT_IPC_FLAG_FIFO);
	if (config->mutex == RT_NULL)
		return -1;

	result = device_config_file_operate(config, 0);

	return result;
}

/*******************************************************************************
 * Function Name  : system_file_operate
 * Description    :  system config file operate
 *
 * Input				: flag :1>>write ; 0>>read
 * Output			: None
 * Return		: None
 *******************************************************************************/
int
device_config_file_operate(DEVICE_CONFIG_TYPEDEF *config, uint8_t flag)
{
	int fd;
	int cnts;
	int result = -1;

	RT_ASSERT(config!=RT_NULL);
	RT_ASSERT(config->mutex!=RT_NULL);

	rt_mutex_take(config->mutex, RT_WAITING_FOREVER);
	fd = open(DEVICE_CONFIG_FILE_NAME,O_RDWR,0x777);
	if(fd < 0)
	{
		unlink(DEVICE_CONFIG_FILE_NAME);
		//system_file_key_init(arg);
		fd = open(DEVICE_CONFIG_FILE_NAME,O_CREAT | O_RDWR,0x777);
		if (fd < 0) {
            RT_DEBUG_LOG(UNTILS_DEBUG, ("Creat Config File failure\n"));
			goto __exit;
		} else {
            RT_DEBUG_LOG(UNTILS_DEBUG, ("Creat Config File success\n"));
            cnts = write(fd, &(config->param), sizeof(config->param));
			if (cnts != sizeof(config->param)) {
				close(fd);
				goto __exit;
			}
			lseek(fd, 0, SEEK_SET);
		}
	}

	if (flag) {
        cnts = write(fd, &(config->param), sizeof(config->param));
		if (cnts == sizeof(config->param))
			result = cnts;
	} else {
        cnts = read(fd, &(config->param), sizeof(config->param));
		if (cnts == sizeof(config->param))
			result = cnts;
	}
	close(fd);
__exit:
	rt_mutex_release(config->mutex);

	return result;
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
print_hex(uint8_t *buf, uint16_t length)
{
	uint32_t i;
	RT_ASSERT(buf != RT_NULL);

	for (i = 0; i < length; i++) {
		rt_kprintf("%02X ", buf[i]);
	}
	rt_kprintf("\n");
}

void
print_char(uint8_t *buf, uint16_t length)
{
	uint32_t i;
	RT_ASSERT(buf != RT_NULL);

	for (i = 0; i < length; i++) {
		rt_kprintf("%c", buf[i]);
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

__INLINE rt_device_t
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

int 
system_init(void)
{
    int result = 0;
    if (device_config_init(&device_config) < 0)
	{
#if (defined RT_USING_FINSH) && (defined UNTILS_DEBUG)
		rt_kprintf("device config init failure");
#endif // RT_USING_FINSH
        result = -1;	
    }
    // get rtc clock
	rtc_device = rt_device_find("rtc");
	if (rtc_device == RT_NULL)
	{
#if (defined RT_USING_FINSH) && (defined UNTILS_DEBUG)
		rt_kprintf("rtc_device is not exist!!!");
#endif // RT_USING_FINSH
        result = -1;
	}
    return result;
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

void sysconfig(void)
{
	rt_uint8_t i;
    uint8_t temp[512];
    
	rt_kprintf("\n******************************* System config file ********************************\n");
	for(i = 0 ; i < TELEPHONE_NUMBERS;i++)
	{
		if(device_config.param.telephone_address[i].flag == 0)
		{
			continue;
		}
		rt_kprintf("Telephone[%d]:%d:%s\n",i,
				   device_config.param.telephone_address[i].flag,
				   device_config.param.telephone_address[i].address);
	}
	for(i = 0 ; i < KEY_NUMBERS;i++)
	{
		rt_memset(temp, '\0', sizeof(temp));
		if(device_config.param.key[i].flag == 0)
		{
			continue;
		}
		rt_kprintf("Key[%d] update:%02x type:%d authority:%d create:%08x start:%08x end:%08x\n",i,
				   device_config.param.key[i].is_updated,
				   device_config.param.key[i].key_type,
				   device_config.param.key[i].operation_type,
				   device_config.param.key[i].created_time,
				   device_config.param.key[i].start_time,
				   device_config.param.key[i].end_time);
        //device_config_key_operate(i, device_config.param.key[i].key_type, temp, 0);
        //rt_kprintf("key : %s\n", temp);
	}
    /*
	  rt_memcpy(temp, "123456", 6);
	  device_config_key_operate(1, KEY_TYPE_KBOARD, temp, 1);
	  device_config_key_operate(1, KEY_TYPE_KBOARD, temp, 0);
	  rt_kprintf("%s\n", temp);
    */
	rt_kprintf("******************************* System config file ********************************\n\n");
}
FINSH_FUNCTION_EXPORT(sysconfig,"show system config file");

#endif
