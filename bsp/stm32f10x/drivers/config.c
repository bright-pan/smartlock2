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
#include "fprint.h"

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
		{0x12,0x34,0x56,0x78,0x90,0x12,0x34,0x56},
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
		//event valid map
		{
			{0,}
		},
	},
};

s32
device_config_account_remove_key(u16);
s32
device_config_account_remove_phone(u16);
/*
    根据钥匙ID获得钥匙的有效使用状况。
    返回：
        -ECONFIG_ERROR 该钥匙不可用。
        1 该钥匙有效。
        0 该钥匙无效。
*/
s32
device_config_get_key_valid(u16 key_id)
{
	s32 result = -ECONFIG_ERROR;
    if (key_id < KEY_NUMBERS) {
        rt_mutex_take(device_config.mutex, RT_WAITING_FOREVER);
        result = (device_config.param.kv_map.data[key_id/32] & bits_mask(key_id%32)) ? 1 : 0;
        rt_mutex_release(device_config.mutex);
    }
    return result;
}
/*
    根据钥匙ID，设置钥匙为有效（1），无效状态（0）。
    返回：
        -ECONFIG_ERROR 该钥匙不可用。
        >= 0, 返回钥匙id，成功设置。
*/
s32
device_config_set_key_valid(u16 key_id, u8 value)
{
    s32 result = -ECONFIG_ERROR;
	if (key_id < KEY_NUMBERS) {
        rt_mutex_take(device_config.mutex, RT_WAITING_FOREVER);
        if (value) {
            device_config.param.kv_map.data[key_id/32] |= bits_mask(key_id%32);
        } else {
            device_config.param.kv_map.data[key_id/32] &= ~bits_mask(key_id%32);
        }
        device_config.param.kv_map.updated_time = sys_cur_date();
        rt_mutex_release(device_config.mutex);
        result = key_id;
    }
    return result;
}
/*
    根据事件ID获得事件的有效使用状况。
    返回：
        -ECONFIG_ERROR 无法获得。
        1 有效。
        0 无效。
*/
s32
device_config_get_event_valid(u16 event_id)
{
	s32 result = -ECONFIG_ERROR;
    if (event_id < EVENT_NUMBERS) {
        rt_mutex_take(device_config.mutex, RT_WAITING_FOREVER);
        result = (device_config.param.ev_map.data[event_id/32] & bits_mask(event_id%32)) ? 1 : 0;
        rt_mutex_release(device_config.mutex);
    }
    return result;
}
/*
    根据事件ID，设置为有效（1），无效状态（0）。
    返回：
        -ECONFIG_ERROR 错误。
        >= 0, 返回id，成功设置。
*/
s32
device_config_set_event_valid(u16 event_id, u8 value)
{
    s32 result = -ECONFIG_ERROR;
	if (event_id < EVENT_NUMBERS) {
        rt_mutex_take(device_config.mutex, RT_WAITING_FOREVER);
        if (value) {
            device_config.param.ev_map.data[event_id/32] |= bits_mask(event_id%32);
        } else {
            device_config.param.ev_map.data[event_id/32] &= ~bits_mask(event_id%32);
        }
        device_config.param.ev_map.updated_time = sys_cur_date();
        rt_mutex_release(device_config.mutex);
        result = event_id;
    }
    return result;
}
s32
device_config_get_phone_valid(u16 phone_id)
{
	s32 result = -ECONFIG_ERROR;
    if (phone_id < PHONE_NUMBERS) {
        rt_mutex_take(device_config.mutex, RT_WAITING_FOREVER);
        result = (device_config.param.pv_map.data[phone_id/32] & bits_mask(phone_id%32)) ? 1 : 0;
        rt_mutex_release(device_config.mutex);
    }
    return result;
}

s32
device_config_set_phone_valid(u16 phone_id, u8 value)
{
    s32 result = -ECONFIG_ERROR;
	if (phone_id < PHONE_NUMBERS) {
        rt_mutex_take(device_config.mutex, RT_WAITING_FOREVER);
        if (value) {
            device_config.param.pv_map.data[phone_id/32] |= bits_mask(phone_id%32);
        } else {
            device_config.param.pv_map.data[phone_id/32] &= ~bits_mask(phone_id%32);
        }
        device_config.param.pv_map.updated_time = sys_cur_date();
        rt_mutex_release(device_config.mutex);
        result = phone_id;
    }
    return result;
}

s32
device_config_get_account_valid(u16 account_id)
{
	s32 result = -ECONFIG_ERROR;
    if (account_id < ACCOUNT_NUMBERS) {
        rt_mutex_take(device_config.mutex, RT_WAITING_FOREVER);
        result = (device_config.param.av_map.data[account_id/32] & bits_mask(account_id%32)) ? 1 : 0;
        rt_mutex_release(device_config.mutex);
    }
    return result;
}

s32
device_config_set_account_valid(u16 account_id, u8 value)
{
    s32 result = -ECONFIG_ERROR;
	if(account_id < ACCOUNT_NUMBERS) {
        rt_mutex_take(device_config.mutex, RT_WAITING_FOREVER);
        if (value) {
            device_config.param.av_map.data[account_id/32] |= bits_mask(account_id%32);
        } else {
            device_config.param.av_map.data[account_id/32] &= ~bits_mask(account_id%32);
        }
        device_config.param.av_map.updated_time = sys_cur_date();
        rt_mutex_release(device_config.mutex);
        result = account_id;
    }
    return result;
}

