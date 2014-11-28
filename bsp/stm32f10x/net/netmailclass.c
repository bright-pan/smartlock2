/**
����:�����ʼ���������
�汾:0.1
����:wangzw@yuettak.com
*/
#include "netmailclass.h"
#include "crc16.h"
#include "netfile.h"
#include "stdlib.h"
#include "netprotocol.h"
#include "config.h"
#include "dataSYNC.h"

//#include "untils.h"
//#include "unlockprocess.h"
//#include "apppubulic.h"
//#include "file_update.h"

#ifdef   USEING_CAN_SET_DEBUG
#include "untils.h" //��Ҫʹ������� rt_dprintf
#endif

#ifndef USEING_CAN_SET_DEBUG
#define rt_dprintf    RT_DEBUG_LOG
#endif


#define MAIL_FAULT_RESEND     3
#define MAIL_FAULT_OUTTIME    50

#define NETMAILCLASS_DEBUG      22

#ifndef SYSTEM_SOFTWARE_VER   
#define SYSTEM_SOFTWARE_VER   0x01
#endif


/**
����Э�鷢�ͽӿ�
*/
/** 
@brief  send landed mail
@param  void 
@retval RT_TRUE :succeed,RT_FALSE:fail
*/
rt_sem_t smg_send_wait_sem_crate(void)
{
  char *SemName = RT_NULL;
  rt_sem_t sem = RT_NULL;

  SemName = (char *)rt_calloc(1,RT_NAME_MAX);
  RT_ASSERT(SemName != RT_NULL);

  rt_sprintf(SemName,"NM%d",get_msg_new_order(RT_FALSE)+1);

  sem = rt_sem_create(SemName,0,RT_IPC_FLAG_FIFO);

  RT_ASSERT(sem != RT_NULL);
  
	rt_free(SemName);
	
  return sem;
}

static rt_uint16_t landresendtime = 20;
/** 
@brief  send landed mail
@param  void 
@retval RT_TRUE :succeed,RT_FALSE:fail
*/
void send_net_landed_mail(void)
{
  net_msgmail_p mail;
  net_landed    *UserData = RT_NULL;
  rt_uint32_t   srand_value;

	mail = (net_msgmail_p)rt_calloc(1,sizeof(net_msgmail));
	RT_ASSERT(mail != RT_NULL);

	UserData = (net_landed*)rt_calloc(1,sizeof(net_landed));
	RT_ASSERT(UserData != RT_NULL);
	
  mail->time = 0;
  mail->type = NET_MSGTYPE_LANDED;
  mail->resend = 3;
  mail->outtime = landresendtime;
  mail->sendmode = ASYN_MODE;
  mail->col.byte = get_msg_new_order(RT_TRUE);

	rt_memcpy((char *)UserData->id,
	        (const char *)NetParameterConfig.id,8);
	//k1�����        
	srand(net_get_date());
	srand_value = rand();
	net_uint32_copy_string(NetParameterConfig.key1,srand_value);
	srand(net_get_date());
	srand_value = rand();
	net_uint32_copy_string(&NetParameterConfig.key1[4],srand_value);
	
	rt_memcpy((char *)UserData->k1,
	        (const char *)NetParameterConfig.key1,8);
	UserData->version = SYSTEM_SOFTWARE_VER;

#if(NETMAILCLASS_DEBUG)
	{
		rt_uint8_t i;

		rt_kprintf("K0:\n");
		for(i=0;i<8; i++)
		{
			rt_kprintf("%x,",UserData->k1[i]);
		}
		rt_kprintf("\n");
	}
#endif

	mail->user = UserData;
  
  net_msg_send_mail(mail);

	RT_ASSERT(mail != RT_NULL);
  rt_free(mail);
}

void net_mail_heart(void)
{
	net_msgmail_p mail;
  net_heart    *UserData = RT_NULL;

	/*if(net_event_process(1,NET_ENVET_FILERQ) == 0)
	{
		rt_kprintf("Is dealing with the file\n");
		
		return ;
	}*/
	
	mail = (net_msgmail_p)rt_calloc(1,sizeof(*mail));
	RT_ASSERT(mail != RT_NULL);

	UserData = rt_calloc(1,sizeof(*UserData));
	RT_ASSERT(UserData != RT_NULL);
	
  mail->time = 0;
  mail->type = NET_MSGTYPE_HEART;
  mail->resend = 3;
  mail->outtime = MAIL_FAULT_OUTTIME; 
  mail->sendmode = ASYN_MODE;//�첽
  mail->col.byte = get_msg_new_order(RT_TRUE);

	#ifdef USEING_MOTOR_API
	//UserData->door_status = (motor_status() == RT_TRUE)?0:1;
	#endif
	
	mail->user = UserData;
  
  net_msg_send_mail(mail);

  RT_ASSERT(mail != RT_NULL);
  rt_free(mail);
}

int set_heart_callback(void)
{
	Net_Mail_Heart_callback(net_mail_heart);

	return 0;
}
INIT_APP_EXPORT(set_heart_callback);

/*
����:���ñ����ʼ�
*/
rt_err_t msg_mail_alarm(rt_uint8_t alarm,rt_uint8_t LockStatus,rt_uint32_t time)
{
	rt_uint8_t result;
	net_msgmail_p mail = RT_NULL;
	net_alarm_user *UserData = RT_NULL;
	
	//��ȡ��Դ
	mail = (net_msgmail_p)rt_calloc(1,sizeof(net_msgmail));
	UserData = rt_calloc(1,sizeof(net_alarm_user));
	RT_ASSERT(mail != RT_NULL);
	RT_ASSERT(UserData != RT_NULL);
	mail->user = UserData;
	
	UserData->result.complete = smg_send_wait_sem_crate();
  RT_ASSERT(UserData != RT_NULL);

	//�����ʼ�
	mail->type = NET_MSGTYPE_ALARM;   //�ʼ�����
  mail->resend = MAIL_FAULT_RESEND;									//�ط�����
  mail->outtime = MAIL_FAULT_OUTTIME;              //��ʱ��
  mail->sendmode = SYNC_MODE;       //ͬ��
  mail->col.byte = get_msg_new_order(RT_TRUE);
	//����˽������
	UserData->alarm.lock_status = LockStatus;
	UserData->alarm.type = alarm;
	net_uint32_copy_string(UserData->alarm.time,time);
	//�����ʼ�
  net_msg_send_mail(mail);
  rt_sem_take(UserData->result.complete,RT_WAITING_FOREVER);
  rt_sem_delete(UserData->result.complete);
  rt_dprintf(NETMAILCLASS_DEBUG,("message send result:%d\n",UserData->result.result));
  result = UserData->result.result;
  
  //�ͷ���Դ
  RT_ASSERT(UserData != RT_NULL);
	rt_free(UserData);
	RT_ASSERT(mail != RT_NULL);
	rt_free(mail);

  return (result == 0)?RT_EOK:RT_ERROR;
}

/*
����:���ù��ϱ���
*/
rt_err_t msg_mail_fault(rt_uint8_t fault,rt_uint32_t time)
{
	rt_uint8_t result;
	net_msgmail_p mail = RT_NULL;
	net_fault_user *UserData = RT_NULL;

	//��ȡ��Դ
	mail = (net_msgmail_p)rt_calloc(1,sizeof(net_msgmail));
	UserData = rt_calloc(1,sizeof(net_fault_user));
	RT_ASSERT(mail != RT_NULL);
	RT_ASSERT(UserData != RT_NULL);
	mail->user = UserData;
	UserData->result.complete = smg_send_wait_sem_crate();
  RT_ASSERT(UserData != RT_NULL);

	//�����ʼ�
	mail->type = NET_MSGTYPE_FAULT;   //�ʼ�����
  mail->resend = MAIL_FAULT_RESEND;									//�ط�����
  mail->outtime = MAIL_FAULT_OUTTIME;              //��ʱ��
  mail->sendmode = SYNC_MODE;       //ͬ��
  mail->col.byte = get_msg_new_order(RT_TRUE);
	//����˽������
	UserData->fault.type = fault;
	net_uint32_copy_string(UserData->fault.time,time);
	//�����ʼ�
  net_msg_send_mail(mail);
  rt_sem_take(UserData->result.complete,RT_WAITING_FOREVER);
  rt_sem_delete(UserData->result.complete);
  rt_dprintf(NETMAILCLASS_DEBUG,("message send result:%d\n",UserData->result.result));
  result = UserData->result.result;
  
  //�ͷ���Դ
  RT_ASSERT(UserData != RT_NULL);
	rt_free(UserData);
	RT_ASSERT(mail != RT_NULL);
	rt_free(mail);

  return (result == 0)?RT_EOK:RT_ERROR;
}

/*
����:Կ�׿���
*/
rt_err_t msg_mail_opendoor(rt_uint8_t type,rt_uint16_t account,rt_uint16_t key,rt_uint32_t time)
{
	rt_uint8_t result;
	net_msgmail_p mail = RT_NULL;
	net_opendoor_user *UserData = RT_NULL;

	//��ȡ��Դ
	mail = (net_msgmail_p)rt_calloc(1,sizeof(net_msgmail));
	UserData = rt_calloc(1,sizeof(net_opendoor_user));
	RT_ASSERT(mail != RT_NULL);
	RT_ASSERT(UserData != RT_NULL);
	mail->user = UserData;
	UserData->result.complete = smg_send_wait_sem_crate();
  RT_ASSERT(UserData != RT_NULL);

	//�����ʼ�
	mail->type = NET_MSGTYPE_OPENDOOR;   //�ʼ�����
  mail->resend = MAIL_FAULT_RESEND;									//�ط�����
  mail->outtime = MAIL_FAULT_OUTTIME;              //��ʱ��
  mail->sendmode = SYNC_MODE;       //ͬ��
  mail->col.byte = get_msg_new_order(RT_TRUE);
	//����˽������
	UserData->opendoor.type = type;
	net_uint16_copy_string(UserData->opendoor.account,account);
	net_uint16_copy_string(UserData->opendoor.key,key);
	net_uint32_copy_string(UserData->opendoor.time,time);
	//�����ʼ�
  net_msg_send_mail(mail);
  rt_sem_take(UserData->result.complete,RT_WAITING_FOREVER);
  rt_sem_delete(UserData->result.complete);
  rt_dprintf(NETMAILCLASS_DEBUG,("message send result:%d\n",UserData->result.result));
  result = UserData->result.result;
  
  //�ͷ���Դ
  RT_ASSERT(UserData != RT_NULL);
	rt_free(UserData);
	RT_ASSERT(mail != RT_NULL);
	rt_free(mail);

  return (result == 0)?RT_EOK:RT_ERROR;
}

