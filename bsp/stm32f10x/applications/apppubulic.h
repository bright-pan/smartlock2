#ifndef __APPPUBULIC_H__
#define __APPPUBULIC_H__
#include "rtthread.h"
#include "untils.h"

#define SYS_FPRINT_REGISTER  (0X01<<0)
#define SYS_EVENT_ALL        0xffffffff

//����ƽ����
rt_uint16_t 
get_average_value(rt_uint16_t dat[],rt_uint8_t num);

//�����ļ�����������
void 
config_file_mutex_op(rt_bool_t way);

//���һ������Կ�׵�λ��
rt_uint16_t 
get_new_key_pos(void);

//���ָ��Կ�׵�����
rt_uint16_t 
get_fprint_key_num(void);

//�޸�Կ��ʹ��״̬
void 
set_key_using_status(rt_uint16_t key,KEY_TYPE type,rt_uint8_t status);

//���ָ��Կ���Ƿ��Ѿ�����
rt_bool_t 
check_fprint_pos_inof(rt_uint16_t pos);

//����Կ�׵ĸ��±�־
void 
set_key_update_flag(rt_uint16_t pos,rt_uint8_t new_status);

//�����Ҫ���µ�Կ��λ��
rt_uint16_t 
get_key_update_pos(void);

//��⿪��ʱ��Ȩ��
rt_bool_t 
check_open_access(rt_uint16_t pos);

//ϵͳ��ǰʱ��
rt_uint32_t 
sys_cur_date(void);

//ϵͳ�¼�������
rt_uint8_t 
system_event_process(rt_uint8_t mode,rt_uint32_t type);

//���һ��λ�õ�Կ������
KEY_TYPE 
get_key_type(rt_uint16_t pos);


#endif

