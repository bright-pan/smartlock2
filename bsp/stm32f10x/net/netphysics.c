/**
功能:网络硬件层处理
版本:0.1
作者:wangzw@yuettak.com
*/
#include "netphysics.h"
#include "netmailclass.h"
//#include "bdcom.h"
//#include "comm.h"
//#include "appconfig.h"
#ifdef   USEING_RAM_DEBUG
#include "untils.h" //主要使用里面的 rt_dprintf
#endif

#ifndef USEING_RAM_DEBUG
#define rt_dprintf    RT_DEBUG_LOG
#endif

#define NETPY_DEBUG_THREAD 21
#define SHOW_PRINTF_INFO   0    //打印调试信息
#define SHOW_STATUS_INFO   1

#define TCP_BUF_SIZE       1024 //接收缓冲区

/*
功能:在一个数据buffer中发现一个报文包
参数:buffer 数据buffer地址  size 现有数据大小
*/
rt_size_t find_package_end(rt_uint8_t *buffer,rt_size_t size)
{
  rt_size_t i;
  rt_uint8_t FlagStr[2] = {0,0};
  rt_uint16_t length;

  net_string_copy_uint16(&length,buffer);

	if(length >= TCP_BUF_SIZE)//长度大于缓冲区长度
	{
		rt_kprintf("Recv message length(%d) abnormal is bad message ! ! !\n",length);
		return size;
	}
	rt_dprintf(NET_MSG_PACK_RECV,("Receive the packet length:%d = 0X%x\n",length,length));
	/*#ifdef 0
	{
    rt_uint8_t i;
		for(i = 0;i < length;i++)
    {
      rt_dprintf(NET_RECV_MSG_INFO,("%02X",buffer[i]));
    }
    rt_dprintf(NET_RECV_MSG_INFO,("\n"));
	}
	#endif*/
  for(i = 0; i < size; i++)
  {
    FlagStr[0] = FlagStr[1];
    FlagStr[1] = buffer[i];
    rt_dprintf(NET_MSG_PACK_RECV,("[find:0x%02X 0x%02X]\n",FlagStr[0],FlagStr[1]));
    if((FlagStr[0] == 0x0d) && (FlagStr[1] == 0x0a))
    {
    	if(length <= i+1)
    	{
    		rt_dprintf(NET_MSG_PACK_RECV,("recv message succeed (length:%d)\n",size));
        return i+1;
    	}
    }
  }
  if(i > length+4)//长度错误
  {
  	rt_kprintf("recv size error2\n");
		return size;
  }
  
  return 0;
}

