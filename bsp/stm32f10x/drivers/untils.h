/*********************************************************************
 * Filename:      untils.h
 *
 *
 * Author:        Bright Pan <loststriker@gmail.com>
 * Created at:    2013-05-22 09:26:03
 *
 *
 *
 * Change Log:
 *
 * Copyright (C) 2013 Yuettak Co.,Ltd
 ********************************************************************/

#ifndef _UNTILS_H_
#define _UNTILS_H_

#include <rtthread.h>
#include <rthw.h>
#include <stm32f10x.h>



#define TELEPHONE_NUMBERS 10
#define TELEPHONE_ADDRESS_LENGTH 20
#define KEY_NUMBERS 10
#define TCP_DOMAIN_LENGTH 20

#define KEY_FPRINT_CODE_SIZE 498
#define KEY_RFID_CODE_SIZE 4
#define KEY_KBOARD_CODE_SIZE 6

typedef struct {

	uint8_t flag;
	char address[TELEPHONE_ADDRESS_LENGTH];

}TELEPHONE_ADDRESS_TYPEDEF;

typedef enum {

	KEY_TYPE_FPRINT = 0,
	KEY_TYPE_RFID = 1,
	KEY_TYPE_KBOARD = 2,
	KEY_TYPE_ERROR = 0XFF,
}KEY_TYPE;

typedef enum {

	OPERATION_TYPE_FOREVER = 0,
	OPERATION_TYPE_EVERYDAY = 1,
	OPERATION_TYPE_WORKDAY = 2,
	OPERATION_TYPE_ONCE = 3,

}OPERATION_TYPE;

/* key code offset:
 * for rfid key is base 512, one rfid key is 4 bytes
 * for finger print key is base 512*3, one finger print key is 498 bytes
 * for number key is base is base 512*2, one number key is 6 bytes
 */
typedef struct {

	uint8_t flag;
	uint8_t is_updated;
	KEY_TYPE key_type;
	OPERATION_TYPE operation_type;
	uint32_t created_time;
	uint32_t start_time;
	uint32_t end_time;

}KEY_TYPEDEF;

typedef struct {

	char domain[TCP_DOMAIN_LENGTH];
	int32_t port;

}TCP_DOMAIN_TYPEDEF;

typedef struct 
{
	uint8_t timeout;
}SYS_ALARM_ARG_TYPEDEF;

typedef struct {

	TELEPHONE_ADDRESS_TYPEDEF telephone_address[TELEPHONE_NUMBERS];
	KEY_TYPEDEF key[KEY_NUMBERS];
	TCP_DOMAIN_TYPEDEF tcp_domain[5];
	uint8_t lock_gate_alarm_time;
	uint8_t device_id[8];        //device id
	uint8_t CDKEY[8];			 //Serial Number
	uint8_t key0[8];
	uint8_t key1[8];
	uint8_t device_status;       //device activated state
  uint8_t password[6];
	SYS_ALARM_ARG_TYPEDEF alarm_arg[4];
}DEVICE_PARAMETERS_TYPEDEF;

#define DEVICE_CONFIG_FILE_KEY_BASE (((sizeof(DEVICE_PARAMETERS_TYPEDEF) / 1024) + 1) * 1024)
#define DEVICE_CONFIG_FILE_KEY_SIZE (512)
#define DEVICE_CONFIG_FILE_KEY_OFFSET(x) (DEVICE_CONFIG_FILE_KEY_BASE + (DEVICE_CONFIG_FILE_KEY_SIZE * (x)))

typedef struct {

	rt_mutex_t mutex;
	DEVICE_PARAMETERS_TYPEDEF param;

}DEVICE_CONFIG_TYPEDEF;

typedef struct
{
	uint8_t device_id[6];    //device id
	uint8_t CDKEY[8];        //serial number
	uint8_t flag;             //save flag

}PRODUCT_ID;

extern DEVICE_CONFIG_TYPEDEF device_config;

int
device_config_init(DEVICE_CONFIG_TYPEDEF *config);
int
device_config_file_operate(DEVICE_CONFIG_TYPEDEF *config, uint8_t flag);
int
device_config_superpwd_verify(const uint8_t *buf);
int
device_config_superpwd_save(uint8_t *buf);
int
device_config_key_operate(uint16_t key_id, KEY_TYPE key_type, uint8_t *buf, uint8_t flag);
int
device_config_key_verify(KEY_TYPE type, const uint8_t *buf);
int
device_config_key_create(KEY_TYPE type, uint8_t *buf);
void
print_hex(uint8_t *buf, uint16_t length);
void
print_char(uint8_t *buf, uint16_t length);
void
delay_us(uint32_t time);

rt_device_t
device_enable(const char *name);
#ifndef __GNUC__
void *
memmem(const void *haystack,
	   rt_size_t haystack_len,
	   const void *needle,
	   rt_size_t needle_len);
#endif

#endif
