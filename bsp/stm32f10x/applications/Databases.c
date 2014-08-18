#include "Databases.h"
#include "crc16.h"			//提供crc16
#include "netcomm.h"		//提供系统时间

#define DATABASES_CRC16_FUN(A,B)				net_crc16(A,B)
#define DATABASES_DATE_FUN()						net_get_date()

#define STRING_DEVICE_ID								"\x99\x99\x15\x10\x90\x00\x01\x50"
#define STRING_DEVICE_CDKEY							"\x9C\x9E\x11\x36\xD3\x64\xAF\xA9"
#define STRING_NETARG_IP								"115.29.235.194"
#define STRING_NETARG_PORT							6868
#define STRING_NETARG_DESKEY0						"\x01\x02\x03\x04\x05\x06\x99\x99"

#define FILE_OPEN_ERROR_PRINTF(A)				rt_kprintf("File %s not open at %s:%d \n",(A), __FUNCTION__, __LINE__)
#define FILE_RDWD_ERROR_PRINTF					rt_kprintf("File RDWD File %s:%d \n", __FUNCTION__, __LINE__)
#define PER_KEY_TYPE_ALL_NUM						(ACCOUNT_MAX_NUM*KEY_MAX_NUM)
#define PHONE_ALL_NUM										(ACCOUNT_MAX_NUM*TELEPHONE_MAX_NUM)

#define LOCK_DEVICE_SAVE_OFFSET					0
#define NET_PARAMETER_SAVE_OFFSET				(LOCK_DEVICE_SAVE_OFFSET+sizeof(LockDeviceType))//上一个起始地址+上一个要保存的数据大小
#define ACCOUNT_SAVE_OFFSET							(NET_PARAMETER_SAVE_OFFSET+sizeof(DeviceNetParameterType))
#define PASSWORD_SAVE_OFFSET						(ACCOUNT_SAVE_OFFSET+sizeof(AccountType)*ACCOUNT_MAX_NUM)
#define FPRINT_SAVE_OFFSET							(PASSWORD_SAVE_OFFSET+sizeof(PasswordType)*PER_KEY_TYPE_ALL_NUM)
#define PHONE_SAVE_OFFSET								(FPRINT_SAVE_OFFSET+sizeof(FprintType)*PER_KEY_TYPE_ALL_NUM)
#define ACCOUNT_MAP_SAVE_OFFSET					(PHONE_SAVE_OFFSET+sizeof(PhoneType)*PHONE_ALL_NUM)
#define PASSWORD_MAP_SAVE_OFFSET				(ACCOUNT_MAP_SAVE_OFFSET+sizeof(DataMapType)+PER_KEY_TYPE_ALL_NUM/8+4)
#define FPRINT_MAP_SAVE_OFFSET					(PASSWORD_MAP_SAVE_OFFSET+sizeof(DataMapType)+PER_KEY_TYPE_ALL_NUM/8+4)
#define PHONE_MAP_SAVE_OFFSET						(FPRINT_MAP_SAVE_OFFSET+sizeof(DataMapType)+PER_KEY_TYPE_ALL_NUM/8+4)


static rt_mutex_t config_file_mutex = RT_NULL;
static void config_file_mutex_manage(rt_bool_t status);

////////////////////////////////////////////////////////////////////////////////////////////////////
//数据映射信息
static DataMapType_p data_map_create(rt_size_t size)
{
	DataMapType_p map = RT_NULL;
	if(size < 1)
	{
		rt_kprintf("map create size is error!!!\n");
		return RT_NULL;
	}
	map = rt_calloc(1,sizeof(*map));
	RT_ASSERT(map != RT_NULL);
	if(size % 8 == 0)
	{
		map->MapSize= size / 8;
	}
	else
	{
		map->MapSize= size / 8;
		map->MapSize++;
	}
	map->BitSize = size;
	
	map->MapByte = rt_calloc(1,map->MapSize);
	RT_ASSERT(map->MapByte!= RT_NULL);
	return map;
}

static void data_map_delete(DataMapType_p map)
{
	rt_free(map->MapByte);
	rt_free(map);
}