//检测物理连接是否在线
rt_err_t netprotocol_connect_status(void)
{
	rt_device_t dev;
	volatile rt_uint8_t  status;
	
	dev = rt_device_find("Blooth");
	if(dev == RT_NULL)
	{
		rt_kprintf("Blooth open fail \n");
	}
	if(!(dev->open_flag & RT_DEVICE_OFLAG_OPEN))
	{
	  rt_dprintf(NETPY_DEBUG_THREAD,("open blooth module\n"));
	  rt_device_open(dev,RT_DEVICE_OFLAG_OPEN);
	}
	rt_device_control(dev,3,(void *)&status);

	if(status == 1)
	{
		return RT_EOK;
	}

	return RT_ERROR;
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
  volatile rt_uint32_t ClearBufTime = 0;
  rt_device_t hw_dev = RT_NULL;
   
  while(1)
  {
  	if(hw_dev == RT_NULL)
  	{
			hw_dev = rt_device_find("Blooth");
			if(hw_dev == RT_NULL)
			{
				rt_kprintf("net hw_dev is RT_NULL\n");
			  return ;
			}
			if(!(hw_dev->open_flag & RT_DEVICE_OFLAG_OPEN))
			{
			  rt_dprintf(NETPY_DEBUG_THREAD,("open %s device\n",hw_dev->parent.name));
			  rt_device_open(hw_dev,RT_DEVICE_OFLAG_RDWR);
			}
  	}
    recv_data = rt_calloc(1,TCP_BUF_SIZE);

		RT_ASSERT(recv_data != RT_NULL);

		//请求连接阶段
		//net_event_process(0,NET_ENVET_CONNECT);
    //net_event_process(2,NET_ENVET_CONNECT);
    while(1)
    {
      int mq_result;
      net_message message;
      net_recvmsg_p recvmail;

			//如果收到重新连接事件
    	if(net_event_process(2,NET_ENVET_RELINK) == 0)
    	{
				rt_dprintf(SHOW_STATUS_INFO,("relink TCP/IP !!!!\n"));

				//清除所有登陆报文
				clear_wnd_cmd_all(NET_MSGTYPE_TIME);
				//rt_thread_delay(RT_TICK_PER_SECOND*3);
        send_net_landed_mail();
        RT_ASSERT(recv_data != RT_NULL);
				rt_free(recv_data);
				break;
    	}

    	//三次登陆失败
    	if(net_event_process(2,NET_ENVET_LOGINFAIL) == 0)
    	{
    		//断开蓝牙连接
    		rt_dprintf(SHOW_STATUS_INFO,("Login Fail Auto Disconnect!!!\n"));
				rt_device_control(hw_dev,5,RT_NULL);
    	}

			//接收数据
      bytes_received = rt_device_read(hw_dev,0,recv_data+SavePos,TCP_BUF_SIZE - (1+SavePos));
      //分析数据有效性
      if(bytes_received > 0)
      {      	
      	ClearBufTime = 0;
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
          	rt_dprintf(NET_RECV_MAIL_ADDR,("Send mailbox addr %X\n",recvmail));
          	if(rt_mb_send(net_datrecv_mb,(rt_uint32_t)recvmail) != RT_EOK)
          	{
          	  rt_dprintf(SHOW_PRINTF_INFO,
						          	  ("%s mail full send fail !!!\n",
						          	  net_datrecv_mb->parent.parent.name));
						          	  
              RT_ASSERT(recvmail != RT_NULL);
              rt_free(recvmail);
          	}

          	//打印调试信息
          	rt_dprintf(NET_RECV_ENC_DATA,("\nReceives encrypted data:\n<<<<<"));
            for(i = 0;i < MsgEndPos;i++)
            {
              rt_dprintf(NET_RECV_ENC_DATA,("%02X",recv_data[i]));
            }
            rt_dprintf(NET_RECV_ENC_DATA,("\n"));

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
      else
      {
				ClearBufTime++;
				
				if(ClearBufTime > 200)
				{
					ClearBufTime = 0;
          //清空时间
          rt_device_read(hw_dev,0,recv_data,TCP_BUF_SIZE);
					SavePos = 0;
					MsgEndPos = 0;
					//rt_kprintf("Clear net buf\n");
				}
      }

      //发送数据
      mq_result = rt_mq_recv(net_datsend_mq,(void *)&message,sizeof(net_message),1);
      if(mq_result == RT_EOK)
      {
        if(message.buffer != RT_NULL)
        {
        	if(SHOW_PRINTF_INFO == 1)
        	{
            rt_size_t i;
	          rt_uint8_t *buf = RT_NULL;

	          rt_dprintf(NETPY_DEBUG_THREAD,(">>>>>"));
	          buf = message.buffer;
						
	          for(i=0;i<message.length+4;i++)
	          {
	            rt_dprintf(NETPY_DEBUG_THREAD,("%02X",*(buf++)));
	          }
	          rt_dprintf(NETPY_DEBUG_THREAD,("\n"));
        	}
        	//rt_thread_delay(100);
          //发送
          rt_device_write(hw_dev,0,message.buffer,message.length+4);
        }
        rt_sem_release(message.sendsem);
      }

      //蓝牙链接断开
    	if(netprotocol_connect_status() == RT_ERROR)
    	{
    		if(net_event_process(1,NET_ENVET_CONNECT) == 1)
    		{
          //请求连接阶段
          net_event_process(0,NET_ENVET_CONNECT);
          rt_dprintf(SHOW_STATUS_INFO,("Blooth physics disconnect!!!\n"));
    		}
    		net_event_process(2,NET_ENVET_ONLINE);
    		//清除所有登陆报文
				clear_wnd_cmd_all(NET_MSGTYPE_LANDED);
				rt_thread_delay(1);

				//线程进入休眠
				rt_thread_entry_sleep(rt_thread_self());
				continue;
    	}
			else
			{
				if(net_event_process(1,NET_ENVET_CONNECT) == 0)
				{
					rt_dprintf(SHOW_STATUS_INFO,("Blooth physics connect^_^\n"));
					//请求连接成功
    			net_event_process(2,NET_ENVET_CONNECT);
    			net_event_process(0,NET_ENVET_RELINK);
				}
				rt_thread_delay(1);

				//线程进入工作
				rt_thread_entry_work(rt_thread_self());
			}
    }
  }
}



int netprotocol_thread_init(void)
{
  rt_thread_t id = RT_NULL;

  id = rt_thread_create("NPDU",
                         netprotocol_thread_entry, RT_NULL,
                         512,NPDU_THREAD_PRI_IS, 20);

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


