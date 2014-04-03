/**
����:�����ʼ���������
�汾:0.1
����:wangzw@yuettak.com
*/
#include "netmailclass.h"
#include "crc16.h"
#include "netfile.h"
#include "netkey.h"
#include "netphone.h"

#define MAIL_FAULT_RESEND     3
#define MAIL_FAULT_OUTTIME    1000


/**
����Э�鷢�ͽӿ�
*/
/** 
@brief  send landed mail
@param  void 
@retval RT_TRUE :succeed,RT_FALSE:fail
*/
void send_net_landed_mail(void)
{
  net_msgmail mail;

  mail.user = RT_NULL;
  mail.time = 0;
  mail.type = NET_MSGTYPE_LANDED;
  mail.resend = 3;
  mail.outtime = 600;
  mail.sendmode = ASYN_MODE;
  mail.col.byte = net_order.byte;
  net_order.bit.col++;
  net_msg_send_mail(&mail);
}

/*
����:���ñ����ʼ�
*/
rt_uint8_t msg_mail_alarm(rt_uint8_t alarm,rt_uint8_t LockStatus,rt_uint32_t time)
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
	UserData->result.complete = rt_sem_create("NAlarm",0,RT_IPC_FLAG_FIFO);
  RT_ASSERT(UserData != RT_NULL);

	//�����ʼ�
	mail->type = NET_MSGTYPE_ALARM;   //�ʼ�����
  mail->resend = MAIL_FAULT_RESEND;									//�ط�����
  mail->outtime = MAIL_FAULT_OUTTIME;              //��ʱ��
  mail->sendmode = SYNC_MODE;       //ͬ��
  mail->col.byte = get_msg_new_order();
	//����˽������
	UserData->alarm.lock_status = LockStatus;
	UserData->alarm.type = alarm;
	net_uint32_copy_string(UserData->alarm.time,time);
	//�����ʼ�
  net_msg_send_mail(mail);
  rt_sem_take(UserData->result.complete,RT_WAITING_FOREVER);
  rt_sem_delete(UserData->result.complete);
  rt_kprintf("send result = %d\n",UserData->result.result);
  result = UserData->result.result;
  
  //�ͷ���Դ
	rt_free(UserData);
	rt_free(mail);

	return result;
}

/*
����:���ù��ϱ���
*/
rt_uint8_t msg_mail_fault(rt_uint8_t fault,rt_uint32_t time)
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
	UserData->result.complete = rt_sem_create("NFault",0,RT_IPC_FLAG_FIFO);
  RT_ASSERT(UserData != RT_NULL);

	//�����ʼ�
	mail->type = NET_MSGTYPE_FAULT;   //�ʼ�����
  mail->resend = MAIL_FAULT_RESEND;									//�ط�����
  mail->outtime = MAIL_FAULT_OUTTIME;              //��ʱ��
  mail->sendmode = SYNC_MODE;       //ͬ��
  mail->col.byte = get_msg_new_order();
	//����˽������
	UserData->fault.type = fault;
	net_uint32_copy_string(UserData->fault.time,time);
	//�����ʼ�
  net_msg_send_mail(mail);
  rt_sem_take(UserData->result.complete,RT_WAITING_FOREVER);
  rt_sem_delete(UserData->result.complete);
  rt_kprintf("send result = %d\n",UserData->result.result);
  result = UserData->result.result;
  
  //�ͷ���Դ
	rt_free(UserData);
	rt_free(mail);

	return result;
}