static rt_size_t data_map_empty(DataMapType_p map)
{
	rt_size_t i;
	rt_size_t temp;
	
	RT_ASSERT(map != RT_NULL);
	for(i=0;i<map->BitSize;i++)
	{
		temp = i%8;
		if(!(map->MapByte[temp]>>(7-temp)&0x01))
		{
			return i;
		}
	}
	return map->BitSize;
}

//RT_TRUE 标记 else 清除
static rt_err_t data_map_manage(DataMapType_p map,rt_size_t pos,rt_bool_t operate)
{
	RT_ASSERT(map);
	if(pos >= map->BitSize-1)
	{
		rt_kprintf("map pos is error!!!\n");
		return RT_ERROR;
	}

	if(operate == RT_TRUE)
	{
		map->MapByte[pos/8] |= (0x01<<(7-pos%8)); 
	}
	else
	{
  	map->MapByte[pos/8] &= ~((0x01<<(7-pos%8))); 
	}
}

void data_map_show(DataMapType_p map)
{
	rt_size_t i;
	rt_size_t temp;
	
	RT_ASSERT(map != RT_NULL);

	rt_kprintf("\n");
	for(i=0;i< 16;i++)
	{
		rt_kprintf("|%02d|",i);
	}

	for(i=0;i<map->BitSize;i++)
	{
		temp = i%8;
		
		if(i % 16 == 0)
		{
			rt_kprintf("\n");
		}
		rt_kprintf("[%1d] ",(map->MapByte[i/8]>>(7-temp)&0x01));
	}

	rt_kprintf("\n");
}

//数据库中的位映射域管理 
static rt_err_t databases_map_manage(DataMapType_p map,MapTypeDef type,rt_bool_t operate)
{
	int			 file_id;
	rt_err_t result = RT_EOK;
	
	if(map == RT_NULL)
	{
		return RT_ERROR;
	}

	config_file_mutex_manage(RT_TRUE);
	file_id = open(DATABASE_FILE_NAME,O_RDWR,0x777);
	if(file_id < 0)
	{
		FILE_OPEN_ERROR_PRINTF(DATABASE_FILE_NAME);
		result = RT_ERROR;
	}
	else
	{
		switch(type)
		{
			case MAP_TYPE_ACCOUNT:
			{
				lseek(file_id,ACCOUNT_MAP_SAVE_OFFSET,SEEK_SET);
				break;
			}
			case MAP_TYPE_PASSWORD:
			{
				lseek(file_id,PASSWORD_MAP_SAVE_OFFSET,SEEK_SET);
				break;
			}
			case MAP_TYPE_FPRINT:
			{
				lseek(file_id,FPRINT_MAP_SAVE_OFFSET,SEEK_SET);
				break;
			}
			case MAP_TYPE_PHONE:
			{
				lseek(file_id,PHONE_MAP_SAVE_OFFSET,SEEK_SET);
				break;
			}
			default:
			{
				rt_kprintf("Map is type is error!!!\n");
				break;
			}
		}
		if(operate == RT_TRUE)
		{
			//设置
			write(file_id,map->MapByte,map->MapSize);
		}
		else
		{
			//读取
			read(file_id,map->MapByte,map->MapSize);
		}
	}
	close(file_id);
	
	config_file_mutex_manage(RT_FALSE);
	return result;
}
////////////////////////////////////////////////////////////////////////////////////////////////////


//配置文件操作锁 RT_TRUE 上锁 
static void config_file_mutex_manage(rt_bool_t status)
{
	if(config_file_mutex == RT_NULL)
	{
		config_file_mutex = rt_mutex_create("c_file",RT_IPC_FLAG_FIFO);
		RT_ASSERT(config_file_mutex != RT_NULL);
	}
	if(RT_TRUE == status)
	{	
		rt_mutex_take(config_file_mutex,RT_WAITING_FOREVER);
	}
	else
	{
		rt_mutex_release(config_file_mutex);
	}
}

