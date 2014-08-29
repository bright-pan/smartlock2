/*********************************************************************
 * Filename:			config.c
 *
 * Description:
 *
 * Author:				Bright Pan
 * Email:				bright_pan@yuettak.com
 * Date:				2014-08-27
 *
 * Modify:
 *
 * Copyright (C) 2014 Yuettak Co.,Ltd
 ********************************************************************/

#include <dfs_init.h>
#include <dfs_elm.h>
#include <dfs_fs.h>
#include <dfs_posix.h>
#include "config.h"
#include "untils.h"

//#include "eeprom.h"
//#include "funtable.h"
#define CONFIG_DEBUG 1

#define ECONFIG_ERROR 1
#define ECONFIG_FULL 2
#define ECONFIG_EXIST 3

#define DEVICE_CONFIG_FILE_NAME	"/config"

static struct device_configure device_config = {
	RT_NULL,
	{
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
		// device status
		0,
		"123456",
		//lock gate alarm time
		30,
		{
			"iyuet.com",
			6800
			//"115.29.235.194",
			//6868
		},
		//Four kinds of alarm parameters
		{
			{1},
			{1},
			{1},
			{1},
		},
		//account valid map
		{
			{0,},
		},
		//key valid map
		{
			{0,},
		},
		//phone valid map
		{
			{0,}
		},
	},
};

s32
device_config_account_remove_key(u16);
/*
    根据钥匙ID获得钥匙的有效使用状况。
    返回：
        1 该钥匙有效。
        0 该钥匙无效。
*/
s8
device_config_get_key_valid(u16 key_id)
{
	s8 result = -ECONFIG_ERROR;
    RT_ASSERT(key_id < KEY_NUMBERS);
    result = (device_config.param.kv_map.data[key_id/32] & bits_mask(key_id%32)) ? 1 : 0;
    return result;
}
/*
    根据钥匙ID，设置钥匙为有效（1），无效状态（0）。
    无返回。
*/
void
device_config_set_key_valid(u16 key_id, u8 value)
{
	RT_ASSERT(key_id < KEY_NUMBERS);
	if (value) {
		device_config.param.kv_map.data[key_id/32] |= bits_mask(key_id%32);
	} else {
		device_config.param.kv_map.data[key_id/32] &= ~bits_mask(key_id%32);
	}
}

s8
device_config_get_phone_valid(u16 phone_id)
{
	s8 result = -ECONFIG_ERROR;
    RT_ASSERT(phone_id < PHONE_NUMBERS);
	result = (device_config.param.pv_map.data[phone_id/32] & bits_mask(phone_id%32)) ? 1 : 0;
	return result;
}

void
device_config_set_phone_valid(u16 phone_id, u8 value)
{
	RT_ASSERT(phone_id < PHONE_NUMBERS);
	if (value) {
		device_config.param.pv_map.data[phone_id/32] |= bits_mask(phone_id%32);
	} else {
		device_config.param.pv_map.data[phone_id/32] &= ~bits_mask(phone_id%32);
	}
}

s8
device_config_get_account_valid(u16 account_id)
{
	s8 result = -ECONFIG_ERROR;
    RT_ASSERT(account_id < ACCOUNT_NUMBERS);
	result = (device_config.param.av_map.data[account_id/32] & bits_mask(account_id%32)) ? 1 : 0;
	return result;
}

void
device_config_set_account_valid(u16 account_id, u8 value)
{
	RT_ASSERT(account_id < ACCOUNT_NUMBERS);
	if (value) {
		device_config.param.av_map.data[account_id/32] |= bits_mask(account_id%32);
	} else {
		device_config.param.av_map.data[account_id/32] &= ~bits_mask(account_id%32);
	}
}

