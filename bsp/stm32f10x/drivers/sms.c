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

#define SMS_RESEND_NUM	3

/* PDU构造 */
#define	INTERNATIONAL_ADDRESS_TYPE		0x91
#define	LOCAL_ADDRESS_TYPE				0xA1

#define SMSC_LENGTH_DEFAULT				0x08

#define FIRST_OCTET_DEFAULT				0x11
#define	TP_MR_DEFAULT					0x00
#define	TP_TYPE_DEFAULT					INTERNATIONAL_ADDRESS_TYPE//国际地址;
#define	TP_PID_DEFAULT					0x00//普通GSM 协议,点对点方式;
#define	TP_DCS_DEFAULT					0X08//UCS2编码方式;
#define	TP_VP_DEFAULT					0XC2//5分钟有效期限;

typedef struct {

	uint8_t SMSC_Length;//计算方式不同;
	uint8_t SMSC_Type_Of_Address;
	uint8_t SMSC_Address_Value[7];

}SMSC_TYPE;

typedef struct {

	uint8_t TP_OA_Length;//计算方式不同;
	uint8_t TP_OA_Type_Of_Address;
	uint8_t TP_OA_Address_Value[7];

}TP_OA_TYPE;

typedef struct {

	uint8_t TP_DA_Length;//计算方式不同;
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
	TP_OA_TYPE TP_OA;//9字节;
	uint8_t TP_PID;
	uint8_t TP_DCS;
	TP_SCTS_TYPE TP_SCTS;//7字节;
	uint8_t TP_UDL;//用户长度必须小于140 个字节;
	uint8_t TP_UD[140];

}SMS_RECEIVE_TPDU_TYPE;