//锁设备信息的管理  RT_TRUE 设置
rt_err_t lock_device_info_manage(LockDeviceType_p info,rt_bool_t operate)
{
	rt_size_t size;
	rt_err_t result = RT_EOK;
	int file_id;

	//上锁
	config_file_mutex_manage(RT_TRUE);
	
	file_id = open(DATABASE_FILE_NAME,O_RDWR,0x777);
	if(file_id < 0)
	{
		FILE_OPEN_ERROR_PRINTF(DATABASE_FILE_NAME);
		result = RT_ERROR;
	}
	else
	{
		if(RT_TRUE == operate)
		{
			size = write(file_id,info,sizeof(*info));
			if(size != sizeof(*info))
			{
				FILE_RDWD_ERROR_PRINTF;
				result = RT_ERROR;
			}
		}
		else
		{
			size = read(file_id,info,sizeof(*info));
			if(size != sizeof(*info))
			{
				FILE_RDWD_ERROR_PRINTF;
				result = RT_ERROR;
			}
		}

		close(file_id);
	}
	//解锁
	config_file_mutex_manage(RT_FALSE);
	return result;
}

//网络参数  RT_TRUE 设置
rt_err_t net_parameter_info_manage(DeviceNetParameterType_p info,rt_bool_t operate)
{
	rt_size_t size;
	rt_err_t result = RT_EOK;
	int file_id;

	//上锁
	config_file_mutex_manage(RT_TRUE);
	
	file_id = open(DATABASE_FILE_NAME,O_RDWR,0x777);
	if(file_id < 0)
	{
		FILE_OPEN_ERROR_PRINTF(DATABASE_FILE_NAME);
		result = RT_ERROR;
	}
	else
	{
		if(RT_TRUE == operate)
		{
			lseek(file_id,NET_PARAMETER_SAVE_OFFSET,SEEK_SET);
			size = write(file_id,info,sizeof(*info));
			if(size != sizeof(*info))
			{
				FILE_RDWD_ERROR_PRINTF;
				result = RT_ERROR;
			}
		}
		else
		{
			lseek(file_id,NET_PARAMETER_SAVE_OFFSET,SEEK_SET);
			size = read(file_id,info,sizeof(*info));
			if(size != sizeof(*info))
			{
				FILE_RDWD_ERROR_PRINTF;
				result = RT_ERROR;
			}
		}

		close(file_id);
	}
	//解锁
	config_file_mutex_manage(RT_FALSE);
	return result;
}



//账号管理  pos 操作的位置 RT_TRUE 设置
rt_err_t account_data_manage(AccountType_p data,rt_uint8_t pos,rt_bool_t operate)
{
	rt_size_t size;
	rt_err_t result = RT_EOK;
	int file_id;

	//上锁
	config_file_mutex_manage(RT_TRUE);
	
	file_id = open(DATABASE_FILE_NAME,O_RDWR,0x777);
	if(file_id < 0)
	{
		FILE_OPEN_ERROR_PRINTF(DATABASE_FILE_NAME);
		result = RT_ERROR;
	}
	else
	{
		if(RT_TRUE == operate)
		{
			lseek(file_id,ACCOUNT_SAVE_OFFSET+(pos*sizeof(*data)),SEEK_SET);
			size = write(file_id,data,sizeof(*data));
			if(size != sizeof(*data))
			{
				FILE_RDWD_ERROR_PRINTF;
				result = RT_ERROR;
			}
		}
		else
		{
			lseek(file_id,ACCOUNT_SAVE_OFFSET+(pos*sizeof(*data)),SEEK_SET);
			size = read(file_id,data,sizeof(*data));
			if(size != sizeof(*data))
			{
				FILE_RDWD_ERROR_PRINTF;
				result = RT_ERROR;
			}
		}

		close(file_id);
	}
	//解锁
	config_file_mutex_manage(RT_FALSE);
	return result;
}


