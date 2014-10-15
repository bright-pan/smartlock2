/*********************************************************************
 * Filename:      sms.c
 *
 *
 * Author:        Bright Pan <loststriker@gmail.com>
 * Created at:    2013-05-06 09:33:10
 *
 *
 * Change Log:
 *
 * Copyright (C) 2013 Yuettak Co.,Ltd
 ********************************************************************/

#include "sms.h"
#include "appconfig.h"
#include "gsm.h"
#include "config.h"

#define SMS_DEBUG 1

#define SMS_THREAD_PRI  				(SMS_THREAD_PRI_IS) 		//短信线程优先级
#define SMS_RECV_MAIL_OUTTIME   ((RT_TICK_PER_SECOND)*1)//短信线程接收邮件超时

char smsc[20] = {0,};
char phone_call[20] = {0,};
typedef struct 
{
	rt_list_t		  list;
	ALARM_TYPEDEF alarm_type;
	rt_uint32_t   counter;
	rt_uint32_t   timeout;
}SMSTimeLag,*SMSTimeLag_p;

#define SMS_RESEND_NUM	3
#define SMS_MAIL_MAX_MSGS 20

/* PDU鏋勯�� */
#define	INTERNATIONAL_ADDRESS_TYPE		0x91
#define	LOCAL_ADDRESS_TYPE				0xA1

#define SMSC_LENGTH_DEFAULT				0x08

#define FIRST_OCTET_DEFAULT				0x11
#define	TP_MR_DEFAULT					0x00
#define	TP_TYPE_DEFAULT					INTERNATIONAL_ADDRESS_TYPE//鍥介檯鍦板潃;
#define	TP_PID_DEFAULT					0x00//鏅�欸SM 鍗忚,鐐瑰鐐规柟寮�;
#define	TP_DCS_DEFAULT					0X08//UCS2缂栫爜鏂瑰紡;
#define	TP_VP_DEFAULT					0XC2//5鍒嗛挓鏈夋晥鏈熼檺;

typedef struct {

	uint8_t SMSC_Length;//璁＄畻鏂瑰紡涓嶅悓;
	uint8_t SMSC_Type_Of_Address;
	uint8_t SMSC_Address_Value[7];

}SMSC_TYPE;

typedef struct {

	uint8_t TP_OA_Length;//璁＄畻鏂瑰紡涓嶅悓;
	uint8_t TP_OA_Type_Of_Address;
	uint8_t TP_OA_Address_Value[7];

}TP_OA_TYPE;

typedef struct {

	uint8_t TP_DA_Length;//璁＄畻鏂瑰紡涓嶅悓;
	uint8_t TP_DA_Type_Of_Address;
	uint8_t TP_DA_Address_Value[7];

}TP_DA_TYPE;

typedef struct {

	uint8_t TP_SCTS_Year;//BCD;
	uint8_t TP_SCTS_Month;
	uint8_t TP_SCTS_Day;
	uint8_t TP_SCTS_Hour;
	uint8_t TP_SCTS_minute;
	uint8_t TP_SCTS_Second;
	uint8_t TP_SCTS_Time_Zone;

}TP_SCTS_TYPE;

typedef struct{

	uint8_t sms_head_length;
	uint8_t sms_laber_length;
	uint8_t sms_head_surplus_length;
	uint8_t sms_laber;
	uint8_t sms_numbers;
	uint8_t sms_index;

}SMS_HEAD_6;



typedef struct {

	uint8_t First_Octet;
	TP_OA_TYPE TP_OA;//9瀛楄妭;
	uint8_t TP_PID;
	uint8_t TP_DCS;
	TP_SCTS_TYPE TP_SCTS;//7瀛楄妭;
	uint8_t TP_UDL;//鐢ㄦ埛闀垮害蹇呴』灏忎簬140 涓瓧鑺�;
	uint8_t TP_UD[140];

}SMS_RECEIVE_TPDU_TYPE;

typedef struct {

	uint8_t	First_Octet;
	uint8_t	TP_MR;
	TP_DA_TYPE	TP_DA;//9瀛楄妭;
	uint8_t	TP_PID;
	uint8_t	TP_DCS;
	uint8_t	TP_VP;// 1涓瓧鑺�;
	uint8_t	TP_UDL;//鐢ㄦ埛闀垮害蹇呴』灏忎簬140涓瓧鑺�;
	uint8_t	TP_UD[420];

}SMS_SEND_TPDU_TYPE;

typedef struct {

	SMSC_TYPE SMSC;
	SMS_RECEIVE_TPDU_TYPE TPDU;

}SMS_RECEIVE_PDU_FRAME;

typedef struct {

	SMSC_TYPE SMSC;
	SMS_SEND_TPDU_TYPE TPDU;

}SMS_SEND_PDU_FRAME;

static rt_mq_t sms_mq = RT_NULL;

#if(SMS_SEND_ASTRICT_IS == 1)
const ALARM_TYPEDEF PictureSMS[] =
{
	4,													 //数量
	ALARM_TYPE_LOCK_SHELL,			 // lock shell alarm type
	ALARM_TYPE_LOCK_TEMPERATURE, // lock temperatrue
	ALARM_TYPE_CAMERA_IRDASENSOR, // camera irda sensor
	ALARM_TYPE_FPRINT_KEY_ERROR,
};
#endif


