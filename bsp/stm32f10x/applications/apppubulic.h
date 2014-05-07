#ifndef __APPPUBULIC_H__
#define __APPPUBULIC_H__
#include "rtthread.h"
#include "untils.h"

#define SYS_FPRINT_REGISTER  (0X01<<0)
#define SYS_EVENT_ALL        0xffffffff

//计算平均数
rt_uint16_t 
get_average_value(rt_uint16_t dat[],rt_uint8_t num);

//配置文件互斥量操作
void 
config_file_mutex_op(rt_bool_t way);

//获得一个新增钥匙的位置
rt_uint16_t 
get_new_key_pos(void);

//获得指纹钥匙的总数
rt_uint16_t 
get_fprint_key_num(void);

//修改钥匙使用状态
void 
set_key_using_status(rt_uint16_t key,KEY_TYPE type,rt_uint8_t status);

//检测指纹钥匙是否已经存在
rt_bool_t 
check_fprint_pos_inof(rt_uint16_t pos);

//设置钥匙的更新标志
void 
set_key_update_flag(rt_uint16_t pos,rt_uint8_t new_status);

//获得需要更新的钥匙位置
rt_uint16_t 
get_key_update_pos(void);

//检测开门时的权限
rt_bool_t 
check_open_access(rt_uint16_t pos);

//系统当前时间
rt_uint32_t 
sys_cur_date(void);

//系统事件处理函数
rt_uint8_t 
system_event_process(rt_uint8_t mode,rt_uint32_t type);

//获得一个位置的钥匙类型
KEY_TYPE 
get_key_type(rt_uint16_t pos);


#endif

