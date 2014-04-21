/**
����:����Ӳ���㴦��
�汾:0.1
����:wangzw@yuettak.com
*/
#include "netphysics.h"
#include "netmailclass.h"
#include "bdcom.h"
#include "comm.h"

#define SHOW_PRINTF_INFO   0    //��ӡ������Ϣ

#define TCP_BUF_SIZE       1024 //���ջ�����

/*
����:��һ������buffer�з���һ�����İ�
����:buffer ����buffer��ַ  size �������ݴ�С
*/
rt_size_t find_package_end(rt_uint8_t *buffer,rt_size_t size)
{
  rt_size_t i;
  rt_uint8_t FlagStr[2];
  rt_uint16_t length;

  net_string_copy_uint16(&length,buffer);
  RT_DEBUG_LOG(SHOW_PRINTF_INFO,("Receive the packet length:%d = %x!!\n",length,length));

	if(length >= TCP_BUF_SIZE)//���ȴ��ڻ���������
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
  if(i > length)//���ȴ���
  {
		return size;
  }
  return 0;
}


/*
����:���������ӿ��̴߳�����
*/
void netprotocol_thread_entry(void *arg)
{ 
  rt_uint8_t *recv_data;
  rt_size_t SavePos = 0;   //�������ݱ���λ��
  //rt_size_t bytenum = 0;   //�յ��ֽ����ܺ�
  rt_size_t MsgEndPos = 0; //һ�����Ľ�����λ��
  rt_size_t bytes_received;
  while(1)
  {
    recv_data = rt_calloc(1,TCP_BUF_SIZE);

		RT_ASSERT(recv_data != RT_NULL);

		net_event_process(0,NET_ENVET_CONNECT);
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
	    	
		   	RT_DEBUG_LOG(SHOW_PRINTF_INFO,("Invalid data is received gprs:\n"));
	    	for(i = 0;i < bytes_received;i++)
		    {
		      RT_DEBUG_LOG(SHOW_PRINTF_INFO,("%c",recv_data[i]));
		    }
		   	RT_DEBUG_LOG(SHOW_PRINTF_INFO,("\n"));
				rt_thread_delay(100);
    	}
    }
    net_event_process(2,NET_ENVET_CONNECT);
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
		//���ӳɹ���ʼ��½ 
		send_net_landed_mail();
    while(1)
    {
      int mq_result;
      net_message message;
      net_recvmsg_p recvmail;

			//����յ����������¼�
    	if(net_event_process(2,NET_ENVET_RELINK) == 0)
    	{
				RT_DEBUG_LOG(SHOW_PRINTF_INFO,("relink TCP/IP !!!!\n"));
				gsm_set_link(0);
				rt_free(recv_data);
				break;
    	}
    	
    	//��������
      bytes_received = comm_recv_gprs_data(recv_data+SavePos,TCP_BUF_SIZE - (1+SavePos));
      if(bytes_received > 0)
      {
      	SavePos += bytes_received;
        while(1)
        {
          //�Ұ�β
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
          	  RT_DEBUG_LOG(SHOW_PRINTF_INFO,
						          	  ("%s mail full send fail !!!\n",
						          	  net_datrecv_mb->parent.parent.name));
              rt_free(recvmail);
          	}

          	//��ӡ������Ϣ
          	RT_DEBUG_LOG(SHOW_RECV_MSG_INFO,("\nReceives the encrypted data:\n"));
            for(i = 0;i < MsgEndPos;i++)
            {
              RT_DEBUG_LOG(SHOW_RECV_MSG_INFO,("%02X",recv_data[i]));
            }
            RT_DEBUG_LOG(SHOW_RECV_MSG_INFO,("\n"));

            //���ҵ����ĺ��������ƶ���buffer�׵�ַ��
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
    rt_kprintf("%s thread init fail !\n",id->name);

    return 1;
  }

  rt_thread_startup(id);
  return 0;
}
INIT_APP_EXPORT(netprotocol_thread_init);


#ifdef RT_USING_FINSH
#include <finsh.h>


#endif