/*
����:��ر���
*/
rt_err_t msg_mail_battery(rt_uint8_t status,rt_uint8_t capacity,rt_uint32_t time)
{
	rt_uint8_t result;
	net_msgmail_p mail = RT_NULL;
	net_battery_user *UserData = RT_NULL;

	//��ȡ��Դ
	mail = (net_msgmail_p)rt_calloc(1,sizeof(net_msgmail));
	UserData = rt_calloc(1,sizeof(net_battery_user));
	RT_ASSERT(mail != RT_NULL);
	RT_ASSERT(UserData != RT_NULL);
	mail->user = UserData;
	UserData->result.complete = smg_send_wait_sem_crate();
  RT_ASSERT(UserData != RT_NULL);

	//�����ʼ�
	mail->type = NET_MSGTYPE_BATTERY;   //�ʼ�����
  mail->resend = MAIL_FAULT_RESEND;	  //�ط�����
  mail->outtime = MAIL_FAULT_OUTTIME; //��ʱ��
  mail->sendmode = SYNC_MODE;         //ͬ��
  mail->col.byte = get_msg_new_order(RT_TRUE);
  
	//����˽������
	UserData->battery.status = status;
	UserData->battery.capacity = capacity;
	net_uint32_copy_string(UserData->battery.time,time);
	//�����ʼ�
  net_msg_send_mail(mail);
  rt_sem_take(UserData->result.complete,RT_WAITING_FOREVER);
  rt_sem_delete(UserData->result.complete);
  rt_dprintf(NETMAILCLASS_DEBUG,("message send result:%d\n",UserData->result.result));
  result = UserData->result.result;
  
  //�ͷ���Դ
  RT_ASSERT(UserData != RT_NULL);
	rt_free(UserData);
	RT_ASSERT(mail != RT_NULL);
	rt_free(mail);

  return (result == 0)?RT_EOK:RT_ERROR;
}

/*
����:ʱ��ͬ��
*/
rt_err_t msg_mail_adjust_time(void)
{
	rt_uint8_t result;
	net_msgmail_p mail = RT_NULL;
	net_time_user *UserData = RT_NULL;
		  
	//��ȡ��Դ
	mail = (net_msgmail_p)rt_calloc(1,sizeof(net_msgmail));
	UserData = rt_calloc(1,sizeof(net_time_user));
	RT_ASSERT(mail != RT_NULL);
	RT_ASSERT(UserData != RT_NULL);
	mail->user = UserData;
	UserData->result.complete = smg_send_wait_sem_crate();
	RT_ASSERT(UserData != RT_NULL);

	//�����ʼ�
	mail->type = NET_MSGTYPE_TIME;   //�ʼ�����
	mail->resend = MAIL_FAULT_RESEND;                 //�ط�����
	mail->outtime = MAIL_FAULT_OUTTIME;              //��ʱ��
	mail->sendmode = SYNC_MODE;       //ͬ��
	mail->col.byte = get_msg_new_order(RT_TRUE);
	//����˽������
	net_copy_date_str(UserData->date.time);
	//�����ʼ�
	net_msg_send_mail(mail);
	rt_sem_take(UserData->result.complete,RT_WAITING_FOREVER);
	rt_sem_delete(UserData->result.complete);
	rt_dprintf(NETMAILCLASS_DEBUG,("message send result:%d\n",UserData->result.result));
	result = UserData->result.result;

	//�ͷ���Դ
	RT_ASSERT(UserData != RT_NULL);
	rt_free(UserData);
	RT_ASSERT(mail != RT_NULL);
	rt_free(mail);

  return (result == 0)?RT_EOK:RT_ERROR;
}

/*
����:�û��澯����
*/
rt_err_t msg_mail_alarmarg(rt_uint8_t Type,rt_uint8_t arg)
{
	rt_uint8_t result;
	net_msgmail_p mail = RT_NULL;
	net_alarmarg_user *UserData = RT_NULL;

	//��ȡ��Դ
	mail = (net_msgmail_p)rt_calloc(1,sizeof(net_msgmail));
	UserData = rt_calloc(1,sizeof(net_alarmarg_user));
	RT_ASSERT(mail != RT_NULL);
	RT_ASSERT(UserData != RT_NULL);
	mail->user = UserData;
	UserData->result.complete = smg_send_wait_sem_crate();
  RT_ASSERT(UserData != RT_NULL);

	//�����ʼ�
	mail->type = NET_MSGTYPE_ALARMARG;   //�ʼ�����
  mail->resend = MAIL_FAULT_RESEND;									//�ط�����
  mail->outtime = MAIL_FAULT_OUTTIME;              //��ʱ��
  mail->sendmode = SYNC_MODE;       //ͬ��
  mail->col.byte = get_msg_new_order(RT_TRUE);
	//����˽������
	UserData->args.type = Type;
	UserData->args.arg = arg;
	//�����ʼ�
  net_msg_send_mail(mail);
  rt_sem_take(UserData->result.complete,RT_WAITING_FOREVER);
  rt_sem_delete(UserData->result.complete);
  rt_dprintf(NETMAILCLASS_DEBUG,("message send result:%d\n",UserData->result.result));
  result = UserData->result.result;
  
  //�ͷ���Դ
  RT_ASSERT(UserData != RT_NULL);
	rt_free(UserData);
	RT_ASSERT(mail != RT_NULL);
	rt_free(mail);

  return (result == 0)?RT_EOK:RT_ERROR;
}

/*
����:�ն��ֻ����
*/
rt_err_t msg_mail_phoneadd(rt_uint16_t PhID,rt_uint16_t flag,rt_uint8_t buf[],rt_uint32_t date)
{
	rt_uint8_t result;
	net_msgmail_p mail = RT_NULL;
	net_phoneadd_user *UserData = RT_NULL;
	
	//��ȡ��Դ
	mail = (net_msgmail_p)rt_calloc(1,sizeof(net_msgmail));
	UserData = rt_calloc(1,sizeof(net_keyadd_user));
	RT_ASSERT(mail != RT_NULL);
	RT_ASSERT(UserData != RT_NULL);
	mail->user = UserData;
	
	UserData->result.complete = smg_send_wait_sem_crate();
  RT_ASSERT(UserData != RT_NULL);

	//�����ʼ�
	mail->type = NET_MSGTYPE_PHONEADD;   					//�ʼ�����
  mail->resend = MAIL_FAULT_RESEND;							//�ط�����
  mail->outtime = MAIL_FAULT_OUTTIME;           //��ʱ��
  mail->sendmode = SYNC_MODE;       						//ͬ��
  mail->col.byte = get_msg_new_order(RT_TRUE);
  
	//����˽������
	net_uint16_copy_string(UserData->data.pos,PhID);
	net_uint16_copy_string(UserData->data.flag,flag);
	rt_memcpy(UserData->data.data,buf,12);
	net_uint16_copy_string(UserData->data.date,date);
	
	//�����ʼ�
  net_msg_send_mail(mail);
  rt_sem_take(UserData->result.complete,RT_WAITING_FOREVER);
  rt_sem_delete(UserData->result.complete);
  rt_dprintf(NETMAILCLASS_DEBUG,("message send result:%d\n",UserData->result.result));
  result = UserData->result.result;
  
  //�ͷ���Դ
	RT_ASSERT(UserData != RT_NULL);
	rt_free(UserData);
	RT_ASSERT(mail != RT_NULL);
	rt_free(mail);

  return (result == 0)?RT_EOK:RT_ERROR;
}

/*
����:����ֻ�������
*/
void msg_mail_phoneadd_ack(net_recvmsg_p RMail,rt_uint8_t result)
{
	net_msgmail_p mail = RT_NULL;
	net_phoneadd_ack *UserData = RT_NULL;

	//��ȡ��Դ
	mail = (net_msgmail_p)rt_calloc(1,sizeof(net_msgmail));
	UserData = rt_calloc(1,sizeof(net_phoneadd_ack));
	RT_ASSERT(mail != RT_NULL);
	RT_ASSERT(UserData != RT_NULL);
	mail->user = UserData;
	
	//�����ʼ�
	mail->type = NET_MSGTYPE_PHONEADD_ACK;   	//�ʼ�����
  mail->resend = 0;													//�ط�����
  mail->outtime = MAIL_FAULT_OUTTIME;       //��ʱ��
  mail->sendmode = ASYN_MODE;       				//�첽
  mail->col = RMail->col;
	//����˽������
	//UserData->pos = ;
	rt_memcpy(UserData->pos,RMail->data.phoneadd.pos,2);
	UserData->result = result;
	//�����ʼ�
  net_msg_send_mail(mail);
  
  //�ͷ���Դ
  RT_ASSERT(mail != RT_NULL);
	rt_free(mail);
}

/*
����:�ն��ֻ����
*/
rt_err_t msg_mail_phondel(rt_uint16_t PhID,rt_uint32_t date)
{
	rt_uint8_t result;
	net_msgmail_p mail = RT_NULL;
	net_phonedel_user *UserData = RT_NULL;
	
	//��ȡ��Դ
	mail = (net_msgmail_p)rt_calloc(1,sizeof(net_msgmail));
	UserData = rt_calloc(1,sizeof(net_keyadd_user));
	RT_ASSERT(mail != RT_NULL);
	RT_ASSERT(UserData != RT_NULL);
	mail->user = UserData;
	
	UserData->result.complete = smg_send_wait_sem_crate();
  RT_ASSERT(UserData != RT_NULL);

	//�����ʼ�
	mail->type = NET_MSGTYPE_PHONEDELETE;   					//�ʼ�����
  mail->resend = MAIL_FAULT_RESEND;							//�ط�����
  mail->outtime = MAIL_FAULT_OUTTIME;           //��ʱ��
  mail->sendmode = SYNC_MODE;       						//ͬ��
  mail->col.byte = get_msg_new_order(RT_TRUE);
  
	//����˽������
	net_uint16_copy_string(UserData->data.pos,PhID);
	net_uint16_copy_string(UserData->data.date,date);
	
	//�����ʼ�
  net_msg_send_mail(mail);
  rt_sem_take(UserData->result.complete,RT_WAITING_FOREVER);
  rt_sem_delete(UserData->result.complete);
  rt_dprintf(NETMAILCLASS_DEBUG,("message send result:%d\n",UserData->result.result));
  result = UserData->result.result;
  
  //�ͷ���Դ
	RT_ASSERT(UserData != RT_NULL);
	rt_free(UserData);
	RT_ASSERT(mail != RT_NULL);
	rt_free(mail);

  return (result == 0)?RT_EOK:RT_ERROR;
}