s16
device_config_get_key_code_size(u16 key_type)
{
	s16 result = -ECONFIG_ERROR;

	switch (key_type)
	{
		case KEY_TYPE_FPRINT:
			{
				result = KEY_FPRINT_CODE_SIZE;
				break;
			}
		case KEY_TYPE_RFID:
			{
				result = KEY_RFID_CODE_SIZE;
				break;
			}
		case KEY_TYPE_KBOARD:
			{
				result = KEY_KBOARD_CODE_SIZE;
				break;
			}
		default :
			{
		RT_DEBUG_LOG(CONFIG_DEBUG, ("the key type is invalid\n"));
				result = 0;
				break;
			}
	}
	return result;
}
/*
    根据钥匙ID来读取钥匙信息。
    key_id: 钥匙ID
    k：钥匙信息
    flag：1，写入；0，读出。
    返回：
        -ECONFIG_ERROR，读取出错。
        >=0, 成功读取钥匙的ID。
*/
s32
device_config_key_operate(u16 key_id, struct key *k, u8 flag)
{
	s32 fd;
	s32 result;
	s32 len;

    RT_ASSERT(key_id < KEY_NUMBERS);

	result = -ECONFIG_ERROR;
	rt_mutex_take(device_config.mutex, RT_WAITING_FOREVER);
	fd = open(DEVICE_CONFIG_FILE_NAME, O_RDWR, 0x777);
	if (fd >= 0)
	{
		lseek(fd, DEVICE_CONFIG_FILE_KEY_OFFSET(key_id), SEEK_SET);
		if (flag)
			len = write(fd, k, sizeof(*k));
		else
			len = read(fd, k, sizeof(*k));
		if (len == sizeof(*k))
			result = len;
		close(fd);
	}

	rt_mutex_release(device_config.mutex);
	return result;
}
/*
    检验钥匙的密码是否存在，存在则返回钥匙ID。
    key_type:钥匙类型。
    buf: 检验密码的地址。
    length：密码的长度。
    返回：
        -ECONFIG_ERROR，钥匙库里没有这把钥匙。
        >=0, 成功校验返回钥匙的ID。
*/
s32
device_config_key_verify(u16 key_type, const u8 *buf, u16 length)
{
    s32 result;
    u16 i;
	struct key k;
    s32 len;

	result = -ECONFIG_ERROR;

    rt_mutex_take(device_config.mutex, RT_WAITING_FOREVER);

	for (i = 0; i < KEY_NUMBERS; i++) {
		if (device_config_get_key_valid(i)) {
			len = device_config_key_operate(i, &k, 0);
			if (len >= 0) {
				if (k.head.key_type && k.head.key_type == key_type) {
					//len = device_config_get_key_code_size(k.head.key_type);
					if (!rt_memcmp(&k.data, buf, length)) {
						result = i;
						break;
					}
				}
			}
		}
	}
	rt_mutex_release(device_config.mutex);
    return result;
}
/*
    钥匙信息初始化。
*/

void
device_config_key_head_init(struct key_head *kh, u16 key_type)
{
	rt_memset(kh, 0, sizeof(*kh));
    kh->is_updated = 0;
	kh->account = ACCOUNT_ID_INVALID;
    kh->key_type = key_type;
    kh->operation_type = OPERATION_TYPE_FOREVER;
    kh->updated_time = sys_cur_date();
    kh->start_time = 0;
    kh->end_time = 0;
}

/*
    创建钥匙。
    key_type: 钥匙类型。参检KEY_TYPE_FPRINT,KEY_TYPE_KBOARD，等宏定义。
    buf：钥匙的编码地址。
    length：编码长度。
    返回：
        -ECONFIG_ERROR， 创建失败。
        -ECONFIG_FULL, 钥匙库已满。
        -ECONFIG_EXIST, 钥匙已存在。
        >=0, 创建成功的钥匙编号。
*/
s32
device_config_key_create(u16 key_type, u8 *buf, u16 length)
{
    int result;
    u16 i;
	struct key k;

	result = -ECONFIG_ERROR;
	rt_mutex_take(device_config.mutex, RT_WAITING_FOREVER);
    rt_memset(&k, 0, sizeof(k));
	if (device_config_key_verify(key_type, buf, length) < 0) {
		for (i = 0; i < KEY_NUMBERS; i++) {
			if (!device_config_get_key_valid(i)) {
				device_config_key_head_init(&k.head, key_type);
				rt_memcpy(&k.data, buf, length);
				if (device_config_key_operate(i, &k, 1) < 0)
					goto __exit;
				device_config_set_key_valid(i, 1);
				if (device_config_file_operate(&device_config, 1) < 0)
					goto __exit;
				result = i;
				break;
			}
		}
        if (i == KEY_NUMBERS)
            result = -ECONFIG_FULL;
    } else {
		result = -ECONFIG_EXIST;
	}
__exit:
	rt_mutex_release(device_config.mutex);
    return result;
}