//密码管理  pos 操作的位置 RT_TRUE 设置
rt_err_t password_data_manage(PasswordType_p data,rt_size_t pos,rt_bool_t operate)
{
	rt_size_t size;
	rt_err_t result = RT_EOK;
	int file_id;

	//上锁
	config_file_mutex_manage(RT_TRUE);
	
	file_id = open(DATABASE_FILE_NAME,O_RDWR,0x777);
	if(file_id < 0)
	{
		FILE_OPEN_ERROR_PRINTF(DATABASE_FILE_NAME);
		result = RT_ERROR;
	}
	else
	{
		if(RT_TRUE == operate)
		{
			lseek(file_id,PASSWORD_SAVE_OFFSET+(pos*sizeof(*data)),SEEK_SET);
			size = write(file_id,data,sizeof(*data));
			if(size != sizeof(*data))
			{
				FILE_RDWD_ERROR_PRINTF;
				result = RT_ERROR;
			}
		}
		else
		{
			lseek(file_id,PASSWORD_SAVE_OFFSET+(pos*sizeof(*data)),SEEK_SET);
			size = read(file_id,data,sizeof(*data));
			if(size != sizeof(*data))
			{
				FILE_RDWD_ERROR_PRINTF;
				result = RT_ERROR;
			}
		}

		close(file_id);
	}
	//解锁
	config_file_mutex_manage(RT_FALSE);
	return result;
}

//账号管理  pos 操作的位置 RT_TRUE 设置
rt_err_t fprint_data_manage(FprintType_p data,rt_size_t pos,rt_bool_t operate)
{
	rt_size_t size;
	rt_err_t result = RT_EOK;
	int file_id;

	//上锁
	config_file_mutex_manage(RT_TRUE);
	
	file_id = open(DATABASE_FILE_NAME,O_RDWR,0x777);
	if(file_id < 0)
	{
		FILE_OPEN_ERROR_PRINTF(DATABASE_FILE_NAME);
		result = RT_ERROR;
	}
	else
	{
		if(RT_TRUE == operate)
		{
			lseek(file_id,FPRINT_SAVE_OFFSET+(pos*sizeof(*data)),SEEK_SET);
			size = write(file_id,data,sizeof(*data));
			if(size != sizeof(*data))
			{
				FILE_RDWD_ERROR_PRINTF;
				result = RT_ERROR;
			}
		}
		else
		{
			lseek(file_id,FPRINT_SAVE_OFFSET+(pos*sizeof(*data)),SEEK_SET);
			size = read(file_id,data,sizeof(*data));
			if(size != sizeof(*data))
			{
				FILE_RDWD_ERROR_PRINTF;
				result = RT_ERROR;
			}
		}

		close(file_id);
	}
	//解锁
	config_file_mutex_manage(RT_FALSE);
	return result;
}

//手机号  pos 操作的位置 RT_TRUE 设置
rt_err_t phone_data_manage(PhoneType_p data,rt_size_t pos,rt_bool_t operate)
{
	rt_size_t size;
	rt_err_t result = RT_EOK;
	int file_id;

	//上锁
	config_file_mutex_manage(RT_TRUE);
	
	file_id = open(DATABASE_FILE_NAME,O_RDWR,0x777);
	if(file_id < 0)
	{
		FILE_OPEN_ERROR_PRINTF(DATABASE_FILE_NAME);
		result = RT_ERROR;
	}
	else
	{
		if(RT_TRUE == operate)
		{
			lseek(file_id,PHONE_SAVE_OFFSET+(pos*sizeof(*data)),SEEK_SET);
			size = write(file_id,data,sizeof(*data));
			if(size != sizeof(*data))
			{
				FILE_RDWD_ERROR_PRINTF;
				result = RT_ERROR;
			}
		}
		else
		{
			lseek(file_id,PHONE_SAVE_OFFSET+(pos*sizeof(*data)),SEEK_SET);
			size = read(file_id,data,sizeof(*data));
			if(size != sizeof(*data))
			{
				FILE_RDWD_ERROR_PRINTF;
				result = RT_ERROR;
			}
		}

		close(file_id);
	}
	//解锁
	config_file_mutex_manage(RT_FALSE);
	return result;
}

