/**
����:ͨ������Э�������ļ��ķ��������
�汾:v0.1
����:wangzw <wangzw@yuettak.com>
*/
#include "netfile.h"
#include <cyg/crc/crc.h>

#define NET_MAIL_OK  0




/*
����:�����ļ������ʼ�

*/
static void file_msg_mail_send(net_msgmail_p mail,rt_uint8_t *buffer,rt_uint16_t size)
{
  net_filedata_user *file;
  
  mail->time = 0;
  mail->type = NET_MSGTYPE_FILEDATA;
  mail->resend = 3;
  mail->outtime = 0xffff;
  mail->sendmode = SYNC_MODE;
  mail->col.byte = net_order.byte;
  net_order.bit.col++;
  
  mail->user = file = (net_filedata_user *)rt_calloc(1,sizeof(net_filedata_user));
  RT_ASSERT(file!=RT_NULL);
  file->data.data  = buffer;
  RT_ASSERT(file->data.data!=RT_NULL);
	
  file->length = size;//����С
  file->result.complete = rt_sem_create("netsend",0,RT_IPC_FLAG_FIFO);
  
  net_msg_send_mail(mail);
}

/*
����:���ļ��л��һ����������
����:FileID    �ļ�����  
     PackOrder �����
     ReadSize  ����С
     buffer    ���ݴ�ŵ�ַ
*/
static void get_file_pack_data(int FileID,
 															rt_uint32_t PackOrder,
 															rt_uint16_t ReadSize,
 															rt_uint8_t buffer[])
{
	rt_uint32_t				Offset = 0;

	Offset = PackOrder * NET_FILE_BUF_SIZE;		//read data pos
	if(lseek(FileID,Offset,SEEK_SET) == -1)  //file fixed position
	{
		return;
	}
	buffer[0] = (PackOrder&0xff000000)>>24;
	buffer[1] = (PackOrder&0x00ff0000)>>16;
	buffer[2] = (PackOrder&0x0000ff00)>>8;
	buffer[3] = (PackOrder&0x000000ff)>>0;
	
	read(FileID,(void*)(buffer+4),ReadSize);
}

/*
����:�����ļ�������
*/
static rt_size_t get_file_packets(rt_size_t FileSize,rt_uint32_t PackSize)
{
	rt_size_t PackNum;
	
	PackNum = FileSize / PackSize;
	if(FileSize % PackSize != 0)
	{
		PackNum++;
	}
	return PackNum;
}
/*
����:����ĳһ���ļ�
����:FileName �ļ�����
     PackOrder �����
����ֵ: ���͵Ľ��
*/
static rt_int8_t send_file_pack(char *FileName,rt_uint32_t PackOrder,rt_uint16_t ReadSize,net_msgmail_p mail)
{
	int FileID;
	rt_uint8_t *buffer = RT_NULL;

	//�����ڴ�
	buffer = (rt_uint8_t *)rt_calloc(1,ReadSize + 4);
	RT_ASSERT(buffer != RT_NULL);
	
	FileID = open(FileName,O_RDONLY,0x777);
	if(FileID < 0)
	{
		//���ļ�ʧ��
		return -1;
	}

	//���һ����������
	get_file_pack_data(FileID,PackOrder,ReadSize,buffer);
	//����һ����
	file_msg_mail_send(mail,buffer,ReadSize);
	//�ͷ���Դ
	close(FileID);
	//rt_free(buffer);
	
	return 0;
}
/*
����:��ʼ���ʼ�����
*/
void init_net_msgmail(net_msgmail_p mail[],rt_uint8_t size)
{
	rt_uint8_t i;

	for(i = 0 ; i < size ;i++)
	{
		mail[i] = RT_NULL;
	}
}

/*
����:ɾ�������е�ĳһ��
*/
void delete_net_msgmail(net_msgmail_p mail[],rt_uint8_t pos)
{
	if(mail[pos] != RT_NULL)
	{
		rt_free(mail[pos]);
		mail[pos] = RT_NULL;
	}
}

