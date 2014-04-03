/**
����:�����ʼ���������
�汾:0.1
����:wangzw@yuettak.com
*/
#include "netmailclass.h"

#define MAIL_FAULT_OUTTIME    600
#define MAIL_FAULT_RESEND     3
#define SYS_APP_BIN_FILE_NAME "/app.bin"

/**
����Э�鷢�ͽӿ�
*/
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
  mail->resend = MAIL_FAULT_RESEND;									//�ط�����
  mail->outtime = MAIL_FAULT_OUTTIME;              //��ʱ��
  mail->sendmode = SYNC_MODE;       //ͬ��
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
����:�ն����Կ��
*/
rt_uint8_t msg_mail_addrkey(net_keyadd_user *KeyData)
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
	//
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











/**
����:���մ���

*/
//�յ��İ������ṹ
typedef struct 
{
	rt_uint8_t *buf;   //��ǰ�������
	rt_size_t  size;   //�����С
	rt_size_t  bitmax; //λ�������ֵ
}FilePackPosMap;

//�ļ����������ṹ
typedef struct
{
	FilePackPosMap *PackMap;
	//rt_size_t      CurPackNum; //��ǰ�յ��İ�����
	rt_size_t      FileSize;   //�ļ���С
	rt_uint8_t     PackSize;   //����С
	rt_size_t      PackNum;    //������
	rt_uint32_t    CRC32;      //crc32
}NetFileInfo,*NetFileInfo_p;

//�����ļ�ʱ���ڼ�¼�ļ���Ϣ
static NetFileInfo_p NetRecvFileInfo = RT_NULL;


/*
����:�����ļ���Ϣ�ṹ
*/
static void net_fileinfo_set(NetFileInfo_p info,net_recvmsg_p mail)
{	
	info->PackSize = mail->data.filerq.request.packsize;
	net_string_copy_uint32(&info->FileSize,mail->data.filerq.request.size);
	net_string_copy_uint32(&info->PackNum,mail->data.filerq.request.packnum);
	net_string_copy_uint32(&info->CRC32,mail->data.filerq.request.crc32);
	rt_kprintf("\nCRC32    = %x\n",info->CRC32);
	rt_kprintf("FileSize = %d\n",info->FileSize);
	rt_kprintf("PackNum  = %d\n",info->PackNum);
	rt_kprintf("PackSize = %d\n",info->PackSize);
}

//#ifdef 1
FilePackPosMap * file_pack_pos_create(rt_size_t PackNum)
{
	FilePackPosMap *map;

	map = rt_calloc(1,sizeof(FilePackPosMap));
	RT_ASSERT(map != RT_NULL);
	if(PackNum % 8 == 0)
	{
		map->size = PackNum / 8;
	}
	else
	{
		map->size = PackNum / 8;
		map->size++;
	}
	map->bitmax = PackNum;
	
	map->buf = rt_calloc(1,map->size);
	RT_ASSERT(map->buf != RT_NULL);
	
	return map;
}

void file_pack_pos_delete(FilePackPosMap *map)
{
	rt_free(map->buf);
	rt_free(map);
}

void file_pack_pos_add(FilePackPosMap *Map,rt_size_t PackPos)
{
	rt_size_t BytePos;
	rt_uint8_t BitPos;

	if(PackPos >= Map->bitmax)
	{
		rt_kprintf("bit max is %d\n",Map->bitmax);
		return ;
	}
	BytePos = PackPos / 8;
	BitPos = PackPos % 8;
	Map->buf[BytePos] |= 1<<(7-BitPos);
}
/*
����:�ļ��������Ƿ�������
*/
rt_int8_t file_pack_complete(FilePackPosMap *Map)
{
	rt_size_t BytePos;
	rt_size_t BitPos;
	rt_size_t CompleteNum = 0;
	
	for(BytePos = 0;BytePos < Map->size;BytePos++)
	{
		for(BitPos = 0 ; BitPos < 8;BitPos++)
		{
			if((Map->buf[BytePos]>>(7-BitPos) & 0x01) == 0)
			{
				return -1;
			}
			else
			{
				CompleteNum++;
				if(CompleteNum >= Map->bitmax)
				{
					return CompleteNum;
				}
			}
		}
	}
	return CompleteNum;
}