/*
  鏅鸿兘閿佽嚜鍔ㄦ柇鐢碉紝灏嗚嚜鍔ㄥ惎鍔ㄧ數姹犱緵鐢�
  0x667A,0x80FD,0x9501,0x81EA,0x52A8,0x65AD,0x7535,0xFF0C,0x5C06,0x81EA,0x52A8,0x542F,0x52A8,0x7535,0x6C60,0x4F9B,0x7535

  鏅鸿兘閿佺數姹犵數閲忚繕鍓�50%锛岃灏藉揩鍏呯數
  0x667A,0x80FD,0x9501,0x7535,0x6C60,0x7535,0x91CF,0x8FD8,0x5269,0x0035,0x0030,0x0025,0xFF0C,0x8BF7,0x5C3D,0x5FEB,0x5145,0x7535

  鏅鸿兘閿佺數姹犵數閲忚繕鍓�20%锛岃娉ㄦ剰瀹夊叏骞跺敖蹇厖鐢�
  0x667A,0x80FD,0x9501,0x7535,0x6C60,0x7535,0x91CF,0x8FD8,0x5269,0x0032,0x0030,0x0025,0xFF0C,0x8BF7,0x6CE8,0x610F,0x5B89,0x5168,0x5E76,0x5C3D,0x5FEB,0x5145,0x7535

  鏅鸿兘閿佹憚鍍忓ご琚伄鎸★紝璇锋敞鎰忓畨鍏�
  0x667A,0x80FD,0x9501,0x6444,0x50CF,0x5934,0x88AB,0x906E,0x6321,0xFF0C,0x8BF7,0x6CE8,0x610F,0x5B89,0x5168
  鏅鸿兘閿佹憚鍍忓ご琚伄鎸★紝璇锋敞鎰忓畨鍏�
  0x53D1,0x751F,0x706B,0x707E,0xFF0C,0x8BF7,0x6CE8,0x610F,0x5B89,0x5168
  鏅鸿兘閿佹鍦ㄨ鍒囧壊寮�鍚紝娓呮敞鎰忓畨鍏�
  0x667A,0x80FD,0x9501,0x6B63,0x5728,0x88AB,0x5207,0x5272,0x5F00,0x542F,0xFF0C,0x6E05,0x6CE8,0x610F,0x5B89,0x5168

  鏅鸿兘閿佹琚毚鍔涘紑鍚紝璇锋敞鎰忓畨鍏�
  0x667A,0x80FD,0x9501,0x6B63,0x88AB,0x66B4,0x529B,0x5F00,0x542F,0xFF0C,0x8BF7,0x6CE8,0x610F,0x5B89,0x5168

  鏅鸿兘閿佹琚潪娉曞紑鍚紝璇锋敞鎰忓畨鍏�
  0x667A,0x80FD,0x9501,0x6B63,0x88AB,0x975E,0x6CD5,0x5F00,0x542F,0xFF0C,0x8BF7,0x6CE8,0x610F,0x5B89,0x5168

  鎽勫儚澶存ā鍧楁晠闅滐紝璇疯仈绯荤粡閿�鍟嗘垨鍘傚
  0x6444,0x50CF,0x5934,0x6A21,0x5757,0x6545,0x969C,0xFF0C,0x8BF7,0x8054,0x7CFB,0x7ECF,0x9500,0x5546,0x6216,0x5382,0x5BB6

  RFID妯″潡鏁呴殰锛岃鑱旂郴缁忛攢鍟嗘垨鍘傚
  0x0052,0x0046,0x0049,0x0044,0x6A21,0x5757,0x6545,0x969C,0xFF0C,0x8BF7,0x8054,0x7CFB,0x7ECF,0x9500,0x5546,0x6216,0x5382,0x5BB6

  GSM妯″潡鏁呴殰锛岃鑱旂郴缁忛攢鍟嗘垨鍘傚
  0x0047,0x0053,0x004D,0x6A21,0x5757,0x6545,0x969C,0xFF0C,0x8BF7,0x8054,0x7CFB,0x7ECF,0x9500,0x5546,0x6216,0x5382,0x5BB6

  璺濈浼犳劅鍣ㄦ晠闅滐紝璇疯仈绯荤粡閿�鍟嗘垨鍘傚
  0x8DDD,0x79BB,0x4F20,0x611F,0x5668,0x6545,0x969C,0xFF0C,0x8BF7,0x8054,0x7CFB,0x7ECF,0x9500,0x5546,0x6216,0x5382,0x5BB6

  鐢垫簮渚涚數妯″潡鏁呴殰锛岃鑱旂郴缁忛攢鍟嗘垨鍘傚
  0x7535,0x6E90,0x4F9B,0x7535,0x6A21,0x5757,0x6545,0x969C,0xFF0C,0x8BF7,0x8054,0x7CFB,0x7ECF,0x9500,0x5546,0x6216,0x5382,0x5BB6

  閿佽垖鐢垫満鏁呴殰锛岃鑱旂郴缁忛攢鍟嗘垨鍘傚
  0x9501,0x820C,0x7535,0x673A,0x6545,0x969C,0xFF0C,0x8BF7,0x8054,0x7CFB,0x7ECF,0x9500,0x5546,0x6216,0x5382,0x5BB6

  灏婃暚鐨勭敤鎴凤紝浣犵殑闂ㄦ病鏈夐攣濂斤紝璇锋敞鎰忓畨鍏�
  0x5C0A,0x656C,0x7684,0x7528,0x6237,0xFF0C,0x4F60,0x7684,0x95E8,0x6CA1,0x6709,0x9501,0x597D,0xFF0C,0x8BF7,0x6CE8,0x610F,0x5B89,0x5168

  灏婃暚鐨勭敤鎴凤紝浣犵殑闂ㄩ挜娌℃湁鎷斿彇锛岃娉ㄦ剰瀹夊叏
  0x5C0A,0x656C,0x7684,0x7528,0x6237,0xFF0C,0x4F60,0x7684,0x95E8,0x94A5,0x6CA1,0x6709,0x62D4,0x53D6,0xFF0C,0x8BF7,0x6CE8,0x610F,0x5B89,0x5168
*/