//RT_TRUE 设置
rt_err_t account_password_manage(AccountType_p user,PasswordType_p data,rt_bool_t operate)
{
	rt_err_t result;
	rt_uint8_t i;
	
	RT_ASSERT(user != RT_NULL);
	RT_ASSERT(data != RT_NULL);

	if(operate == RT_TRUE)
	{
		for(i=0;i<KEY_MAX_NUM;i++)
		{
			if(user->Key[i].Useing == RT_FALSE)
			{
				//账号中有空钥匙位置
				data->AccountPos = user->SelfPos;//账号位置
				data->Useing = RT_TRUE;						//将钥匙设置为可用
				user->Key[i].DataPos = data->SelfPos;	//设置账号中钥匙的位置
				user->Key[i].CRC16Value = data->CRC16Value = DATABASES_CRC16_FUN(data->password,PASSWORD_MAX_LEN);
				user->Key[i].CreatedTime = DATABASES_DATE_FUN();
				user->Key[i].Type = KEYTYPE_PASSWORD;
				user->Key[i].UseType = USE_TYPE_FOREVER;
				user->Save = RT_TRUE;
				result = account_data_manage(user,user->SelfPos,RT_TRUE);
				if(result == RT_EOK)
				{
					user->Save = RT_FALSE;
				}
				result = password_data_manage(data,data->SelfPos,RT_TRUE);
				break;
			}
		}
		if(i == KEY_MAX_NUM)
		{
			result = RT_ERROR;
		}
	}
	else
	{
		//
	}
	
	return result;
}


//查找密码 返回匹配的钥匙序号
rt_size_t databases_key_matched(KeyTypeDef type,void *data)
{
	DataMapType_p map = RT_NULL;
	rt_size_t			i;
	map = data_map_create(ACCOUNT_MAX_NUM);
	
	switch(type)
	{
		case KEYTYPE_PASSWORD:
		{
			PasswordType_p key;

			key = rt_calloc(1,sizeof(PasswordType));
			RT_ASSERT(key != RT_NULL);
			
			databases_map_manage(map,MAP_TYPE_PASSWORD,RT_FALSE);

			for(i=0;i<map->BitSize;i++)
			{
				rt_size_t temp;
				
				temp = i%8;
				//检测i位置是否有钥匙
				if(!(map->MapByte[temp]>>(7-temp)&0x01))
				{	
					rt_size_t size;
					
					//获取钥匙
					password_data_manage(key,i,RT_FALSE);
					//匹配
					size = rt_memcmp(data,key->password,PASSWORD_MAX_LEN);
					if(size == PASSWORD_MAX_LEN)
					{
						break;//跳出匹配
					}
				}
			}
			rt_free(key);
			break;
		}
		case KEYTYPE_FPRINT:
		{
			FprintType_p key;

			key = rt_calloc(1,sizeof(FprintType));
			RT_ASSERT(key != RT_NULL);
			
			databases_map_manage(map,MAP_TYPE_FPRINT,RT_FALSE);

			for(i=0;i<map->BitSize;i++)
			{
				rt_size_t temp;
				
				temp = i%8;
				//检测i位置是否有钥匙
				if(!(map->MapByte[temp]>>(7-temp)&0x01))
				{	
					rt_size_t size;
					
					//获取钥匙
					fprint_data_manage(key,i,RT_FALSE);
					//匹配
					size = rt_memcmp(data,key->Fprint,FPRINT_MAX_LEN);
					if(size == FPRINT_MAX_LEN)
					{
						break;
					}
				}
			}
			rt_free(key);
			
			break;
		}
		default:
		{
			break;
		}
	}
	data_map_delete(map);

	return i;
}
rt_err_t databases_init(void)
{
	int					file_id;
	
	file_id = open(DATABASE_FILE_NAME,O_RDWR,0x777);
	if(file_id < 0)
	{
		LockDeviceType_p 					device_info;
		DeviceNetParameterType_p 	net_parameter;
		DataMapType_p							map;
		
		file_id = open(DATABASE_FILE_NAME,O_CREAT,0x777);
		if(file_id < 0)
		{
			rt_kprintf("\ninit databases fail!!!\n\n");
			return RT_ERROR;
		}
		rt_kprintf("\ninit databases...\n");
		//初始化设备信息
		device_info = rt_calloc(1,sizeof(*device_info));
		RT_ASSERT(device_info != RT_NULL);
		rt_memcpy(device_info->ID,STRING_DEVICE_ID,sizeof(STRING_DEVICE_ID));
		rt_memcpy(device_info->CDKEY,STRING_DEVICE_CDKEY,sizeof(STRING_DEVICE_CDKEY));
		lock_device_info_manage(device_info,RT_TRUE);
    rt_free(device_info);

		//初始化网络参数
		net_parameter = rt_calloc(1,sizeof(*net_parameter));
		RT_ASSERT(net_parameter != RT_NULL);
		rt_memcpy(net_parameter->Domain,STRING_NETARG_IP,sizeof(STRING_NETARG_IP));
		net_parameter->Port = STRING_NETARG_PORT;
		rt_memcpy(net_parameter->DESKey0,STRING_NETARG_DESKEY0,sizeof(STRING_NETARG_DESKEY0));
		net_parameter_info_manage(net_parameter,RT_TRUE);
    rt_free(net_parameter);

		//初始化映射域
		map = data_map_create(ACCOUNT_MAX_NUM);
		databases_map_manage(map,MAP_TYPE_ACCOUNT,RT_TRUE);
		data_map_delete(map);
		
		map = data_map_create(PER_KEY_TYPE_ALL_NUM);
		databases_map_manage(map,MAP_TYPE_ACCOUNT,RT_TRUE);
		data_map_delete(map);
		
		map = data_map_create(PER_KEY_TYPE_ALL_NUM);
		databases_map_manage(map,MAP_TYPE_ACCOUNT,RT_TRUE);
		data_map_delete(map);
		
		map = data_map_create(PER_KEY_TYPE_ALL_NUM);
		databases_map_manage(map,MAP_TYPE_ACCOUNT,RT_TRUE);
		data_map_delete(map);
		
		map = data_map_create(PHONE_ALL_NUM);
		databases_map_manage(map,MAP_TYPE_ACCOUNT,RT_TRUE);
		data_map_delete(map);
	}
	else
	{
    close(file_id);
	}
  

	rt_kprintf("init databases end!\n\n");
	return RT_EOK;
}

