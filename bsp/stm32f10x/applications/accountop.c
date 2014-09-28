#include"accountop.h"
#include "config.h"
#include "fprint.h"
typedef struct
{
	rt_int16_t AccountPos;
	rt_int16_t PasswordPos;
	rt_int16_t PhonePos;
	rt_uint8_t Save;
}AccountUseStruct;

static AccountUseStruct AccountUse=
{0,0,0,0};

//进入新增功能
rt_err_t account_add_enter(void)
{
	if(AccountUse.Save == 0)
	{
		AccountUse.Save = 1;
    AccountUse.AccountPos  = device_config_account_create(ACCOUNT_ID_INVALID, RT_NULL,0);
	}

	return RT_EOK;
}

//用户退出
rt_err_t account_add_exit(rt_bool_t mode)
{
	rt_int32_t result;
	
	if(mode == RT_TRUE)
	{
		//保存退出
    AccountUse.Save = 0;
	}
	else
	{
		//不保存退出
		AccountUse.Save = 0;
		result = device_config_account_delete(AccountUse.AccountPos, 0, 0);
		if(result < 0)
		{
			return RT_ERROR;
		}
	}

	return RT_EOK;
}

rt_err_t account_valid_check(rt_int32_t pos)
{
	rt_int32_t result;

  result = device_config_get_account_valid(pos);

	if(result == 0)
	{
		return RT_ERROR;
	}
	
	return RT_EOK;
}

rt_err_t account_cur_delete(void)
{
	rt_int32_t  result;
	
	result = device_config_account_delete(AccountUse.AccountPos, 0, 0);
	if(result < 0)
	{
		return RT_ERROR;
	}
	
	return RT_EOK;
}

rt_err_t key_add_password_check(rt_uint8_t *key)
{
	rt_int32_t result;

	result = device_config_key_verify(KEY_TYPE_KBOARD,key,6);
	if(result >= 0)
	{
		return RT_ERROR;
	}

	return RT_EOK;
}

rt_err_t key_check_password_cur_pos(rt_uint8_t *password)
{
  rt_int32_t result;
  struct key *key = RT_NULL;

	result = device_config_key_verify(KEY_TYPE_KBOARD,password,6);
	if(result < 0)
	{
	  return RT_ERROR;
	}
	key = rt_calloc(1,sizeof(struct key));
	device_config_key_operate(result,key,0);
	if(key->head.account == AccountUse.AccountPos)
	{
		rt_free(key);
		return RT_EOK;
	}
	
  rt_free(key);
	return RT_ERROR;
}
//修改密码
rt_err_t admin_modify_password(rt_uint8_t *key)
{
	rt_int32_t result;
  struct key *k;
  
  k = rt_calloc(1,sizeof(*k));
  
  AccountUse.PasswordPos = 0;
  device_config_key_operate(AccountUse.PasswordPos,k,0);
  rt_memcpy((void *)k->data.kboard.code,(const void *)key,KEY_KBOARD_CODE_SIZE);
  result = device_config_key_operate(AccountUse.PasswordPos,k,1);
	if(result < 0)
	{
		return RT_ERROR;
	}
  rt_free(k);
  return RT_EOK;
}

//添加新密码
rt_err_t key_add_password(rt_uint8_t *key)
{
	rt_int32_t result;

	result = device_config_key_create(KEY_ID_INVALID, KEY_TYPE_KBOARD,key,6);
	if(result < 0)
	{
		return RT_ERROR;
	}

	AccountUse.PasswordPos = result;
	return RT_EOK;
}


//给用户添加密码
rt_err_t account_cur_add_password(rt_uint8_t *key)
{
	rt_int32_t result;

	if(AccountUse.PasswordPos < 0)
	{
		rt_kprintf("System Error!!!!\n");
	}
  device_config_key_verify(KEY_TYPE_KBOARD,key,6);

	result = device_config_account_append_key(AccountUse.AccountPos,AccountUse.PasswordPos, 0, 0);
	if(result < 0)
	{
		device_config_key_delete(AccountUse.PasswordPos, 0, 0);
		return RT_ERROR;
	}

	return RT_EOK;
}