const static uint16_t
NUM_UCS_MAP[16] = {

	0x0030,0x0031,0x0032,0x0033,0x0034,
	0x0035,0x0036,0x0037,0x0038,0x0039
};

const static uint16_t
sms_content_lock_shell[] = {

	0x667A,0x80FD,0x9501,0x6B63,0x88AB,
	0x66B4,0x529B,0x5F00,0x542F,0xFF0C,
	0x8BF7,0x6CE8,0x610F,0x5B89,0x5168
};

const static uint16_t
sms_content_lock_temperature[] = {

	0x667A,0x80FD,0x9501,0x6B63,0x5728,
	0x88AB,0x5207,0x5272,0x5F00,0x542F,
	0xFF0C,0x6E05,0x6CE8,0x610F,0x5B89,
	0x5168
};

const static uint16_t
sms_content_gate_temperature[] = {

	0x53D1,0x751F,0x706B,0x707E,0xFF0C,
	0x8BF7,0x6CE8,0x610F,0x5B89,0x5168
};

const static uint16_t
sms_content_lock_gate[] = {

	0x5C0A,0x656C,0x7684,0x7528,0x6237,
	0xFF0C,0x4F60,0x7684,0x95E8,0x6CA1,
	0x6709,0x9501,0x597D,0xFF0C,0x8BF7,
	0x6CE8,0x610F,0x5B89,0x5168
};

const static uint16_t
sms_content_rfid_key_error[] = {

	0x667A,0x80FD,0x9501,0x6B63,0x88AB,
	0x975E,0x6CD5,0x5F00,0x542F,0xFF0C,
	0x8BF7,0x6CE8,0x610F,0x5B89,0x5168
};

const static uint16_t
sms_content_rfid_key_plugin[] = {

	0x5C0A,0x656C,0x7684,0x7528,0x6237,
	0xFF0C,0x4F60,0x7684,0x95E8,0x94A5,
	0x6CA1,0x6709,0x62D4,0x53D6,0xFF0C,
	0x8BF7,0x6CE8,0x610F,0x5B89,0x5168
};

const static uint16_t
sms_content_camera_idrasensor[] = {

	0x667A,0x80FD,0x9501,0x6444,0x50CF,
	0x5934,0x88AB,0x906E,0x6321,0xFF0C,
	0x8BF7,0x6CE8,0x610F,0x5B89,0x5168
};

const static uint16_t
sms_content_battery_working_20m[] = {

	0x667A,0x80FD,0x9501,0x81EA,0x52A8,
	0x65AD,0x7535,0xFF0C,0x5C06,0x81EA,
	0x52A8,0x542F,0x52A8,0x7535,0x6C60,
	0x4F9B,0x7535
};

const static uint16_t
sms_content_battery_remain_50p[] = {

	0x667A,0x80FD,0x9501,0x7535,0x6C60,
	0x7535,0x91CF,0x8FD8,0x5269,0x0035,
	0x0030,0x0025,0xFF0C,0x8BF7,0x5C3D,
	0x5FEB,0x5145,0x7535
};

const static uint16_t
sms_content_battery_remain_20p[] = {

	0x667A,0x80FD,0x9501,0x7535,0x6C60,
	0x7535,0x91CF,0x8FD8,0x5269,0x0032,
	0x0030,0x0025,0xFF0C,0x8BF7,0x6CE8,
	0x610F,0x5B89,0x5168,0x5E76,0x5C3D,
	0x5FEB,0x5145,0x7535
};

const static uint16_t
sms_content_battery_remain_5p[] = {

	0x5C0A,0x656C,0x7684,0x5BA2,0x6237,
	0xFF0C,0x667A,0x80FD,0x9501,0x7535,
	0x6C60,0x7535,0x91CF,0x4E0D,0x8DB3,
	0x0035,0x0025,0xFF0C,0x7CFB,0x7EDF,
	0x8FDB,0x5165,0x673A,0x68B0,0x9501,
	0x72B6,0x6001,0x3002
};

const static uint16_t
sms_content_camera_fault[] = {

	0x6444,0x50CF,0x5934,0x6A21,0x5757,
	0x6545,0x969C,0xFF0C,0x8BF7,0x8054,
	0x7CFB,0x7ECF,0x9500,0x5546,0x6216,
	0x5382,0x5BB6
};

const static uint16_t
sms_content_rfid_fault[] = {

	0x0052,0x0046,0x0049,0x0044,0x6A21,
	0x5757,0x6545,0x969C,0xFF0C,0x8BF7,
	0x8054,0x7CFB,0x7ECF,0x9500,0x5546,
	0x6216,0x5382,0x5BB6
};