/*
����:�����ʼ�����
*/
void destroy_net_msgmail(net_msgmail_p mail[],rt_uint8_t size)
{
	rt_uint8_t i;

	for(i = 0 ; i < size ;i++)
	{
		delete_net_msgmail(mail,i);
	}
}
/*
����:����һ���յ��ʼ�λ��
*/
rt_int8_t find_null_msgmail(net_msgmail_p mail[],rt_uint8_t size)
{
	rt_uint8_t i;

	for(i = 0 ; i < size ;i++)
	{
		if(mail[i] == RT_NULL)
		{
			return i;
		}
	}	
	return -1;
}


/*
����:��ѯ�Ƿ��гɹ��İ�
����:1 �ɹ� 0ʧ��
*/
rt_uint8_t check_msgmail_succeed(net_msgmail_p mail[],rt_uint8_t size,rt_sem_t SendSem)
{
	rt_err_t result;
	rt_uint8_t i;
	rt_uint8_t RunResult = 0;
	net_filedata_user *file;

	for(i = 0 ; i < size ;i++)
	{
		if(mail[i] != RT_NULL)
		{
			if(mail[i]->user != RT_NULL)
			{
				file = mail[i]->user;
				result = rt_sem_take(file->result.complete,10);
				if(result == RT_EOK)
				{	
					RT_DEBUG_LOG(SHOW_NFILE_SRESULT,("File Pack Send Result: %d\n",file->result.result));     
          if(file->result.result == NET_MAIL_OK)
          {
						RunResult++;
          }
          else //����ʧ��
          {
          	RunResult = 0xff;
					  return RunResult;
          }
          //�ͷ���Դ
          
          rt_sem_delete(file->result.complete);
          rt_free(file->data.data);
          rt_free(file);
          delete_net_msgmail(mail,i);
          rt_sem_release(SendSem);
				}
			}
			else
			{
				//rt_kprintf("mail[i]->user == RT_NULL\n\n");
			}
		}
	}	
	return RunResult;
}

/*
����:�����ļ�����
*/
static rt_int8_t send_file_request(char *FileName)
{
	rt_int8_t   Result;        //ִ�н��
	rt_uint32_t FileSize;			 //�ļ���С
	rt_uint32_t PackNum;
	rt_uint32_t CRC32Value;
	net_msgmail_p mail = RT_NULL;
	net_filerequest_user *RequestInfo;

	//��ȡ��Դ
	mail = (net_msgmail_p)rt_calloc(1,sizeof(net_msgmail));
	RT_ASSERT(mail != RT_NULL);
	RequestInfo = (net_filerequest_user *)rt_calloc(1,sizeof(net_filerequest_user));
	RT_ASSERT(RequestInfo != RT_NULL);
	
	Result = get_file_size(FileName,&FileSize);
	if(Result < 0)
	{
		//�ͷ��ڴ���Դ
		rt_free(RequestInfo);
		rt_free(mail);
		return -1;
	}
	mail->type = NET_MSGTYPE_FILEREQUEST;
  mail->time = 0;
  mail->resend = 3;
  mail->outtime = 600;
  mail->sendmode = SYNC_MODE;//ͬ��
  mail->col.byte = net_order.byte;
  net_order.bit.col++;
  
  mail->user = RequestInfo;
	//���������
	PackNum = get_file_packets(FileSize,NET_FILE_BUF_SIZE);
	
	RequestInfo->file.alarm = 0;  //��������
	net_uint32_copy_string(RequestInfo->file.time,1234);//ʱ��
	RequestInfo->file.type = 1;    //�ļ���ʽ
	net_uint32_copy_string(RequestInfo->file.size,FileSize);//�ļ���С
	RequestInfo->file.packsize = 4;//����С512k
	net_uint32_copy_string(RequestInfo->file.packnum,PackNum);//������
	//����CRC32
	file_get_crc32((rt_uint8_t *)FileName,&CRC32Value);
	net_uint32_copy_string(RequestInfo->file.crc32,CRC32Value);
	RT_DEBUG_LOG(SHOW_NFILE_CRC32,("File:%s CRC32:%X\n",FileName,CRC32Value));

	RequestInfo->result.complete = rt_sem_create("filerequ",0,RT_IPC_FLAG_FIFO);
	RT_ASSERT(RequestInfo->result.complete != RT_NULL);

  net_msg_send_mail(mail);
  rt_sem_take(RequestInfo->result.complete,RT_WAITING_FOREVER);
  rt_sem_delete(RequestInfo->result.complete);
  //rt_kprintf("send result = %d\n",RequestInfo->result.result);
  if(RequestInfo->result.result == 0)
  {
		Result = 0;
  }
  else
  {
		Result = -1;
  }
  //�ͷ��ڴ���Դ
  rt_free(RequestInfo);
	rt_free(mail);
	return Result;
}

