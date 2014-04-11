/**
功能:通过调用协议层完成文件的发送与接收
版本:v0.1
作者:wangzw <wangzw@yuettak.com>
*/
#include "netfile.h"
#include <cyg/crc/crc.h>
#include "crc16.h"

#define NET_MAIL_OK  0

#define FILE_RECV_MAX_TIME    10    //文件接收最长时间
#define SYS_APP_BIN_FILE_NAME "/app.bin"




/*
功能:发送文件报文邮件

*/
static void file_msg_mail_send(net_msgmail_p mail,rt_uint8_t *buffer,rt_uint16_t size)
{
  net_filedata_user *file;
  char SemName[RT_NAME_MAX];
  
  mail->time = 0;
  mail->type = NET_MSGTYPE_FILEDATA;
  mail->resend = 3;
  mail->outtime = 3000;
  mail->sendmode = SYNC_MODE;
  mail->col.byte = get_msg_new_order(RT_TRUE);
  
  mail->user = file = (net_filedata_user *)rt_calloc(1,sizeof(net_filedata_user));
  RT_ASSERT(file!=RT_NULL);
  file->data.data  = buffer;
  RT_ASSERT(file->data.data!=RT_NULL);
	
  file->length = size;//包大小
  rt_sprintf(SemName,"NF%d",get_msg_new_order(RT_FALSE));
  file->result.complete = rt_sem_create(SemName,0,RT_IPC_FLAG_FIFO);
  
  net_msg_send_mail(mail);
}

/*
功能:从文件中获得一个包的数据
参数:FileID    文件索引  
     PackOrder 包序号
     ReadSize  包大小
     buffer    数据存放地址
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
功能:计算文件包个数
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
功能:发送某一包文件
参数:FileName 文件名字
     PackOrder 包序号
返回值: 发送的结果
*/
static rt_int8_t send_file_pack(char *FileName,rt_uint32_t PackOrder,rt_uint16_t ReadSize,net_msgmail_p mail)
{
	int FileID;
	rt_uint8_t *buffer = RT_NULL;

	//分配内存
	buffer = (rt_uint8_t *)rt_calloc(1,ReadSize + 4);
	RT_ASSERT(buffer != RT_NULL);
	
	FileID = open(FileName,O_RDONLY,0x777);
	if(FileID < 0)
	{
		//打开文件失败
		return -1;
	}

	//获得一个包的数据
	get_file_pack_data(FileID,PackOrder,ReadSize,buffer);
	//发送一个包
	file_msg_mail_send(mail,buffer,ReadSize);
	//释放资源
	close(FileID);
	//rt_free(buffer);
	
	return 0;
}
/*
功能:初始化邮件队列
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
功能:删除队列中的某一个
*/
void delete_net_msgmail(net_msgmail_p mail[],rt_uint8_t pos)
{
	net_filedata_user *file;
	
	if(mail[pos] != RT_NULL)
	{
		if(mail[pos]->user != RT_NULL)
		{
			file = mail[pos]->user;
			if(file->data.data != RT_NULL)
			{
        rt_free(file->data.data);
        file->data.data = RT_NULL;
			}
			if(file->result.complete != RT_NULL)
			{
				rt_sem_delete(file->result.complete);
				file->result.complete = RT_NULL;
			}
			rt_free(mail[pos]->user);
			mail[pos]->user = RT_NULL;
		}
		rt_free(mail[pos]);
		mail[pos] = RT_NULL;
	}
}