const static uint16_t
sms_content_motor_fault[] = {

	0x9501,0x820C,0x7535,0x673A,0x6545,
	0x969C,0xFF0C,0x8BF7,0x8054,0x7CFB,
	0x7ECF,0x9500,0x5546,0x6216,0x5382,
	0x5BB6
};

const static uint16_t
sms_content_time_prefix[] = {

	0x3010
};

const static uint16_t
sms_content_time_suffix[] = {

	0x60A6,0x5FB7,0x667A,0x80FD,0x3011
};

static const uint8_t
HEX_CHAR_MAP[16] = {

	'0','1','2','3',
	'4','5','6','7',
	'8','9','A','B',
	'C','D','E','F'
};

typedef struct {

	const uint16_t *data;
	uint16_t length;

}SMS_DATA_TYPEDEF;

SMS_DATA_TYPEDEF sms_data[50];

#if(SMS_SEND_ASTRICT_IS == 1)

/** 
@brief 创建短信间隔窗口链表头
@param void
@retval head:头结点地址
*/
static SMSTimeLag_p timelag_list_create()
{
	SMSTimeLag_p head;

	head = rt_calloc(1,sizeof(SMSTimeLag));
	RT_ASSERT(head != RT_NULL);
	
	rt_list_init(&head->list);
	
	return head;
}

/** 
@brief 删除链表
@param head:头结点地址
@retval void
*/
static void timelag_list_delete (SMSTimeLag_p head)
{
	rt_list_t *next;
  SMSTimeLag_p data;

  if(!rt_list_isempty(&head->list))
  {
  	//链表非空
    for (next = head->list.next; next != &head->list;)
    {
      data = rt_list_entry(next,SMSTimeLag,list);
      next = next->next;

			rt_list_remove(&data->list);
			rt_free(data);
			if(rt_list_isempty(&head->list))
			{
				break;
			}
    }
  }
}

/** 
@brief 向窗口中添加一个节点
@param void
@retval RT_EOK	 :Successful operation
@retval RT_ERROR :operation failure
*/
static rt_err_t add_new_node(SMSTimeLag_p head
														  ,ALARM_TYPEDEF type
															,rt_uint32_t timeout)
{
	SMSTimeLag_p data;
	data = rt_calloc(1,sizeof(SMSTimeLag));
	if(data == RT_NULL)
	{
		return RT_ERROR;
	}
	data->alarm_type = type;
	data->timeout = timeout;
	data->counter = 0;
	
	rt_list_insert_after(&head->list,&data->list);

	return RT_EOK;
}

/** 
@brief 删除窗口中超时的节点
@param head:链表入口
@retval void
*/
static void delete_outtime_node(SMSTimeLag_p head)
{
  rt_list_t *next;
  SMSTimeLag_p data;

  if(!rt_list_isempty(&head->list))
  {
  	//链表非空
    for (next = head->list.next; next != &head->list;)
    {
      data = rt_list_entry(next,SMSTimeLag,list);
      next = next->next;

      if(data->counter >= data->timeout)
      {
				rt_list_remove(&data->list);
				rt_free(data);
				if(rt_list_isempty(&head->list))
				{
					break;
				}
      }
    }
  }
}

/** 
@brief 在窗口中查找该类型短信
@param head:链表入口
@retval RT_TRUE :窗口中有这个事件的短信
@retval RT_FALSE :窗口中没有这个事件的短信
*/
static rt_bool_t find_sms_node(SMSTimeLag_p head,ALARM_TYPEDEF type)
{
	rt_list_t *next;
  SMSTimeLag_p data;

  if(!rt_list_isempty(&head->list))
  {
  	//链表非空
    for (next = head->list.next; next != &head->list; next = next->next)
    {
      data = rt_list_entry(next,SMSTimeLag,list);
			if(data->alarm_type == type)
			{
				return RT_TRUE;
			}
    }
  }

  return RT_FALSE;
}

/** 
@brief 增加窗口中的计时器
@param head:链表入口
@retval void
*/
static void update_list_counter(SMSTimeLag_p head)
{
  rt_list_t *next;
  SMSTimeLag_p data;

  if(!rt_list_isempty(&head->list))
  {
  	//链表非空
    for (next = head->list.next; next != &head->list; next = next->next)
    {
      data = rt_list_entry(next,SMSTimeLag,list);
			data->counter++;
    }
  }
}

/** 
@brief 短信发送间隔超时值获取
@param head:链表入口
@retval void
*/
static rt_uint32_t sms_timeout_value_get(ALARM_TYPEDEF type)
{
	rt_uint32_t i;
	rt_uint32_t TimeValue;
	
//	config_file_mutex_op(RT_TRUE);
	/*
	//有图片短信
	for(i=1; i < PictureSMS[0] ;i++)
	{
		if(type == PictureSMS[i])
		{
    	TimeValue = device_config.param.alarm_arg[3].timeout;
		}
	}
	if(type == ALARM_TYPE_RFID_KEY_PLUGIN)
	{
		//钥匙没拔
    TimeValue = device_config.param.alarm_arg[0].timeout;
	}
	else if(type == ALARM_TYPE_LOCK_GATE)
	{
		//门没有关好
    TimeValue = device_config.param.alarm_arg[1].timeout;
	}
	else
	{
		//没有图片的短信
		TimeValue = device_config.param.alarm_arg[2].timeout;
	}
    */
	//config_file_mutex_op(RT_FALSE);

	TimeValue *= 60;
	return TimeValue;
}
#endif

