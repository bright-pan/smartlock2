#ifndef __DATABASE_H__
#define __DATABASE_H__
#include "rtthread.h"
#include <dfs_elm.h>
#include <dfs_fs.h>
#include <dfs.h>
#include <dfs_posix.h>

#define ACCOUNT_MAX_NUM				64

#define KEY_MAX_NUM						8
#define TELEPHONE_MAX_NUM			8
#define PASSWORD_MAX_LEN			6
#define FPRINT_MAX_LEN        512
#define PHONE_MAX_LEN					12
#define DEVICE_ID_LEN					6
#define DEVICE_CDKEY_LEN			6
#define DEVICE_DOMAIN_LEN			20
#define DATABASE_FILE_NAME		"/datainfo"

typedef enum 
{
	KEYTYPE_FPRINT = 0,
	KEYTYPE_RFID = 1,
	KEYTYPE_PASSWORD = 2,
	KEYTYPE_ERROR = 0XFF,
}KeyTypeDef;

typedef enum 
{
	USE_TYPE_FOREVER = 0,
	USE_TYPE_EVERYDAY = 1,
	USE_TYPE_WORKDAY = 2,
	USE_TYPE_ONCE = 3,
}UseTypeDef;

typedef struct 
{
	rt_uint8_t  Useing;			//使用标志
	rt_uint32_t CreatedTime;//创建时间
	rt_uint32_t StartTime;	//开始工作时间
	rt_uint32_t EndTime;		//停止工作时间
	rt_uint32_t DataPos;		//钥匙文件中的位置
	rt_uint16_t CRC16Value;	//钥匙数据CRC值
	KeyTypeDef 	Type;				//钥匙类型
	UseTypeDef UseType;			//使用类型 
}KeyInofDef;

//手机号码
typedef struct 
{
	rt_uint8_t Useing;			//使用标志
	rt_uint8_t DataPos;			//数据位置
}PhoneTypeDef;

//账户结构
typedef struct
{
	rt_uint32_t OpenMode:2;							//开门模式
	rt_uint32_t Useing:1;								//使用标志
	rt_uint32_t Save:1;			  					//保存标志用于更新检测
	rt_uint8_t	SelfPos;								//账号自己所在的位置
	KeyInofDef	Key[KEY_MAX_NUM];				//账号的钥匙位置最大10把钥匙
	PhoneTypeDef Phone[TELEPHONE_MAX_NUM];
}AccountType,*AccountType_p;

//密码数据结构
typedef struct 
{
	rt_uint8_t	Useing;											//使用标志
	rt_uint8_t 	AccountPos;									//账号位置
	rt_uint32_t	SelfPos;										//自己所在的位置
	rt_uint8_t	password[PASSWORD_MAX_LEN];	//密码
	rt_uint16_t CRC16Value;									//crc16数据校验
}PasswordType,*PasswordType_p;

//指纹数据结构
typedef struct 
{
	rt_uint8_t	Useing;											//使用标志
	rt_uint8_t 	AccountPos;									//账号位置
	rt_uint32_t	SelfPos;										//自己所在的位置
	rt_uint8_t 	Fprint[FPRINT_MAX_LEN];			//指纹
	rt_uint16_t CRC16Value;		
}FprintType,*FprintType_p;

//电话数据结构
typedef struct 
{
	rt_uint8_t	Useing;											//使用标志
	rt_uint8_t 	AccountPos;									//账号位置
	rt_uint32_t	SelfPos;										//自己所在的位置
  rt_uint8_t  Phone[PHONE_MAX_LEN];       //手机号码
  rt_uint16_t CRC16Value;		
}PhoneType,*PhoneType_p;
//锁设备类型
typedef struct 
{
	rt_uint8_t ID[6];    //device id
	rt_uint8_t CDKEY[8]; //serial number
}LockDeviceType,*LockDeviceType_p;

//网络参数
typedef struct 
{
	rt_uint8_t 	Domain[DEVICE_DOMAIN_LEN];
	rt_uint16_t Port;
	rt_uint8_t 	DESKey0[8];
}DeviceNetParameterType,*DeviceNetParameterType_p;


typedef struct
{
	rt_uint8_t AccountNum;		//当前账号数量
	rt_uint8_t NetNum;				//当前网络接口数量
}DataBasesType,*DataBasesType_p;

//钥匙检索
typedef struct 
{
	rt_size_t	MapSize;									//总字节大小
	rt_size_t	CurSum;										//当前总数
	rt_size_t	BitSize;									//总位大小
	rt_uint8_t *MapByte;								//每一位代表
}DataMapType,*DataMapType_p;


typedef enum 
{
	MAP_TYPE_ACCOUNT = 1,
	MAP_TYPE_PASSWORD,
	MAP_TYPE_FPRINT,
	MAP_TYPE_PHONE
}MapTypeDef;


//智能锁数据库初始化
rt_err_t databases_init(void);

#endif