/*
功能:销毁邮件队列
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
功能:发现一个空的邮件位置
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
功能:查询是否有成功的包
返回:1 成功 0失败
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
				result = rt_sem_take(file->result.complete,RT_WAITING_NO);
				if(result == RT_EOK)
				{	
					RT_DEBUG_LOG(SHOW_NFILE_SRESULT,("File Pack Send Result: %d\n",file->result.result));     
          if(file->result.result == NET_MAIL_OK)
          {
						RunResult++;
          }
          else //发送失败
          {
          	RunResult = 0xff;
					  return RunResult;
          }
          //释放资源 文件 信号量 邮件
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
功能:发送文件请求
*/
static rt_int8_t send_file_request(char *FileName)
{
	rt_int8_t   Result;        //执行结果
	rt_uint32_t FileSize;			 //文件大小
	rt_uint32_t PackNum;
	rt_uint32_t CRC32Value;
	net_msgmail_p mail = RT_NULL;
	net_filerequest_user *RequestInfo;

	//获取资源
	mail = (net_msgmail_p)rt_calloc(1,sizeof(net_msgmail));
	RT_ASSERT(mail != RT_NULL);
	RequestInfo = (net_filerequest_user *)rt_calloc(1,sizeof(net_filerequest_user));
	RT_ASSERT(RequestInfo != RT_NULL);
	
	Result = get_file_size(FileName,&FileSize);
	if(Result < 0)
	{
		//释放内存资源
		rt_free(RequestInfo);
		rt_free(mail);
		return -1;
	}
	mail->type = NET_MSGTYPE_FILEREQUEST;
  mail->time = 0;
  mail->resend = 3;
  mail->outtime = 600;
  mail->sendmode = SYNC_MODE;//同步
  mail->col.byte = net_order.byte;
  net_order.bit.col++;
  
  mail->user = RequestInfo;
	//计算包数量
	PackNum = get_file_packets(FileSize,NET_FILE_BUF_SIZE);
	
	RequestInfo->file.alarm = 0;  //报警类型
	net_uint32_copy_string(RequestInfo->file.time,1234);//时间
	RequestInfo->file.type = 1;    //文件格式
	net_uint32_copy_string(RequestInfo->file.size,FileSize);//文件大小
	RequestInfo->file.packsize = 4;//包大小512k
	net_uint32_copy_string(RequestInfo->file.packnum,PackNum);//包数量
	//计算CRC32
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
  //释放内存资源
  rt_free(RequestInfo);
	rt_free(mail);
	return Result;
}