/*
    钥匙删除， 先删除钥匙与账户直接的关系映射，然后再使钥匙无效。
    key_id: 需要删除的钥匙ID。
    返回：
        -ECONFIG_ERROR,删除出错。
        >=0, 删除成功的钥匙ID。
*/
s32
device_config_key_delete(u16 key_id)
{
    s32 result = -ECONFIG_ERROR;

    RT_ASSERT(key_id < KEY_NUMBERS);
    rt_mutex_take(device_config.mutex, RT_WAITING_FOREVER);
    result = device_config_account_remove_key(key_id);
    if (result >= 0) {
        device_config_set_key_valid(key_id, 0);
        if (device_config_file_operate(&device_config, 1) >= 0)
            result = key_id;
    }
	rt_mutex_release(device_config.mutex);
    return result;
}
/*
    根据账户ID来读取账户信息。
    account_id: 账户ID
    ah：钥匙信息头指针。
    flag：1，写入；0，读出。
    返回：
        -ECONFIG_ERROR，读取出错。
        >=0, 成功读取账户的ID。
*/
s32
device_config_account_operate(u16 account_id, struct account_head *ah, u8 flag)
{
	s32 fd;
	s32 result;
	s32 len;

    RT_ASSERT(account_id < ACCOUNT_NUMBERS);

	result = -ECONFIG_ERROR;
	rt_mutex_take(device_config.mutex, RT_WAITING_FOREVER);
	fd = open(DEVICE_CONFIG_FILE_NAME, O_RDWR, 0x777);
	if (fd >= 0)
	{
		lseek(fd, DEVICE_CONFIG_FILE_ACCOUNT_OFFSET(account_id), SEEK_SET);
		if (flag)
			len = write(fd, ah, sizeof(*ah));
		else
			len = read(fd, ah, sizeof(*ah));
		if (len == sizeof(*ah))
			result = len;
		close(fd);
	}
	rt_mutex_release(device_config.mutex);
	return result;
}
/*
    检验账户的名称是否存在，存在则返回账户ID。
    buf: 检验账户的地址。
    length：账户的长度。
    返回：
        -ECONFIG_ERROR，账户库里没有这个账户。
        >=0, 成功校验返回账户的ID。
*/
s32
device_config_account_verify(u8 *name, u16 length)
{
	s32 result;
	u16 i;
    s32 len;
	struct account_head ah;

	RT_ASSERT(length <= ACCOUNT_NAME_LENGTH);
	result = -ECONFIG_ERROR;
	rt_mutex_take(device_config.mutex, RT_WAITING_FOREVER);
	for (i = 0; i < ACCOUNT_NUMBERS; i++) {
		if (device_config_get_account_valid(i)) {
			len = device_config_account_operate(i, &ah, 0);
			if (len >= 0) {
				if (!rt_memcmp(ah.name, name, length)) {
					result = i;
					break;
				}
			}
		}
	}
	rt_mutex_release(device_config.mutex);
	return result;
}

