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
		
}

//钥匙正确
void gprs_key_right_mail(rt_uint16_t pos)
{
	GPRS_KeyRightUser_p user = RT_NULL;
	struct key 					*keydata = RT_NULL;
	
	user = rt_calloc(1,sizeof(*user));
	RT_ASSERT(user != RT_NULL);

	keydata = rt_calloc(1,sizeof(*keydata));
	RT_ASSERT(keydata != RT_NULL);
	
	device_config_key_operate(pos,keydata,0);

	user->keypos = pos;
	user->keytype = keydata->head.key_type;

	rt_free(keydata);
	
	send_gprs_mail(ALARM_TYPE_KEY_RIGHT,net_get_date(),user);
}

//钥匙错误
void gprs_key_error_mail(rt_uint8_t type)
{
	GPRS_KeyErrorUser_p user = RT_NULL;

	user = rt_calloc(1,sizeof(*user));

	user->type = type;
	
	send_gprs_mail(ALARM_TYPE_KEY_ERROR,net_get_date(),user);
}

//账户添加
void gprs_account_add_mail(rt_uint16_t pos)
{
	struct account_head *data;
	rt_uint8_t 					result;
	rt_int32_t          ah_result;
	
	data = rt_calloc(1,sizeof(*data));
	
	ah_result = device_config_account_operate(pos,data,0);
	if(ah_result < 0)
	{
		rt_kprintf("GPRS mail account operate fail >>%s",__FUNCTION__);
	}

	result = msg_mail_account_add(pos,data->name,rt_strlen(data->name));
	if(result == 0)
	{
		//成功上传
		data->is_updated = 1;
    device_config_account_operate(pos,data,1);
		rt_kprintf("Account Upload succeed\n");
	}
	else
	{
		rt_kprintf("Account Upload Fail\n");
	}
	rt_free(data);	
}

//账户删除
void gprs_account_del_mail(rt_uint16_t pos)
{
	
}

//手机添加
void gprs_phone_add_mail(rt_uint16_t pos)
{

}
//手机删除