/*
功能:发送一个文件到服务器
参数:type 文件类型  time发送时间  file 文件名称
*/
#define FILE_PACKNUM_MAX      NET_RECV_MSG_MAX-2
static rt_int8_t send_file_process(rt_uint8_t Type,rt_uint32_t Time,char *FileName)
{
	rt_uint32_t FileSize;			 //文件大小
	rt_uint32_t CurPackOrder;	 //包序号
	rt_uint16_t PackNum;       //包数量
	rt_int8_t   Result;        //执行结果
	rt_uint8_t  SendOk;     //运行状态
	rt_uint16_t ReadSize;      //读取当前包的大小
	rt_sem_t    SendSem = RT_NULL; //发送控制信号量
  net_msgmail_p mail[FILE_PACKNUM_MAX];     

	//list_mem();
	//发送文件请求报文
	Result = send_file_request(FileName);
	//list_mem();
	if(Result < 0)
	{
    return -1;
	}
  init_net_msgmail(mail,FILE_PACKNUM_MAX);	
	//发送控制信号量
	SendSem = rt_sem_create("sendpic",FILE_PACKNUM_MAX,RT_IPC_FLAG_FIFO);
	if(SendSem == RT_NULL)
	{
		rt_kprintf("Send Semaphore Create Fail\n");
    rt_sem_delete(SendSem);//删除发送控制信号量
		return -1;
	}
	Result = get_file_size(FileName,&FileSize);
	if(Result < 0)
	{
    rt_sem_delete(SendSem);//删除发送控制信号量
		return -1;
	}
	PackNum = FileSize / NET_FILE_BUF_SIZE;
	if(FileSize % NET_FILE_BUF_SIZE != 0)
	{
		PackNum++;
	}
	RT_DEBUG_LOG(SHOW_NFILE_SEND,("File Of PackNum = %d\n",PackNum));
	ReadSize = NET_FILE_BUF_SIZE;//正常包的大小
	CurPackOrder = 0;
	SendOk = 0;
	while(SendOk < PackNum)
	{
		rt_err_t result;
		rt_int8_t NetMailPos = 0;
		rt_uint8_t RecvResult = 0;

		if(CurPackOrder < PackNum)
		{
			result = rt_sem_take(SendSem,2);
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
		
		//查询前三条是否成功
		RecvResult = check_msgmail_succeed(mail,FILE_PACKNUM_MAX,SendSem);
		if(RecvResult != 0xff)
		{
      SendOk += RecvResult;
      if(RecvResult > 0)
      {
				rt_kprintf("sendok = %d",SendOk);
      }
		}
		else
		{
			RT_DEBUG_LOG(SHOW_NFILE_SEND,("RecvResult:%d\n",RecvResult));
			RT_DEBUG_LOG(SHOW_NFILE_SEND,("Send File Outtime\n"));
			//释放资源
			destroy_net_msgmail(mail,FILE_PACKNUM_MAX);
			rt_sem_delete(SendSem);
			return -1;
		}
	}
	RT_DEBUG_LOG(SHOW_NFILE_SEND,("Send File Complete\n"));
	
	//释放资源
	destroy_net_msgmail(mail,FILE_PACKNUM_MAX);
	rt_sem_delete(SendSem);
	return 0;
}

/*
功能:文件发送线程
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

/**
		文件接收处理
*/
//收到的包描述结构
typedef struct 
{
	rt_uint8_t *buf;   //标记包的数组
	rt_size_t  size;   //数组大小
	rt_size_t  bitmax; //位数的最大值
}FilePackPosMap;

//文件接收描述结构
typedef struct
{
	FilePackPosMap *PackMap;
	//rt_size_t      CurPackNum; //当前收到的包数量
	rt_size_t      FileSize;   //文件大小
	rt_uint8_t     PackSize;   //包大小
	rt_size_t      PackNum;    //包数量
	rt_uint32_t    CRC32;      //crc32
}NetFileInfo,*NetFileInfo_p;

//接收文件时使用的定时器描述
typedef struct
{
	rt_timer_t timer;
	rt_uint8_t cnt;
}NetFileTimer;
//接收文件时用于记录文件信息
static NetFileInfo_p NetRecvFileInfo = RT_NULL;
static NetFileTimer FileRecvTimer = {RT_NULL,0};


/*
功能:设置文件信息结构
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
功能:文件包数据是否接收完成
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
功能:创建一个NetFileInfo实体
返回:NetFileInfo实体的地址
*/
static NetFileInfo_p net_fileinfo_create(rt_size_t PackNum)
{
	NetFileInfo_p FileInfo;
	
	FileInfo = rt_calloc(1,sizeof(NetFileInfo));
	RT_ASSERT(FileInfo != RT_NULL);
	FileInfo->PackMap = file_pack_pos_create(PackNum);
	RT_ASSERT(FileInfo->PackMap != RT_NULL);
	
	return FileInfo;
}

/*
功能:删除NetFileInfo实体
*/
static void net_fileinfo_delete(NetFileInfo_p FileInfo)
{
	file_pack_pos_delete(FileInfo->PackMap);
	rt_free(FileInfo);
}

static void net_file_timer_handler(void *arg)
{
	FileRecvTimer.cnt++;
	if(FileRecvTimer.cnt >= FILE_RECV_MAX_TIME)
	{
		rt_timer_stop(FileRecvTimer.timer);
	}
	rt_kprintf("entry timer\n");
}
void net_file_timer_process(void)
{
	if(FileRecvTimer.cnt >= FILE_RECV_MAX_TIME)
	{
		FileRecvTimer.cnt = 0;
		rt_timer_delete(FileRecvTimer.timer);
		net_event_process(2,NET_ENVET_FILERQ);
		net_fileinfo_delete(NetRecvFileInfo);
		NetRecvFileInfo = RT_NULL;
		rt_kprintf("delete file timer\n");
	}
}

static void net_file_timer_del(void)
{
	FileRecvTimer.cnt = 0;
	rt_timer_stop(FileRecvTimer.timer);
	rt_timer_delete(FileRecvTimer.timer);
	FileRecvTimer.timer = RT_NULL;
}
static void net_file_timer_clear(void)
{
	FileRecvTimer.cnt = 0;
}
/*
功能:处理文件请求
返回: 0 成功 1 失败
*/
rt_uint8_t net_recv_filerq_process(net_recvmsg_p mail)
{	
	volatile rt_uint8_t result = 1;
	rt_uint32_t  PackNum = 0;
	
	if(net_event_process(1,NET_ENVET_FILERQ) == 0)
	{
		rt_kprintf("Being Receive File\n");
		return 1;
	}
	//不同文件类型保存为不同名字
	if(mail->data.filerq.request.type == 2)
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
      //标志在接收文件状态
      net_event_process(0,NET_ENVET_FILERQ);
      if(NetRecvFileInfo != RT_NULL)
      {
        rt_kprintf("NetRecvFileInfo Data Abnormal\n\n");
        net_fileinfo_delete(NetRecvFileInfo);
      }
      //文件信息获取
      net_string_copy_uint32(&PackNum,mail->data.filerq.request.packnum);
      NetRecvFileInfo = net_fileinfo_create(PackNum);
      net_fileinfo_set(NetRecvFileInfo,mail);
      //创建定时器
			FileRecvTimer.timer = rt_timer_create("filerecv",
																						net_file_timer_handler,
																						RT_NULL,6000,
																						RT_TIMER_FLAG_PERIODIC);
			RT_ASSERT(FileRecvTimer.timer != RT_NULL);
			rt_timer_start(FileRecvTimer.timer);
			
      result = 0;
		}
		
	}
	return result;
}

