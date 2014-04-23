#ifndef __APPPUBULIC_H__
#define __APPPUBULIC_H__
#include "rtthread.h"
#include "untils.h"

#define SYS_FPRINT_REGISTER  (0X01<<0)
#define SYS_EVENT_ALL        0xffffffff


rt_uint16_t get_average_value(rt_uint16_t dat[],rt_uint8_t num);

rt_uint16_t get_new_key_pos(void);

rt_uint16_t get_fprint_key_num(void);

void set_key_using_status(rt_uint16_t key,KEY_TYPE type,rt_uint8_t status);

rt_bool_t check_fprint_pos_inof(rt_uint16_t pos);

void set_fprint_update_flag(rt_uint16_t pos,rt_uint8_t new_status);

rt_uint16_t get_fprint_update_pos(void);

rt_uint32_t sys_cur_date(void);

rt_uint8_t system_event_process(rt_uint8_t mode,rt_uint32_t type);

#endif