/*
����:Կ�׿���
*/
rt_uint8_t msg_mail_opendoor(rt_uint8_t type,rt_uint16_t key,rt_uint32_t time)
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
	UserData->result.complete = rt_sem_create("NOPdoor",0,RT_IPC_FLAG_FIFO);
  RT_ASSERT(UserData != RT_NULL);

	//�����ʼ�
	mail->type = NET_MSGTYPE_OPENDOOR;   //�ʼ�����
  mail->resend = MAIL_FAULT_RESEND;									//�ط�����
  mail->outtime = MAIL_FAULT_OUTTIME;              //��ʱ��
  mail->sendmode = SYNC_MODE;       //ͬ��
  mail->col.byte = get_msg_new_order();
	//����˽������
	UserData->opendoor.type = type;
	net_uint16_copy_string(UserData->opendoor.key,key);
	net_uint32_copy_string(UserData->opendoor.time,time);
	//�����ʼ�
  net_msg_send_mail(mail);
  rt_sem_take(UserData->result.complete,RT_WAITING_FOREVER);
  rt_sem_delete(UserData->result.complete);
  rt_kprintf("send result = %d\n",UserData->result.result);
  result = UserData->result.result;
  
  //�ͷ���Դ
	rt_free(UserData);
	rt_free(mail);

	return result;
}

/*
����:��ر���
*/
rt_uint8_t msg_mail_battery(rt_uint8_t status,rt_uint8_t capacity,rt_uint32_t time)
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
	UserData->result.complete = rt_sem_create("NBat",0,RT_IPC_FLAG_FIFO);
  RT_ASSERT(UserData != RT_NULL);

	//�����ʼ�
	mail->type = NET_MSGTYPE_BATTERY;   //�ʼ�����
  mail->resend = MAIL_FAULT_RESEND;	  //�ط�����
  mail->outtime = MAIL_FAULT_OUTTIME; //��ʱ��
  mail->sendmode = SYNC_MODE;         //ͬ��
  mail->col.byte = get_msg_new_order();
  
	//����˽������
	UserData->battery.status = status;
	UserData->battery.capacity = capacity;
	net_uint32_copy_string(UserData->battery.time,time);
	//�����ʼ�
  net_msg_send_mail(mail);
  rt_sem_take(UserData->result.complete,RT_WAITING_FOREVER);
  rt_sem_delete(UserData->result.complete);
  rt_kprintf("send result = %d\n",UserData->result.result);
  result = UserData->result.result;
  
  //�ͷ���Դ
	rt_free(UserData);
	rt_free(mail);

	return result;
}

/*
����:ʱ��ͬ��
*/
  rt_uint8_t msg_mail_adjust_time(void)
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
    UserData->result.complete = rt_sem_create("NTime",0,RT_IPC_FLAG_FIFO);
    RT_ASSERT(UserData != RT_NULL);
  
    //�����ʼ�
    mail->type = NET_MSGTYPE_TIME;   //�ʼ�����
    mail->resend = MAIL_FAULT_RESEND;                 //�ط�����
    mail->outtime = MAIL_FAULT_OUTTIME;              //��ʱ��
    mail->sendmode = SYNC_MODE;       //ͬ��
    mail->col.byte = get_msg_new_order();
    //����˽������
    net_copy_date_str(UserData->date.time);
    //�����ʼ�
    net_msg_send_mail(mail);
    rt_sem_take(UserData->result.complete,RT_WAITING_FOREVER);
    rt_sem_delete(UserData->result.complete);
    rt_kprintf("send result = %d\n",UserData->result.result);
    result = UserData->result.result;
    
    //�ͷ���Դ
    rt_free(UserData);
    rt_free(mail);
  
    return result;
  }