/*
����:ɾ���ֻ�������
*/
void msg_mail_phonedel_ack(net_recvmsg_p RMail,rt_uint8_t result)
{
	net_msgmail_p mail = RT_NULL;
	net_phonedel_ack *UserData = RT_NULL;

	RT_ASSERT(RMail != RT_NULL);
	
	//��ȡ��Դ
	mail = (net_msgmail_p)rt_calloc(1,sizeof(net_msgmail));
	UserData = rt_calloc(1,sizeof(net_phonedel_ack));
	RT_ASSERT(mail != RT_NULL);
	RT_ASSERT(UserData != RT_NULL);
	mail->user = UserData;

	//�����ʼ�
	mail->type = NET_MSGTYPE_PHONEDEL_ACK;   	//�ʼ�����
  mail->resend = 0;													//�ط�����
  mail->outtime = MAIL_FAULT_OUTTIME;       //��ʱ��
  mail->sendmode = ASYN_MODE;       				//�첽
  mail->col = RMail->col;
	//����˽������
	rt_memcpy(UserData->pos,RMail->data.phonedel.pos,2);
	UserData->result = result;
	//�����ʼ�
  net_msg_send_mail(mail);
  
  //�ͷ���Դ
  RT_ASSERT(mail != RT_NULL);
	rt_free(mail);
}

/*
����:�ն����Կ��
*/
rt_err_t msg_mail_keyadd(net_keyadd_user *KeyData)
{
	rt_uint8_t result;
	net_msgmail_p mail = RT_NULL;
	net_keyadd_user *UserData = RT_NULL;
	
	RT_ASSERT(KeyData != RT_NULL);
	//��ȡ��Դ
	mail = (net_msgmail_p)rt_calloc(1,sizeof(net_msgmail));
	//UserData = rt_calloc(1,sizeof(net_keyadd_user));
	RT_ASSERT(mail != RT_NULL);
	UserData = KeyData;
	mail->user = KeyData;
	UserData->result.complete = smg_send_wait_sem_crate();
  RT_ASSERT(UserData != RT_NULL);

	//�����ʼ�
	mail->type = NET_MSGTYPE_KEYADD;   //�ʼ�����
  mail->resend = MAIL_FAULT_RESEND;									//�ط�����
  mail->outtime = MAIL_FAULT_OUTTIME;              //��ʱ��
  mail->sendmode = SYNC_MODE;       //ͬ��
  mail->col.byte = get_msg_new_order(RT_TRUE);
	//����˽������
	//
	//�����ʼ�
  net_msg_send_mail(mail);
  rt_sem_take(UserData->result.complete,RT_WAITING_FOREVER);
  rt_sem_delete(UserData->result.complete);
  rt_dprintf(NETMAILCLASS_DEBUG,("message send result:%d\n",UserData->result.result));
  result = UserData->result.result;
  
  //�ͷ���Դ
	//rt_free(UserData);
	RT_ASSERT(mail != RT_NULL);
	rt_free(mail);

  return (result == 0)?RT_EOK:RT_ERROR;
}

/*
����:�������Կ��Ӧ��
*/
void msg_mail_keyadd_ack(net_recvmsg_p RMail,rt_uint8_t result)
{
  net_msgmail_p mail = RT_NULL;
  net_keyadd_ack *data = RT_NULL;

  //��ȡ��Դ
	mail = (net_msgmail_p)rt_calloc(1,sizeof(net_msgmail));
	RT_ASSERT(mail != RT_NULL);

	data = (net_keyadd_ack *)rt_calloc(1,sizeof(net_keyadd_ack));
	RT_ASSERT(data != RT_NULL);

	//�����ʼ�
	mail->type = NET_MSGTYPE_KEYADD_ACK;	//�ʼ�����
  mail->resend = 0;											//�ط�����
  mail->outtime = MAIL_FAULT_OUTTIME; 	//��ʱ��
  mail->sendmode = ASYN_MODE;						//ͬ��
  mail->col = RMail->col;
  //˽������
  rt_memcpy(data->pos,RMail->data.keyadd.col,2);
  data->result = result;
  mail->user = data;

  //�����ʼ�
  net_msg_send_mail(mail);

  //�ͷ���Դ
  RT_ASSERT(mail != RT_NULL);
	rt_free(mail);
}

//Կ��ɾ��
rt_err_t msg_mail_keydelete(rt_uint16_t pos,rt_uint32_t date)
{
	rt_uint8_t result;
	net_msgmail_p mail = RT_NULL;
	net_keydelete_user *UserData = RT_NULL;

	//��ȡ��Դ
	mail = (net_msgmail_p)rt_calloc(1,sizeof(net_msgmail));
	UserData = rt_calloc(1,sizeof(net_keydelete_user));
	RT_ASSERT(mail != RT_NULL);
	RT_ASSERT(UserData != RT_NULL);
	
	mail->user = UserData;
	UserData->result.complete = smg_send_wait_sem_crate();
  RT_ASSERT(UserData != RT_NULL);

	//�����ʼ�
	mail->type = NET_MSGTYPE_KEYDELETE;   //�ʼ�����
  mail->resend = MAIL_FAULT_RESEND;									//�ط�����
  mail->outtime = MAIL_FAULT_OUTTIME;              //��ʱ��
  mail->sendmode = SYNC_MODE;       //ͬ��
  mail->col.byte = get_msg_new_order(RT_TRUE);
  
	//����˽������
	pos = net_rev16(pos);
	rt_memcpy(UserData->data.pos,&pos,2);
	date = net_rev32(date);
	rt_memcpy(UserData->data.date,&date,4);
	
	//�����ʼ�
  net_msg_send_mail(mail);
  rt_sem_take(UserData->result.complete,RT_WAITING_FOREVER);
  rt_sem_delete(UserData->result.complete);
  rt_dprintf(NETMAILCLASS_DEBUG,("message send result:%d\n",UserData->result.result));
  result = UserData->result.result;
  
  //�ͷ���Դ
  RT_ASSERT(UserData != RT_NULL);
	rt_free(UserData);
	RT_ASSERT(mail != RT_NULL);
	rt_free(mail);

  return (result == 0)?RT_EOK:RT_ERROR;
}

/*
����:�������Կ��Ӧ��
*/
void msg_mail_keydel_ack(net_recvmsg_p RMail,rt_uint8_t result)
{
  net_msgmail_p mail = RT_NULL;
  net_keydel_ack *data = RT_NULL;

  //��ȡ��Դ
	mail = (net_msgmail_p)rt_calloc(1,sizeof(net_msgmail));
	RT_ASSERT(mail != RT_NULL);

	data = (net_keydel_ack *)rt_calloc(1,sizeof(net_keydel_ack));
	RT_ASSERT(data != RT_NULL);

	//�����ʼ�
	mail->type = NET_MSGTYPE_KEYDEL_ACK;	//�ʼ�����
  mail->resend = 0;											//�ط�����
  mail->outtime = MAIL_FAULT_OUTTIME; 	//��ʱ��
  mail->sendmode = ASYN_MODE;						//�첽
  mail->col = RMail->col;

  //˽������
  rt_memcpy(data->pos,RMail->data.keydel.key.pos,2);
  data->result = result;
  mail->user = data;

  //�����ʼ�
  net_msg_send_mail(mail);

  //�ͷ���Դ
  RT_ASSERT(mail != RT_NULL);
	rt_free(mail);
}

/** 
@brief send MSGType type message process result
@param MSGType :message type 
@param result :process result
@retval void
*/
void msg_mail_resultack(net_recvmsg_p RMail,rt_uint8_t result)
{
	net_msgmail_p mail = RT_NULL;
	net_ack *UserData;

	//��ȡ��Դ
	mail = (net_msgmail_p)rt_calloc(1,sizeof(net_msgmail));
	RT_ASSERT(mail != RT_NULL);

	//�����ʼ�
	mail->type = RMail->cmd+0x80;	//�ʼ�����
	mail->resend = 0;     //�ط�����
	mail->outtime = 0;    //��ʱ��
	mail->sendmode = ASYN_MODE;	//�첽
	mail->col = RMail->col;

	//���������� �ڷ�����ɺ�����
	UserData = rt_calloc(1,sizeof(net_ack));
	RT_ASSERT(UserData != RT_NULL);
	mail->user = UserData;
	UserData->result = result;
	
	//�����ʼ�
	net_msg_send_mail(mail);

	//�ͷ���Դ
	RT_ASSERT(mail != RT_NULL);
	rt_free(mail);

}

/*
����:����Ӧ��û������
*/
void msg_mail_nullack(net_recvmsg_p RMail)
{
	net_msgmail_p mail = RT_NULL;

	//��ȡ��Դ
	mail = (net_msgmail_p)rt_calloc(1,sizeof(net_msgmail));
	RT_ASSERT(mail != RT_NULL);

	//�����ʼ�
	mail->type = RMail->cmd+0x80; 							//�ʼ�����
  mail->resend = MAIL_FAULT_RESEND;		//�ط�����
  mail->outtime = MAIL_FAULT_OUTTIME; //��ʱ��
  mail->sendmode = ASYN_MODE;					//ͬ��
  mail->col = RMail->col;
  mail->user = RT_NULL;
	
	//�����ʼ�
  net_msg_send_mail(mail);
  
  //�ͷ���Դ
  RT_ASSERT(mail != RT_NULL);
	rt_free(mail);
}