typedef struct {

	uint8_t	First_Octet;
	uint8_t	TP_MR;
	TP_DA_TYPE	TP_DA;//9字节;
	uint8_t	TP_PID;
	uint8_t	TP_DCS;
	uint8_t	TP_VP;// 1个字节;
	uint8_t	TP_UDL;//用户长度必须小于140个字节;
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

rt_mq_t sms_mq = RT_NULL;

#ifdef USE_SMS_SEND_TYPE2
rt_uint32_t sms_send_time[7] = {0,};
const rt_uint32_t sms_send_map[7]= {SMS_TYPE3,SMS_TYPE4,
									SMS_TYPE6,SMS_TYPE8,
									SMS_TYPE9,SMS_TYPE10,
									SMS_TYPE11};
#endif

/*
  智能锁自动断电，将自动启动电池供电
  0x667A,0x80FD,0x9501,0x81EA,0x52A8,0x65AD,0x7535,0xFF0C,0x5C06,0x81EA,0x52A8,0x542F,0x52A8,0x7535,0x6C60,0x4F9B,0x7535

  智能锁电池电量还剩50%，请尽快充电
  0x667A,0x80FD,0x9501,0x7535,0x6C60,0x7535,0x91CF,0x8FD8,0x5269,0x0035,0x0030,0x0025,0xFF0C,0x8BF7,0x5C3D,0x5FEB,0x5145,0x7535

  智能锁电池电量还剩20%，请注意安全并尽快充电
  0x667A,0x80FD,0x9501,0x7535,0x6C60,0x7535,0x91CF,0x8FD8,0x5269,0x0032,0x0030,0x0025,0xFF0C,0x8BF7,0x6CE8,0x610F,0x5B89,0x5168,0x5E76,0x5C3D,0x5FEB,0x5145,0x7535

  智能锁摄像头被遮挡，请注意安全
  0x667A,0x80FD,0x9501,0x6444,0x50CF,0x5934,0x88AB,0x906E,0x6321,0xFF0C,0x8BF7,0x6CE8,0x610F,0x5B89,0x5168
  智能锁摄像头被遮挡，请注意安全
  0x53D1,0x751F,0x706B,0x707E,0xFF0C,0x8BF7,0x6CE8,0x610F,0x5B89,0x5168
  智能锁正在被切割开启，清注意安全
  0x667A,0x80FD,0x9501,0x6B63,0x5728,0x88AB,0x5207,0x5272,0x5F00,0x542F,0xFF0C,0x6E05,0x6CE8,0x610F,0x5B89,0x5168

  智能锁正被暴力开启，请注意安全
  0x667A,0x80FD,0x9501,0x6B63,0x88AB,0x66B4,0x529B,0x5F00,0x542F,0xFF0C,0x8BF7,0x6CE8,0x610F,0x5B89,0x5168

  智能锁正被非法开启，请注意安全
  0x667A,0x80FD,0x9501,0x6B63,0x88AB,0x975E,0x6CD5,0x5F00,0x542F,0xFF0C,0x8BF7,0x6CE8,0x610F,0x5B89,0x5168

  摄像头模块故障，请联系经销商或厂家
  0x6444,0x50CF,0x5934,0x6A21,0x5757,0x6545,0x969C,0xFF0C,0x8BF7,0x8054,0x7CFB,0x7ECF,0x9500,0x5546,0x6216,0x5382,0x5BB6

  RFID模块故障，请联系经销商或厂家
  0x0052,0x0046,0x0049,0x0044,0x6A21,0x5757,0x6545,0x969C,0xFF0C,0x8BF7,0x8054,0x7CFB,0x7ECF,0x9500,0x5546,0x6216,0x5382,0x5BB6

  GSM模块故障，请联系经销商或厂家
  0x0047,0x0053,0x004D,0x6A21,0x5757,0x6545,0x969C,0xFF0C,0x8BF7,0x8054,0x7CFB,0x7ECF,0x9500,0x5546,0x6216,0x5382,0x5BB6

  距离传感器故障，请联系经销商或厂家
  0x8DDD,0x79BB,0x4F20,0x611F,0x5668,0x6545,0x969C,0xFF0C,0x8BF7,0x8054,0x7CFB,0x7ECF,0x9500,0x5546,0x6216,0x5382,0x5BB6

  电源供电模块故障，请联系经销商或厂家
  0x7535,0x6E90,0x4F9B,0x7535,0x6A21,0x5757,0x6545,0x969C,0xFF0C,0x8BF7,0x8054,0x7CFB,0x7ECF,0x9500,0x5546,0x6216,0x5382,0x5BB6

  锁舌电机故障，请联系经销商或厂家
  0x9501,0x820C,0x7535,0x673A,0x6545,0x969C,0xFF0C,0x8BF7,0x8054,0x7CFB,0x7ECF,0x9500,0x5546,0x6216,0x5382,0x5BB6

  尊敬的用户，你的门没有锁好，请注意安全
  0x5C0A,0x656C,0x7684,0x7528,0x6237,0xFF0C,0x4F60,0x7684,0x95E8,0x6CA1,0x6709,0x9501,0x597D,0xFF0C,0x8BF7,0x6CE8,0x610F,0x5B89,0x5168

  尊敬的用户，你的门钥没有拔取，请注意安全
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

	hex_to_string(send_pdu_string, (uint8_t *)&send_pdu_frame, sms_pdu_length);

	send_ctx_mail(COMM_TYPE_SMS, send_pdu_string, send_pdu_frame.TPDU.TP_UDL + sizeof(send_pdu_frame.TPDU) - sizeof(send_pdu_frame.TPDU.TP_UD));
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

	// initial sms data
	sms_data_init(sms_data);

	while (1)
	{
		// process mail
		rt_memset(&sms_mail_buf, 0, sizeof(sms_mail_buf));
		result = rt_mq_recv(sms_mq, &sms_mail_buf,
							sizeof(sms_mail_buf),
							100);
		if (result == RT_EOK)
		{
			rt_kprintf("\nreceive sms mail < time: %d alarm_type: %s >\n", sms_mail_buf.time, alarm_help_map[sms_mail_buf.alarm_type]);
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


			alarm_telephone_counts = 0;
			//			gsm_muntex_control(RT_TRUE,"SMS");
			while (alarm_telephone_counts < TELEPHONE_NUMBERS)
			{
				if (device_parameters.telephone_address[alarm_telephone_counts].flag)
				{
					rt_kprintf("\nsend sms to ");
					rt_kprintf((char *)(device_parameters.telephone_address[alarm_telephone_counts].address));
					rt_kprintf("\n");
					sms_pdu_ucs_send(device_parameters.telephone_address[alarm_telephone_counts].address, "8613800755500", sms_ucs, sms_ucs_length);
				}
				alarm_telephone_counts++;
			}
			//gsm_muntex_control(RT_FALSE,"SMS");
			//      sms("8613544033975","",0);
			rt_free(sms_ucs);
			sms_ucs = RT_NULL;
		}
		else // receive timeout
		{
			/*
#ifdef USE_SMS_SEND_TYPE2
			rt_uint8_t i;

			for(i=0;i<7;i++)
			{
				if(sms_send_time[i] != 0)
				{
					sms_send_time[i]++;
					if(sms_send_time[i] >= 60*USE_NO_PIC_SMS_T)
					{
						sms_send_time[i] = 0;
						sms_type_event_flag(2,sms_send_map[i]);
					}
					RT_DEBUG_LOG(PRINTF_SMS_COUNT_DOWN,("sms_send_time[%d] = %d\n",i,sms_send_time[i]));
				}
			}
#endif
			*/
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
	if (!time)
	{
		RT_ASSERT(rtc_device != RT_NULL);
		rt_device_control(rtc_device, RT_DEVICE_CTRL_RTC_GET_TIME, &(buf.time));
	}
	else
	{
		buf.time = time;
	}
	if (sms_mq != NULL)
	{
		result = rt_mq_send(sms_mq, &buf, sizeof(SMS_MAIL_TYPEDEF));
		if (result == -RT_EFULL)
		{
			rt_kprintf("sms_mq is full!!!\n");
		}
	}
	else
	{
		rt_kprintf("sms_mq is RT_NULL!!!\n");
	}
}

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
FINSH_FUNCTION_EXPORT(sms, sms[address data length])
FINSH_FUNCTION_EXPORT(send_sms_mail, sms[address data length])
#endif