/*
����:�û��澯����
*/
rt_uint8_t msg_mail_alarmarg(rt_uint8_t Type,rt_uint8_t arg)
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
	UserData->result.complete = rt_sem_create("NAlArg",0,RT_IPC_FLAG_FIFO);
  RT_ASSERT(UserData != RT_NULL);

	//�����ʼ�
	mail->type = NET_MSGTYPE_ALARMARG;   //�ʼ�����
  mail->resend = MAIL_FAULT_RESEND;									//�ط�����
  mail->outtime = MAIL_FAULT_OUTTIME;              //��ʱ��
  mail->sendmode = SYNC_MODE;       //ͬ��
  mail->col.byte = get_msg_new_order();
	//����˽������
	UserData->args.type = Type;
	UserData->args.arg = arg;
	//�����ʼ�
  net_msg_send_mail(mail);
  rt_sem_take(UserData->result.complete,RT_WAITING_FOREVER);
  rt_sem_delete(UserData->result.complete);
  rt_kprintf("send result = %d\n",UserData->result.result);
  result = UserData->result.result;
  
  //�ͷ���Դ
	rt_free(UserData);
	rt_free(mail);

	return result;
}


/*
����:����ֻ�������
*/
void msg_mail_phoneadd_ack(rt_uint8_t pos ,rt_uint8_t result)
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
  mail->col.byte = get_msg_new_order();
	//����˽������
	UserData->pos = pos;
	UserData->result = result;
	//�����ʼ�
  net_msg_send_mail(mail);
  
  //�ͷ���Դ
	rt_free(mail);
}

/*
����:ɾ���ֻ�������
*/
void msg_mail_phonedel_ack(rt_uint8_t pos ,rt_uint8_t result)
{
	net_msgmail_p mail = RT_NULL;
	net_phonedel_ack *UserData = RT_NULL;

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
  mail->col.byte = get_msg_new_order();
	//����˽������
	UserData->pos = pos;
	UserData->result = result;
	//�����ʼ�
  net_msg_send_mail(mail);
  
  //�ͷ���Դ
	rt_free(mail);
}

/*
����:�ն����Կ��
*/
rt_uint8_t msg_mail_keyadd(net_keyadd_user *KeyData)
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
	UserData->result.complete = rt_sem_create("NAlArg",0,RT_IPC_FLAG_FIFO);
  RT_ASSERT(UserData != RT_NULL);

	//�����ʼ�
	mail->type = NET_MSGTYPE_KEYADD;   //�ʼ�����
  mail->resend = MAIL_FAULT_RESEND;									//�ط�����
  mail->outtime = MAIL_FAULT_OUTTIME;              //��ʱ��
  mail->sendmode = SYNC_MODE;       //ͬ��
  mail->col.byte = get_msg_new_order();
	//����˽������
	//
	//�����ʼ�
  net_msg_send_mail(mail);
  rt_sem_take(UserData->result.complete,RT_WAITING_FOREVER);
  rt_sem_delete(UserData->result.complete);
  rt_kprintf("send result = %d\n",UserData->result.result);
  result = UserData->result.result;
  
  //�ͷ���Դ
	//rt_free(UserData);
	rt_free(mail);

	return result;
}

/*
����:�������Կ��Ӧ��
*/
void msg_mail_keyadd_ack(rt_uint16_t pos,rt_uint8_t result)
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
  mail->col.byte = get_msg_new_order(); 

  //˽������
  data->pos = pos;
  data->result = result;
  mail->user = data;

  //�����ʼ�
  net_msg_send_mail(mail);

  //�ͷ���Դ
	rt_free(mail);
}

rt_uint8_t msg_mail_keydelete(rt_uint16_t pos)
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
	UserData->result.complete = rt_sem_create("NAlArg",0,RT_IPC_FLAG_FIFO);
  RT_ASSERT(UserData != RT_NULL);

	//�����ʼ�
	mail->type = NET_MSGTYPE_KEYDELETE;   //�ʼ�����
  mail->resend = MAIL_FAULT_RESEND;									//�ط�����
  mail->outtime = MAIL_FAULT_OUTTIME;              //��ʱ��
  mail->sendmode = SYNC_MODE;       //ͬ��
  mail->col.byte = get_msg_new_order();
  
	//����˽������
	pos = net_rev16(pos);
	rt_memcpy(UserData->data.pos,&pos,2);
	
	//�����ʼ�
  net_msg_send_mail(mail);
  rt_sem_take(UserData->result.complete,RT_WAITING_FOREVER);
  rt_sem_delete(UserData->result.complete);
  rt_kprintf("send result = %d\n",UserData->result.result);
  result = UserData->result.result;
  
  //�ͷ���Դ
	rt_free(UserData);
	rt_free(mail);

	return result;
}