/*
 *  sms content table initialing
 *
 */
static void
sms_data_init(SMS_DATA_TYPEDEF sms_data[])
{
	//lock shell
	sms_data[ALARM_TYPE_LOCK_SHELL].data = sms_content_lock_shell;
	sms_data[ALARM_TYPE_LOCK_SHELL].length = sizeof(sms_content_lock_shell) / sizeof(uint16_t);
	// lock temperature
	sms_data[ALARM_TYPE_LOCK_TEMPERATURE].data = sms_content_lock_temperature;
	sms_data[ALARM_TYPE_LOCK_TEMPERATURE].length = sizeof(sms_content_lock_temperature) / sizeof(uint16_t);
	// gate temperature
	sms_data[ALARM_TYPE_GATE_TEMPERATURE].data = sms_content_gate_temperature;
	sms_data[ALARM_TYPE_GATE_TEMPERATURE].length = sizeof(sms_content_gate_temperature) / sizeof(uint16_t);
	// lock gate status
	sms_data[ALARM_TYPE_LOCK_GATE].data = sms_content_lock_gate;
	sms_data[ALARM_TYPE_LOCK_GATE].length = sizeof(sms_content_lock_gate) / sizeof(uint16_t);
	// rfid key error t alarm type
	sms_data[ALARM_TYPE_RFID_KEY_ERROR].data = sms_content_rfid_key_error;
	sms_data[ALARM_TYPE_RFID_KEY_ERROR].length = sizeof(sms_content_rfid_key_error) / sizeof(uint16_t);
	// rfid key detect alarm type
	sms_data[ALARM_TYPE_RFID_KEY_PLUGIN].data = sms_content_rfid_key_plugin;
	sms_data[ALARM_TYPE_RFID_KEY_PLUGIN].length = sizeof(sms_content_rfid_key_plugin) / sizeof(uint16_t);
	// camera irda sensor alarm
	sms_data[ALARM_TYPE_CAMERA_IRDASENSOR].data = sms_content_camera_idrasensor;
	sms_data[ALARM_TYPE_CAMERA_IRDASENSOR].length = sizeof(sms_content_camera_idrasensor) / sizeof(uint16_t);

	// battry working 20 min
	sms_data[ALARM_TYPE_BATTERY_WORKING_20M].data = sms_content_battery_working_20m;
	sms_data[ALARM_TYPE_BATTERY_WORKING_20M].length = sizeof(sms_content_battery_working_20m) / sizeof(uint16_t);
	// battry remain 50%
	sms_data[ALARM_TYPE_BATTERY_REMAIN_50P].data = sms_content_battery_remain_50p;
	sms_data[ALARM_TYPE_BATTERY_REMAIN_50P].length = sizeof(sms_content_battery_remain_50p) / sizeof(uint16_t);
	// battry working 20%
	sms_data[ALARM_TYPE_BATTERY_REMAIN_20P].data = sms_content_battery_remain_20p;
	sms_data[ALARM_TYPE_BATTERY_REMAIN_20P].length = sizeof(sms_content_battery_remain_20p) / sizeof(uint16_t);
	// battry working 5%
	sms_data[ALARM_TYPE_BATTERY_REMAIN_5P].data = sms_content_battery_remain_5p;
	sms_data[ALARM_TYPE_BATTERY_REMAIN_5P].length = sizeof(sms_content_battery_remain_5p) / sizeof(uint16_t);
	// camera fault
	sms_data[ALARM_TYPE_CAMERA_FAULT].data = sms_content_camera_fault;
	sms_data[ALARM_TYPE_CAMERA_FAULT].length = sizeof(sms_content_camera_fault) / sizeof(uint16_t);
	// rfid fault
	sms_data[ALARM_TYPE_RFID_FAULT].data = sms_content_rfid_fault;
	sms_data[ALARM_TYPE_RFID_FAULT].length = sizeof(sms_content_rfid_fault) / sizeof(uint16_t);
	// motor fault
	sms_data[ALARM_TYPE_MOTOR_FAULT].data = sms_content_motor_fault;
	sms_data[ALARM_TYPE_MOTOR_FAULT].length = sizeof(sms_content_motor_fault) / sizeof(uint16_t);
}


/*
 * convert time to ucs for sms
 *
 */
