/**
功能:网络硬件层处理
版本:0.1
作者:wangzw@yuettak.com
*/
#include "netphysics.h"
#include "netmailclass.h"
#include "bdcom.h"
#include "comm.h"


#define TCP_BUF_SIZE       1024 //接收缓冲区

/*
功能:在一个数据buffer中发现一个报文包
参数:buffer 数据buffer地址  size 现有数据大小
*/
rt_size_t find_package_end(rt_uint8_t *buffer,rt_size_t size)
{
  rt_size_t i;
  rt_uint8_t FlagStr[2];
  rt_uint16_t length;

  net_string_copy_uint16(&length,buffer);
  rt_kprintf("pack length = %d %x\n",length,length);

	if(length >= TCP_BUF_SIZE)//长度大于缓冲区长度
	{
		return size;
	}
	
  for(i = 0; i < size; i++)
  {
    FlagStr[0] = FlagStr[1];
    FlagStr[1] = buffer[i];
    if((FlagStr[0] == 0x0d) && (FlagStr[1] == 0x0a))
    {
    	if(length < i+1)
      return i+1;
    }
  }
  if(i > length)//长度错误
  {
		return size;
  }
  return 0;
}


/*
功能:网络物理层接口线程处理函数
*/
void netprotocol_thread_entry(void *arg)
{ 
  rt_uint8_t *recv_data;
  rt_size_t SavePos = 0;   //最新数据保存位置
  //rt_size_t bytenum = 0;   //收到字节数总和
  rt_size_t MsgEndPos = 0; //一条报文结束的位置
  rt_size_t bytes_received;
  while(1)
  {
    recv_data = rt_calloc(1,TCP_BUF_SIZE);
    if (recv_data == RT_NULL)
    {
        rt_kprintf("No memory\n");
        return;
    }
    while(!gsm_is_link())
    {
    	GSM_Mail_p mail;

			mail = (GSM_Mail_p)rt_calloc(1,sizeof(GSM_Mail));
			mail->ResultSem = rt_sem_create("gsmmail",0,RT_IPC_FLAG_FIFO);
			RT_ASSERT(mail->ResultSem != RT_NULL);
			mail->buf = RT_NULL;
			mail->BufSize = 0;
			mail->SendMode = 1;
			mail->type = GSM_MAIL_LINK;
			gsm_mail_send(mail);
			rt_sem_take(mail->ResultSem,RT_WAITING_FOREVER);
			rt_sem_delete(mail->ResultSem);
			rt_free(mail);
			rt_thread_delay(100);
			{
	      int  i;
	    	
	    	bytes_received = comm_recv_gprs_data(recv_data,TCP_BUF_SIZE);
	    	for(i = 0;i < bytes_received;i++)
		    {
		      rt_kprintf("%c",recv_data[i]);
		    }
				rt_thread_delay(100);
    	}
    }
    #if 0
    while(1)
    {
      int  i;
    	
    	bytes_received = comm_recv_gprs_data(recv_data,TCP_BUF_SIZE);
    	for(i = 0;i < bytes_received;i++)
	    {
	      rt_kprintf("%02X",recv_data[i]);
	    }
			rt_thread_delay(100);
    }
    #endif
		//连接成功开始登陆 
		send_net_landed_mail();
    while(1)
    {
      int mq_result;
      net_message message;
      net_recvmsg_p recvmail;

			//如果收到重新连接事件
    	if(net_event_process(2,NET_ENVET_RELINK) == 0)
    	{
				rt_kprintf("relink TCP/IP\n");
				gsm_set_link(0);
				rt_free(recv_data);
				break;
    	}
    	
    	//接收数据
      bytes_received = comm_recv_gprs_data(recv_data+SavePos,TCP_BUF_SIZE - (1+SavePos));
      if(bytes_received > 0)
      {
      	SavePos += bytes_received;
        while(1)
        {
          //找包尾
          MsgEndPos = find_package_end((rt_uint8_t *)recv_data,SavePos);
          if(MsgEndPos > 0)
          {
            rt_uint16_t i;
            
            recvmail = (net_recvmsg_p)rt_calloc(1,sizeof(net_recvmsg));
            RT_ASSERT(recvmail != RT_NULL);
            if(MsgEndPos <= sizeof(net_recvmsg))
            {
              rt_memcpy(recvmail,recv_data,MsgEndPos);
            }
          	RT_DEBUG_LOG(SHOW_RECV_MAIL_ADDR,("Send mailbox addr %X\n",recvmail));
          	if(rt_mb_send(net_datrecv_mb,(rt_uint32_t)recvmail) != RT_EOK)
          	{
          	  rt_kprintf("send mailbox fail\n");
              rt_free(recvmail);
          	}

          	//打印调试信息
          	rt_kprintf("\nReceives the encrypted data:\n");
            for(i = 0;i < MsgEndPos;i++)
            {
              rt_kprintf("%02X",recv_data[i]);
            }
            rt_kprintf("\n");

            //将找到包的后面数据移动到buffer首地址处
            SavePos -= MsgEndPos;
            rt_memcpy(recv_data,recv_data+MsgEndPos,SavePos);
            //bytenum += MsgEndPos;
          }
          else
          {
            break;
          }
        }
      }
      
      mq_result = rt_mq_recv(net_datsend_mq,(void *)&message,sizeof(net_message),1);
      if(mq_result == RT_EOK)
      {
        if(message.buffer != RT_NULL)
        {
          GSM_Mail_p mail;

					mail = (GSM_Mail_p)rt_calloc(1,sizeof(GSM_Mail));
          mail->ResultSem = rt_sem_create("gsmmail",0,RT_IPC_FLAG_FIFO);
          RT_ASSERT(mail->ResultSem != RT_NULL);
          mail->buf = message.buffer;
          mail->BufSize = message.length+4;
          mail->SendMode = 1;
          mail->type = GSM_MAIL_GPRS;
          gsm_mail_send(mail);
          rt_sem_take(mail->ResultSem,RT_WAITING_FOREVER);
          rt_sem_delete(mail->ResultSem);
          rt_free(mail);
        }
        rt_sem_release(message.sendsem);
      }
    }
  }
}



int netprotocol_thread_init(void)
{
  rt_thread_t id = RT_NULL;

  id = rt_thread_create("NPDU",
                         netprotocol_thread_entry, RT_NULL,
                         512,109, 20);

  if(id == RT_NULL)
  {
    rt_kprintf("NPDU thread init fail !\n");

    return 1;
  }

  rt_thread_startup(id);
  return 0;
}
INIT_APP_EXPORT(netprotocol_thread_init);


#ifdef RT_USING_FINSH
#include <finsh.h>


#endif