/*
����:�ļ�����Ӧ��
*/
void msg_mail_filereq_ack(net_recvmsg_p RMail,rt_uint8_t result)
{
	net_msgmail_p mail = RT_NULL;
	net_filereq_ack_user *UserData;

	//��ȡ��Դ
	mail = (net_msgmail_p)rt_calloc(1,sizeof(net_msgmail));
	RT_ASSERT(mail != RT_NULL);

	//�����ʼ�
	mail->type = NET_MSGTYPE_FILEREQUE_ACK;	//�ʼ�����
	mail->resend = 0;         							//�ط�����
	mail->outtime = 0;        							//��ʱ��
	mail->sendmode = ASYN_MODE;							//ͬ��
	mail->col = RMail->col;

	//���������� �ڷ�����ɺ�����
	UserData = rt_calloc(1,sizeof(net_filereq_ack_user));
	RT_ASSERT(UserData != RT_NULL);
	mail->user = UserData;
	UserData->result.result = result;
	
	//�����ʼ�
	net_msg_send_mail(mail);

	//�ͷ���Դ
	RT_ASSERT(mail != RT_NULL);
	rt_free(mail);
}

/*
����:�ļ���Ӧ��
*/
void msg_mail_fileack(net_recvmsg_p RMail,rt_uint8_t Fresult)
{
	net_msgmail_p mail = RT_NULL;
	net_filedata_ack_user *FileAck;

	//��ȡ��Դ
	mail = (net_msgmail_p)rt_calloc(1,sizeof(net_msgmail));
	RT_ASSERT(mail != RT_NULL);

	//�����ʼ�
	mail->type = NET_MSGTYPE_FILEDATA_ACK;	//�ʼ�����
	mail->resend = 0;         							//�ط�����
	mail->outtime = 0;        							//��ʱ��
	mail->sendmode = ASYN_MODE;							//ͬ��
	mail->col = RMail->col;

	//���������� �ڷ�����ɺ�����
	FileAck = rt_calloc(1,sizeof(net_filedata_ack_user));
	RT_ASSERT(FileAck != RT_NULL);
	mail->user = FileAck;
	FileAck->fileack.result = Fresult;
	rt_memcpy(FileAck->fileack.order,RMail->data.filedata.pos,4);

	//�����ʼ�
	net_msg_send_mail(mail);

	//�ͷ���Դ
	RT_ASSERT(mail != RT_NULL);
	rt_free(mail);
}



/*
����:�˻����
*/
rt_err_t msg_mail_account_add(rt_int16_t account_pos,rt_uint8_t *name,rt_uint32_t date)
{
	rt_uint8_t result;
	net_msgmail_p mail = RT_NULL;
	net_account_add_user *UserData = RT_NULL;
	
	//��ȡ��Դ
	mail = (net_msgmail_p)rt_calloc(1,sizeof(net_msgmail));
	UserData = rt_calloc(1,sizeof(net_account_add_user));
	RT_ASSERT(mail != RT_NULL);
	RT_ASSERT(UserData != RT_NULL);
	mail->user = UserData;
	
	UserData->result.complete = smg_send_wait_sem_crate();
  RT_ASSERT(UserData != RT_NULL);

	//�����ʼ�
	mail->type = NET_MSGTYPE_ACCOUNTADD;   //�ʼ�����
  mail->resend = MAIL_FAULT_RESEND;									//�ط�����
  mail->outtime = MAIL_FAULT_OUTTIME;              //��ʱ��
  mail->sendmode = SYNC_MODE;       //ͬ��
  mail->col.byte = get_msg_new_order(RT_TRUE);
	//����˽������

	account_pos = net_rev16(account_pos);
	rt_memcpy(UserData->account.pos,&account_pos,2);
	rt_memcpy(UserData->account.name,name,20);
	date = net_rev32(date);
	rt_memcpy(UserData->account.date,&date,4);
	
	//�����ʼ�
  net_msg_send_mail(mail);
  rt_sem_take(UserData->result.complete,RT_WAITING_FOREVER);
  rt_sem_delete(UserData->result.complete);
  rt_dprintf(NETMAILCLASS_DEBUG,("message send result:%d\n",UserData->result.result));
  result = UserData->result.result;
  
  //�ͷ���Դ
  RT_ASSERT(UserData != RT_NULL);
	rt_free(UserData);
	RT_ASSERT(mail != RT_NULL);
	rt_free(mail);

  return (result == 0)?RT_EOK:RT_ERROR;
}



/*
����:�˻����Ӧ��
*/
void msg_mail_account_add_ack(net_recvmsg_p RMail,rt_uint8_t result)
{
	net_msgmail_p mail = RT_NULL;
	net_account_ack_user *UserData;

	//��ȡ��Դ
	mail = (net_msgmail_p)rt_calloc(1,sizeof(net_msgmail));
	RT_ASSERT(mail != RT_NULL);

	//�����ʼ�
	mail->type = NET_MSGTYPE_ACCOUNTADD_ACK;	//�ʼ�����
	mail->resend = 0;         							//�ط�����
	mail->outtime = 0;        							//��ʱ��
	mail->sendmode = ASYN_MODE;							//ͬ��
	mail->col = RMail->col;

	//���������� �ڷ�����ɺ�����
	UserData = rt_calloc(1,sizeof(net_account_ack_user));
	RT_ASSERT(UserData != RT_NULL);
	mail->user = UserData;
	UserData->ack.result = result;
	rt_memcpy(UserData->ack.pos,RMail->data.AccountAdd.pos,2);
	//�����ʼ�
	net_msg_send_mail(mail);

	//�ͷ���Դ
	RT_ASSERT(mail != RT_NULL);
	rt_free(mail);
}

/*
����:�˻�ɾ��
*/
rt_err_t msg_mail_account_del(rt_int16_t account_pos,rt_uint32_t date)
{
	rt_uint8_t result;
	net_msgmail_p mail = RT_NULL;
	net_account_del_user *UserData = RT_NULL;

	//��ȡ��Դ
	mail = (net_msgmail_p)rt_calloc(1,sizeof(net_msgmail));
	UserData = rt_calloc(1,sizeof(net_account_del_user));
	RT_ASSERT(mail != RT_NULL);
	RT_ASSERT(UserData != RT_NULL);
	mail->user = UserData;

	UserData->result.complete = smg_send_wait_sem_crate();
	RT_ASSERT(UserData != RT_NULL);

	//�����ʼ�
	mail->type = NET_MSGTYPE_ACCOUNTDEL;   //�ʼ�����
	mail->resend = MAIL_FAULT_RESEND;                 //�ط�����
	mail->outtime = MAIL_FAULT_OUTTIME;              //��ʱ��
	mail->sendmode = SYNC_MODE;       //ͬ��
	mail->col.byte = get_msg_new_order(RT_TRUE);
	//����˽������

	account_pos = net_rev16(account_pos);
	rt_memcpy(UserData->account.pos,&account_pos,2);
	date = net_rev32(date);
	rt_memcpy(UserData->account.date,&date,4);

	//�����ʼ�
	net_msg_send_mail(mail);
	rt_sem_take(UserData->result.complete,RT_WAITING_FOREVER);
	rt_sem_delete(UserData->result.complete);
	rt_dprintf(NETMAILCLASS_DEBUG,("message send result:%d\n",UserData->result.result));
	result = UserData->result.result;

	//�ͷ���Դ
	RT_ASSERT(UserData != RT_NULL);
	rt_free(UserData);
	RT_ASSERT(mail != RT_NULL);
	rt_free(mail);

  return (result == 0)?RT_EOK:RT_ERROR;
}

  
/*
����:�˻�ɾ��Ӧ��
*/
void msg_mail_account_del_ack(net_recvmsg_p RMail,rt_uint8_t result)
{
	net_msgmail_p mail = RT_NULL;
	net_account_ack_user *UserData;

	//��ȡ��Դ
	mail = (net_msgmail_p)rt_calloc(1,sizeof(net_msgmail));
	RT_ASSERT(mail != RT_NULL);

	//�����ʼ�
	mail->type = NET_MSGTYPE_ACCOUNTDEL_ACK;  //�ʼ�����
	mail->resend = 0;                       //�ط�����
	mail->outtime = 0;                      //��ʱ��
	mail->sendmode = ASYN_MODE;             //ͬ��
	mail->col = RMail->col;

	//���������� �ڷ�����ɺ�����
	UserData = rt_calloc(1,sizeof(net_account_del_user));
	RT_ASSERT(UserData != RT_NULL);
	mail->user = UserData;
	UserData->ack.result = result;
	rt_memcpy(UserData->ack.pos,RMail->data.AccountDel.pos,2);
	//�����ʼ�
	net_msg_send_mail(mail);

	//�ͷ���Դ
	RT_ASSERT(mail != RT_NULL);
	rt_free(mail);
}


/*
����:Կ�װ�
*/
rt_err_t msg_mail_keybind(rt_uint16_t key_pos,rt_uint16_t account_pos,rt_uint32_t date)
{
	rt_uint8_t result;
	net_msgmail_p mail = RT_NULL;
	net_keybind_user *UserData = RT_NULL;

	//��ȡ��Դ
	mail = (net_msgmail_p)rt_calloc(1,sizeof(net_msgmail));
	UserData = rt_calloc(1,sizeof(net_phonebind_user));
	RT_ASSERT(mail != RT_NULL);
	RT_ASSERT(UserData != RT_NULL);
	mail->user = UserData;

	UserData->result.complete = smg_send_wait_sem_crate();
	RT_ASSERT(UserData != RT_NULL);

	//�����ʼ�
	mail->type = NET_MSGTYPE_KEYBIND;   //�ʼ�����
	mail->resend = MAIL_FAULT_RESEND;                 //�ط�����
	mail->outtime = MAIL_FAULT_OUTTIME;              //��ʱ��
	mail->sendmode = SYNC_MODE;       //ͬ��
	mail->col.byte = get_msg_new_order(RT_TRUE);
	//����˽������

	account_pos = net_rev16(account_pos);
	rt_memcpy(UserData->data.AccountPos,&account_pos,2);

	key_pos = net_rev16(key_pos);
	rt_memcpy(UserData->data.KeyPos,&key_pos,2);
	
	date = net_rev32(date);
	rt_memcpy(UserData->data.Date,&date,4);

	//�����ʼ�
	net_msg_send_mail(mail);
	rt_sem_take(UserData->result.complete,RT_WAITING_FOREVER);
	rt_sem_delete(UserData->result.complete);
	rt_dprintf(NETMAILCLASS_DEBUG,("message send result:%d\n",UserData->result.result));
	result = UserData->result.result;

	//�ͷ���Դ
	RT_ASSERT(UserData != RT_NULL);
	rt_free(UserData);
	RT_ASSERT(mail != RT_NULL);
	rt_free(mail);

  return (result == 0)?RT_EOK:RT_ERROR;
}