//rt_uint8_t filebuffer[3*1024];
rt_uint8_t net_file_packdata_process(net_recvmsg_p mail)
{
	rt_uint8_t result = 1;
	int fileid ;
	rt_uint16_t PackSizeRmap[6] = {64,128,256,0,512,1024};
	
	//如果是文件包接收状态
	if(net_event_process(1,NET_ENVET_FILERQ) == 0)
	{
		rt_uint32_t CurWritePos;
		
		fileid = open(SYS_APP_BIN_FILE_NAME,O_RDWR,0x777);
		if(fileid < 0)
		{
			//接收数据异常
			rt_kprintf("Receive data anomalies\n");
			result = 2;
		}
		net_string_copy_uint32(&CurWritePos,mail->data.filedata.pos);
		lseek(fileid,CurWritePos*PackSizeRmap[NetRecvFileInfo->PackSize],DFS_SEEK_SET);
		rt_kprintf("write size %d\n",write(fileid,mail->data.filedata.data,mail->lenmap.bit.data-4));
		close(fileid);
		net_file_timer_clear();
		/*rt_memcpy(filebuffer+CurWritePos*PackSizeRmap[NetRecvFileInfo->PackSize],
							mail->data.filedata.data,mail->lenmap.bit.data-4);*/
							
		file_pack_pos_add(NetRecvFileInfo->PackMap,CurWritePos);
		rt_kprintf("Recv ok Num:%d\n",file_pack_complete(NetRecvFileInfo->PackMap));
		if(file_pack_complete(NetRecvFileInfo->PackMap) != -1)
		{
			rt_uint32_t crc32;

			//rt_kprintf("RAM FILE crc32:%x\n",cyg_ether_crc32(filebuffer,NetRecvFileInfo->FileSize));
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
			net_file_timer_del();
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

/** 
@brief net upload file
@param FileName :file of name 
@retval RT_EOK	 :anon_upload_enableyes
@retval RT_ERROR :can‘nt upload file
*/
rt_err_t net_upload_file(char *FileName)
{
  rt_thread_t thread_id;

	thread_id = rt_thread_find("Upload");
  if(thread_id != RT_NULL)
  {
		return RT_ERROR;
  }
  
	thread_id = rt_thread_create("Upload",
	                            net_file_entry,
	                            (void *)FileName,
	                            1024,
	                            111, 
	                            20);
	                            
	RT_ASSERT(thread_id != RT_NULL);
	rt_thread_startup(thread_id);

	return RT_EOK;
}

#ifdef RT_USING_FINSH
#include <finsh.h>

void send_file(char *FileName)
{
	rt_thread_t thread_id;

	thread_id = rt_thread_create("Upload",
															net_file_entry,(void *)FileName,
                         			1024,111, 20);
  RT_ASSERT(thread_id != RT_NULL);
  rt_thread_startup(thread_id);
}
FINSH_FUNCTION_EXPORT(send_file,"(FileBName) send test file");

void netfiletest(void)
{
	while(1)
	{
		send_file("\1.jpg");
	}
}
FINSH_FUNCTION_EXPORT(netfiletest,"test net file send");

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
		net_set_message(&data,&msg);//设置报文信息准备打包
		
		net_pack_data(&message,&data);//打包		
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
	rt_kprintf("sizeof = %d\n",sizeof(message_type));
}
FINSH_FUNCTION_EXPORT(testcrc32,"test misc.c file crc32");

FINSH_FUNCTION_EXPORT(testfp,"(MSGType)Send Ack Data Is NULL Message");

#endif