//检测手机号
rt_err_t user_phone_add_check(rt_uint8_t *phone)
{
	rt_int32_t pos;
	rt_uint8_t len;

	len = rt_strlen(phone);
	RT_ASSERT(phone != RT_NULL);
	pos = device_config_phone_verify(phone,len);
	rt_kprintf("%s This Phone pos %d len %d\n",phone,pos,len);
	if(pos < 0)
	{
		return RT_EOK;
	}

	return RT_ERROR;
}

//创建手机号码
rt_err_t phone_data_create(rt_uint8_t *phone)
{
	rt_int32_t pos;
	
	pos = device_config_phone_create(PHONE_ID_INVALID,phone,PHONE_ADDRESS_LENGTH);
	if(pos < 0)
	{
		return RT_ERROR;
	}
	AccountUse.PhonePos = pos;
	
	return RT_EOK;
}

rt_err_t user_cur_add_phone(rt_uint8_t *phone)
{
	rt_int32_t result;
	
	phone_data_create(phone);

	if(AccountUse.PhonePos < 0)
	{
		return RT_ERROR;
	}
	result = device_config_account_append_phone(AccountUse.AccountPos,AccountUse.PhonePos, 0, 0);
	if(result < 0)
	{
		device_config_phone_delete(AccountUse.PhonePos, 0, 0);
		return RT_ERROR;
	}

	return RT_EOK;
}

//管理员手机修改
rt_err_t admin_modify_phone(rt_uint8_t *phone)
{
  struct account_head *ah;

  ah = rt_calloc(1,sizeof(*ah));
  
	device_config_account_operate(0,ah,0);

	if(ah->phone[0] != KEY_ID_INVALID)
	{
    device_config_account_remove_phone(ah->phone[0]);
	}
	rt_free(ah);

	return user_cur_add_phone(phone);
}

//获取用户最大数量
rt_int32_t user_valid_num(void)
{
	rt_int32_t num;
	
	num = device_config_account_counts();
	if(num < 0)
	{
		return 0;
	}
	
	return num;
}
//当前用户有效密码的个数
rt_uint32_t user_valid_password_num(void)
{
	rt_uint16_t i;
	rt_uint32_t num = 0;
	struct account_head ah;
  struct key *key = RT_NULL;

	key = rt_calloc(1,sizeof(struct key));
	RT_ASSERT(key != RT_NULL);
	
	device_config_account_operate(AccountUse.AccountPos, &ah, 0);
	for(i=0;i<ACCOUNT_KEY_NUMBERS;i++)
	{
		if(ah.key[i] != KEY_ID_INVALID)
		{
			rt_kprintf("ah.key[i] = %d\n",ah.key[i] );
			device_config_key_operate(ah.key[i],key,0);
			if(key->head.key_type == KEY_TYPE_KBOARD)
			{
        num++;
			}
		}
	}
	
	rt_free(key);
	return num;
}

//获得当前用户的手机个数
rt_int32_t user_valid_phone_num(void)
{
	rt_uint16_t i;
	rt_uint32_t num = 0;
	struct account_head ah;

	device_config_account_operate(AccountUse.AccountPos, &ah, 0);
	
	for(i=0;i<ACCOUNT_PHONE_NUMBERS;i++)
	{
		if(ah.phone[i] != PHONE_ID_INVALID)
		{
			num++;
		}	
	}

	return num;
}

//获得当前用的指纹个数
rt_uint32_t user_valid_fprint_num(void)
{
	rt_uint16_t i;
	rt_uint32_t num = 0;
	struct account_head ah;
  struct key *key;

  key = rt_calloc(1,sizeof(struct key));
  RT_ASSERT(key != RT_NULL);
  
	device_config_account_operate(AccountUse.AccountPos, &ah, 0);
	
	for(i=0;i<ACCOUNT_KEY_NUMBERS;i++)
	{
		if(ah.key[i] != KEY_ID_INVALID)
		{
			rt_kprintf("ah.key[i] = %d\n",ah.key[i] );
			device_config_key_operate(ah.key[i],key,0);
			if(key->head.key_type == KEY_TYPE_FPRINT)
			{
        num++;
			}
		}	
	}
	rt_free(key);
	
	return num;
}