void file_pack_map_show(FilePackPosMap *Map)
{
	rt_size_t i;
	rt_size_t j;
	
	for(i = 0;i < Map->size;i++)
	{
		for(j = 0 ; j < 8;j++)
		{
			rt_kprintf("%d",Map->buf[i]>>(7-j) &0x01);
		}
		rt_kprintf("\n");
	}
}
void testfp(rt_uint8_t pos)
{
  FilePackPosMap *map;
  rt_int8_t result;

  map = file_pack_pos_create(22);
  file_pack_pos_add(map,pos);
  file_pack_map_show(map);
  result = file_pack_complete(map);
  rt_kprintf("complete = %d\n",result);
  

}
//#endif

/*
����:����һ��NetFileInfoʵ��
����:NetFileInfoʵ��ĵ�ַ
*/
static NetFileInfo_p net_fileinfo_create(rt_size_t PackNum)
{
	NetFileInfo_p FileInfo;
	
	FileInfo = rt_calloc(1,sizeof(NetFileInfo));
	RT_ASSERT(FileInfo != RT_NULL);
	FileInfo->PackMap = file_pack_pos_create(PackNum);
	
	return FileInfo;
}

/*
����:ɾ��NetFileInfoʵ��
*/
static void net_fileinfo_delete(NetFileInfo_p FileInfo)
{
	file_pack_pos_delete(FileInfo->PackMap);
	rt_free(FileInfo);
}

/*
����:�����ļ�����
����: 0 �ɹ� 1 ʧ��
*/
rt_uint8_t net_recv_filerq_process(net_recvmsg_p mail)
{	
	rt_uint8_t result = 1;
	rt_size_t  PackNum = 0;
	
	if(net_event_process(1,NET_ENVET_FILERQ) == 0)
	{
		rt_kprintf("Being Receive File\n");
		return 1;
	}
	//��ͬ�ļ����ͱ���Ϊ��ͬ����
	if(mail->data.filerq.request.type == 1)
	{
		int FileID;
		
		unlink(SYS_APP_BIN_FILE_NAME);
		FileID = open(SYS_APP_BIN_FILE_NAME,O_CREAT|O_RDWR,0x777);
		if(FileID < 0)
		{
			rt_kprintf("Creat APP.BIN File Fail\n");
			result = 1;
		}
		else
		{
      close(FileID);
      //��־�ڽ����ļ�״̬
      net_event_process(0,NET_ENVET_FILERQ);
      if(NetRecvFileInfo != RT_NULL)
      {
        rt_kprintf("NetRecvFileInfo Data Abnormal\n\n");
        net_fileinfo_delete(NetRecvFileInfo);
      }
      //�ļ���Ϣ��ȡ
      net_string_copy_uint32(&PackNum,mail->data.filerq.request.packnum);
      NetRecvFileInfo = net_fileinfo_create(PackNum);
      net_fileinfo_set(NetRecvFileInfo,mail);
  
      result = 0;
		}
		
	}
	return result;
}

rt_uint8_t filebuffer[3*1024];

