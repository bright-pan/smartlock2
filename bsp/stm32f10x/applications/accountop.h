#ifndef __ACCOUNTOP_H__
#define __ACCOUNTOP_H___
#include "config.h"

typedef struct
{
	rt_int32_t id;
	rt_uint8_t name[ACCOUNT_NAME_LENGTH];
}UserInfoDef,*UserInfoDef_p;

rt_err_t account_add_enter(void);

rt_err_t account_add_exit(rt_bool_t mode);

rt_err_t key_add_password_check(rt_uint8_t *key);

rt_err_t key_add_password(rt_uint8_t *key);

rt_err_t account_cur_add_password(rt_uint8_t *key);

rt_int32_t user_valid_num(void);

void user_get_info(UserInfoDef_p user,rt_int32_t id);

void user_get_info_continuous(UserInfoDef user[],rt_int32_t *start_id,rt_int32_t num,rt_uint8_t flag);

rt_int32_t user_valid_phone_num(void);

rt_uint32_t user_valid_password_num(void);

rt_uint32_t user_valid_fprint_num(void);

rt_err_t user_add_fprint(rt_uint32_t outtime);

rt_err_t user_phone_add_check(rt_uint8_t *phone);

rt_err_t user_cur_add_phone(rt_uint8_t *phone);

rt_uint32_t account_cur_pos_get(void);

//Կ��ƥ��
rt_err_t key_password_verify(rt_uint8_t *password);

//�û���ЧID���
rt_int32_t account_valid_check(rt_int32_t pos);

#endif