static uint16_t *
sms_time_ucs(const struct tm *tm_time,
			 const uint16_t *prefix, uint8_t prefix_length,
			 const uint16_t *suffix, uint8_t suffix_length,
			 uint16_t *time_ucs, uint16_t *length)
{
	uint16_t year;
	uint16_t *time_ucs_bk = time_ucs;

	*length += prefix_length;
	while (prefix_length-- > 0)
	{
		*time_ucs++ = *prefix++;
	}
	// year
	year = tm_time->tm_year;
	year %= 10000;
	*time_ucs++ = NUM_UCS_MAP[year / 1000];
	year %= 1000;
	*time_ucs++ = NUM_UCS_MAP[year / 100];
	year %= 100;
	*time_ucs++ = NUM_UCS_MAP[year / 10];
	*time_ucs++ = NUM_UCS_MAP[year % 10];
	*time_ucs++ = 0x002D;// -
	// month
	*time_ucs++ = NUM_UCS_MAP[tm_time->tm_mon / 10];
	*time_ucs++ = NUM_UCS_MAP[tm_time->tm_mon % 10];
	*time_ucs++ = 0x002D;// -
	// day
	*time_ucs++ = NUM_UCS_MAP[tm_time->tm_mday / 10];
	*time_ucs++ = NUM_UCS_MAP[tm_time->tm_mday % 10];
	*time_ucs++ = 0x0020;// 0x0020
	// hour
	*time_ucs++ = NUM_UCS_MAP[tm_time->tm_hour / 10];
	*time_ucs++ = NUM_UCS_MAP[tm_time->tm_hour % 10];
	*time_ucs++ = 0x003A;// 0x003A
	// minute
	*time_ucs++ = NUM_UCS_MAP[tm_time->tm_min / 10];
	*time_ucs++ = NUM_UCS_MAP[tm_time->tm_min % 10];
	*time_ucs++ = 0x003A;// 0x003A
	// second
	*time_ucs++ = NUM_UCS_MAP[tm_time->tm_sec / 10];
	*time_ucs++ = NUM_UCS_MAP[tm_time->tm_sec % 10];
	*time_ucs++ = 0x0020;// 0x0020

	*length += 20;

	*length += suffix_length;
	while (suffix_length-- > 0)
	{
		*time_ucs++ = *suffix++;
	}

	return time_ucs_bk;
}


static void
sms_pdu_phone_address_init(uint8_t *octet, const char *phone_address)
{
	int8_t length = strlen(phone_address);
	int8_t temp = 0;
	char buf[20];
	char *buf_bk = NULL;

	temp = (length & 0x01) ? (length + 1) : length;
	buf_bk = buf;
	memcpy(buf, phone_address, length);

	temp >>= 1;

	while (temp-- > 0)
	{
		*octet = (*buf_bk++ - '0') & 0x0f;
		*octet++ |= ((*buf_bk++ - '0') << 4) & 0xf0;
	}
	if (length & 0x01)
	{
		*--octet |= 0xf0;
	}
}

static void
sms_pdu_head_init(char *smsc_address, char *dest_address, SMS_SEND_PDU_FRAME *pdu, uint8_t tp_udl)
{
	// SMSC initial
	pdu->SMSC.SMSC_Length = 0x08;
	pdu->SMSC.SMSC_Type_Of_Address = 0x91;
	sms_pdu_phone_address_init(pdu->SMSC.SMSC_Address_Value, smsc_address);

	pdu->TPDU.First_Octet = 0x11;
	pdu->TPDU.TP_MR = 0x00;
	// DA initial
	pdu->TPDU.TP_DA.TP_DA_Length = 0x0D;
	pdu->TPDU.TP_DA.TP_DA_Type_Of_Address = 0x91;
	sms_pdu_phone_address_init(pdu->TPDU.TP_DA.TP_DA_Address_Value, dest_address);

	pdu->TPDU.TP_PID = 0x00;
	pdu->TPDU.TP_DCS = 0x08;
	pdu->TPDU.TP_VP = 0xC2;
	pdu->TPDU.TP_UDL = tp_udl;

}

static void
hex_to_string(uint8_t *string, uint8_t *hex, uint8_t hex_length)
{
	while (hex_length-- > 0)
	{
		*string++ = HEX_CHAR_MAP[(*hex >> 4) & 0x0f];
		*string++ = HEX_CHAR_MAP[*hex++ & 0x0f];
	}
}

