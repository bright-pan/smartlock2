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
//#include "eeprom.h"
//#include "funtable.h"

#define SYSTEM_CONFIG_FILE_NAME					"/config"

rt_mutex_t	system_file_mutex = RT_NULL;

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

DEVICE_PARAMETERS_TYPEDEF device_parameters = {
	//alarm telephone
	{
		{
			0,
			"8613316975697"
		},
		{
			1,
			"8618675557185"
		},
	},
	//key
	{
		{
			0,
			KEY_TYPE_NUMBER,
			OPERATION_TYPE_FOREVER,
			0, 0,
		},
		{
			0,
			KEY_TYPE_RFID,
			OPERATION_TYPE_FOREVER,
			0, 0,
		},
		{
			0,
			KEY_TYPE_FINGER_PRINT,
			OPERATION_TYPE_FOREVER,
			0, 0,
		},
	},
	{
		"iyuet.com",
		4123
	},
	//lock gate alarm time
	30,
	//device id
	{0x01,0xA1,0x00,0x01,0x01,0xA9,0x01,0x01},
	//CDKEY
	{0x9C,0x9E,0x11,0x36,0xD3,0x64,0xAF,0xA9},
	//key0
	{0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38},
	//key1
	{0x00,0x00,0xCB,0x17,0x62,0x2F,0x7A,0xC5},
	//
	0,
};