/*
����:����һ���ļ���������
����:type �ļ�����  time����ʱ��  file �ļ�����
*/
#define FILE_PACKNUM_MAX      3
static rt_int8_t send_file_process(rt_uint8_t Type,rt_uint32_t Time,char *FileName)
{
	rt_uint32_t FileSize;			 //�ļ���С
	rt_uint32_t CurPackOrder;	 //�����
	rt_uint16_t PackNum;       //������
	rt_int8_t   Result;        //ִ�н��
	rt_uint8_t  SendOk;     //����״̬
	rt_uint16_t ReadSize;      //��ȡ��ǰ���Ĵ�С
	rt_sem_t    SendSem = RT_NULL; //���Ϳ����ź���
  net_msgmail_p mail[FILE_PACKNUM_MAX];     

	//list_mem();
	//�����ļ�������
	Result = send_file_request(FileName);
	//list_mem();
	if(Result < 0)
	{
    return -1;
	}
  init_net_msgmail(mail,FILE_PACKNUM_MAX);	
	//���Ϳ����ź���
	SendSem = rt_sem_create("sendpic",FILE_PACKNUM_MAX,RT_IPC_FLAG_FIFO);
	if(SendSem == RT_NULL)
	{
		rt_kprintf("Send Semaphore Create Fail\n");
    rt_sem_delete(SendSem);//ɾ�����Ϳ����ź���
		return -1;
	}
	Result = get_file_size(FileName,&FileSize);
	if(Result < 0)
	{
    rt_sem_delete(SendSem);//ɾ�����Ϳ����ź���
		return -1;
	}
	PackNum = FileSize / NET_FILE_BUF_SIZE;
	if(FileSize % NET_FILE_BUF_SIZE != 0)
	{
		PackNum++;
	}
	RT_DEBUG_LOG(SHOW_NFILE_SEND,("File Of PackNum = %d\n",PackNum));
	ReadSize = NET_FILE_BUF_SIZE;//�������Ĵ�С
	CurPackOrder = 0;
	SendOk = 0;
	while(SendOk < PackNum)
	{
		rt_err_t result;
		rt_int8_t NetMailPos = 0;
		rt_uint8_t RecvResult = 0;

		rt_thread_delay(10);
		if(CurPackOrder < PackNum)
		{
			result = rt_sem_take(SendSem,10);
			if(result == RT_EOK)
			{	
				if((CurPackOrder +1) == PackNum)
				{
					if(FileSize % NET_FILE_BUF_SIZE != 0)
					{
						ReadSize = FileSize % NET_FILE_BUF_SIZE;
					}
				}
				NetMailPos = find_null_msgmail(mail,FILE_PACKNUM_MAX);
				if(NetMailPos < 0)
				{
					rt_kprintf("NetMailPos = %d Value Error\n",NetMailPos);
					break;
				}
				mail[NetMailPos] = rt_calloc(1,sizeof(net_msgmail));
				RT_ASSERT(mail[NetMailPos] != RT_NULL);
				send_file_pack(FileName,CurPackOrder,ReadSize,mail[NetMailPos]);
				CurPackOrder++;
			}
		}
		
		//��ѯǰ�����Ƿ�ɹ�
		RecvResult = check_msgmail_succeed(mail,FILE_PACKNUM_MAX,SendSem);
		if(RecvResult != 0xff)
		{
      SendOk += RecvResult;
		}
		else
		{
			RT_DEBUG_LOG(SHOW_NFILE_SEND,("RecvResult:%d\n",RecvResult));
			RT_DEBUG_LOG(SHOW_NFILE_SEND,("Send File Outtime\n"));
			//�ͷ���Դ
			destroy_net_msgmail(mail,FILE_PACKNUM_MAX);
			rt_sem_delete(SendSem);
			return -1;
		}
	}
	RT_DEBUG_LOG(SHOW_NFILE_SEND,("Send File Complete\n"));
	
	//�ͷ���Դ
	destroy_net_msgmail(mail,FILE_PACKNUM_MAX);
	rt_sem_delete(SendSem);
	return 0;
}