/*
����:Կ�װ�Ӧ��
*/
void msg_mail_keybind_ack(net_recvmsg_p RMail,rt_uint8_t result)
{
	net_msgmail_p mail = RT_NULL;
	net_keybind_ack_user *UserData;

	//��ȡ��Դ
	mail = (net_msgmail_p)rt_calloc(1,sizeof(net_msgmail));
	RT_ASSERT(mail != RT_NULL);

	//�����ʼ�
	mail->type = NET_MSGTYPE_KEYBIND_ACK;  //�ʼ�����
	mail->resend = 0;                       //�ط�����
	mail->outtime = 0;                      //��ʱ��
	mail->sendmode = ASYN_MODE;             //ͬ��
	mail->col = RMail->col;

	//���������� �ڷ�����ɺ�����
	UserData = rt_calloc(1,sizeof(net_keybind_ack_user));
	RT_ASSERT(UserData != RT_NULL);
	mail->user = UserData;
	UserData->data.result = result;
	rt_memcpy(UserData->data.pos,RMail->data.KeyBind.AccountPos,2);
	//�����ʼ�
	net_msg_send_mail(mail);

	//�ͷ���Դ
	RT_ASSERT(mail != RT_NULL);
	rt_free(mail);
}

/*
����:�绰��
*/
rt_err_t msg_mail_phonebind(rt_uint16_t phone_pos,rt_uint16_t account_pos,rt_uint32_t date)
{
	rt_uint8_t result;
	net_msgmail_p mail = RT_NULL;
	net_phonebind_user *UserData = RT_NULL;

	//��ȡ��Դ
	mail = (net_msgmail_p)rt_calloc(1,sizeof(net_msgmail));
	UserData = rt_calloc(1,sizeof(net_phonebind_user));
	RT_ASSERT(mail != RT_NULL);
	RT_ASSERT(UserData != RT_NULL);
	mail->user = UserData;

	UserData->result.complete = smg_send_wait_sem_crate();
	RT_ASSERT(UserData != RT_NULL);

	//�����ʼ�
	mail->type = NET_MSGTYPE_PHONEBIND;   //�ʼ�����
	mail->resend = MAIL_FAULT_RESEND;                 //�ط�����
	mail->outtime = MAIL_FAULT_OUTTIME;              //��ʱ��
	mail->sendmode = SYNC_MODE;       //ͬ��
	mail->col.byte = get_msg_new_order(RT_TRUE);
	//����˽������

	account_pos = net_rev16(account_pos);
	rt_memcpy(UserData->data.AccountPos,&account_pos,2);

	phone_pos = net_rev16(phone_pos);
	rt_memcpy(UserData->data.PhonePos,&phone_pos,2);

	date = net_rev32(date);
	rt_memcpy(UserData->data.Date,&date,4);

	//�����ʼ�
	net_msg_send_mail(mail);
	rt_sem_take(UserData->result.complete,RT_WAITING_FOREVER);
	rt_sem_delete(UserData->result.complete);
	rt_dprintf(NETMAILCLASS_DEBUG,("message send result:%d\n",UserData->result.result));
	result = UserData->result.result;

	//�ͷ���Դ
	RT_ASSERT(UserData != RT_NULL);
	rt_free(UserData);
	RT_ASSERT(mail != RT_NULL);
	rt_free(mail);

  return (result == 0)?RT_EOK:RT_ERROR;
}

/*
����:�绰��Ӧ��
*/
void msg_mail_phonebind_ack(net_recvmsg_p RMail,rt_uint8_t result)
{
	net_msgmail_p mail = RT_NULL;
	net_keyphone_ack_user *UserData;

	//��ȡ��Դ
	mail = (net_msgmail_p)rt_calloc(1,sizeof(net_msgmail));
	RT_ASSERT(mail != RT_NULL);

	//�����ʼ�
	mail->type = NET_MSGTYPE_PHONEBIND_ACK;  //�ʼ�����
	mail->resend = 0;                       //�ط�����
	mail->outtime = 0;                      //��ʱ��
	mail->sendmode = ASYN_MODE;             //ͬ��
	mail->col = RMail->col;

	//���������� �ڷ�����ɺ�����
	UserData = rt_calloc(1,sizeof(net_keybind_ack_user));
	RT_ASSERT(UserData != RT_NULL);
	mail->user = UserData;
	UserData->data.result = result;
	rt_memcpy(UserData->data.pos,RMail->data.PhoneBind.AccountPos,2);
	//�����ʼ�
	net_msg_send_mail(mail);

	//�ͷ���Դ
	RT_ASSERT(mail != RT_NULL);
	rt_free(mail);
}

void msg_mail_datasync_ack(net_recvmsg_p RMail,rt_uint8_t result)
{
	net_msgmail_p mail = RT_NULL;
	net_datasync_ack_user *UserData;

	//��ȡ��Դ
	mail = (net_msgmail_p)rt_calloc(1,sizeof(net_msgmail));
	RT_ASSERT(mail != RT_NULL);

	//�����ʼ�
	mail->type = NET_MSGTYPE_DATA_SYNC_ACK;  //�ʼ�����
	mail->resend = 0;                       //�ط�����
	mail->outtime = 0;                      //��ʱ��
	mail->sendmode = ASYN_MODE;             //ͬ��
	mail->col = RMail->col;

	//���������� �ڷ�����ɺ�����
	UserData = rt_calloc(1,sizeof(*UserData));
	RT_ASSERT(UserData != RT_NULL);
	mail->user = UserData;
	UserData->data.result = result;
	//�����ʼ�
	net_msg_send_mail(mail);

	//�ͷ���Դ
	RT_ASSERT(mail != RT_NULL);
	rt_free(mail);
}

//�˻�ӳ�������
rt_err_t msg_mail_accmapadd(rt_uint8_t *MapByte,rt_size_t ByteLength,rt_uint32_t date)
{
	rt_uint8_t            result;
	net_msgmail_p         mail = RT_NULL;
	net_accmapadd_user    *UserData = RT_NULL;

	//��ȡ��Դ
	mail = (net_msgmail_p)rt_calloc(1,sizeof(net_msgmail));
	UserData = rt_calloc(1,sizeof(*UserData));
	RT_ASSERT(mail != RT_NULL);
	RT_ASSERT(UserData != RT_NULL);
	mail->user = UserData;

	UserData->result.complete = smg_send_wait_sem_crate();
	RT_ASSERT(UserData != RT_NULL);

	//�����ʼ�
	mail->type = NET_MSGTYPE_ACCMAPADD;   						//�ʼ�����
	mail->resend = MAIL_FAULT_RESEND;                 //�ط�����
	mail->outtime = MAIL_FAULT_OUTTIME;              //��ʱ��
	mail->sendmode = SYNC_MODE;      								 //ͬ��
	mail->col.byte = get_msg_new_order(RT_TRUE);
	//����˽������

	UserData->DataLen = ByteLength;
	UserData->data.MapByte = MapByte;
	date = net_rev32(date);
	rt_memcpy(UserData->data.Date,&date,4);

	//�����ʼ�
	net_msg_send_mail(mail);
	rt_sem_take(UserData->result.complete,RT_WAITING_FOREVER);
	rt_sem_delete(UserData->result.complete);
	rt_dprintf(NETMAILCLASS_DEBUG,("message send result:%d\n",UserData->result.result));
	result = UserData->result.result;

	//�ͷ���Դ
	RT_ASSERT(UserData != RT_NULL);
	rt_free(UserData);
	RT_ASSERT(mail != RT_NULL);
	rt_free(mail);

  return (result == 0)?RT_EOK:RT_ERROR;
}

//�˻�����У��
rt_err_t msg_mail_accdatcks(rt_uint16_t  ID, rt_uint32_t date)
{
	rt_uint8_t            result;
	net_msgmail_p         mail = RT_NULL;
	net_accdatcks_user    *UserData = RT_NULL;

	//��ȡ��Դ
	mail = (net_msgmail_p)rt_calloc(1,sizeof(net_msgmail));
	UserData = rt_calloc(1,sizeof(*UserData));
	RT_ASSERT(mail != RT_NULL);
	RT_ASSERT(UserData != RT_NULL);
	mail->user = UserData;

	UserData->result.complete = smg_send_wait_sem_crate();
	RT_ASSERT(UserData != RT_NULL);

	//�����ʼ�
	mail->type = NET_MSGTYPE_ACCDATCKS;   						//�ʼ�����
	mail->resend = MAIL_FAULT_RESEND;                 //�ط�����
	mail->outtime = MAIL_FAULT_OUTTIME;              //��ʱ��
	mail->sendmode = SYNC_MODE;      								 //ͬ��
	mail->col.byte = get_msg_new_order(RT_TRUE);
	//����˽������

  ID = net_rev16(ID);
	rt_memcpy(UserData->data.ID,&ID,2);

	date = net_rev32(date);
	rt_memcpy(UserData->data.Date,&date,4);

	//�����ʼ�
	net_msg_send_mail(mail);
	rt_sem_take(UserData->result.complete,RT_WAITING_FOREVER);
	rt_sem_delete(UserData->result.complete);
	rt_dprintf(NETMAILCLASS_DEBUG,("message send result:%d\n",UserData->result.result));
	result = UserData->result.result;

	//�ͷ���Դ
	RT_ASSERT(UserData != RT_NULL);
	rt_free(UserData);
	RT_ASSERT(mail != RT_NULL);
	rt_free(mail);

  return (result == 0)?RT_EOK:RT_ERROR;
}