//获取
void user_get_info(UserInfoDef_p user,rt_int32_t id)
{
	struct account_head ah;
	
	device_config_account_operate(id, &ah, 0);
	rt_memcpy(user->name,ah.name,ACCOUNT_NAME_LENGTH);
	user->id = id;
}

//注册指纹
rt_err_t user_add_fprint(rt_uint32_t outtime)
{
	rt_uint8_t *buf;
	rt_int32_t result;
	u16 KeyPos;
	
	buf = rt_calloc(1,1024);
	rt_memset(buf,0,1024);
	result = fp_enroll(&KeyPos,buf,outtime);
	if(result < 0)
	{
		rt_kprintf("Fingerprint acquisition failure\n");
		rt_free(buf);

		return RT_ERROR;
	}

	rt_kprintf("正在绑定用户\n");
	result = device_config_account_append_key(AccountUse.AccountPos,result,0, 0);
	if(result < 0)
	{
		rt_kprintf("Fingerprint binding failure\n");
		rt_free(buf);

		return RT_ERROR;
	}
	rt_free(buf);

	return RT_EOK;
}

//修改管理员指纹
rt_err_t admin_modify_fprint(rt_uint32_t outtime)
{
  struct account_head *ah;

  ah = rt_calloc(1,sizeof(*ah));
  
	device_config_account_operate(0,ah,0);

	if(ah->key[1] != KEY_ID_INVALID)
	{
    device_config_account_remove_key(ah->key[1]);
	}
	rt_free(ah);

	return user_add_fprint(outtime);
}

void user_get_info_continuous(UserInfoDef user[],rt_int32_t *start_id,rt_int32_t num,rt_uint8_t flag)
{
	rt_uint8_t i;
	
	rt_memset(user,0,sizeof(UserInfoDef)*num);

	if(flag == 0)
	{
		//向后找
		for(i = 0;i < num;i++)
		{
			rt_int32_t id;

			rt_kprintf("start id:%d\n",*start_id);
			id = device_config_account_next_valid((*start_id),1);
			rt_kprintf("find id : %d\n",id);
			if(id >= 0)
			{
				user_get_info(&user[i],id);
				(*start_id) = id;
				(*start_id)++;
			}
			else
			{
				(*start_id)--;
				break;
			}
		}
	}
	else
	{
		//向前找
		for(i = 0;i < num;i++)
		{
			rt_int32_t id;

			rt_kprintf("start id:%d\n",*start_id);
			id = device_config_account_next_valid((*start_id),0);
			rt_kprintf("find id : %d\n",id);
			if(id >= 0)
			{
				user_get_info(&user[3-i],id);
				(*start_id) = id;
				if((*start_id) > 0)
				{
					(*start_id)--;
				}
			}
			else
			{
				(*start_id)--;
				break;
			}
		}
	}
}

rt_int32_t key_password_verify(rt_uint8_t *password)
{
	rt_int32_t result;
	
  result = device_config_key_verify(KEY_TYPE_KBOARD,password,6);
  
	return result;
}
//设置当前操作的账户
rt_err_t account_set_use(rt_int32_t id)
{
	rt_uint8_t result;

	rt_kprintf("Your Current Choose ID %d\n",id);
	result = device_config_get_account_valid(id);
	if(result == 0)
	{
		return RT_ERROR;
	}
	
	AccountUse.AccountPos = id;

	return RT_EOK;
}

rt_uint32_t account_cur_pos_get(void)
{
	return AccountUse.AccountPos;
}