void
device_config_account_head_init(struct account_head *ah, u8 *name, u16 length)
{
    s32 i;
    rt_memset(ah, 0, sizeof(*ah));
    rt_memcpy(ah->name, name, length);
	ah->name[length] = '\0';
    ah->is_updated = 0;
    ah->updated_time = sys_cur_date();
    for (i = 0; i < ACCOUNT_KEY_NUMBERS; ++i)
		ah->key[i] = KEY_ID_INVALID;
    for (i = 0; i < ACCOUNT_PHONE_NUMBERS; ++i)
		ah->phone[i] = PHONE_ID_INVALID;
}
/*
    创建账户。
    name：账户的名称。
    length：账户长度。
    返回：
        -ECONFIG_ERROR， 创建失败。
        -ECONFIG_FULL, 账户库已满。
        -ECONFIG_EXIST, 账户已存在。
        >=0, 创建成功的账户ID。
*/
s32
device_config_account_create(u8 *name, u8 length)
{
    int result;
    u16 i;
	struct account_head ah;

	RT_ASSERT(length < ACCOUNT_NAME_LENGTH);
	result = -ECONFIG_ERROR;
	rt_mutex_take(device_config.mutex, RT_WAITING_FOREVER);
	if (device_config_account_verify(name, length) < 0) {
		for (i = 0; i < ACCOUNT_NUMBERS; i++) {
			if (!device_config_get_account_valid(i)) {
				device_config_account_head_init(&ah, name, length);
				if (device_config_account_operate(i, &ah, 1) < 0)
					goto __exit;
				device_config_set_account_valid(i, 1);
				if (device_config_file_operate(&device_config, 1) < 0)
					goto __exit;
				result = i;
				break;
			}
		}
        if (i == ACCOUNT_NUMBERS)
            result = -ECONFIG_FULL;
	} else {
		result = -ECONFIG_EXIST;
	}
__exit:
	rt_mutex_release(device_config.mutex);
    return result;
}

/*
    账户删除， 先删除钥匙与账户直接的关系映射，然后再使账户无效。
    key_id: 需要删除的账户ID。
    返回：
        -ECONFIG_ERROR,删除出错。
        >=0, 删除成功的账户ID。
*/
s32
device_config_account_delete(u16 account_id)
{
    s32 result, i;
    struct account_head ah;
    
    RT_ASSERT(account_id < ACCOUNT_NUMBERS);
	result = -ECONFIG_ERROR;
    rt_mutex_take(device_config.mutex, RT_WAITING_FOREVER);
	
    result = device_config_account_operate(account_id, &ah, 0);
    if (result >= 0) {
        for (i = 0; i < ACCOUNT_KEY_NUMBERS; ++i) {
            if (ah.key[i] != KEY_ID_INVALID)
                device_config_key_delete(ah.key[i]);
        }
    }
    device_config_set_account_valid(account_id, 0);
	if (device_config_file_operate(&device_config, 1) >= 0)
		result = account_id;

	rt_mutex_release(device_config.mutex);
    return result;
}
/*
    根据电话ID来读取电话信息。
    phone_id: 电话ID
    ph：电话信息
    flag：1，写入；0，读出。
    返回：
        -ECONFIG_ERROR，读取出错。
        >=0, 成功读取电话的ID。
*/
s32
device_config_phone_operate(u16 phone_id, struct phone_head *ph, u8 flag)
{
	s32 fd;
	s32 result;
	u16 len;

    RT_ASSERT(phone_id < PHONE_NUMBERS);

	result = -ECONFIG_ERROR;
	rt_mutex_take(device_config.mutex, RT_WAITING_FOREVER);
	fd = open(DEVICE_CONFIG_FILE_NAME, O_RDWR, 0x777);
	if (fd >= 0)
	{
		lseek(fd, DEVICE_CONFIG_FILE_PHONE_OFFSET(phone_id), SEEK_SET);
		if (flag)
			len = write(fd, ph, sizeof(*ph));
		else
			len = read(fd, ph, sizeof(*ph));
		if (len  == sizeof(*ph))
			result = len;
		close(fd);
	}

	rt_mutex_release(device_config.mutex);
	return result;
}
/* 电话信息头初始化*/

