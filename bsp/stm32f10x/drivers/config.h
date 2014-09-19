/*********************************************************************
 * Filename:			config.h
 *
 * Description:
 *
 * Author:
 * Email:				lenovo@BRIGHT
 * Date:				2014-08-27
 *
 * Modify:
 *
 * Copyright (C) 2014 Yuettak Co.,Ltd
 ********************************************************************/
#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <rtthread.h>
#include <rthw.h>
#include <stm32f10x.h>

#define PHONE_ADDRESS_LENGTH 20
#define PHONE_NUMBERS 5*32 // x32
#define PHONE_MAP_SIZE (PHONE_NUMBERS / sizeof(u32))
#define PHONE_AUTH_CALL 0x0001
#define PHONE_AUTH_SMS  0x0002
#define PHONE_AUTH_INVALID 0x0000
#define PHONE_ID_INVALID 0xffff

#define KEY_NUMBERS 10*32 // x32
#define KEY_MAP_SIZE (KEY_NUMBERS / sizeof(u32))
#define KEY_ID_INVALID 0xffff

#define ACCOUNT_NUMBERS 10*32 // x32
#define ACCOUNT_KEY_NUMBERS 10
#define ACCOUNT_PHONE_NUMBERS 10
#define ACCOUNT_MAP_SIZE (ACCOUNT_NUMBERS / sizeof(u32))
#define ACCOUNT_ID_INVALID 0xffff
#define ACCOUNT_NAME_LENGTH 20
#define ACCOUNT_HAVE_KEY_NUMBERS 5
#define ACCOUNT_HAVE_PHONE_NUMBERS 5

#define KEY_FPRINT_CODE_SIZE 512
#define KEY_RFID_CODE_SIZE 4
#define KEY_KBOARD_CODE_SIZE 6

#define	KEY_TYPE_INVALID 0
#define	KEY_TYPE_FPRINT 1
#define	KEY_TYPE_RFID 2
#define	KEY_TYPE_KBOARD 3

#define	KEY_OPERATION_TYPE_FOREVER 1
#define	KEY_OPERATION_TYPE_ONCE 2
#define	KEY_OPERATION_TYPE_WEEKLY 3

#define TCP_DOMAIN_LENGTH 20

struct phone_head {

	u16 account;
	u16 auth;
	char address[PHONE_ADDRESS_LENGTH];
	u32 updated_time;
};

/* key code offset:
 * for rfid key is base 512, one rfid key is 4 bytes
 * for finger print key is base 512*3, one finger print key is 498 bytes
 * for number key is base is base 512*2, one number key is 6 bytes
 */
struct key_rfid_code {
	u8 code[KEY_RFID_CODE_SIZE];
};

struct key_fprint_code {
	u8 code[KEY_FPRINT_CODE_SIZE];
};

struct key_kboard_code {
	u8 code[KEY_KBOARD_CODE_SIZE];
};
struct key_head {
	u8 is_updated;
	u16 account;
	u16 key_type;
	u16 operation_type;
	u32 updated_time;
	u32 start_time;
	u32 end_time;
};
union key_data{
	struct key_rfid_code rfid;
	struct key_fprint_code fprint;
	struct key_kboard_code kboard;
};

struct key {
	struct key_head head;
	union key_data data;
};

struct account_head {

	u8 is_updated;
	s8 name[ACCOUNT_NAME_LENGTH];
	u16 key[ACCOUNT_KEY_NUMBERS];
	u16 phone[ACCOUNT_PHONE_NUMBERS];
	u32 updated_time;
};

struct account_valid_map {
	u32 data[KEY_MAP_SIZE];
};

struct key_valid_map {
	u32 data[KEY_MAP_SIZE];
};

struct phone_valid_map {
	u32 data[PHONE_MAP_SIZE];
};

struct tcp_domain_port {
	s8 domain[TCP_DOMAIN_LENGTH];
	u32 port;
};

struct sys_alarm_arg {
	u8 timeout;
};

struct device_parameters {
	u8 device_id[8];        //device id
	u8 serial[8];			 //Serial Number
	u8 key0[8];
	u8 key1[8];
	u8 device_status;       //device activated state
	u8 password[6];
	u8 lock_gate_alarm_time;
	struct tcp_domain_port tcp_dp[5];
	struct sys_alarm_arg alarm_arg[4];
	struct account_valid_map av_map;
	struct key_valid_map kv_map;
	struct phone_valid_map pv_map;
};

#define PAGE_SIZE (512)
#define PAGE_SIZE_OF(x) ((((x) / PAGE_SIZE) + 1) * PAGE_SIZE)



