/**
����:����Ӳ���㴦��
�汾:0.1
����:wangzw@yuettak.com
*/
#include "netphysics.h"


#define TCP_BUF_SIZE       1024 //���ջ�����


static char server_ip[30] = "NULL";
static rt_uint32_t server_port = 5200;

/*
����:��һ������buffer�з���һ�����İ�
����:buffer ����buffer��ַ  size �������ݴ�С
*/
rt_size_t find_package_end(rt_uint8_t *buffer,rt_size_t size)
{
  rt_size_t i;
  rt_uint8_t FlagStr[2];
  
  for(i = 0; i < size; i++)
  {
    FlagStr[0] = FlagStr[1];
    FlagStr[1] = buffer[i];
    if((FlagStr[0] == 0x0d) && (FlagStr[1] == 0x0a))
    {
      return i+1;
    }
  }
  return 0;
}


/*
����:���������ӿ��̴߳�����
*/
void netprotocol_thread_entry(void *arg)
{ 
  char *recv_data;
  rt_size_t SavePos = 0;   //�������ݱ���λ��
  rt_size_t bytenum = 0;   //�յ��ֽ����ܺ�
  rt_size_t MsgEndPos = 0; //һ�����Ľ�����λ��
  struct hostent *host;
  int sock, bytes_received;
  struct sockaddr_in server_addr;

	rt_kprintf("please set server IP and Port\n");
  while(1)
  {
		if(server_ip[0] == 'N' &&
		server_ip[1] == 'U' &&
		server_ip[2] == 'L' &&
		server_ip[3] == 'L')
		{
			rt_thread_delay(10);
		}
		else
		{
			break;
		}
  }
  
  
  while(1)
  {
    //host = gethostbyname("192.168.1.6");  //��������
    host = gethostbyname(server_ip);  //��������
    
    recv_data = rt_calloc(1,TCP_BUF_SIZE);
    if (recv_data == RT_NULL)
    {
        rt_kprintf("No memory\n");
        return;
    }
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        rt_kprintf("Socket error\n");
        rt_free(recv_data);
        return;
    }
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    server_addr.sin_addr = *((struct in_addr *)host->h_addr);
    rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));
    
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
        rt_kprintf("Connect fail!\n");
        closesocket(sock);
        rt_free(recv_data);
        continue;
    }
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
				rt_kprintf("relink TCP/IP\n");
				closesocket(sock);
				rt_free(recv_data);
				break;
    	}
    	
    	//��������
      bytes_received = recv(sock,recv_data+SavePos,TCP_BUF_SIZE - (1+SavePos), MSG_DONTWAIT);
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
          	rt_memcpy(recvmail,recv_data,MsgEndPos);
          	RT_DEBUG_LOG(SHOW_RECV_MAIL_ADDR,("Send mailbox addr %X\n",recvmail));
          	if(rt_mb_send(net_datrecv_mb,(rt_uint32_t)recvmail) != RT_EOK)
          	{
          	  rt_kprintf("send mailbox fail\n");
              rt_free(recvmail);
          	}
          	rt_kprintf("\nReceives the encrypted data:\n");
            for(i = 0;i < MsgEndPos;i++)
            {
              rt_kprintf("%02X",recv_data[i]);
            }
            rt_kprintf("\n");

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
        int result = 0;

        if(message.buffer != RT_NULL)
        {
          result = send(sock,(void *)message.buffer,message.length+4, 0);
        }
        rt_sem_release(message.sendsem);
        if(result <= 0)
        {
          rt_kprintf("send fail\n");
          closesocket(sock);
          rt_free(recv_data);
          break;
        } 
      }
    }
  }
}



int netprotocol_thread_init(void)
{
  rt_thread_t id = RT_NULL;

  id = rt_thread_create("NPDU",
                         netprotocol_thread_entry, RT_NULL,
                         512, 28, 20);

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

void set_server(char* ip,rt_uint32_t port)
{
	rt_strncpy(server_ip,ip,rt_strlen(ip));
	server_port = port;
}
FINSH_FUNCTION_EXPORT(set_server,"(ip,port) set server name or IP address and port");

void netsever(void)
{
	set_server("192.168.1.107",5200);
	
}
FINSH_FUNCTION_EXPORT(netsever,"test file send");

#endif