//Կ��ӳ�������
rt_err_t msg_mail_keymapadd(rt_uint8_t *MapByte,rt_size_t ByteLength,rt_uint32_t date)
{
	rt_uint8_t            result;
	net_msgmail_p         mail = RT_NULL;
	net_keymapadd_user    *UserData = RT_NULL;

	//��ȡ��Դ
	mail = (net_msgmail_p)rt_calloc(1,sizeof(net_msgmail));
	UserData = rt_calloc(1,sizeof(*UserData));
	RT_ASSERT(mail != RT_NULL);
	RT_ASSERT(UserData != RT_NULL);
	
	mail->user = UserData;

	//����ͬ���ź���
	UserData->result.complete = smg_send_wait_sem_crate();
	RT_ASSERT(UserData != RT_NULL);

	//�����ʼ�
	mail->type = NET_MSGTYPE_KEYMAPADD;   						//�ʼ�����
	mail->resend = MAIL_FAULT_RESEND;                 //�ط�����
	mail->outtime = MAIL_FAULT_OUTTIME;              //��ʱ��
	mail->sendmode = SYNC_MODE;      								 //ͬ��
	mail->col.byte = get_msg_new_order(RT_TRUE);
	
	//����˽������
	UserData->DataLen = ByteLength;
	UserData->data.MapByte = MapByte;
	date = net_rev32(date);
	rt_memcpy(UserData->data.Date,&date,4);

	//�����ʼ�
	net_msg_send_mail(mail);

	//�ȴ����ͽ��
	rt_sem_take(UserData->result.complete,RT_WAITING_FOREVER);
	rt_sem_delete(UserData->result.complete);
	rt_dprintf(NETMAILCLASS_DEBUG,("message send result:%d\n",UserData->result.result));
	result = UserData->result.result;

	//�ͷ���Դ
	RT_ASSERT(UserData != RT_NULL);
	rt_free(UserData);
	RT_ASSERT(mail != RT_NULL);
	rt_free(mail);

  return (result == 0)?RT_EOK:RT_ERROR;
}

//Կ������У��
rt_err_t msg_mail_keydatcks(rt_uint16_t  ID, rt_uint32_t date)
{
	rt_uint8_t            result;
	net_msgmail_p         mail = RT_NULL;
	net_keydatcks_user    *UserData = RT_NULL;

	//��ȡ��Դ
	mail = (net_msgmail_p)rt_calloc(1,sizeof(net_msgmail));
	UserData = rt_calloc(1,sizeof(*UserData));
	RT_ASSERT(mail != RT_NULL);
	RT_ASSERT(UserData != RT_NULL);
	mail->user = UserData;

	UserData->result.complete = smg_send_wait_sem_crate();
	RT_ASSERT(UserData != RT_NULL);

	//�����ʼ�
	mail->type = NET_MSGTYPE_KEYDATCKS;   						//�ʼ�����
	mail->resend = MAIL_FAULT_RESEND;                 //�ط�����
	mail->outtime = MAIL_FAULT_OUTTIME;              //��ʱ��
	mail->sendmode = SYNC_MODE;      								 //ͬ��
	mail->col.byte = get_msg_new_order(RT_TRUE);
	//����˽������

  ID = net_rev16(ID);
	rt_memcpy(UserData->data.ID,&ID,2);

	date = net_rev32(date);
	rt_memcpy(UserData->data.Date,&date,4);

	//�����ʼ�
	net_msg_send_mail(mail);
	rt_sem_take(UserData->result.complete,RT_WAITING_FOREVER);
	rt_sem_delete(UserData->result.complete);
	rt_dprintf(NETMAILCLASS_DEBUG,("message send result:%d\n",UserData->result.result));
	result = UserData->result.result;

	//�ͷ���Դ
	RT_ASSERT(UserData != RT_NULL);
	rt_free(UserData);
	RT_ASSERT(mail != RT_NULL);
	rt_free(mail);

  return (result == 0)?RT_EOK:RT_ERROR;
}

//�ֻ�ӳ�������
rt_err_t msg_mail_phmapadd(rt_uint8_t *MapByte,rt_size_t ByteLength,rt_uint32_t date)
{
	rt_uint8_t            result;
	net_msgmail_p         mail = RT_NULL;
	net_phmapadd_user    *UserData = RT_NULL;

	//��ȡ��Դ
	mail = (net_msgmail_p)rt_calloc(1,sizeof(net_msgmail));
	UserData = rt_calloc(1,sizeof(*UserData));
	RT_ASSERT(mail != RT_NULL);
	RT_ASSERT(UserData != RT_NULL);
	
	mail->user = UserData;

	//����ͬ���ź���
	UserData->result.complete = smg_send_wait_sem_crate();
	RT_ASSERT(UserData != RT_NULL);

	//�����ʼ�
	mail->type = NET_MSGTYPE_PHMAPADD;   						//�ʼ�����
	mail->resend = MAIL_FAULT_RESEND;                 //�ط�����
	mail->outtime = MAIL_FAULT_OUTTIME;              //��ʱ��
	mail->sendmode = SYNC_MODE;      								 //ͬ��
	mail->col.byte = get_msg_new_order(RT_TRUE);
	
	//����˽������
	UserData->DataLen = ByteLength;
	UserData->data.MapByte = MapByte;
	date = net_rev32(date);
	rt_memcpy(UserData->data.Date,&date,4);

	//�����ʼ�
	net_msg_send_mail(mail);

	//�ȴ����ͽ��
	rt_sem_take(UserData->result.complete,RT_WAITING_FOREVER);
	rt_sem_delete(UserData->result.complete);
	rt_dprintf(NETMAILCLASS_DEBUG,("message send result:%d\n",UserData->result.result));
	result = UserData->result.result;

	//�ͷ���Դ
	RT_ASSERT(UserData != RT_NULL);
	rt_free(UserData);
	RT_ASSERT(mail != RT_NULL);
	rt_free(mail);

  return (result == 0)?RT_EOK:RT_ERROR;
}

//�ֻ�����У��
rt_err_t msg_mail_phdatcks(rt_uint16_t  ID, rt_uint32_t date)
{
	rt_uint8_t            result;
	net_msgmail_p         mail = RT_NULL;
	net_phdatcks_user    *UserData = RT_NULL;

	//��ȡ��Դ
	mail = (net_msgmail_p)rt_calloc(1,sizeof(net_msgmail));
	UserData = rt_calloc(1,sizeof(*UserData));
	RT_ASSERT(mail != RT_NULL);
	RT_ASSERT(UserData != RT_NULL);
	mail->user = UserData;

	UserData->result.complete = smg_send_wait_sem_crate();
	RT_ASSERT(UserData != RT_NULL);

	//�����ʼ�
	mail->type = NET_MSGTYPE_PHDATCKS;   						//�ʼ�����
	mail->resend = MAIL_FAULT_RESEND;                 //�ط�����
	mail->outtime = MAIL_FAULT_OUTTIME;              //��ʱ��
	mail->sendmode = SYNC_MODE;      								 //ͬ��
	mail->col.byte = get_msg_new_order(RT_TRUE);
	//����˽������

  ID = net_rev16(ID);
	rt_memcpy(UserData->data.ID,&ID,2);

	date = net_rev32(date);
	rt_memcpy(UserData->data.Date,&date,4);

	//�����ʼ�
	net_msg_send_mail(mail);
	rt_sem_take(UserData->result.complete,RT_WAITING_FOREVER);
	rt_sem_delete(UserData->result.complete);
	rt_dprintf(NETMAILCLASS_DEBUG,("message send result:%d\n",UserData->result.result));
	result = UserData->result.result;

	//�ͷ���Դ
	RT_ASSERT(UserData != RT_NULL);
	rt_free(UserData);
	RT_ASSERT(mail != RT_NULL);
	rt_free(mail);

  return (result == 0)?RT_EOK:RT_ERROR;
}

//��¼ӳ�������
rt_err_t msg_mail_recmapadd(rt_uint8_t *MapByte,rt_size_t ByteLength,rt_uint32_t date)
{
	rt_uint8_t            result;
	net_msgmail_p         mail = RT_NULL;
	net_recmapadd_user    *UserData = RT_NULL;

	//��ȡ��Դ
	mail = (net_msgmail_p)rt_calloc(1,sizeof(net_msgmail));
	UserData = rt_calloc(1,sizeof(*UserData));
	RT_ASSERT(mail != RT_NULL);
	RT_ASSERT(UserData != RT_NULL);
	
	mail->user = UserData;

	//����ͬ���ź���
	UserData->result.complete = smg_send_wait_sem_crate();
	RT_ASSERT(UserData != RT_NULL);

	//�����ʼ�
	mail->type = NET_MSGTYPE_RECMAPADD;   						//�ʼ�����
	mail->resend = MAIL_FAULT_RESEND;                 //�ط�����
	mail->outtime = MAIL_FAULT_OUTTIME;              //��ʱ��
	mail->sendmode = SYNC_MODE;      								 //ͬ��
	mail->col.byte = get_msg_new_order(RT_TRUE);
	
	//����˽������
	UserData->DataLen = ByteLength;
	UserData->data.MapByte = MapByte;
	date = net_rev32(date);
	rt_memcpy(UserData->data.Date,&date,4);

	//�����ʼ�
	net_msg_send_mail(mail);

	//�ȴ����ͽ��
	rt_sem_take(UserData->result.complete,RT_WAITING_FOREVER);
	rt_sem_delete(UserData->result.complete);
	rt_dprintf(NETMAILCLASS_DEBUG,("message send result:%d\n",UserData->result.result));
	result = UserData->result.result;

	//�ͷ���Դ
	RT_ASSERT(UserData != RT_NULL);
	rt_free(UserData);
	RT_ASSERT(mail != RT_NULL);
	rt_free(mail);

  return (result == 0)?RT_EOK:RT_ERROR;
}

//��¼����У��
rt_err_t msg_mail_recdatcks(rt_uint16_t  ID, rt_uint32_t date)
{
	rt_uint8_t            result;
	net_msgmail_p         mail = RT_NULL;
	net_recdatcks_user    *UserData = RT_NULL;

	//��ȡ��Դ
	mail = (net_msgmail_p)rt_calloc(1,sizeof(net_msgmail));
	UserData = rt_calloc(1,sizeof(*UserData));
	RT_ASSERT(mail != RT_NULL);
	RT_ASSERT(UserData != RT_NULL);
	mail->user = UserData;

	UserData->result.complete = smg_send_wait_sem_crate();
	RT_ASSERT(UserData != RT_NULL);

	//�����ʼ�
	mail->type = NET_MSGTYPE_RECDATCKS;   						//�ʼ�����
	mail->resend = MAIL_FAULT_RESEND;                 //�ط�����
	mail->outtime = MAIL_FAULT_OUTTIME;              //��ʱ��
	mail->sendmode = SYNC_MODE;      								 //ͬ��
	mail->col.byte = get_msg_new_order(RT_TRUE);
	//����˽������

  ID = net_rev16(ID);
	rt_memcpy(UserData->data.ID,&ID,2);

	date = net_rev32(date);
	rt_memcpy(UserData->data.Date,&date,4);

	//�����ʼ�
	net_msg_send_mail(mail);
	rt_sem_take(UserData->result.complete,RT_WAITING_FOREVER);
	rt_sem_delete(UserData->result.complete);
	rt_dprintf(NETMAILCLASS_DEBUG,("message send result:%d\n",UserData->result.result));
	result = UserData->result.result;

	//�ͷ���Դ
	RT_ASSERT(UserData != RT_NULL);
	rt_free(UserData);
	RT_ASSERT(mail != RT_NULL);
	rt_free(mail);

  return (result == 0)?RT_EOK:RT_ERROR;
}