s32
device_config_get_key_code_size(u16 key_type)
{
	s32 result = -ECONFIG_ERROR;

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
        case KEY_TYPE_RF433:
			{
				result = KEY_RF433_CODE_SIZE;
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

    result = -ECONFIG_ERROR;

    if (key_id >= KEY_NUMBERS)
        return result;

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
    根据事件ID来读取钥匙信息。
    event_id: 事件ID
    e：钥匙信息
    flag：1，写入；0，读出。
    返回：
        -ECONFIG_ERROR，读取出错。
        >=0, 成功读取的ID。
*/
s32
device_config_event_operate(u16 event_id, struct event *e, u8 flag)
{
	s32 fd;
	s32 result;
	s32 len;

    result = -ECONFIG_ERROR;

    if (event_id >= EVENT_NUMBERS)
        return result;

	rt_mutex_take(device_config.mutex, RT_WAITING_FOREVER);
	fd = open(DEVICE_CONFIG_FILE_NAME, O_RDWR, 0x777);
	if (fd >= 0)
	{
		lseek(fd, DEVICE_CONFIG_FILE_EVENT_OFFSET(event_id), SEEK_SET);
		if (flag)
			len = write(fd, e, sizeof(*e));
		else
			len = read(fd,e, sizeof(*e));
		if (len == sizeof(*e))
			result = len;
		close(fd);
	}
	rt_mutex_release(device_config.mutex);
	return result;
}

/*
    创建事件。
    event_id: 指定ID，
            这个ID如果被占用，则返回错误；
            如果是无效ID则自动创建最小可用ID。
    event_type: 事件类型。参检EVENT_TYPE_ALARM,EVENT_TYPE_UNLOCK，等宏定义。
    is_updated: 更新标志。
    ed： 事件数据。
    返回：
        -ECONFIG_ERROR， 创建失败。
        -ECONFIG_FULL, 库已满。
        -ECONFIG_EXIST, 已存在。
        >=0, 创建成功的编号。
*/
s32
device_config_event_create(u16 event_id, u16 event_type, u8 is_updated, union event_data *ed)
{
    int result;
    s32 i;
	struct event e;

	result = -ECONFIG_ERROR;
    if (event_id >= EVENT_NUMBERS && event_id != EVENT_ID_INVALID)
        return result;
    rt_memset(&e, 0, sizeof(e));
    if (event_id != EVENT_ID_INVALID) {
        i = event_id;
        if (!device_config_get_event_valid(i)) {
            e.head.is_updated = is_updated;
            e.head.updated_time = sys_cur_date();
            e.head.event_type = event_type;
            e.data = *ed;
            if (device_config_event_operate(i, &e, 1) < 0)
                goto __exit;
            device_config_set_event_valid(i, 1);
            if (device_config_file_operate(&device_config, 1) < 0)
                goto __exit;
            result = i;
        }
    } else {
        for (i = 0; i < EVENT_NUMBERS; i++) {
            if (!device_config_get_event_valid(i)) {
                e.head.is_updated = is_updated;
                e.head.updated_time = sys_cur_date();
                e.head.event_type = event_type;
                e.data = *ed;
                if (device_config_event_operate(i, &e, 1) < 0)
                    goto __exit;
                device_config_set_event_valid(i, 1);
                if (device_config_file_operate(&device_config, 1) < 0)
                    goto __exit;
                result = i;
                break;
            }
        }
        if (i == EVENT_NUMBERS)
            result = -ECONFIG_FULL;
    }
__exit:
    return result;
}
/*
    事件删除。
    event_id: 需要删除的ID。
    返回：
        -ECONFIG_ERROR,删除出错。
        >=0, 删除成功的ID。
*/
s32
device_config_event_delete(u16 event_id)
{
    s32 result = -ECONFIG_ERROR;
    
    if (event_id >= EVENT_NUMBERS)
        return result;
    device_config_set_event_valid(event_id, 0);
    if (device_config_file_operate(&device_config, 1) >= 0)
        result = event_id;
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
    s32 i;
	struct key *k;
    s32 len;

	result = -ECONFIG_ERROR;
    k = rt_malloc(sizeof(*k));
    if (k == RT_NULL)
        goto __exit;
	for (i = 0; i < KEY_NUMBERS; i++) {
		if (device_config_get_key_valid(i) > 0) {
			len = device_config_key_operate(i, k, 0);
			if (len >= 0) {
				if (k->head.key_type && k->head.key_type == key_type) {
					//len = device_config_get_key_code_size(k.head.key_type);
					if (!rt_memcmp(&k->data, buf, length)) {
						result = i;
						break;
					}
				}
			}
		}
	}
__exit:
    rt_free(k);
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
    kh->operation_type = KEY_OPERATION_TYPE_FOREVER;
    kh->updated_time = sys_cur_date();
    kh->start_time = 0;
    kh->end_time = 0;
}

/*
    创建钥匙。
    key_id: 指定钥匙ID，
            这个ID如果被占用，则返回错误；
            如果是无效ID则自动创建最小可用ID。
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
device_config_key_create(u16 key_id, u16 key_type, void *buf, u16 length)
{
    int result;
    s32 i;
	struct key k;

	result = -ECONFIG_ERROR;
    if (key_id >= KEY_NUMBERS && key_id != KEY_ID_INVALID)
        return result;
    rt_memset(&k, 0, sizeof(k));
    i = device_config_get_key_code_size(key_type);
    length = length > i ? i :length; 
	if (device_config_key_verify(key_type, buf, length) < 0) {
        if (key_id != KEY_ID_INVALID) {
            i = key_id;
            if (!device_config_get_key_valid(i)) {
                device_config_key_head_init(&k.head, key_type);
                rt_memcpy(&k.data, buf, length);
                if (device_config_key_operate(i, &k, 1) < 0)
                    goto __exit;
                device_config_set_key_valid(i, 1);
                if (device_config_file_operate(&device_config, 1) < 0)
                    goto __exit;
                result = i;
            }
        } else {
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
        }
    } else {
		result = -ECONFIG_EXIST;
	}
__exit:
    return result;
}

s32
device_config_key_set(u16 key_id, struct key *new_key, u32 op_time)
{
    s32 result;
    s32 i;
	struct key k;
    s32 len;

	result = -ECONFIG_ERROR;
    
    rt_memset(&k, 0, sizeof(k));
    i = key_id;
    if (device_config_get_key_valid(i) > 0) {
            len = device_config_key_operate(i, &k, 0);
            if (len >= 0) {
                    if (op_time > k.head.updated_time) {
                        len = device_config_key_verify(new_key->head.key_type, (u8 *)&new_key->data, device_config_get_key_code_size(new_key->head.key_type));
                        if (len < 0 || len == i) {
                            k.head.is_updated = 0;
                            k.head.key_type = new_key->head.key_type;
                            k.head.operation_type = new_key->head.operation_type;
                            k.head.start_time = new_key->head.start_time;
                            k.head.end_time = new_key->head.end_time;
                            k.head.updated_time = op_time;
                            k.data = new_key->data;
                            len = device_config_key_operate(i, &k, 1);
                            if (len >= 0)
                                result = i;
                        } else {
                            result = -ECONFIG_EXIST;
                        }
                    }
            }

    } else {
        len = device_config_key_create(i, new_key->head.key_type, &new_key->data, device_config_get_key_code_size(new_key->head.key_type));
        if (len >= 0) {
            len = device_config_key_operate(i, &k, 0);
            if (len >= 0) {
                k.head.is_updated = 0;
                k.head.key_type = new_key->head.key_type;
                k.head.operation_type = new_key->head.operation_type;
                k.head.start_time = new_key->head.start_time;
                k.head.end_time = new_key->head.end_time;
                k.head.updated_time = op_time;
                len = device_config_key_operate(i, &k, 1);
                if (len >= 0)
                    result = i;
            }
        } else {
            result = len;
        }
    }
    return result;
}

/*
    钥匙删除， 先删除钥匙与账户直接的关系映射，然后再使钥匙无效。
    key_id: 需要删除的钥匙ID。
    op_time: 操作时间。
    flag：使用操作时间比较, 1,比较，0，不比较。
    返回：
        -ECONFIG_ERROR,删除出错。
        >=0, 删除成功的钥匙ID。
*/
s32
device_config_key_delete(u16 key_id, u32 op_time, u8 flag)
{
    s32 result = -ECONFIG_ERROR;
    struct key k;
    if (key_id >= KEY_NUMBERS)
        return result;
    if (flag) {
        if (device_config_get_key_valid(key_id) > 0 && 
            device_config_key_operate(key_id, &k, 0) >= 0) {
            if (op_time > k.head.updated_time)
            {
                result = device_config_account_remove_key(key_id);
                if (result >= 0) {
                    device_config_set_key_valid(key_id, 0);
                    if (device_config_file_operate(&device_config, 1) >= 0) {
                        if (k.head.key_type == KEY_TYPE_FPRINT)
                            fp_delete(key_id, 1);
                        result = key_id;
                    }
                }
            }
        }
    } else {
        result = device_config_account_remove_key(key_id);
        if (result >= 0) {
            device_config_set_key_valid(key_id, 0);
            if (device_config_file_operate(&device_config, 1) >= 0)
                result = key_id;
        }
    }
    return result;
}

/*
    计算有效钥匙的数量。
    返回：
        -ECONFIG_ERROR，没有获得有效的数据。
        >=0, 有效数量。
*/
s32
device_config_key_counts(void)
{
	s32 result;
    s32 counts;
	s32 i;

    counts = 0;
	result = -ECONFIG_ERROR;
	for (i = 0; i < KEY_NUMBERS; i++) {
		if (device_config_get_key_valid(i) > 0) {
            ++counts;
		}
	}
    result = counts;
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
	s32 len;

	result = -ECONFIG_ERROR;
    if (phone_id >= PHONE_NUMBERS)
        return result;

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
    ph->account = ACCOUNT_ID_INVALID;
    ph->auth = PHONE_AUTH_SMS;
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
	s32 i;
    s32 len;
	struct phone_head ph;
    u8 temp[PHONE_ADDRESS_LENGTH];
    result = -ECONFIG_ERROR;
	if (length > PHONE_ADDRESS_LENGTH)
        return result;

    rt_memset(temp, '\0', PHONE_ADDRESS_LENGTH);
    rt_memcpy(temp, buf, length);
	for (i = 0; i < PHONE_NUMBERS; i++) {
		if (device_config_get_phone_valid(i)) {
			len = device_config_phone_operate(i, &ph, 0);
			if (len >= 0) {
				if (!rt_memcmp(ph.address, temp, PHONE_ADDRESS_LENGTH)) {
					result = i;
					break;
				}
			}
		}
	}
	return result;
}

/*
    创建电话。
    phone_id:电话ID
    buf：电话号码。
    length：电话长度。
    返回：
        -ECONFIG_ERROR， 创建失败。
        -ECONFIG_FULL, 电话库已满。
        -ECONFIG_EXIST, 电话已存在。
        >=0, 创建成功的电话ID。
*/
s32
device_config_phone_create(u16 phone_id, u8 *buf, u8 length)
{
    int result;
    s32 i;
	struct phone_head ph;
    u8 temp[PHONE_ADDRESS_LENGTH];
    result = -ECONFIG_ERROR;
	if ((phone_id >= PHONE_NUMBERS && phone_id != PHONE_ID_INVALID) || length > PHONE_ADDRESS_LENGTH)
        return result;
    rt_memset(temp, '\0', PHONE_ADDRESS_LENGTH);
    rt_memcpy(temp, buf, length);
	if (device_config_phone_verify(temp, PHONE_ADDRESS_LENGTH) < 0) {
        if (phone_id == PHONE_ID_INVALID) {
            for (i = 0; i < PHONE_NUMBERS; i++) {
                if (!device_config_get_phone_valid(i)) {
                    device_config_phone_head_init(&ph, temp, PHONE_ADDRESS_LENGTH);
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
            i = phone_id;
            if (!device_config_get_phone_valid(i)) {
                device_config_phone_head_init(&ph, temp, PHONE_ADDRESS_LENGTH);
                if (device_config_phone_operate(i, &ph, 1) < 0)
                    goto __exit;
                device_config_set_phone_valid(i, 1);
                if (device_config_file_operate(&device_config, 1) < 0)
                    goto __exit;
                result = i;
            }
        }
	} else {
		result = -ECONFIG_EXIST;
	}
__exit:
    return result;
}
/*
    创建账户。
    phone_id: 指定手机ID，如果为无效ID则选则最小无效ID。
    buf：手机号码。
    length：账户长度。
    auth: 权限
    op_time: 操作时间。
    返回：
        -ECONFIG_ERROR， 创建失败。
        -ECONFIG_FULL, 账户库已满。
        -ECONFIG_EXIST, 账户已存在。
        >=0, 创建成功的账户ID。
*/
s32
device_config_phone_set(u16 phone_id, u8 *buf, u8 length, u16 auth, u32 op_time)
{
    s32 result;
    s32 i;
    struct phone_head ph;
    s32 len;

    result = -ECONFIG_ERROR;
	if (phone_id >= PHONE_NUMBERS || length > PHONE_ADDRESS_LENGTH)
        return result;
    i = phone_id;
    if (device_config_get_phone_valid(i) > 0) {
            len = device_config_phone_operate(i, &ph, 0);
            if (len >= 0) {
                if (op_time > ph.updated_time) { 
                    len = device_config_phone_verify(buf, length);
                    if (len < 0 || len == i) {
                        rt_memcpy(ph.address, buf, ACCOUNT_NAME_LENGTH);
                        ph.auth = auth;
                        ph.updated_time = op_time;
                        len = device_config_phone_operate(i, &ph, 1);
                        if (len >= 0)
                            result = i;
                    } else {
                        result = -ECONFIG_EXIST;
                    }
                }
            }
    } else {
        len = device_config_phone_create(i, buf, length);
        if (len >= 0) {
            result = i;
        } else {
            result = len;
        }
    }
    return result;
}
/*
    手机删除， 先删除手机与账户直接的关系映射，然后再使手机无效。
    phone_id: 需要删除的手机ID。
    op_time: 操作时间。
    flag：使用操作时间比较, 1,比较，0，不比较。
    返回：
        -ECONFIG_ERROR,删除出错。
        >=0, 删除成功的手机ID。
*/
s32
device_config_phone_delete(u16 phone_id, u32 op_time, u8 flag)
{
    s32 result;
    struct phone_head ph;
	result = -ECONFIG_ERROR;

    if (phone_id >= PHONE_NUMBERS)
        return result;

    if (flag) {
        if (device_config_get_phone_valid(phone_id) > 0 && 
            device_config_phone_operate(phone_id, &ph, 0) >= 0) {
            if (op_time > ph.updated_time)
            {
                result = device_config_account_remove_phone(phone_id);
                if (result >= 0) {
                    device_config_set_phone_valid(phone_id, 0);
                    if (device_config_file_operate(&device_config, 1) >= 0)
                        result = phone_id;
                }
            }
        }
    } else {
        result = device_config_account_remove_phone(phone_id);
        if (result >= 0) {
            device_config_set_phone_valid(phone_id, 0);
            if (device_config_file_operate(&device_config, 1) >= 0)
                result = phone_id;
        }
    }
    return result;
}
/*
    计算有效手机的数量。
    返回：
        -ECONFIG_ERROR，没有获得有效的数据。
        >=0, 有效数量。
*/
s32
device_config_phone_counts(void)
{
	s32 result;
    s32 counts;
	s32 i;

    counts = 0;
	result = -ECONFIG_ERROR;
	for (i = 0; i < PHONE_NUMBERS; i++) {
		if (device_config_get_phone_valid(i) > 0) {
            ++counts;
		}
	}
    result = counts;
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

	result = -ECONFIG_ERROR;
    if (account_id >= ACCOUNT_NUMBERS)
        return result;

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
	s32 i;
    s32 len;
	struct account_head ah;
    u8 temp[ACCOUNT_NAME_LENGTH];
	result = -ECONFIG_ERROR;
	if (length > ACCOUNT_NAME_LENGTH)
        return result;
    rt_memset(temp, '\0', ACCOUNT_NAME_LENGTH);
    rt_memcpy(temp, name, length);
	for (i = 0; i < ACCOUNT_NUMBERS; i++) {
		if (device_config_get_account_valid(i) > 0) {
			len = device_config_account_operate(i, &ah, 0);
			if (len >= 0) {
				if (!rt_memcmp(ah.name, temp, ACCOUNT_NAME_LENGTH)) {
					result = i;
					break;
				}
			}
		}
	}
	return result;
}
/*
    根据账户ID和钥匙起始位置查找下一个有效钥匙ID。
    account_id：账户ID。
    key_pos：钥匙起始位置。
    flag : 1，向前找；0，向后找。
    返回：
        -ECONFIG_ERROR，获取错误。
        >=0, 成功校验返回账户中下一个有效钥匙的位置ID。
*/
s32
device_config_account_next_key_pos(u16 account_id, u8 key_pos, u8 flag)
{
	s32 result;
	s32 i;
    s32 len;
	struct account_head ah;

	result = -ECONFIG_ERROR;
	if (account_id >= ACCOUNT_NUMBERS || key_pos >= ACCOUNT_KEY_NUMBERS)
        return result;

    if (device_config_get_account_valid(account_id) > 0) {
        len = device_config_account_operate(i, &ah, 0);
        if (len >= 0) {
            if (flag) {
                for (i = key_pos; i < ACCOUNT_KEY_NUMBERS; ++i) {
                    if (ah.key[i] != KEY_ID_INVALID)
                        result = i;
                        break;
                }
            } else {
                for (i = key_pos; i >= 0; --i) {
                    if (ah.key[i] != KEY_ID_INVALID)
                        result = i;
                        break;
                }
            }
        }
    }
	return result;
}

/*
    根据账户ID计算账户内有效钥匙数量。
    account_id：账户ID。
    返回：
        -ECONFIG_ERROR，获取错误。
        >=0, 返回账户中有效钥匙数量。
*/
s32
device_config_account_key_counts(u16 account_id)
{
	s32 result;
	s32 i;
    s32 len;
    s32 counts;
	struct account_head ah;

    counts = 0;
	result = -ECONFIG_ERROR;
	if (account_id >= ACCOUNT_NUMBERS)
        return result;

    if (device_config_get_account_valid(account_id) > 0) {
        len = device_config_account_operate(i, &ah, 0);
        if (len >= 0) {
            for (i = 0; i < ACCOUNT_KEY_NUMBERS; ++i) {
                if (ah.key[i] != KEY_ID_INVALID)
                    ++counts;
            }
            result = counts;
        }
    }
	return result;
}
/*
    检验账户ID后面是否有正确的ID，存在则返回账户ID。
    account_id：起始ID
    flag : 1，向前找；0，向后找。
    返回：
        -ECONFIG_ERROR，账户库里没有有效账户。
        >=0, 成功校验返回账户的ID。
*/
s32
device_config_account_next_valid(u16 account_id, u8 flag)
{
	s32 result;
	s32 i;

	result = -ECONFIG_ERROR;
    if (account_id >= ACCOUNT_NUMBERS)
        return result;

    if (flag) {
        for (i = account_id; i < ACCOUNT_NUMBERS; ++i) {
            if (device_config_get_account_valid(i) > 0) {
                result = i;
                break;
            }
        }
    } else {
        for (i = account_id; i >=0; --i) {
            if (device_config_get_account_valid(i) > 0) {
                result = i;
                break;
            }
        }
    }
	return result;
}
/*
    计算有效账户的数量。
    返回：
        -ECONFIG_ERROR，账户库里没有有效账户。
        >=0, 有效账户数量。
*/
s32
device_config_account_counts(void)
{
	s32 result;
    s32 counts;
	s32 i;

    counts = 0;
	result = -ECONFIG_ERROR;
	for (i = 0; i < ACCOUNT_NUMBERS; i++) {
		if (device_config_get_account_valid(i) > 0) {
            ++counts;
		}
	}
    result = counts;
	return result;
}
void
device_config_account_head_init(struct account_head *ah, u8 *name, u16 length)
{
    s32 i;
    rt_memset(ah, 0, sizeof(*ah));
    rt_memcpy(ah->name, name, length);
    ah->is_updated = 0;
    ah->updated_time = sys_cur_date();
    for (i = 0; i < ACCOUNT_KEY_NUMBERS; ++i)
		ah->key[i] = KEY_ID_INVALID;
    for (i = 0; i < ACCOUNT_PHONE_NUMBERS; ++i)
		ah->phone[i] = PHONE_ID_INVALID;
}
/*
    创建账户。
    account_id: 指定账户ID，如果为无效ID则选则最小无效ID。
    name：账户的名称，如果为空，则自动生成USER_XX格式的账户名称。
    length：账户长度。
    返回：
        -ECONFIG_ERROR， 创建失败。
        -ECONFIG_FULL, 账户库已满。
        -ECONFIG_EXIST, 账户已存在。
        >=0, 创建成功的账户ID。
*/
s32
device_config_account_create(u16 account_id, u8 *name, u8 length)
{
    int result;
    u16 i;
	struct account_head ah;
    u8 temp[ACCOUNT_NAME_LENGTH];
    
	result = -ECONFIG_ERROR;
	if ((account_id >= ACCOUNT_NUMBERS && account_id != ACCOUNT_ID_INVALID) || length > ACCOUNT_NAME_LENGTH)
        return result;

    if (account_id == ACCOUNT_ID_INVALID) {
        if (name != RT_NULL) {
            rt_memset(temp, '\0', ACCOUNT_NAME_LENGTH);
            rt_memcpy(temp, name, length);
            if (device_config_account_verify(temp, ACCOUNT_NAME_LENGTH) < 0) {
                for (i = 0; i < ACCOUNT_NUMBERS; i++) {
                    if (!device_config_get_account_valid(i)) {
                        device_config_account_head_init(&ah, temp, ACCOUNT_NAME_LENGTH);
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
        } else {
            for (i = 0; i < ACCOUNT_NUMBERS; i++) {
                if (!device_config_get_account_valid(i)) {
                    rt_memset(temp, '\0', ACCOUNT_NAME_LENGTH);
                    snprintf((char *)temp, ACCOUNT_NAME_LENGTH, "USER%d", i);
                    device_config_account_head_init(&ah, temp, ACCOUNT_NAME_LENGTH);
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
        }
    } else {
        if (name != RT_NULL) {
            rt_memset(temp, '\0', ACCOUNT_NAME_LENGTH);
            rt_memcpy(temp, name, length);
            if (device_config_account_verify(temp, ACCOUNT_NAME_LENGTH) < 0) {
                i = account_id;
                if (!device_config_get_account_valid(i)) {
                    device_config_account_head_init(&ah, temp, ACCOUNT_NAME_LENGTH);
                    if (device_config_account_operate(i, &ah, 1) < 0)
                        goto __exit;
                    device_config_set_account_valid(i, 1);
                    if (device_config_file_operate(&device_config, 1) < 0)
                        goto __exit;
                    result = i;
                }
                if (i == ACCOUNT_NUMBERS)
                    result = -ECONFIG_FULL;
            } else {
                result = -ECONFIG_EXIST;
            }
        } else {
            i = account_id;
            if (!device_config_get_account_valid(i)) {
                rt_memset(temp, '\0', ACCOUNT_NAME_LENGTH);
                snprintf((char *)temp, ACCOUNT_NAME_LENGTH, "USER%d", i);
                device_config_account_head_init(&ah, temp, ACCOUNT_NAME_LENGTH);
                if (device_config_account_operate(i, &ah, 1) < 0)
                    goto __exit;
                device_config_set_account_valid(i, 1);
                if (device_config_file_operate(&device_config, 1) < 0)
                    goto __exit;
                result = i;
            }
            if (i == ACCOUNT_NUMBERS)
                result = -ECONFIG_FULL;
        }
    }
__exit:
    return result;
}
/*
    创建账户。
    account_id: 指定账户ID，如果为无效ID则选则最小无效ID。
    name：账户的名称，如果为空，则自动生成USER_XX格式的账户名称。
    length：账户长度。
    op_time: 操作时间。
    返回：
        -ECONFIG_ERROR， 创建失败。
        -ECONFIG_FULL, 账户库已满。
        -ECONFIG_EXIST, 账户已存在。
        >=0, 创建成功的账户ID。
*/
s32
device_config_account_set(u16 account_id, u8 *name, u8 length, u32 op_time)
{
    s32 result;
    s32 i;
    struct account_head ah;
    s32 len;
    u8 temp[ACCOUNT_NAME_LENGTH];

    result = -ECONFIG_ERROR;

    i = account_id;
    if (device_config_get_account_valid(i) > 0) {
            len = device_config_account_operate(i, &ah, 0);
            if (len >= 0) {
                if (op_time > ah.updated_time) {
                    rt_memset(temp, '\0', ACCOUNT_NAME_LENGTH);
                    rt_memcpy(temp, name, length);
                    len = device_config_account_verify(temp, ACCOUNT_NAME_LENGTH);
                    if (len < 0 || len == i) {
                        rt_memcpy(ah.name, temp, ACCOUNT_NAME_LENGTH);
                        ah.updated_time = op_time;
                        len = device_config_account_operate(i, &ah, 1);
                        if (len >= 0)
                            result = i;
                    } else {
                        result = -ECONFIG_EXIST;
                    }
                }
                
            }

    } else {
        result = device_config_account_create(i, name, length);
    }
    return result;
}

/*
    账户删除， 先删除钥匙与账户直接的关系映射，然后再使账户无效。
    account_id: 需要删除的账户ID。
    op_time: 操作时间。
    flag：使用操作时间比较, 1,比较，0，不比较。
    返回：
        -ECONFIG_ERROR,删除出错。
        >=0, 删除成功的账户ID。
*/
s32
device_config_account_delete(u16 account_id, u32 op_time, u8 flag)
{
    s32 result, i;
    struct account_head ah;
	result = -ECONFIG_ERROR;
    if (account_id >= ACCOUNT_NUMBERS)
        return result;

    result = device_config_account_operate(account_id, &ah, 0);
    if (result >= 0) {
        if (flag) {
            if (op_time > ah.updated_time) {
                for (i = 0; i < ACCOUNT_KEY_NUMBERS; ++i) {
                    if (ah.key[i] != KEY_ID_INVALID)
                        device_config_key_delete(ah.key[i], 0, 0);
                    }
                for (i = 0; i < ACCOUNT_PHONE_NUMBERS; ++i) {
                    if (ah.phone[i] != PHONE_ID_INVALID)
                        device_config_phone_delete(ah.phone[i], 0, 0);
                }
                device_config_set_account_valid(account_id, 0);
                if (device_config_file_operate(&device_config, 1) >= 0)
                    result = account_id;
            }
        } else {
            for (i = 0; i < ACCOUNT_KEY_NUMBERS; ++i) {
                if (ah.key[i] != KEY_ID_INVALID)
                    device_config_key_delete(ah.key[i], 0, 0);
            }
            for (i = 0; i < ACCOUNT_PHONE_NUMBERS; ++i) {
                if (ah.phone[i] != PHONE_ID_INVALID)
                    device_config_phone_delete(ah.phone[i], 0, 0);
            }
            device_config_set_account_valid(account_id, 0);
            if (device_config_file_operate(&device_config, 1) >= 0)
                result = account_id;
        }
    }
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
    u16 temp;

	result = -ECONFIG_ERROR;
	for (i = 0; i < ACCOUNT_KEY_NUMBERS; ++i) {
        rt_mutex_take(device_config.mutex, RT_WAITING_FOREVER);
		temp = ah->key[i];
        rt_mutex_release(device_config.mutex);
        if (temp == KEY_ID_INVALID) {
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
    u16 temp;

	result = -ECONFIG_ERROR;
	for (i = 0; i < ACCOUNT_KEY_NUMBERS; ++i) {
        rt_mutex_take(device_config.mutex, RT_WAITING_FOREVER);
		temp = ah->key[i];
        rt_mutex_release(device_config.mutex);
		if (temp == key_id) {
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
    op_time: 操作时间。
    flag：使用操作时间比较, 1,比较，0，不比较。
    返回：
        -ECONFIG_ERROR， 绑定失败。
        -ECONFIG_FULL, 账户钥匙已满。
        -ECONFIG_EXIST, 账户钥匙已存在。
        >=0, 创建成功的钥匙ID。
*/
s32
device_config_account_append_key(u16 account_id, u16 key_id, u32 op_time, u8 flag)
{
	s32 result;
	struct account_head ah;
	struct key k;
	s32 pos;

	result = -ECONFIG_ERROR;
	if (account_id >= ACCOUNT_NUMBERS || key_id >= KEY_NUMBERS)
        return result;

	if (device_config_get_account_valid(account_id) > 0 &&
		device_config_account_operate(account_id, &ah, 0) >= 0) {
			pos = device_config_account_get_key_pos(&ah, key_id);
			/* key is not in account */
			if (pos < 0) {
				pos = device_config_account_get_invalid_key_pos(&ah);
				if (pos >= 0) {
					if (device_config_get_key_valid(key_id) > 0 &&
						device_config_key_operate(key_id, &k, 0) >= 0) {
                        if (flag) {
                            if (op_time >= k.head.updated_time) {
                                if (k.head.account != ACCOUNT_ID_INVALID) {
                                    device_config_account_remove_key(key_id);
                                }
                                k.head.account = account_id;
                                k.head.is_updated = 0;
                                k.head.updated_time = op_time;
                                device_config_key_operate(key_id, &k, 1);
                                ah.key[pos] = key_id;
                                ah.is_updated = 0;
                                ah.updated_time = op_time;
                                device_config_account_operate(account_id, &ah, 1);
                                result = key_id;
                            }
                        } else {
                            if (k.head.account != ACCOUNT_ID_INVALID) {
                                device_config_account_remove_key(key_id);
                            }
                            k.head.account = account_id;
                            k.head.is_updated = 1;
                            k.head.updated_time = sys_cur_date();
                            device_config_key_operate(key_id, &k, 1);
                            ah.key[pos] = key_id;
                            ah.is_updated = 1;
                            ah.updated_time = k.head.updated_time;
                            device_config_account_operate(account_id, &ah, 1);
                            result = key_id;
                        }
					}
				} else {
					result = -ECONFIG_FULL;
				}
			} else {
				result = -ECONFIG_EXIST;
			}
	}
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

	result = -ECONFIG_ERROR;
	if (key_id >= KEY_NUMBERS)
        return result;

	if (device_config_get_key_valid(key_id) > 0 &&
		device_config_key_operate(key_id, &k, 0) >= 0) {
		/* key has valid account */
		if (k.head.account != ACCOUNT_ID_INVALID) {
            if (device_config_get_account_valid(k.head.account) > 0 &&
				device_config_account_operate(k.head.account, &ah, 0) >= 0) {
				pos = device_config_account_get_key_pos(&ah, key_id);
				if (pos >= 0) {
					ah.key[pos] = KEY_ID_INVALID;
                    ah.updated_time = sys_cur_date();
					device_config_account_operate(k.head.account, &ah, 1);
                    result = key_id;
				}
			}
			k.head.account = ACCOUNT_ID_INVALID;
            k.head.updated_time = ah.updated_time;
			device_config_key_operate(key_id, &k, 1);
		} else {
            result = key_id;
        }
	}
//__exit:
	return result;
}

/*
    在账户里获取手机空隙位置。
    ah：账户信息头。
    返回：账户手机空隙位置。
*/
s32
device_config_account_get_invalid_phone_pos(struct account_head *ah)
{
	s32 result;
	s32 i;
    u16 temp;

	result = -ECONFIG_ERROR;
	for (i = 0; i < ACCOUNT_PHONE_NUMBERS; ++i) {
        rt_mutex_take(device_config.mutex, RT_WAITING_FOREVER);
		temp = ah->phone[i];
        rt_mutex_release(device_config.mutex);
		if (temp == PHONE_ID_INVALID) {
			result = i;
			break;
		}
	}
	return result;
}
/*
    根据钥匙ID在账户里查找手机位置。
    ah：账户信息头。
    phone_id:手机id。
    返回：账户手机空隙位置。
*/
s32
device_config_account_get_phone_pos(struct account_head *ah, u16 phone_id)
{
	s32 result;
	s32 i;
    u16 temp;
    
	result = -ECONFIG_ERROR;
	for (i = 0; i < ACCOUNT_PHONE_NUMBERS; ++i) {        
        rt_mutex_take(device_config.mutex, RT_WAITING_FOREVER);
		temp = ah->phone[i];
        rt_mutex_release(device_config.mutex);
		if (temp == phone_id) {
			result = i;
			break;
		}
	}
	return result;
}

/*
    将手机绑定到账户里（需要注意的3个情况，账户里可能已经绑定该手机，或者账户手机已满，或者手机已经绑定别的账户的情况。
    account_id：账户id。
    phone_id；手机id。
    op_time: 操作时间。
    flag：使用操作时间比较, 1,比较，0，不比较。
    返回：
        -ECONFIG_ERROR， 绑定失败。
        -ECONFIG_FULL, 账户手机已满。
        -ECONFIG_EXIST, 账户手机已存在。
        >=0, 创建成功的钥匙ID。
*/
s32
device_config_account_append_phone(u16 account_id, u16 phone_id, u32 op_time, u8 flag)
{
	s32 result;
	struct account_head ah;
	struct phone_head ph;
	s32 pos;

	result = -ECONFIG_ERROR;
	if (account_id >= ACCOUNT_NUMBERS || phone_id >= KEY_NUMBERS)
        return result;

	if (device_config_get_account_valid(account_id) >= 0 &&
		device_config_account_operate(account_id, &ah, 0) >= 0) {
			pos = device_config_account_get_phone_pos(&ah, phone_id);
			/* phone is not in account */
			if (pos < 0) {
				pos = device_config_account_get_invalid_phone_pos(&ah);
				if (pos >= 0) {
					if (device_config_get_phone_valid(phone_id) &&
						device_config_phone_operate(phone_id, &ph, 0) >= 0) {
                        if (flag) {
                            if (op_time >= ph.updated_time) {
                                if (ph.account != ACCOUNT_ID_INVALID) {
                                    device_config_account_remove_phone(phone_id);
                                }
                                ph.account = account_id;
                                ph.updated_time = op_time;
                                ph.is_update = 0;
                                device_config_phone_operate(phone_id, &ph, 1);
                                ah.phone[pos] = phone_id;
                                ah.is_updated = 0;
                                ah.updated_time = op_time;
                                device_config_account_operate(account_id, &ah, 1);
                                result = phone_id;
                            }
                        } else {
                            if (ph.account != ACCOUNT_ID_INVALID) {
                                device_config_account_remove_phone(phone_id);
                            }
                            ph.account = account_id;
                            ph.updated_time = sys_cur_date();
                            ph.is_update = 1;
                            device_config_phone_operate(phone_id, &ph, 1);
                            ah.phone[pos] = phone_id;
                            ah.is_updated = 1;
                            ah.updated_time = ph.updated_time;
                            device_config_account_operate(account_id, &ah, 1);
                            result = phone_id;
                        }
					}
				} else {
					result = -ECONFIG_FULL;
				}
			} else {
				result = -ECONFIG_EXIST;
			}
	}
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
device_config_account_remove_phone(u16 phone_id)
{
	s32 result;
	struct account_head ah;
	struct phone_head ph;
	s32 pos;

	result = -ECONFIG_ERROR;
	if (phone_id >= PHONE_NUMBERS)
        return result;

	if (device_config_get_phone_valid(phone_id) &&
		device_config_phone_operate(phone_id, &ph, 0) >= 0) {
		/* phone has valid account */
		if (ph.account != ACCOUNT_ID_INVALID) {
            if (device_config_get_account_valid(ph.account) >= 0 &&
				device_config_account_operate(ph.account, &ah, 0) >= 0) {
				pos = device_config_account_get_phone_pos(&ah, phone_id);
				if (pos >= 0) {
					ah.phone[pos] = PHONE_ID_INVALID;
					device_config_account_operate(ph.account, &ah, 1);
                    result = phone_id;
				}
			}
			ph.account = ACCOUNT_ID_INVALID;
			device_config_phone_operate(phone_id, &ph, 1);
		} else {
            result = phone_id;
        }
	}
//__exit:
	return result;
}


s32
device_config_key_index(int(*callback)(struct key *, int id, void *arg1, void *arg2), void *arg1, void *arg2)
{
    s32 result;
    u16 i;
	struct key k;
    s32 len;

	result = -ECONFIG_ERROR;

	for (i = 0; i < KEY_NUMBERS; i++) {
		if (device_config_get_key_valid(i) > 0) {
			len = device_config_key_operate(i, &k, 0);
			if (len >= 0) {
                result = callback(&k, i, arg1, arg2);
			}
		}
	}
    return result;
}

s32
device_config_phone_index(int(*callback)(struct phone_head *, int id, void *args), void *args)
{
    s32 result;
    u16 i;
	struct phone_head ph;
    s32 len;

	result = -ECONFIG_ERROR;

	for (i = 0; i < PHONE_NUMBERS; i++) {
        len = device_config_get_phone_valid(i);
		if (len > 0) {
			len = device_config_phone_operate(i, &ph, 0);
			if (len >= 0) {
                result = callback(&ph, i, args);
			}
		}
	}
    return result;
}

s32
device_config_account_index(int(*callback)(struct account_head *, int id, void *args), void *args)
{
    s32 result;
    u16 i;
	struct account_head ah;
    s32 len;

	result = -ECONFIG_ERROR;

	for (i = 0; i < ACCOUNT_NUMBERS; i++) {
        len = device_config_get_account_valid(i);
		if (len > 0) {
			len = device_config_account_operate(i, &ah, 0);
			if (len >= 0) {
                result = callback(&ah, i, args);
			}
		}
	}
    return result;
}

s32
device_config_event_index(int(*callback)(struct event *, int id ,void *args), void *args)
{
    s32 result;
    u16 i;
	struct event evt;
    s32 len;

	result = -ECONFIG_ERROR;

	for (i = 0; i < EVENT_NUMBERS; i++) {
        len = device_config_get_event_valid(i);
		if (len > 0) {
			len = device_config_event_operate(i, &evt, 0);
			if (len >= 0) {
                result = callback(&evt, i, args);
			}
		}
	}
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
device_config_device_id_operate(u8 *device_id, u8 flag)
{
	int fd;
	int cnts;
	int result = -ECONFIG_ERROR;

    struct device_configure *config = &device_config;
	RT_ASSERT(config!=RT_NULL);
	RT_ASSERT(config->mutex!=RT_NULL);

	rt_mutex_take(config->mutex, RT_WAITING_FOREVER);


	if (flag) {
        if ((fd = open(DEVICE_CONFIG_FILE_NAME,O_RDWR,0x777)) >= 0)
        {
            rt_memcpy(config->param.device_id, device_id, 8); 
            cnts = write(fd, &(config->param), sizeof(config->param));
            if (cnts == sizeof(config->param))
                result = cnts;
            close(fd);
        }

	} else {
        rt_memcpy(device_id, config->param.device_id, 8);
        result = 1;
	}
	
	rt_mutex_release(config->mutex);
	return result;
}
int
device_config_key0_operate(u8 *key0, u8 flag)
{
	int fd;
	int cnts;
	int result = -ECONFIG_ERROR;

    struct device_configure *config = &device_config;
	RT_ASSERT(config!=RT_NULL);
	RT_ASSERT(config->mutex!=RT_NULL);

	rt_mutex_take(config->mutex, RT_WAITING_FOREVER);


	if (flag) {
        if ((fd = open(DEVICE_CONFIG_FILE_NAME,O_RDWR,0x777)) >= 0)
        {
            rt_memcpy(config->param.key0, key0, 8); 
            cnts = write(fd, &(config->param), sizeof(config->param));
            if (cnts == sizeof(config->param))
                result = cnts;
            close(fd);
        }

	} else {
        rt_memcpy(key0, config->param.key0, 8);
        result = 1;
	}
	
	rt_mutex_release(config->mutex);
	return result;
}

int
device_config_pv_operate(struct phone_valid_map *pv_map, u8 flag)
{
	int fd;
	int cnts;
	int result = -ECONFIG_ERROR;

    struct device_configure *config = &device_config;
	RT_ASSERT(config!=RT_NULL);
	RT_ASSERT(config->mutex!=RT_NULL);

	rt_mutex_take(config->mutex, RT_WAITING_FOREVER);


	if (flag) {
        if ((fd = open(DEVICE_CONFIG_FILE_NAME,O_RDWR,0x777)) >= 0)
        {
            rt_memcpy(&config->param.pv_map, pv_map, sizeof(*pv_map)); 
            cnts = write(fd, &(config->param), sizeof(config->param));
            if (cnts == sizeof(config->param))
                result = cnts;
            close(fd);
        }

	} else {
        rt_memcpy(pv_map, &config->param.pv_map, sizeof(*pv_map));
        result = 1;
	}
	
	rt_mutex_release(config->mutex);
	return result;
}

int
device_config_av_operate(struct account_valid_map *av_map, u8 flag)
{
	int fd;
	int cnts;
	int result = -ECONFIG_ERROR;

    struct device_configure *config = &device_config;
	RT_ASSERT(config!=RT_NULL);
	RT_ASSERT(config->mutex!=RT_NULL);

	rt_mutex_take(config->mutex, RT_WAITING_FOREVER);


	if (flag) {
        if ((fd = open(DEVICE_CONFIG_FILE_NAME,O_RDWR,0x777)) >= 0)
        {
            rt_memcpy(&config->param.av_map, av_map, sizeof(*av_map)); 
            cnts = write(fd, &(config->param), sizeof(config->param));
            if (cnts == sizeof(config->param))
                result = cnts;
            close(fd);
        }

	} else {
        rt_memcpy(av_map, &config->param.av_map, sizeof(*av_map));
        result = 1;
	}
	
	rt_mutex_release(config->mutex);
	return result;
}

int
device_config_kv_operate(struct key_valid_map *kv_map, u8 flag)
{
	int fd;
	int cnts;
	int result = -ECONFIG_ERROR;

    struct device_configure *config = &device_config;
	RT_ASSERT(config!=RT_NULL);
	RT_ASSERT(config->mutex!=RT_NULL);

	rt_mutex_take(config->mutex, RT_WAITING_FOREVER);


	if (flag) {
        if ((fd = open(DEVICE_CONFIG_FILE_NAME,O_RDWR,0x777)) >= 0)
        {
            rt_memcpy(&config->param.kv_map, kv_map, sizeof(*kv_map)); 
            cnts = write(fd, &(config->param), sizeof(config->param));
            if (cnts == sizeof(config->param))
                result = cnts;
            close(fd);
        }

	} else {
        rt_memcpy(kv_map, &config->param.kv_map, sizeof(*kv_map));
        result = 1;
	}
	
	rt_mutex_release(config->mutex);
	return result;
}

int
device_config_ev_operate(struct event_valid_map *ev_map, u8 flag)
{
	int fd;
	int cnts;
	int result = -ECONFIG_ERROR;

    struct device_configure *config = &device_config;
	RT_ASSERT(config!=RT_NULL);
	RT_ASSERT(config->mutex!=RT_NULL);

	rt_mutex_take(config->mutex, RT_WAITING_FOREVER);


	if (flag) {
        if ((fd = open(DEVICE_CONFIG_FILE_NAME,O_RDWR,0x777)) >= 0)
        {
            rt_memcpy(&config->param.ev_map, ev_map, sizeof(*ev_map)); 
            cnts = write(fd, &(config->param), sizeof(config->param));
            if (cnts == sizeof(config->param))
                result = cnts;
            close(fd);
        }

	} else {
        rt_memcpy(ev_map, &config->param.ev_map, sizeof(*ev_map));
        result = 1;
	}
	
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
    rt_kprintf("phone_is_valid = %d\n", device_config_get_phone_valid(phone_id));
  rt_kprintf("phone_is_update = 0x%x\n", ph.is_update);
	rt_kprintf("phone_account = 0x%x\n", ph.account);
	rt_kprintf("phone_auth = 0x%x\n", ph.auth);
	rt_kprintf("phone_address = %s\n", ph.address);
	rt_kprintf("phone_updated_time = %d\n", ph.updated_time);
}

void device_config_key_display(u16 key_id)
{
	struct key k;
	device_config_key_operate(key_id, &k, 0);
    rt_kprintf("k_is_valid = %d\n", device_config_get_key_valid(key_id));
	rt_kprintf("k_account = 0x%x\n", k.head.account);
	rt_kprintf("k_is_updated = 0x%x\n", k.head.is_updated);
	rt_kprintf("k_key_type = 0x%x\n", k.head.key_type);
	rt_kprintf("k_operation_type = 0x%x\n", k.head.operation_type);
	rt_kprintf("k_updated_time = 0x%x\n", k.head.updated_time);
	rt_kprintf("k_start_time = 0x%x\n", k.head.start_time);
	rt_kprintf("k_end_time = 0x%x\n", k.head.end_time);
    rt_kprintf("----------data----------\n");

	print_hex(&k.data, device_config_get_key_code_size(k.head.key_type));
}

void device_config_account_display(u16 account_id)
{
	struct account_head ah;
    u32 i;
	device_config_account_operate(account_id, &ah, 0);
    rt_kprintf("account_is_valid = %d\n", device_config_get_account_valid(account_id));
	rt_kprintf("account_is_updated = %d\n", ah.is_updated);
	rt_kprintf("account_updated_time = 0x%x\n", ah.updated_time);
	rt_kprintf("account_name = %s\n", ah.name);
	rt_kprintf("account_key: ");
    for (i = 0; i < ACCOUNT_KEY_NUMBERS; ++i)
		rt_kprintf("0x%x ", ah.key[i]);
	rt_kprintf("\naccount_phone: ");
    for (i = 0; i < ACCOUNT_PHONE_NUMBERS; ++i)
		rt_kprintf("0x%x ", ah.phone[i]);
	rt_kprintf("\n");
}
s32 device_config_key_set_test(u16 key_id, u16 k_type, u8 *buf, u16 o_type, u32 s_time, u32 e_time, u32 op_time)
{
    s32 result = -1;
    struct key *k;
    k = rt_malloc(sizeof(*k));
    if (k == RT_NULL)
        return result;
    rt_memset(k, '\0', sizeof(*k));
    k->head.key_type = k_type;
    rt_memcpy(&k->data, buf, device_config_get_key_code_size(k_type));
    k->head.operation_type = o_type;
    k->head.start_time = s_time;
    k->head.end_time = e_time;
    result = device_config_key_set(key_id, k, op_time);
    rt_free(k);
    return result;
}
s32 device_config_key_valid_display(int x)
{
    int i;
    for (i=0; i<KEY_MAP_SIZE/x; i++)
    {
        print_hex(&device_config.param.kv_map.data[i], x * sizeof(device_config.param.kv_map.data[0]));
    }
}
s32 device_config_account_valid_display(int x)
{
    int i;
    for (i=0; i<ACCOUNT_MAP_SIZE/x; i++)
    {
        print_hex(&device_config.param.av_map.data[i], x * sizeof(device_config.param.av_map.data[0]));
    }
}
s32 device_config_phone_valid_display(int x)
{
    int i;
    for (i=0; i<PHONE_MAP_SIZE/x; i++)
    {
        print_hex(&device_config.param.pv_map.data[i], x * sizeof(device_config.param.pv_map.data[0]));
    }
}
s32 device_config_event_valid_display(int x)
{
    int i;
    for (i=0; i<EVENT_MAP_SIZE/x; i++)
    {
        print_hex(&device_config.param.ev_map.data[i], x * sizeof(device_config.param.ev_map.data[0]));
    }
}
FINSH_FUNCTION_EXPORT_ALIAS(device_config_key_valid_display, devcfg_kvd, [x]);
FINSH_FUNCTION_EXPORT_ALIAS(device_config_account_valid_display, devcfg_avd, [x]);
FINSH_FUNCTION_EXPORT_ALIAS(device_config_phone_valid_display, devcfg_pvd, [x]);
FINSH_FUNCTION_EXPORT_ALIAS(device_config_event_valid_display, devcfg_evd, [x]);
FINSH_FUNCTION_EXPORT_ALIAS(device_config_phone_create, devcfg_pcr, [phone_id name len]);
FINSH_FUNCTION_EXPORT_ALIAS(device_config_phone_set, devcfg_ps, [phone_id name len op_time]);
FINSH_FUNCTION_EXPORT_ALIAS(device_config_get_phone_valid, devcfg_gpv, [phone_id]);
FINSH_FUNCTION_EXPORT_ALIAS(device_config_set_phone_valid, devcfg_spv, [phone_id value]);
FINSH_FUNCTION_EXPORT_ALIAS(device_config_phone_delete, devcfg_pd, [phone_id op_time flag]);
FINSH_FUNCTION_EXPORT_ALIAS(device_config_phone_verify, devcfg_pv, [phone_buf length]);
FINSH_FUNCTION_EXPORT_ALIAS(device_config_phone_display, devcfg_pds, [phone_id]);

FINSH_FUNCTION_EXPORT_ALIAS(device_config_key_create, devcfg_kcr, [key_id key_type buf length]);
FINSH_FUNCTION_EXPORT_ALIAS(device_config_key_set_test, devcfg_ks, [key_id k_type buf o_type s_time e_time op_time]);
FINSH_FUNCTION_EXPORT_ALIAS(device_config_get_key_valid, devcfg_gkv, [key_id]);
FINSH_FUNCTION_EXPORT_ALIAS(device_config_set_key_valid, devcfg_skv, [key_id value]);
FINSH_FUNCTION_EXPORT_ALIAS(device_config_key_delete, devcfg_kd, [key_id op_time flag]);
FINSH_FUNCTION_EXPORT_ALIAS(device_config_key_verify, devcfg_kv, [key_buf length]);
FINSH_FUNCTION_EXPORT_ALIAS(device_config_key_display, devcfg_kds, [key_id]);

FINSH_FUNCTION_EXPORT_ALIAS(device_config_account_create, devcfg_acr, [account_id name length]);
FINSH_FUNCTION_EXPORT_ALIAS(device_config_account_set, devcfg_as, [account_id name length op_time]);
FINSH_FUNCTION_EXPORT_ALIAS(device_config_get_account_valid, devcfg_gav, [account_id]);
FINSH_FUNCTION_EXPORT_ALIAS(device_config_set_account_valid, devcfg_sav, [account_id value]);
FINSH_FUNCTION_EXPORT_ALIAS(device_config_account_delete, devcfg_ad, [account_id]);
FINSH_FUNCTION_EXPORT_ALIAS(device_config_account_display, devcfg_ads, [account_id]);
FINSH_FUNCTION_EXPORT_ALIAS(device_config_account_next_valid, devcfg_anv, [account_id]);

FINSH_FUNCTION_EXPORT_ALIAS(device_config_account_append_key, devcfg_aak, [account_id key_id op_time flag]);
FINSH_FUNCTION_EXPORT_ALIAS(device_config_account_remove_key, devcfg_ark, [key_id]);

FINSH_FUNCTION_EXPORT_ALIAS(device_config_account_append_phone, devcfg_aap, [account_id phone_id op_time flag]);
FINSH_FUNCTION_EXPORT_ALIAS(device_config_account_remove_phone, devcfg_arp, [phone_id]);
#endif
