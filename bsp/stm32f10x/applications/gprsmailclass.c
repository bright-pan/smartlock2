#include "gprsmailclass.h"

//Կ�����
void gprs_Key_add_mail(rt_uint16_t pos)
{
	rt_uint16_t *keypos = RT_NULL;

	keypos = rt_calloc(1,sizeof(*keypos));
	
	send_gprs_mail(ALARM_TYPE_KEY_ADD,net_get_date(),keypos);
}
//Կ��ɾ��
void gprs_key_del_mail(rt_uint16_t pos)
{
	msg_mail_keydelete(pos,net_get_date());
}

//Կ����ȷ
void gprs_key_right_mail(rt_uint16_t pos)
{
	GPRSUserDef_p				user = RT_NULL;
	struct key 					*keydata = RT_NULL;
	
	user = rt_calloc(1,sizeof(*user));
	RT_ASSERT(user != RT_NULL);

	keydata = rt_calloc(1,sizeof(*keydata));
	RT_ASSERT(keydata != RT_NULL);
	
	device_config_key_operate(pos,keydata,0);

	user->keyright.pos = pos;

	user->keyright.type = keydata->head.key_type;

	rt_free(keydata);
	
	send_gprs_mail(ALARM_TYPE_KEY_RIGHT,net_get_date(),user);
}

//Կ�״���
void gprs_key_error_mail(rt_uint8_t type)
{
	GPRSUserDef_p  user = RT_NULL;

	user = rt_calloc(1,sizeof(*user));

	user->keyerr.type = type;
	
	send_gprs_mail(ALARM_TYPE_KEY_ERROR,net_get_date(),user);
}

//�˻����
void gprs_account_add_mail(rt_uint16_t pos)
{
	GPRSUserDef_p         user = RT_NULL;
  struct account_head   *ah = RT_NULL;

	user = rt_calloc(1,sizeof(*user));
	ah = rt_calloc(1,sizeof(*ah));
	
	device_config_account_operate(pos,ah,0);
	user->AccountAdd.date = ah->updated_time;
	user->AccountAdd.pos = pos;
	rt_memcpy(user->AccountAdd.name,(void *)ah->name,ACCOUNT_NAME_LENGTH);
	
	send_gprs_mail(ALARM_TYPE_GPRS_ADD_ACCOUNT,net_get_date(),user);

	rt_free(ah);
}

//�˻�ɾ��
void gprs_account_del_mail(rt_uint16_t pos)
{
	msg_mail_account_del(pos,net_get_date());
}

//�ֻ����
void gprs_phone_add_mail(rt_uint16_t pos)
{
	GPRSUserDef_p         user = RT_NULL;
  struct phone_head   	*data = RT_NULL;

	user = rt_calloc(1,sizeof(*user));
	data = rt_calloc(1,sizeof(*data));
	
	device_config_phone_operate(pos,data,0);
	user->PhoneAdd.pos = pos;
	user->PhoneAdd.auth = data->auth;
	user->PhoneAdd.date = data->updated_time;
	rt_memcpy(user->PhoneAdd.code,(void *)data->address,12);
	
	send_gprs_mail(ALARM_TYPE_GPRS_ADD_PHONE,net_get_date(),user);

	rt_free(data);
}
//�ֻ�ɾ��
void gprs_phone_del_mail(rt_uint16_t pos)
{
	msg_mail_keydelete(pos,net_get_date());
}