rt_uint8_t net_file_packdata_process(net_recvmsg_p mail)
{
	rt_uint8_t result = 1;
	int fileid ;
	rt_uint16_t PackSizeRmap[6] = {64,128,256,0,512,1024};
	
	//������ļ�������״̬
	if(net_event_process(1,NET_ENVET_FILERQ) == 0)
	{
		rt_uint32_t CurWritePos;
		
		fileid = open(SYS_APP_BIN_FILE_NAME,O_RDWR,0x777);
		if(fileid < 0)
		{
			//���������쳣
			rt_kprintf("Receive data anomalies\n");
			result = 2;
		}
		net_string_copy_uint32(&CurWritePos,mail->data.filedata.pos);
		lseek(fileid,CurWritePos*PackSizeRmap[NetRecvFileInfo->PackSize],DFS_SEEK_SET);
		rt_kprintf("write size %d\n",write(fileid,mail->data.filedata.data,mail->lenmap.bit.data-4));
		close(fileid);
		rt_memcpy(filebuffer+CurWritePos*PackSizeRmap[NetRecvFileInfo->PackSize],
							mail->data.filedata.data,mail->lenmap.bit.data-4);
							
		file_pack_pos_add(NetRecvFileInfo->PackMap,CurWritePos);
		rt_kprintf("Recv ok Num:%d\n",file_pack_complete(NetRecvFileInfo->PackMap));
		if(file_pack_complete(NetRecvFileInfo->PackMap) != -1)
		{
			rt_uint32_t crc32;

			rt_kprintf("RAM FILE crc32:%x\n",cyg_ether_crc32(filebuffer,NetRecvFileInfo->FileSize));
			file_get_crc32(SYS_APP_BIN_FILE_NAME,&crc32);
			if(crc32 != NetRecvFileInfo->CRC32)
			{
				result = 2;
				rt_kprintf("File Recv Fail CRC32 Error\n\n\n");
			}
			else
			{	
				rt_kprintf("File Recv Succeed\n");
				result = 0;
			}
			net_event_process(2,NET_ENVET_FILERQ);
			net_fileinfo_delete(NetRecvFileInfo);
			NetRecvFileInfo = RT_NULL;
		}
		else
		{
      result = 0;
		}
	}
	else
	{
		rt_kprintf("Now not Recv File\n");
	}
	return result;
}

rt_bool_t net_mail_crc16_check(net_recvmsg_p Mail)
{
	rt_uint16_t CRC16Right;
	rt_uint16_t CurCRC16;
	rt_uint16_t CRCLength;
	rt_uint8_t  *tmp;
	
	CRCLength = Mail->lenmap.bit.check +
							Mail->lenmap.bit.data +
							Mail->lenmap.bit.col +
							Mail->lenmap.bit.cmd;

	tmp = (rt_uint8_t *)Mail;
	rt_memcpy((void *)&CRC16Right,(const void *)(tmp+CRCLength+2),2);

  CRC16Right = net_rev16(CRC16Right);

	Mail->lenmap.bype = net_rev16(Mail->lenmap.bype);
	
	CurCRC16 = net_crc16((unsigned char *)tmp+2,CRCLength);

	Mail->lenmap.bype = net_rev16(Mail->lenmap.bype);
	
	rt_kprintf("CRC16Right = %04X\n",CRC16Right);
	rt_kprintf("CurCRC16   = %04X\n",CurCRC16);
	if(CurCRC16 == CRC16Right)
	{
		return RT_TRUE;
	}
	
	return RT_FALSE;
}

/*
����:���մ����ָ�뺯��
����: 0: �ɹ� 1: ʧ��
*/
rt_uint8_t net_message_recv_process(net_recvmsg_p Mail,void *UserData)
{
	rt_uint8_t result = 0;
	rt_bool_t   CRC16Result;
	rt_uint32_t shift;
	rt_uint16_t shift16; 

	CRC16Result = net_mail_crc16_check(Mail);
	switch(Mail->cmd)
	{
	  case NET_MSGTYPE_FILEREQUEST:
	  {
	  	//�ļ�����
			if(CRC16Result == RT_TRUE)
			{
				result = net_recv_filerq_process(Mail);
		  	if(result == 0)
		  	{
					//����Ӧ��
					msg_mail_nullack(NET_MSGTYPE_FILEREQUE_ACK);
		  	}
			}
	  	
	    break;
	  }
	  case NET_MSGTYPE_FILEDATA:
	  {
	  	//�ļ�����
	  	rt_size_t PackOrder;

	  	if(CRC16Result == RT_TRUE)
	  	{
				result = net_file_packdata_process(Mail);

				net_string_copy_uint32(&PackOrder,Mail->data.filedata.pos);
				msg_mail_fileack(PackOrder,result);
	  	}
	  	else
	  	{
				msg_mail_fileack(PackOrder,2);
	  	}
	  	
			break;
	  }
	  default:
	  {
	    break;
	  }
	}

	return result;
}


int InitNetRecvFun(void)
{
	Net_Set_MsgRecv_Callback(net_message_recv_process);
	
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
FINSH_FUNCTION_EXPORT(testfp,"(MSGType)Send Ack Data Is NULL Message");



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