/** 
@brief send MSGType type message process result
@param MSGType :message type 
@param result :process result
@retval void
*/
void msg_mail_resultack(message_type MSGType,rt_uint8_t result)
{
	net_msgmail_p mail = RT_NULL;
	net_ack *UserData;

	//��ȡ��Դ
	mail = (net_msgmail_p)rt_calloc(1,sizeof(net_msgmail));
	RT_ASSERT(mail != RT_NULL);

	//�����ʼ�
	mail->type = NET_MSGTYPE_FILEREQUE_ACK;	//�ʼ�����
	mail->resend = 0;         							//�ط�����
	mail->outtime = 0;        							//��ʱ��
	mail->sendmode = ASYN_MODE;							//ͬ��
	mail->col.byte = get_msg_new_order();

	//���������� �ڷ�����ɺ�����
	UserData = rt_calloc(1,sizeof(net_ack));
	RT_ASSERT(UserData != RT_NULL);
	mail->user = UserData;
	UserData->result = result;
	
	//�����ʼ�
	net_msg_send_mail(mail);

	//�ͷ���Դ
	rt_free(mail);

}

/*
����:����Ӧ��û������
*/
void msg_mail_nullack(message_type MSGType)
{
	net_msgmail_p mail = RT_NULL;

	//��ȡ��Դ
	mail = (net_msgmail_p)rt_calloc(1,sizeof(net_msgmail));
	RT_ASSERT(mail != RT_NULL);

	//�����ʼ�
	mail->type = MSGType; 							//�ʼ�����
  mail->resend = MAIL_FAULT_RESEND;		//�ط�����
  mail->outtime = MAIL_FAULT_OUTTIME; //��ʱ��
  mail->sendmode = ASYN_MODE;					//ͬ��
  mail->col.byte = get_msg_new_order();
  mail->user = RT_NULL;
	
	//�����ʼ�
  net_msg_send_mail(mail);
  
  //�ͷ���Դ
	rt_free(mail);
}

/*
����:�ļ�����Ӧ��
*/
void msg_mail_filereq_ack(rt_uint8_t result)
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
	mail->col.byte = get_msg_new_order();

	//���������� �ڷ�����ɺ�����
	UserData = rt_calloc(1,sizeof(net_filereq_ack_user));
	RT_ASSERT(UserData != RT_NULL);
	mail->user = UserData;
	UserData->result.result = result;
	
	//�����ʼ�
	net_msg_send_mail(mail);

	//�ͷ���Դ
	rt_free(mail);
}