#define DEVICE_CONFIG_FILE_PARAMETERS_BASE (0)
#define DEVICE_CONFIG_FILE_PARAMETERS_SIZE (PAGE_SIZE_OF(sizeof(struct device_parameters)))
#define DEVICE_CONFIG_FILE_PARAMETERS_END (DEVICE_CONFIG_FILE_PARAMETERS_BASE + DEVICE_CONFIG_FILE_PARAMETERS_SIZE)

#define DEVICE_CONFIG_FILE_PHONE_BASE (DEVICE_CONFIG_FILE_PARAMETERS_END)
#define DEVICE_CONFIG_FILE_PHONE_SIZE (PAGE_SIZE_OF(PHONE_NUMBERS * sizeof(struct phone_head)))
#define DEVICE_CONFIG_FILE_PHONE_END (DEVICE_CONFIG_FILE_PHONE_BASE + DEVICE_CONFIG_FILE_PARAMETERS_SIZE)

#define DEVICE_CONFIG_FILE_PHONE_OFFSET(x) (DEVICE_CONFIG_FILE_PHONE_BASE + ((x) * sizeof(struct phone_head)))

#define DEVICE_CONFIG_FILE_ACCOUNT_BASE (DEVICE_CONFIG_FILE_PHONE_END)
#define DEVICE_CONFIG_FILE_ACCOUNT_SIZE (PAGE_SIZE_OF(ACCOUNT_NUMBERS * sizeof(struct account_head)))
#define DEVICE_CONFIG_FILE_ACCOUNT_END (DEVICE_CONFIG_FILE_ACCOUNT_BASE + DEVICE_CONFIG_FILE_ACCOUNT_SIZE)

#define DEVICE_CONFIG_FILE_ACCOUNT_OFFSET(x) (DEVICE_CONFIG_FILE_ACCOUNT_BASE + ((x) * sizeof(struct account_head)))

#define DEVICE_CONFIG_FILE_KEY_BASE (DEVICE_CONFIG_FILE_ACCOUNT_END)
#define DEVICE_CONFIG_FILE_KEY_SIZE (PAGE_SIZE_OF(KEY_NUMBERS * sizeof(struct key)))
#define DEVICE_CONFIG_FILE_KEY_END (DEVICE_CONFIG_FILE_KEY_BASE + DEVICE_CONFIG_FILE_KEY_SIZE)

#define DEVICE_CONFIG_FILE_KEY_OFFSET(x) (DEVICE_CONFIG_FILE_KEY_BASE + ((x) * sizeof(struct key)))
#define DEVICE_CONFIG_FILE_KEY_DATA_OFFSET(x) (DEVICE_CONFIG_FILE_KEY_BASE + ((x) * sizeof(struct key)) + sizeof(struct key_head))

/*
#define DEVICE_CONFIG_FILE_KEY_KBOARD_BASE (DEVICE_CONFIG_FILE_KEY_HEAD_BASE)
#define DEVICE_CONFIG_FILE_KEY_KBOARD_SIZE (PAGE_SIZE_OF(KEY_NUMBERS * sizeof(struct key_kboard_code)))
#define DEVICE_CONFIG_FILE_KEY_KBOARD_END (DEVICE_CONFIG_FILE_KEY_KBOARD_BASE + DEVICE_CONFIG_FILE_KEY_KBOARD_SIZE)

#define DEVICE_CONFIG_FILE_KEY_FPRINT_BASE (DEVICE_CONFIG_FILE_KEY_KBOARD_END)
#define DEVICE_CONFIG_FILE_KEY_FPRINT_SIZE (PAGE_SIZE_OF(KEY_NUMBERS * sizeof(struct key_fprint_code)))
#define DEVICE_CONFIG_FILE_KEY_FPRINT_END (DEVICE_CONFIG_FILE_KEY_FPRINT_BASE + DEVICE_CONFIG_FILE_KEY_FPRINT_SIZE)
*/
struct device_configure {
	rt_mutex_t mutex;
	struct device_parameters param;
};

typedef struct
{
	u8 device_id[6];    //device id
	u8 CDKEY[8];        //serial number
	u8 flag;             //save flag

}PRODUCT_ID;

//extern struct device_config device_config;

s32
device_config_account_remove_phone(u16 phone_id);
s32
device_config_account_append_phone(u16 account_id, u16 phone_id);
int
device_config_file_operate(struct device_configure *config, u8 flag);
s32 
device_config_key_index(int(*callback)(struct key *, void *arg1, void *arg2, void *arg3), void *arg1, void *arg2);
int
system_init(void);

#endif /* _CONFIG_H_ */
