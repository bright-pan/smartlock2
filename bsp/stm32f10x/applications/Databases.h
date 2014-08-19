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
	rt_uint8_t  Useing;			//ʹ�ñ�־
	rt_uint32_t CreatedTime;//����ʱ��
	rt_uint32_t StartTime;	//��ʼ����ʱ��
	rt_uint32_t EndTime;		//ֹͣ����ʱ��
	rt_uint32_t DataPos;		//Կ���ļ��е�λ��
	rt_uint16_t CRC16Value;	//Կ������CRCֵ
	KeyTypeDef 	Type;				//Կ������
	UseTypeDef UseType;			//ʹ������ 
}KeyInofDef;

//�ֻ�����
typedef struct 
{
	rt_uint8_t Useing;			//ʹ�ñ�־
	rt_uint8_t DataPos;			//����λ��
}PhoneTypeDef;

//�˻��ṹ
typedef struct
{
	rt_uint32_t OpenMode:2;							//����ģʽ
	rt_uint32_t Useing:1;								//ʹ�ñ�־
	rt_uint32_t Save:1;			  					//�����־���ڸ��¼��
	rt_uint8_t	SelfPos;								//�˺��Լ����ڵ�λ��
	KeyInofDef	Key[KEY_MAX_NUM];				//�˺ŵ�Կ��λ�����10��Կ��
	PhoneTypeDef Phone[TELEPHONE_MAX_NUM];
}AccountType,*AccountType_p;

//�������ݽṹ
typedef struct 
{
	rt_uint8_t	Useing;											//ʹ�ñ�־
	rt_uint8_t 	AccountPos;									//�˺�λ��
	rt_uint32_t	SelfPos;										//�Լ����ڵ�λ��
	rt_uint8_t	password[PASSWORD_MAX_LEN];	//����
	rt_uint16_t CRC16Value;									//crc16����У��
}PasswordType,*PasswordType_p;

//ָ�����ݽṹ
typedef struct 
{
	rt_uint8_t	Useing;											//ʹ�ñ�־
	rt_uint8_t 	AccountPos;									//�˺�λ��
	rt_uint32_t	SelfPos;										//�Լ����ڵ�λ��
	rt_uint8_t 	Fprint[FPRINT_MAX_LEN];			//ָ��
	rt_uint16_t CRC16Value;		
}FprintType,*FprintType_p;

//�绰���ݽṹ
typedef struct 
{
	rt_uint8_t	Useing;											//ʹ�ñ�־
	rt_uint8_t 	AccountPos;									//�˺�λ��
	rt_uint32_t	SelfPos;										//�Լ����ڵ�λ��
  rt_uint8_t  Phone[PHONE_MAX_LEN];       //�ֻ�����
  rt_uint16_t CRC16Value;		
}PhoneType,*PhoneType_p;
//���豸����
typedef struct 
{
	rt_uint8_t ID[6];    //device id
	rt_uint8_t CDKEY[8]; //serial number
}LockDeviceType,*LockDeviceType_p;

//�������
typedef struct 
{
	rt_uint8_t 	Domain[DEVICE_DOMAIN_LEN];
	rt_uint16_t Port;
	rt_uint8_t 	DESKey0[8];
}DeviceNetParameterType,*DeviceNetParameterType_p;


typedef struct
{
	rt_uint8_t AccountNum;		//��ǰ�˺�����
	rt_uint8_t NetNum;				//��ǰ����ӿ�����
}DataBasesType,*DataBasesType_p;

//Կ�׼���
typedef struct 
{
	rt_size_t	MapSize;									//���ֽڴ�С
	rt_size_t	CurSum;										//��ǰ����
	rt_size_t	BitSize;									//��λ��С
	rt_uint8_t *MapByte;								//ÿһλ����
}DataMapType,*DataMapType_p;


typedef enum 
{
	MAP_TYPE_ACCOUNT = 1,
	MAP_TYPE_PASSWORD,
	MAP_TYPE_FPRINT,
	MAP_TYPE_PHONE
}MapTypeDef;


//���������ݿ��ʼ��
rt_err_t databases_init(void);

#endif