/*
����:�ļ������߳�
*/
void net_file_entry(void *arg)
{
	if(arg != RT_NULL)
	{
		send_file_process(1,1,(char *)arg);
	}
	else
	{
    send_file_process(1,1,"/FAVICON.ICO");
	}
}


#ifdef RT_USING_FINSH
#include <finsh.h>

void send_file(char *FileName)
{
	rt_thread_t thread_id;

	thread_id = rt_thread_create("netfile",
															net_file_entry,(void *)FileName,
                         			1024, 104, 20);
  RT_ASSERT(thread_id != RT_NULL);
  rt_thread_startup(thread_id);
}
FINSH_FUNCTION_EXPORT(send_file,"(FileBName) send test file");


void fileack(rt_uint8_t startcol,rt_uint8_t num)
{
	net_encrypt data;
	net_msgmail msg;
	net_message message;
	rt_uint8_t i ;
	net_filedat_ack *file;
	rt_uint32_t  col= 0;

	file = rt_calloc(1,sizeof(net_filedat_ack));

	rt_kprintf("Make File ACK Message:\n");
	for(i = 0 ;i < num; i++)
	{
		extern void net_set_message(net_encrypt_p msg_data,net_msgmail_p MsgMail);
		extern void net_pack_data(net_message *message,net_encrypt *data);
		
		msg.type = NET_MSGTYPE_FILEDATA_ACK;
		msg.col.bit.col = startcol++;
		col++;
		net_uint32_copy_string(file->order,col);
		file->result = 0;
		msg.user = file;
		net_set_message(&data,&msg);//���ñ�����Ϣ׼�����
		
		net_pack_data(&message,&data);//���		
	}

}
FINSH_FUNCTION_EXPORT(fileack,"Make File ACK Message");

void read3fle(void)
{
	int file;
	rt_uint8_t data;

	file = open("/3.bmp",O_RDONLY,0x777);
	while(1)
	{
		if(read(file,&data,1) == 0)
		{
			break;
		}
		else
		{
			rt_kprintf("%02X",data);
		}
	}
	close(file);
}
FINSH_FUNCTION_EXPORT(read3fle,"Make File ACK Message");

void resetcol(void)
{
	net_order.byte = 0;
	net_order.bit.col = 1;
}
FINSH_FUNCTION_EXPORT(resetcol,"test file send");


void createfile(void)
{
	rt_uint32_t i;
	rt_uint8_t  data;
	rt_uint8_t pos;
	int file;

	file = open("/test.txt",O_CREAT|O_RDWR,0x777);
	for(i = 0;i < 1024*4 ;i++)
	{
		data = i;
		data = data%128+0x30;
		if(data == '0')
		{
			write(file,"\n\r",2);
			pos++;
			data = pos+0x30;
			write(file,&data,1);
		}
		else
		{
      write(file,&data,1);
		}
		
	}
	close(file);
}
FINSH_FUNCTION_EXPORT(createfile,"createfile");

void FileShowHex(const char *filename)
{
	int id;
	rt_uint8_t data;

	id = open(filename,O_RDONLY,0x777);
	if(id < 0)
	{
		return ;
	}
	while(1)
	{
		if(read(id,&data,1) == 1)
		{
      rt_kprintf("%02X");
		}
		else
		{
			break;
		}
	}
	close(id);
}
FINSH_FUNCTION_EXPORT(FileShowHex,"HEX Show File");

void testcrc32(void)
{
	rt_uint32_t crc32;
	
	file_get_crc32("/misc.c",&crc32);
	rt_kprintf("crc32: %X\n",crc32);
}
FINSH_FUNCTION_EXPORT(testcrc32,"test misc.c file crc32");

#endif