static int8_t
sms_pdu_ucs_send(char *dest_address, char *smsc_address, uint16_t *content, uint8_t length)
{
	SMS_SEND_PDU_FRAME send_pdu_frame;
	//	GSM_MAIL_TYPEDEF gsm_mail_buf;
	uint8_t *pdu_data;
	uint8_t *send_pdu_string;
	//  char *at_temp, *process_buf;
	uint16_t sms_pdu_length;
	int8_t send_result = 0;//AT_RESPONSE_ERROR;

	if (length > 70)
	{
		length = 70;
	}

	//send_pdu_frame = (SMS_SEND_PDU_FRAME *)rt_malloc(sizeof(SMS_SEND_PDU_FRAME));
	memset(&send_pdu_frame, 0, sizeof(SMS_SEND_PDU_FRAME));

	pdu_data = send_pdu_frame.TPDU.TP_UD;

	send_pdu_string = (uint8_t *)rt_malloc(512);
	memset(send_pdu_string, '\0', 512);

	sms_pdu_head_init(smsc_address, dest_address, &send_pdu_frame, length << 1);

	sms_pdu_length = send_pdu_frame.TPDU.TP_UDL + sizeof(send_pdu_frame.SMSC) + sizeof(send_pdu_frame.TPDU) - sizeof(send_pdu_frame.TPDU.TP_UD);

	while (length-- > 0)
	{
		*pdu_data++ = (uint8_t)(0x00ff & (*content >> 8));
		*pdu_data++ = (uint8_t)(0x00ff & *content++);
	}

	hex_to_string(send_pdu_string + 2, (uint8_t *)&send_pdu_frame, sms_pdu_length);
	*(uint16_t *)send_pdu_string = (uint16_t)(send_pdu_frame.TPDU.TP_UDL + sizeof(send_pdu_frame.TPDU) - sizeof(send_pdu_frame.TPDU.TP_UD));
	//send_ctx_mail(COMM_TYPE_SMS, 0, 0, send_pdu_string, (sms_pdu_length << 1) + 2);
    send_gsm_ctrl_mail(GSM_CTRL_OPEN,RT_NULL,0,1);
    send_gsm_sms_mail(send_pdu_string, (sms_pdu_length << 1) + 2, 1);
    send_gsm_ctrl_mail(GSM_CTRL_CLOSE,RT_NULL,0,1);
	/*
	gsm_mail_buf.send_mode = GSM_MODE_CMD;
	gsm_mail_buf.result = &send_result;
	gsm_mail_buf.result_sem = rt_sem_create("s_ret", 0, RT_IPC_FLAG_FIFO);
	gsm_mail_buf.mail_data.cmd.index = AT_CMGS;
	gsm_mail_buf.mail_data.cmd.delay = 50;
	gsm_mail_buf.mail_data.cmd.cmd_data.cmgs.length = send_pdu_frame.TPDU.TP_UDL + sizeof(send_pdu_frame.TPDU) - sizeof(send_pdu_frame.TPDU.TP_UD);
	gsm_mail_buf.mail_data.cmd.cmd_data.cmgs.buf = send_pdu_string;

	if (mq_gsm != NULL)
	{
		result = rt_mq_send(mq_gsm, &gsm_mail_buf, sizeof(GSM_MAIL_TYPEDEF));
		if (result == -RT_EFULL)
		{
			rt_kprintf("mq_gsm is full!!!\n");
			send_result = AT_RESPONSE_ERROR;
		}
		else
		{
			rt_sem_take(gsm_mail_buf.result_sem, RT_WAITING_FOREVER);
		}
		rt_sem_delete(gsm_mail_buf.result_sem);
	}
	else
	{
		rt_kprintf("mq_gsm is RT_NULL!!!\n");
		send_result = AT_RESPONSE_ERROR;
	}

	if (send_result == AT_RESPONSE_OK)
	{
		send_result = 1;
	}
	else
	{
		send_result = 0;
	}
	*/
	rt_free(send_pdu_string);
	send_pdu_string = RT_NULL;
	//rt_free(gsm_mail_buf);
	return send_result;
}

#if(SMS_SEND_ASTRICT_IS == 1)
SMSTimeLag_p TimeOutWindow = RT_NULL;

#endif

int phone_sms_callback(struct phone_head *ph, void *arg1, void *arg2, void *arg3)
{
    s32 result = -1;
    uint16_t *sms_ucs = arg1;
    uint16_t sms_ucs_length = *(u16 *)arg2;
    if (ph->account != PHONE_ID_INVALID && ph->auth & PHONE_AUTH_SMS) {
        sms_pdu_ucs_send(ph->address, smsc, sms_ucs, sms_ucs_length);
        result = ph->account;
    }
    return result;
}

void
sms_thread_entry(void *parameter)
{
	rt_err_t result;
	struct tm *tm_time;
	uint8_t alarm_telephone_counts = 0;
	uint16_t *sms_ucs, *sms_ucs_bk, sms_ucs_length;
	const uint16_t *temp_ucs;
	uint16_t temp_ucs_length;
	SMS_MAIL_TYPEDEF sms_mail_buf;
#if(SMS_SEND_ASTRICT_IS == 1)

	TimeOutWindow = timelag_list_create();
	RT_ASSERT(TimeOutWindow != RT_NULL);
#endif
	// initial sms data
	sms_data_init(sms_data);

	while (1)
	{
		// process mail
		rt_memset(&sms_mail_buf, 0, sizeof(sms_mail_buf));
		result = rt_mq_recv(sms_mq, &sms_mail_buf,
												sizeof(sms_mail_buf),
												SMS_RECV_MAIL_OUTTIME);
		if (result == RT_EOK)
		{
			#if(SMS_SEND_ASTRICT_IS == 1)
			if(find_sms_node(TimeOutWindow,sms_mail_buf.alarm_type) == RT_TRUE)
			{
				rt_thread_delay(1);
				continue;
			}
			else
			{
				rt_uint32_t AlarmTimeOut;

				AlarmTimeOut = sms_timeout_value_get(sms_mail_buf.alarm_type);
				add_new_node(TimeOutWindow,sms_mail_buf.alarm_type,AlarmTimeOut);
			}
			#endif
			
#if (defined RT_USING_FINSH) && (defined SMS_DEBUG)
			rt_kprintf("\nreceive sms mail < time: %d alarm_type: %s >\n", sms_mail_buf.time, alarm_help_map[sms_mail_buf.alarm_type]);
#endif
			// sms content process
			sms_ucs_length = 0;
			sms_ucs = (uint16_t *)rt_malloc(sizeof(uint16_t) * 256);
			sms_ucs_bk = sms_ucs;

			temp_ucs = sms_data[sms_mail_buf.alarm_type].data;
			temp_ucs_length = sms_data[sms_mail_buf.alarm_type].length;
			sms_ucs_length += temp_ucs_length;

			while (temp_ucs_length-- > 0)
			{
				*sms_ucs_bk++ = *temp_ucs++;
			}
			// sms time process
			tm_time = localtime(&sms_mail_buf.time);
			tm_time->tm_year += 1900;
			tm_time->tm_mon += 1;
			sms_time_ucs(tm_time,
						 sms_content_time_prefix, sizeof(sms_content_time_prefix)/sizeof(uint16_t),
						 sms_content_time_suffix, sizeof(sms_content_time_suffix)/sizeof(uint16_t),
						 sms_ucs_bk,
						 &sms_ucs_length);
			// send sms
            /*
			alarm_telephone_counts = 0;
			while (alarm_telephone_counts < TELEPHONE_NUMBERS)
			{
				if (device_config.param.telephone_address[alarm_telephone_counts].flag)
				{
#if (defined RT_USING_FINSH) && (defined SMS_DEBUG)
					rt_kprintf("\nsend sms to ");
					rt_kprintf((char *)(device_config.param.telephone_address[alarm_telephone_counts].address));
					rt_kprintf("\n");
#endif
					sms_pdu_ucs_send(device_config.param.telephone_address[alarm_telephone_counts].address, smsc, sms_ucs, sms_ucs_length);
				}
				alarm_telephone_counts++;
			}
            */
            device_config_phone_index(phone_sms_callback, sms_ucs, &sms_ucs_length);
			rt_free(sms_ucs);
			sms_ucs = RT_NULL;
		}
		else // receive timeout
		{
			#if(SMS_SEND_ASTRICT_IS == 1)			
            /*
			if(system_event_process(2,SYS_CLEAR_SMS_CNT) == 0)
			{
				timelag_list_delete(TimeOutWindow);
			}
			update_list_counter(TimeOutWindow);
			delete_outtime_node(TimeOutWindow);*/
			#endif
		}
	}
}