/*********************************************************************
 *process receive net messge 
 *
 *********************************************************************
 */
 
/*
����:���մ����ָ�뺯��
����: 0: �ɹ� 1: ʧ��
*/
rt_uint8_t net_message_recv_process(net_recvmsg_p Mail,void *UserData)
{
	rt_uint8_t result = 0; 
	rt_err_t   ProcessResult;

	switch(Mail->cmd)
	{
	  case NET_MSGTYPE_FILEREQUEST:
	  {
	  	//�ļ�����
	  	#ifdef USEING_FILE_API
			result = net_recv_filerq_process(Mail);
			#endif
			msg_mail_filereq_ack(Mail,result);
	  	
	    break;
	  }
	  case NET_MSGTYPE_FILEDATA:
	  {
	  	//�ļ�����
	  	rt_size_t PackOrder;

			#ifdef USEING_FILE_API
			result = net_file_packdata_process(Mail);
			net_string_copy_uint32(&PackOrder,Mail->data.filedata.pos);
			#endif
			msg_mail_fileack(Mail,result);
				
			break;
	  }
	  case NET_MSGTYPE_FILEDATA_ACK:
	  {
	  	net_filedata_user *data;

	  	data = (net_filedata_user*)net_get_wnd_user(Mail);
	  	if(data != RT_NULL)
	  	{
        data->sendresult = Mail->data.filedata_ack.result;
	  	}
			
			break;
	  }
	 	case NET_MSGTYPE_PHONEADD:
	 	{
	 		//�ֻ����������
	 		#ifdef USEING_PHONE_API
	 		ProcessResult = net_phone_add_process(Mail);
	 		result = (ProcessResult == RT_EOK)?1:0;
	 		#endif
	 		msg_mail_phoneadd_ack(Mail,result);
			break;
	 	}
	 	case NET_MSGTYPE_PHONEDELETE:
	 	{
	 		//�ֻ�������ɾ��
	 		#ifdef USEING_PHONE_API
	 		ProcessResult = net_phone_del_process(Mail);
			result = (ProcessResult == RT_EOK)?1:0;
			#endif
			msg_mail_phonedel_ack(Mail,result);
			break;
	 	}
	 	case NET_MSGTYPE_KEYADD:
	 	{
	 		//Կ�����
	 		#ifdef USEING_KEY_API
	 		//result = keylib_mutex_op(RT_TRUE,RT_WAITING_NO);
	 		result = RT_EOK;
			if(result == RT_EOK)
			{
				ProcessResult = net_key_add_process(Mail);

	 			result = (ProcessResult == RT_EOK)?1:0;
	 			//keylib_mutex_op(RT_FALSE,RT_WAITING_NO);
			}
			else
			{
				result = 0;
			}
	 		#endif
	 		msg_mail_keyadd_ack(Mail,result);
			break;
	 	}
	 	case NET_MSGTYPE_KEYADD_ACK:
	 	{
	 		//���Կ��Ӧ��
	 		result = (Mail->data.KeyAddAck.keyAck.result == 1)?0:1;
			break;
	 	}
	 	case NET_MSGTYPE_KEYDELETE:
	 	{
	 		//Կ��ɾ��
	 		#ifdef USEING_KEY_API
	 		ProcessResult = net_key_del_process(Mail);
	 		result = (ProcessResult == RT_EOK)?1:0;
	 		#endif
	 		msg_mail_keydel_ack(Mail,result);
			break;
	 	}
	 	case NET_MSGTYPE_KEYDEL_ACK:
	 	{
			break;
	 	}
	 	case NET_MSGTYPE_ALARMARG:
	 	{
	 		//������������
	 		#ifdef USEING_SYSCONFIG_API
	 		ProcessResult = net_modify_alarm_arg(Mail);
	 		result = (ProcessResult == RT_EOK)?1:0;
	 		#endif
	 		msg_mail_resultack(Mail,result);
			break;
	 	}
	 	case NET_MSGTYPE_ALARMARG_ACK:
	 	{
	 		//��������Ӧ��
			break;
	 	}
	 	case NET_MSGTYPE_MOTOR:
	 	{
	 		//���
      #ifdef USEING_MOTOR_API
	 		ProcessResult = net_motor_Control(Mail);
	 		result = (ProcessResult == RT_EOK)?1:0;
      #endif
	 		msg_mail_resultack(Mail,result);
			break;
	 	}
	 	case NET_MSGTYPE_TIME_ACK:
	 	{
	 		//��ʱӦ��
      #ifdef USEING_SYS_TIME_API
	 		ProcessResult = net_set_system_time(Mail);
      #endif
	 		result = (ProcessResult == RT_EOK)?1:0;
			break;
	 	}
	 	case NET_MSGTYPE_CAMERA:
	 	{
	 		//Զ������
      #ifdef USEING_CAMERA_API
	 		ProcessResult = net_photograph(Mail);
	 		result = (ProcessResult == RT_EOK)?1:0;
      #endif
	 		msg_mail_resultack(Mail,result);
			break;
	 	}
		case NET_MSGTYPE_UPDATE:
	 	{
	 		//ϵͳ����
      #ifdef USEING_SYS_UPDATE
	 		ProcessResult = application_update("/app.bin");
	 		result = (ProcessResult == RT_EOK)?1:0;
      #endif
	 		msg_mail_resultack(Mail,result);
	 		rt_kprintf("NET_MSGTYPE_UPDATE\n");
			break;
	 	}
	 	case NET_MSGTYPE_SETK0:
	 	{
	 		//����k0
      #ifdef USEING_SYSCONFIG_API
	 		ProcessResult = net_set_key0(Mail);
	 		result = (ProcessResult == RT_EOK)?1:0;
      #endif
	 		msg_mail_resultack(Mail,result);
			break;
	 	}
	 	case NET_MSGTYPE_TERMINAL:
	 	{
	 		
			break;
	 	}
	 	case NET_MSGTYPE_DOMAIN:
	 	{
      //����ulr
      #ifdef USEING_SYSCONFIG_API
	 		ProcessResult = net_set_domain(Mail);
	 		result = (ProcessResult == RT_EOK)?1:0;
      #endif
	 		msg_mail_resultack(Mail,result);
			break;
	 	}
	 	case NET_MSGTYPE_DOORMODE:
	 	{
      //����ulr
      #ifdef USEING_SYSCONFIG_API
	 		ProcessResult = net_set_domain(Mail);
	 		result = (ProcessResult == RT_EOK)?1:0;
      #endif
	 		msg_mail_resultack(Mail,result);
			break;
	 	}
	 	case NET_MSGTYPE_ACCOUNTADD:
		{
			//�˻����
			#ifdef USEING_ACCOUNT_API
      ProcessResult  = net_account_add_process(Mail);
			result = (ProcessResult == RT_EOK)?1:0;
			#endif
 			msg_mail_account_add_ack(Mail,result);
			break;
		}	
		case NET_MSGTYPE_ACCOUNTADD_ACK:
		{
			//�˻����Ӧ��
   
			break;
		}	
		case NET_MSGTYPE_ACCOUNTDEL:
		{
			//�˻�ɾ��
			#ifdef USEING_ACCOUNT_API
			ProcessResult  = net_account_del_process(Mail);
			result = (ProcessResult == RT_EOK)?1:0;
			#endif
 			msg_mail_account_del_ack(Mail,result);
			break;
		}	
		case NET_MSGTYPE_ACCOUNTDEL_ACK:
		{
			//�˻�ɾ��Ӧ��
   
			break;
		}	
		case NET_MSGTYPE_KEYBIND:
		{
			//Կ�װ�
			#ifdef USEING_BIND_API
			ProcessResult  = net_bind_key_process(Mail);
			result = (ProcessResult == RT_EOK)?1:0;
			#endif
   		msg_mail_keybind_ack(Mail,result);
			break;
		}	
		case NET_MSGTYPE_KEYBIND_ACK:
		{
			//Կ�װ�Ӧ��
    
			break;
		}	
		case NET_MSGTYPE_PHONEBIND:
		{
			//�ֻ���

			#ifdef USEING_BIND_API
			ProcessResult  = net_bind_phone_process(Mail);
			result = (ProcessResult == RT_EOK)?1:0;
			#endif

   		msg_mail_phonebind_ack(Mail,result);
			break;
		}	
		case NET_MSGTYPE_PHONEBIND_ACK:
		{
			//�绰��Ӧ��
    
			break;
		}	
		case NET_MSGTYPE_ACCMAPADD_ACK:
		{
			//�˻�ӳ�������Ӧ��
			#ifdef USEING_NEW_DATA_SYNC
			net_accmapadd_result(Mail);
			#endif
			break;
		}
		case NET_MSGTYPE_ACCDATCKS_ACK:
		{
			//�˻�����У��Ӧ��
			#ifdef USEING_NEW_DATA_SYNC
			net_accdatcks_result(Mail);
			#endif
			break;
		}
		case NET_MSGTYPE_KEYMAPADD_ACK:
		{
			//Կ��ӳ�������
			#ifdef USEING_NEW_DATA_SYNC
			net_keymapadd_result(Mail);
			#endif
			break;
		}
		case NET_MSGTYPE_KEYDATCKS_ACK:
		{
			//Կ������У��Ӧ��
			#ifdef USEING_NEW_DATA_SYNC
			net_keydatcks_result(Mail);
			#endif
			break;
		}
		case NET_MSGTYPE_PHMAPADD_ACK:
		{
			//�ֻ�ӳ�������Ӧ��
			#ifdef USEING_NEW_DATA_SYNC
			net_phmapadd_result(Mail);
			#endif
			break;
		}
		case NET_MSGTYPE_PHDATCKS_ACK:
		{
			//�ֻ�����У��Ӧ��
			#ifdef USEING_NEW_DATA_SYNC
			net_phdatcks_result(Mail);
			#endif
			break;
		}
		case NET_MSGTYPE_RECMAPADD_ACK:
		{
			//��¼ӳ�������Ӧ��
			#ifdef USEING_NEW_DATA_SYNC
			net_recmapadd_result(Mail);
			#endif
			break;
		}
		case NET_MSGTYPE_RECDATCKS_ACK:
		{
			//��¼����У��Ӧ��
			#ifdef USEING_NEW_DATA_SYNC
			net_recdatcks_result(Mail);
			#endif
			break;
		}
		case NET_MSGTYPE_DATA_SYNC:
		{
			//����ͬ��


			ProcessResult  = net_data_sync(Mail);
			result = (ProcessResult == RT_EOK)?1:0;

   		msg_mail_datasync_ack(Mail,result);

			break;
		}
	  default:
	  {
	    break;
	  }
	}

	return result;
}
//#define FILE_UPLOAD_TEST
#ifdef FILE_UPLOAD_TEST
void fileSendT(void)
{
	rt_thread_t id;
	
	id = rt_thread_find("Upload");
	if(id == RT_NULL)
	{
		mq();
		rt_thread_delay(1000);
		send_file("/1.jpg");
	}
}
#endif