/*
����:�ļ���Ӧ��
*/
void msg_mail_fileack(rt_size_t PackOrder,rt_uint8_t Fresult)
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
	mail->col.byte = get_msg_new_order();

	//���������� �ڷ�����ɺ�����
	FileAck = rt_calloc(1,sizeof(net_filedata_ack_user));
	RT_ASSERT(FileAck != RT_NULL);
	mail->user = FileAck;
	FileAck->fileack.result = Fresult;
	net_uint32_copy_string(FileAck->fileack.order,PackOrder);
	//�����ʼ�
	net_msg_send_mail(mail);

	//�ͷ���Դ
	rt_free(mail);
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
			result = net_recv_filerq_process(Mail);
			msg_mail_filereq_ack(result);
	  	
	    break;
	  }
	  case NET_MSGTYPE_FILEDATA:
	  {
	  	//�ļ�����
	  	rt_size_t PackOrder;

			result = net_file_packdata_process(Mail);
			net_string_copy_uint32(&PackOrder,Mail->data.filedata.pos);
			msg_mail_fileack(PackOrder,result);
				
			break;
	  }
	 	case NET_MSGTYPE_PHONEADD:
	 	{
	 		ProcessResult = net_phone_add_process(Mail);

	 		result = (ProcessResult == RT_EOK)?1:0;
	 		msg_mail_phoneadd_ack(Mail->data.phoneadd.pos,result);
			break;
	 	}
	 	case NET_MSGTYPE_PHONEDELETE:
	 	{
	 		ProcessResult = net_phone_del_process(Mail);

			result = (ProcessResult == RT_EOK)?1:0;
			msg_mail_phonedel_ack(Mail->data.phonedel.pos,result);
			break;
	 	}
	 	case NET_MSGTYPE_KEYADD:
	 	{
	 		rt_uint16_t pos;
	 		
	 		ProcessResult = net_key_add_process(Mail);

	 		result = (ProcessResult == RT_EOK)?1:0;
	 		net_string_copy_uint16(&pos,Mail->data.keyadd.key.col);
	 		msg_mail_keyadd_ack(pos,result);
			break;
	 	}
	 	case NET_MSGTYPE_KEYADD_ACK:
	 	{
			break;
	 	}
	 	case NET_MSGTYPE_KEYDELETE:
	 	{
	 		rt_uint16_t pos;

	 		ProcessResult = net_key_del_process(Mail);

	 		result = (ProcessResult == RT_EOK)?1:0;
	 		net_string_copy_uint16(&pos,Mail->data.keydel.key.pos);
			break;
	 	}
	 	case NET_MSGTYPE_KEYDEL_ACK:
	 	{
			break;
	 	}
	 	case NET_MSGTYPE_ALARMARG:
	 	{
			break;
	 	}
	 	case NET_MSGTYPE_ALARMARG_ACK:
	 	{
			break;
	 	}
	  default:
	  {
	    break;
	  }
	}

	return result;
}

static void net_msg_thread_process(void)
{
	net_file_timer_process();
}

int InitNetRecvFun(void)
{
	Net_Set_MsgRecv_Callback(net_message_recv_process);
	Net_NetMsg_thread_callback(net_msg_thread_process);
	return 0;
}
INIT_APP_EXPORT(InitNetRecvFun);

























#ifdef RT_USING_FINSH
#include <finsh.h>

FINSH_FUNCTION_EXPORT(msg_mail_alarm,"(Alarm,Lock,Time)Send Alarm Mail Message");
FINSH_FUNCTION_EXPORT(msg_mail_fault,"(Type,Time)Send Fault Mail Message");
FINSH_FUNCTION_EXPORT(msg_mail_battery,"(Statu,Capacity,Time)Send Battery Mail Message");
FINSH_FUNCTION_EXPORT(msg_mail_opendoor,"(Type,KeyCode,Time)Send OpenDoor Mail Message");
FINSH_FUNCTION_EXPORT(msg_mail_adjust_time,"(void)Send Adjust Time Message");
FINSH_FUNCTION_EXPORT(msg_mail_alarmarg,"(Type,arg)Send Set Alarm Argument Message");
FINSH_FUNCTION_EXPORT(msg_mail_nullack,"(MSGType)Send Ack Data Is NULL Message");
FINSH_FUNCTION_EXPORT(msg_mail_phoneadd_ack,"(pos result)Send Add Phone Message");
FINSH_FUNCTION_EXPORT(msg_mail_phonedel_ack,"(pos result)Send delete Phone Message");





void TestAddKey(void)
{
	net_keyadd_user *KeyData;
	rt_uint16_t col;
	
	KeyData = rt_calloc(1,sizeof(net_keyadd_user));
	RT_ASSERT(KeyData != RT_NULL);

	col = 0xabcd;
	col = net_rev16(col);
	rt_memcpy(KeyData->data.col,&col,2);
	KeyData->data.type = 1;
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

#endif