//创建超级用户
void admin_create(void)
{
	rt_int32_t result;
	rt_int32_t keypos;

	result = device_config_account_next_valid(0,1);
	if(result == 0)
	{
		rt_kprintf("Administrator exist\n");
	}
	else
	{
    result = device_config_account_create("Admin",rt_strlen("Admin"));
	  if(result == 0)
	  {
	  	rt_kprintf("Administrator create OK\n");
	  	result = device_config_key_create(KEY_TYPE_KBOARD,"123456",6);
			if(result >= 0)
			{
				rt_kprintf("Administrator key Create OK\n");
				result = device_config_account_append_key(0,result);
				if(result >= 0)
				{
					rt_kprintf("Administrator append OK\n");
				}
				else
				{
          rt_kprintf("Admin append Fail\n");
          RT_ASSERT(RT_NULL == RT_NULL);
				}
			}
			else
			{
        rt_kprintf("Admin key create Fail\n");
        RT_ASSERT(RT_NULL == RT_NULL);
			}
	  } 
	  else
	  {
	  	rt_kprintf("Admin Create Fail\n");
			RT_ASSERT(RT_NULL == RT_NULL);
	  }
	}
}

//管理员密码匹配
rt_err_t admin_password_verify(rt_uint8_t *password)
{
	rt_int32_t Pos;
	struct key *k;
 
  Pos = key_password_verify(password);
  if(Pos < 0)
  {
		return RT_ERROR;
  }
  k = rt_calloc(1,sizeof(*k));
  device_config_key_operate(Pos,k,0);
	if(k->head.account != 0)
	{
		rt_free(k);
		return RT_ERROR;
	}
	
  rt_free(k);
	return RT_EOK;
}
#ifdef RT_USING_FINSH
#include <finsh.h>

void account_info(void)
{
	rt_kprintf("",AccountUse.AccountPos);
	rt_kprintf("",AccountUse.Save);
}
FINSH_FUNCTION_EXPORT(account_info,account information);


void show_account(rt_uint8_t id)
{
	struct account_head ah;
  rt_uint8_t i;

	device_config_account_operate(id, &ah, 0);
	rt_kprintf("------------------------------------------------|\n");
	if(device_config_get_account_valid(id) > 0)
	{
		rt_kprintf("User name:%s update flag:%d\n",ah.name,ah.is_updated);

		for(i=0;i<ACCOUNT_KEY_NUMBERS;i++)
		{
			if(ah.key[i] != KEY_ID_INVALID)
			{
				struct key  *buf;
				rt_uint8_t j;

				buf = rt_calloc(1,sizeof(struct key));
				
				device_config_key_operate(ah.key[i],buf,0);
				rt_kprintf("Key ID:%03d Password :",ah.key[i]);
				if(buf->head.key_type == KEY_TYPE_KBOARD)
				{
					rt_kprintf("%s\n",buf->data.kboard.code);
				}
				else if(buf->head.key_type == KEY_TYPE_FPRINT)
				{
					rt_kprintf("This is fingerprint\n");
				}
				rt_free(buf);
			}
		}

		for(i=0;i<ACCOUNT_PHONE_NUMBERS;i++)
		{
			if(ah.phone[i] != PHONE_ID_INVALID)
			{
				struct phone_head *ph;
				
				ph = rt_calloc(1,sizeof(struct phone_head));
				device_config_phone_operate(ah.phone[i],ph,0);
				rt_kprintf("Phone ID:%03d Code:%011s\n",ah.phone[i],ph->address);
				rt_free(ph);
			}
			
		}
	}
	else
	{
		rt_kprintf("Account %03d is invalid\n",id);
	}
}
FINSH_FUNCTION_EXPORT(show_account,show user info);

void all_account(rt_uint8_t mode)
{
	rt_uint32_t i;

	for(i=0;i<ACCOUNT_NUMBERS;i++)
	{
		struct account_head ah;

		if(mode == 0)
		{
      device_config_account_operate(i, &ah, 0);
      if(device_config_get_account_valid(i) > 0)
      {
        show_account(i);
      }
		}
		else
		{
      show_account(i);
		}	
	}
}
FINSH_FUNCTION_EXPORT(all_account,show all user info);
#endif