void
send_sms_mail(ALARM_TYPEDEF alarm_type, time_t time)
{
	SMS_MAIL_TYPEDEF buf;
	extern rt_device_t rtc_device;
	rt_err_t result;
	//send mail
	buf.alarm_type = alarm_type;
	if (time)
		buf.time = time;
	else
        buf.time = sys_cur_date();

	if (sms_mq != NULL) {
		result = rt_mq_send(sms_mq, &buf, sizeof(SMS_MAIL_TYPEDEF));
		if (result == -RT_EFULL)
            RT_DEBUG_LOG(SMS_DEBUG,("sms_mq is full!!!\n"));
	} else {
        RT_DEBUG_LOG(SMS_DEBUG,("sms_mq is RT_NULL!!!\n"));
    }
    
}

int
rt_sms_init(void)
{
	rt_thread_t sms_thread;

	// initial sms msg queue
	sms_mq = rt_mq_create("sms", sizeof(SMS_MAIL_TYPEDEF),
						  SMS_MAIL_MAX_MSGS, RT_IPC_FLAG_FIFO);
    if (sms_mq == RT_NULL)
        return -1;
	// initial sms thread
	sms_thread = rt_thread_create("sms",
								  sms_thread_entry, RT_NULL,
								  1024, SMS_THREAD_PRI, 5);
	if (sms_thread == RT_NULL)
        return -1;

    rt_thread_startup(sms_thread);

    return 0;

}

INIT_APP_EXPORT(rt_sms_init);

#ifdef RT_USING_FINSH
#include <finsh.h>

static char temp[100];

uint16_t default_data[] = {0x667A,0x80FD,0x9501,0x6B63,0x88AB,
						   0x66B4,0x529B,0x5F00,0x542F,0xFF0C,
						   0x8BF7,0x6CE8,0x610F,0x5B89,0x5168};

void sms(char *address, short *data, char length)
{
	rt_device_t device;
	memset(temp, '\0', 100);
	//device = rt_device_find(DEVICE_NAME_GSM_USART);
	if (device != RT_NULL)
	{
		if (length == 0)
		{
			sms_pdu_ucs_send(address,"8613800755500",default_data, 15);
		}
		else
		{
			sms_pdu_ucs_send(address,"8613800755500",default_data, length);
		}
	}
	else
	{
		//rt_kprintf("device %s is not exist!\n", DEVICE_NAME_GSM_USART);
	}
}
FINSH_FUNCTION_EXPORT(sms, sms[address data length]);
FINSH_FUNCTION_EXPORT(send_sms_mail, sms[address data length]);

#if(SMS_SEND_ASTRICT_IS == 1)

void sms_timeoutwin(void)
{
	rt_list_t *next;
  SMSTimeLag_p data;

  rt_kprintf("---------------------------------------------------------------------------------\n");

  if(!rt_list_isempty(&TimeOutWindow->list))
  {
  	//链表非空
    for (next = TimeOutWindow->list.next; next != &TimeOutWindow->list; next = next->next)
    {
      data = rt_list_entry(next,SMSTimeLag,list);
			rt_kprintf("prev[%08X]<-list[%08X]->netx[%08X] Type=%02d Counter=%03d Timeout=%03d\n"
									,data->list.prev
									,&data->list
									,data->list.next
									,data->alarm_type
									,data->counter
									,data->timeout);
    }
  }
  rt_kprintf("---------------------------------------------------------------------------------\n");
}
FINSH_FUNCTION_EXPORT(sms_timeoutwin, "show sms timeout list");

#endif
#endif