void
device_config_phone_head_init(struct phone_head *ph, u8 *buf, u8 length)
{
    rt_memset(ph->address, '\0', sizeof(ph->address));
    rt_memcpy(ph->address, buf, length);
    ph->address[PHONE_ADDRESS_LENGTH - 1] = '\0';
    ph->account = ACCOUNT_ID_INVALID;
    ph->auth = PHONE_AUTH_INVALID;
    ph->updated_time = sys_cur_date();
}
/*
    检验电话是否存在，存在则返回电话ID。
    buf: 电话存储区域。
    length：电话的长度。
    返回：
        -ECONFIG_ERROR，电话库里没有这把钥匙。
        >=0, 成功校验返回电话的ID。
*/
s32
device_config_phone_verify(u8 *buf, u16 length)
{
	s32 result;
	u16 i;
    s32 len;
	struct phone_head ph;

	RT_ASSERT(length <= PHONE_ADDRESS_LENGTH);
	result = -ECONFIG_ERROR;
	rt_mutex_take(device_config.mutex, RT_WAITING_FOREVER);
	for (i = 0; i < PHONE_NUMBERS; i++) {
		if (device_config_get_phone_valid(i)) {
			len = device_config_phone_operate(i, &ph, 0);
			if (len >= 0) {
				if (!rt_memcmp(ph.address, buf, length)) {
					result = i;
					break;
				}
			}
		}
	}

	rt_mutex_release(device_config.mutex);
	return result;
}

/*
    创建电话。
    buf：电话号码。
    length：电话长度。
    返回：
        -ECONFIG_ERROR， 创建失败。
        -ECONFIG_FULL, 电话库已满。
        -ECONFIG_EXIST, 电话已存在。
        >=0, 创建成功的电话ID。
*/
s32
device_config_phone_create(u8 *buf, u8 len)
{
    int result;
    u16 i;
	struct phone_head ph;

	RT_ASSERT(len <= PHONE_ADDRESS_LENGTH);
	result = -ECONFIG_ERROR;
	rt_mutex_take(device_config.mutex, RT_WAITING_FOREVER);
	if (device_config_phone_verify(buf, len) < 0) {
		for (i = 0; i < PHONE_NUMBERS; i++) {
			if (!device_config_get_phone_valid(i)) {
				device_config_phone_head_init(&ph, buf, len);
				if (device_config_phone_operate(i, &ph, 1) < 0)
					goto __exit;
				device_config_set_phone_valid(i, 1);
				if (device_config_file_operate(&device_config, 1) < 0)
					goto __exit;
				result = i;
				break;
			}
		}
        if (i == PHONE_NUMBERS)
            result = -ECONFIG_FULL;
	} else {
		result = -ECONFIG_EXIST;
	}
__exit:
	rt_mutex_release(device_config.mutex);
    return result;
}

s32
device_config_phone_delete(u16 phone_id)
{
    s32 result;
    RT_ASSERT(phone_id < PHONE_NUMBERS);

	result = -ECONFIG_ERROR;
    rt_mutex_take(device_config.mutex, RT_WAITING_FOREVER);
	device_config_set_phone_valid(phone_id, 0);
	if (device_config_file_operate(&device_config, 1) >= 0)
		result = phone_id;

	rt_mutex_release(device_config.mutex);
    return result;
}
/*
    在账户里获取钥匙空隙位置。
    ah：账户信息头。
    返回：账户钥匙空隙位置。
*/
s32
device_config_account_get_invalid_key_pos(struct account_head *ah)
{
	s32 result;
	s32 i;

	result = -ECONFIG_ERROR;
	for (i = 0; i < ACCOUNT_KEY_NUMBERS; ++i) {
		if (ah->key[i] == KEY_ID_INVALID) {
			result = i;
			break;
		}
	}
	return result;
}
/*
    根据钥匙ID在账户里查找钥匙位置。
    ah：账户信息头。
    key_id:钥匙id。
    返回：账户钥匙空隙位置。
*/
s32
device_config_account_get_key_pos(struct account_head *ah, u16 key_id)
{
	s32 result;
	s32 i;

	result = -ECONFIG_ERROR;
	for (i = 0; i < ACCOUNT_KEY_NUMBERS; ++i) {
		if (ah->key[i] == key_id) {
			result = i;
			break;
		}
	}
	return result;
}

