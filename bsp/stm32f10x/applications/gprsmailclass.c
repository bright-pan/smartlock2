#include "gprsmailclass.h"

//钥匙添加
void gprs_Key_add_mail(rt_uint16_t pos)
{
	rt_uint16_t *keypos = RT_NULL;

	keypos = rt_calloc(1,sizeof(*keypos));
	
	send_gprs_mail(ALARM_TYPE_KEY_ADD,net_get_date(),keypos);
}
//钥匙删除
void gprs_key_del_mail(rt_uint16_t pos)
{
	msg_mail_keydelete(pos,net_get_date());
}

//钥匙正确
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
	rt_kprintf("right ID %d type %d\n",user->keyright.pos,user->keyright.type);
	rt_free(keydata);
	
	send_gprs_mail(ALARM_TYPE_KEY_RIGHT,net_get_date(),user);
}

//钥匙错误
void gprs_key_error_mail(rt_uint8_t type)
{
	GPRSUserDef_p  user = RT_NULL;

	user = rt_calloc(1,sizeof(*user));

	user->keyerr.type = type;
	
	send_gprs_mail(ALARM_TYPE_KEY_ERROR,net_get_date(),user);
}

//账户添加
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

//账户删除
void gprs_account_del_mail(rt_uint16_t pos)
{
	msg_mail_account_del(pos,net_get_date());
}

//手机添加
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
//手机删除
void gprs_phone_del_mail(rt_uint16_t pos)
{
	msg_mail_keydelete(pos,net_get_date());
}

/** 
@brief  key upload process
@param  flag :模式 
				@arg 0:映射域上传
				@arg 1:账户上传
				@arg 2:钥匙上传
				@arg 3:手机上传
				@arg 4:记录上传
@retval none
*/
void gprs_datamap_upload_mail(rt_uint8_t flag)
{
	GPRSUserDef_p         user = RT_NULL;

	user = rt_calloc(1,sizeof(*user));
	RT_ASSERT(user != RT_NULL);
	user->MapUpload.MapType = 0x01<<flag;
	send_gprs_mail(ALARM_TYPE_GPRS_DATAMAP_UPLOAD,net_get_date(),user);
}

