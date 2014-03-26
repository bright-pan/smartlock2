#ifndef __NETCOMM_H__
#define __NETCOMM_H__
#include "rtthread.h"
#include <dfs.h>
#include <dfs_posix.h>

rt_uint16_t net_rev16(rt_uint16_t data);

rt_uint32_t net_rev32(rt_uint32_t data);

void net_uint16_copy_string(rt_uint8_t str[],rt_uint16_t data);

void net_uint32_copy_string(rt_uint8_t str[],rt_uint32_t data);

void net_string_copy_uint16(rt_uint16_t *data,rt_uint8_t str[]);

void net_string_copy_uint32(rt_uint32_t *data,rt_uint8_t str[]);

rt_int8_t file_get_crc32(rt_uint8_t *FileName,rt_uint32_t *crc32);


rt_int8_t get_file_size(const char* name ,rt_uint32_t *size);

void net_copy_date_str(rt_uint8_t *time);

rt_uint32_t net_get_date(void);

#endif