/*
    将钥匙绑定到账户里（需要注意的3个情况，账户里可能已经绑定该钥匙，或者账户钥匙已满，或者钥匙已经绑定别的账户的情况。
    account_id：账户id。
    key_id；钥匙id。
    返回：
        -ECONFIG_ERROR， 绑定失败。
        -ECONFIG_FULL, 账户钥匙已满。
        -ECONFIG_EXIST, 账户钥匙已存在。
        >=0, 创建成功的钥匙ID。
*/
s32
device_config_account_append_key(u16 account_id, u16 key_id)
{
	s32 result;
	struct account_head ah;
	struct key k;
	s32 pos;

	RT_ASSERT(account_id < ACCOUNT_NUMBERS && key_id < KEY_NUMBERS);

	result = -ECONFIG_ERROR;
	rt_mutex_take(device_config.mutex, RT_WAITING_FOREVER);
	if (device_config_get_account_valid(account_id) &&
		device_config_account_operate(account_id, &ah, 0) >= 0) {
			pos = device_config_account_get_key_pos(&ah, key_id);
			/* key is not in account */
			if (pos < 0) {
				pos = device_config_account_get_invalid_key_pos(&ah);
				if (pos >= 0) {
					if (device_config_get_key_valid(key_id) &&
						device_config_key_operate(key_id, &k, 0) >= 0) {
						if (k.head.account != ACCOUNT_ID_INVALID) {
							device_config_account_remove_key(key_id);
						}
						k.head.account = account_id;
						device_config_key_operate(key_id, &k, 1);
						ah.key[pos] = key_id;
						device_config_account_operate(account_id, &ah, 1);
                        result = key_id;
					}
				} else {
					result = -ECONFIG_FULL;
				}
			} else {
				result = -ECONFIG_EXIST;
			}
	}
	rt_mutex_release(device_config.mutex);
//__exit:
	return result;
}

/*
    移除钥匙与账户之间的绑定关系。
    key_id：钥匙ID。
    返回：
        >=0, 钥匙ID
        -ECONFIG_ERROR,移除失败。
    
*/
s32
device_config_account_remove_key(u16 key_id)
{
	s32 result;
	struct account_head ah;
	struct key k;
	s32 pos;

	RT_ASSERT(key_id < KEY_NUMBERS);

	result = -ECONFIG_ERROR;
	rt_mutex_take(device_config.mutex, RT_WAITING_FOREVER);
	if (device_config_get_key_valid(key_id) &&
		device_config_key_operate(key_id, &k, 0) >= 0) {
		/* key has valid account */
		if (k.head.account != ACCOUNT_ID_INVALID) {
            if (device_config_get_account_valid(k.head.account) &&
				device_config_account_operate(k.head.account, &ah, 0) >= 0) {
				pos = device_config_account_get_key_pos(&ah, key_id);
				if (pos >= 0) {
					ah.key[pos] = KEY_ID_INVALID;
					device_config_account_operate(k.head.account, &ah, 1);
				}
			}
			k.head.account = ACCOUNT_ID_INVALID;
			device_config_key_operate(key_id, &k, 1);
		}
	}
	rt_mutex_release(device_config.mutex);
//__exit:
	return result;
}

s32 
device_config_key_index(int(*callback)(struct key *, void *, void *, void *), void *arg1, void *arg2)
{
    s32 result;
    u16 i;
	struct key k;
    s32 len;

	result = -ECONFIG_ERROR;

    rt_mutex_take(device_config.mutex, RT_WAITING_FOREVER);

	for (i = 0; i < KEY_NUMBERS; i++) {
		if (device_config_get_key_valid(i)) {
			len = device_config_key_operate(i, &k, 0);
			if (len >= 0) {
                result = callback(&k, arg1, arg2, &i);
			}
		}
	}
	rt_mutex_release(device_config.mutex);
    return result;
}