static void net_msg_thread_process(void)
{
	#ifdef USEING_FILE_API
	net_file_timer_process();
	#endif
	#ifdef FILE_UPLOAD_TEST
	fileSendT();
	#endif
}


void Net_Param_Init(void)
{
	rt_uint8_t *data;
	rt_uint8_t i;
	
	rt_kprintf("Net param set:\n");
	data = rt_calloc(1,9);
	RT_ASSERT(data != RT_NULL);
	
	device_config_device_id_operate(data,0);
	rt_kprintf("Device ID  >>>");
	for(i = 0 ; i < 8;i++)
	{
		rt_kprintf("%x",data[i]);
	}
	net_config_parameter_set(1,data);
	rt_memset(data,0,9);
	device_config_key0_operate(data,0);
	rt_kprintf("\n");
	
	rt_kprintf("Device Key0>>>");
	for(i = 0 ; i < 8;i++)
	{
		rt_kprintf("%x",data[i]);
	}
	rt_kprintf("\n");
	net_config_parameter_set(2,data);
}

int net_protocol_init_set(void)
{
	//���ý��մ���
	Net_Set_MsgRecv_Callback(net_message_recv_process);
	//�����̴߳���
	Net_NetMsg_thread_callback(net_msg_thread_process);

	//���� ID  key0
	Net_thread_init_callback(Net_Param_Init);
	return 0;
}
INIT_APP_EXPORT(net_protocol_init_set);





















#ifdef RT_USING_FINSH
#include <finsh.h>

FINSH_FUNCTION_EXPORT_ALIAS(msg_mail_alarm,packet_alarm,"(Alarm,Lock,Time)");
FINSH_FUNCTION_EXPORT_ALIAS(msg_mail_fault,packet_fault,"(Type,Time)");
FINSH_FUNCTION_EXPORT_ALIAS(msg_mail_battery,packet_bat,"(Statu,Capacity,Time)");
FINSH_FUNCTION_EXPORT_ALIAS(msg_mail_opendoor,packet_opendoor,"(Type,KeyCode,Time)");
FINSH_FUNCTION_EXPORT_ALIAS(msg_mail_adjust_time,packet_adjtime,"(void)");
FINSH_FUNCTION_EXPORT_ALIAS(msg_mail_alarmarg,packet_alarmarg,"(Type,arg)");
FINSH_FUNCTION_EXPORT_ALIAS(msg_mail_nullack,packet_nullack,"(MSGType)");
FINSH_FUNCTION_EXPORT_ALIAS(msg_mail_phoneadd_ack,packet_phoneaddack,"(pos result)");
FINSH_FUNCTION_EXPORT_ALIAS(msg_mail_phonedel_ack,packet_phonedelack,"(pos result)");
FINSH_FUNCTION_EXPORT_ALIAS(msg_mail_account_add,packet_accountadd,"(pos,name,date)");





void TestAddKey(void)
{
	net_keyadd_user *KeyData;
	rt_uint16_t col;
	
	KeyData = rt_calloc(1,sizeof(net_keyadd_user));
	RT_ASSERT(KeyData != RT_NULL);

	col = 0;
	col = net_rev16(col);
	rt_memcpy(KeyData->data.col,&col,2);
	KeyData->data.type = 0;
	rt_memcpy(KeyData->data.createt,"\x12\x34\x56\x78",4);
	KeyData->data.accredit = 1;
	rt_memcpy(KeyData->data.start_t,"\x00\x09\x00\xa",4);
	rt_memcpy(KeyData->data.stop_t, "\x00\x0a\x00\xa",4);
	KeyData->DataLen = 498;
	KeyData->data.data = rt_calloc(1,KeyData->DataLen);
	RT_ASSERT(KeyData->data.data != RT_NULL);
	/*{
		rt_uint16_t i;

		for(i = 0 ;i < KeyData->DataLen; i++)
		{
			*(KeyData->data.data+i) = '';
		}
	}*/

  msg_mail_keyadd(KeyData);
  
  RT_ASSERT(KeyData->data.data != RT_NULL);
	rt_free(KeyData->data.data);
	RT_ASSERT(KeyData != RT_NULL);
  rt_free(KeyData);
}
FINSH_FUNCTION_EXPORT(TestAddKey,"test key add message");

void filelseek(void)
{
	int id;

	id = open("/lseek.txt",O_CREAT|O_RDWR,0x777);
	if(id < 0)
	{
    return ;
	}
  rt_kprintf("%d",lseek(id,50,DFS_SEEK_SET));
  write(id,"123456",6);
	close(id);
}
FINSH_FUNCTION_EXPORT(filelseek,"lseek funtion test");

void msg_test(rt_uint8_t cmd)
{
	switch(cmd)
	{
		case 0:
		{
			net_keyadd_user *KeyData;

			KeyData = rt_calloc(1,sizeof(net_keyadd_user));

			net_uint16_copy_string(KeyData->data.col,11);
			net_uint32_copy_string(KeyData->data.createt,net_get_date());
			net_uint32_copy_string(KeyData->data.start_t,net_get_date());
			net_uint32_copy_string(KeyData->data.stop_t,net_get_date()+1*60);
			KeyData->data.type = 2;
			KeyData->DataLen = 6;
			KeyData->data.data = rt_calloc(1,KeyData->DataLen);
			KeyData->data.accredit = 1;
			
			rt_memcpy(KeyData->data.data,"456789",6);
			msg_mail_keyadd(KeyData);
			rt_free(KeyData);
			rt_free(KeyData->data.data);
			break;
		}
		case 1:
		{
			msg_mail_account_add(11,"wzw_test",net_get_date());
			break;
		}
		case 2:
		{
			msg_mail_keybind(11,11,net_get_date());
			break;
		}
		case 3:
		{
			msg_mail_opendoor(2,1,11,net_get_date());
			break;
		}
		case 4:
		{
			msg_mail_alarm(2,1,net_get_date());
			break;
		}
		default:
		{
			break;
		}
	}
}
FINSH_FUNCTION_EXPORT(msg_test,msg_test(void));

void upload_map(rt_uint8_t type)
{
	void 				*mapdata;
	rt_uint32_t *mapaddr = RT_NULL;
	rt_size_t   mapsize;
	rt_size_t   i;

	switch(type)
	{
		case 0:
		{
			//�˻�
			struct account_valid_map *data = RT_NULL;
			
			mapdata = (void *)rt_calloc(1,sizeof(struct account_valid_map));
			data = mapdata;
			mapaddr = (rt_uint32_t *)data->data;
			mapsize = ACCOUNT_MAP_SIZE*4;
			device_config_av_operate((void *)mapdata,0);
			rt_kprintf("account map size %d Bypte\n",mapsize);
			break;
		}
		case 1:
		{
			//Կ��
			struct key_valid_map *data = RT_NULL;
			
			mapdata = (void *)rt_calloc(1,sizeof(struct key_valid_map));
			data = mapdata;
			mapaddr = (rt_uint32_t *)data->data;
			mapsize = KEY_MAP_SIZE*4;
			device_config_kv_operate((void *)mapdata,0);
			rt_kprintf("key map size %d Bypte\n",mapsize);
			break;
		}
		case 2:
		{
			//�ֻ�
			struct phone_valid_map *data = RT_NULL;
			
			mapdata = (void *)rt_calloc(1,sizeof(struct phone_valid_map));
			data = mapdata;
			mapaddr = (rt_uint32_t *)data->data;
			mapsize = PHONE_MAP_SIZE*4;
			device_config_pv_operate((void *)mapdata,0);
			rt_kprintf("phone map size %d Bypte\n",mapsize);
			break;
		}
		case 3:
		{
			//�¼�
			struct event_valid_map *data = RT_NULL;
			
			mapdata = (void *)rt_calloc(1,sizeof(struct event_valid_map));
			data = mapdata;
			mapaddr = (rt_uint32_t *)data->data;
			mapsize = EVENT_MAP_SIZE*4;
			device_config_ev_operate((void *)mapdata,0);
			rt_kprintf("phone map size %d Bypte\n",mapsize);
			break;
		}
		default:
		{
			break;
		}
	}
	

	for(i = 0 ; i < mapsize/4;i++)
	{
		if(i % 10 == 0)
		{
			rt_kprintf("\n");
		}
		rt_kprintf("%08X",mapaddr[i]);
	}
  rt_kprintf("\n");

	switch(type)
	{
		case 0:
		{
			msg_mail_accmapadd((rt_uint8_t *)mapaddr,mapsize,net_get_date());
			break;
		}
		case 1:
		{
			msg_mail_keymapadd((rt_uint8_t *)mapaddr,mapsize,net_get_date());
			break;
		}
		case 2:
		{
			msg_mail_phmapadd((rt_uint8_t *)mapaddr,mapsize,net_get_date());
			break;
		}
		case 3:
		{
			msg_mail_recmapadd((rt_uint8_t *)mapaddr,mapsize,net_get_date());
			break;
		}
		default:
		{
			break;
		}
	}

	rt_free(mapdata);

	
}

FINSH_FUNCTION_EXPORT(upload_map,upload_map[type]--upload [0:account 1:key 2:phone 3:record] mapbyte);

void landtimeset(rt_uint16_t data)
{
	landresendtime = data;
}
FINSH_FUNCTION_EXPORT(landtimeset,set land resend time);

#endif