#ifdef RT_USING_FINSH
#include <finsh.h>

void data_map_test(void)
{
  DataMapType_p             map;
  PasswordType_p 						data;

	//测试位映射
	map = data_map_create(ACCOUNT_MAX_NUM);

	data_map_manage(map,12,RT_TRUE);
	data_map_manage(map,3,RT_TRUE);
	data_map_manage(map,6,RT_TRUE);
	data_map_show(map);
	databases_map_manage(map,MAP_TYPE_ACCOUNT,RT_TRUE);
	data_map_delete(map);

	map = data_map_create(ACCOUNT_MAX_NUM);
	databases_map_manage(map,MAP_TYPE_ACCOUNT,RT_FALSE);
	data_map_show(map);
	data_map_delete(map);

	//测试密码匹配
	map = data_map_create(PER_KEY_TYPE_ALL_NUM);
	data = rt_calloc(1,sizeof(PasswordType));
	
	data_map_manage(map,15,RT_TRUE);
	databases_map_manage(map,MAP_TYPE_PASSWORD,RT_TRUE);
  data_map_show(map);

	data->Useing = 1;
	data->SelfPos = 15;
	rt_memcpy(data->password,"\x12\x34\x56\x78\x9a\bc",6);
	password_data_manage(data,15,RT_TRUE);
	data_map_delete(map);
	rt_free(data);

	{
		rt_size_t result;

		result = databases_key_matched(KEYTYPE_PASSWORD,"\x12\x34\x56\x78\x9a\bc");

		rt_kprintf("test key_matched %d\n",result);
	}
}
FINSH_FUNCTION_EXPORT(data_map_test, "test map bit");

void passwordShow(rt_size_t pos)
{
	PasswordType_p 						data;
	data = rt_calloc(1,sizeof(PasswordType));

	rt_free(data);
}
#endif