int
device_config_init(struct device_configure *config)
{
	int result;

	config->mutex = rt_mutex_create("m_config", RT_IPC_FLAG_FIFO);
	if (config->mutex == RT_NULL)
		return -ECONFIG_ERROR;
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
device_config_file_operate(struct device_configure *config, u8 flag)
{
	int fd;
	int cnts;
	int result = -ECONFIG_ERROR;

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
			RT_DEBUG_LOG(CONFIG_DEBUG, ("Creat Config File failure\n"));
			goto __exit;
		} else {
			RT_DEBUG_LOG(CONFIG_DEBUG, ("Creat Config File success\n"));
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

int
system_init(void)
{
    int result = 0;

    if (device_config_init(&device_config) < 0)
	{
		RT_DEBUG_LOG(CONFIG_DEBUG, ("device config init failure"));
		result = -ECONFIG_ERROR;
    }
    return result;
}

#ifdef RT_USING_FINSH
#include <finsh.h>

void device_config_phone_display(u16 phone_id)
{
	struct phone_head ph;
	device_config_phone_operate(phone_id, &ph, 0);
	rt_kprintf("phone_account = %x\n", ph.account);
	rt_kprintf("phone_auth = %x\n", ph.auth);
	rt_kprintf("phone_auth = %s\n", ph.address);
}

void device_config_key_display(u16 key_id)
{
	struct key k;
	device_config_key_operate(key_id, &k, 0);
	rt_kprintf("k_account = %x\n", k.head.account);
	rt_kprintf("k_is_updated = %x\n", k.head.is_updated);
	rt_kprintf("k_key_type = %x\n", k.head.key_type);
	rt_kprintf("k_operation_type = %x\n", k.head.operation_type);
	rt_kprintf("k_updated_time = %x\n", k.head.updated_time);
	rt_kprintf("k_start_time = %x\n", k.head.start_time);
	rt_kprintf("k_end_time = %x\n", k.head.end_time);

	print_hex(&k.data, device_config_get_key_code_size(k.head.key_type));
}

void device_config_account_display(u16 account_id)
{
	struct account_head ah;
    u32 i;
	device_config_account_operate(account_id, &ah, 0);
	rt_kprintf("account_is_updated = %x\n", ah.is_updated);
	rt_kprintf("account_updated_time = %x\n", ah.updated_time);
	rt_kprintf("account_name = %s\n", ah.name);
	rt_kprintf("account_key: ");
    for (i = 0; i < ACCOUNT_KEY_NUMBERS; ++i)
		rt_kprintf("%x ", ah.key[i]);
	rt_kprintf("\naccount_phone: ");
    for (i = 0; i < ACCOUNT_PHONE_NUMBERS; ++i)
		rt_kprintf("%x ", ah.phone[i]);
	rt_kprintf("\n");
}

FINSH_FUNCTION_EXPORT_ALIAS(device_config_phone_create, devcfg_pcr, [name len]);
FINSH_FUNCTION_EXPORT_ALIAS(device_config_get_phone_valid, devcfg_gpv, [phone_id]);
FINSH_FUNCTION_EXPORT_ALIAS(device_config_set_phone_valid, devcfg_spv, [phone_id value]);
FINSH_FUNCTION_EXPORT_ALIAS(device_config_phone_delete, devcfg_pd, [phone_id]);
FINSH_FUNCTION_EXPORT_ALIAS(device_config_phone_display, devcfg_pds, [phone_id]);

FINSH_FUNCTION_EXPORT_ALIAS(device_config_key_create, devcfg_kcr, [key_type buf length]);
FINSH_FUNCTION_EXPORT_ALIAS(device_config_get_key_valid, devcfg_gkv, [key_id]);
FINSH_FUNCTION_EXPORT_ALIAS(device_config_set_key_valid, devcfg_skv, [key_id value]);
FINSH_FUNCTION_EXPORT_ALIAS(device_config_key_delete, devcfg_kd, [key_id]);
FINSH_FUNCTION_EXPORT_ALIAS(device_config_key_display, devcfg_kds, [key_id]);

FINSH_FUNCTION_EXPORT_ALIAS(device_config_account_create, devcfg_acr, [name length]);
FINSH_FUNCTION_EXPORT_ALIAS(device_config_get_account_valid, devcfg_gav, [account_id]);
FINSH_FUNCTION_EXPORT_ALIAS(device_config_set_account_valid, devcfg_sav, [account_id value]);
FINSH_FUNCTION_EXPORT_ALIAS(device_config_account_delete, devcfg_ad, [account_id]);
FINSH_FUNCTION_EXPORT_ALIAS(device_config_account_display, devcfg_ads, [account_id]);

FINSH_FUNCTION_EXPORT_ALIAS(device_config_account_append_key, devcfg_aak, [account_id key_id]);

FINSH_FUNCTION_EXPORT_ALIAS(device_config_account_remove_key, devcfg_ark, [key_id]);

#endif
