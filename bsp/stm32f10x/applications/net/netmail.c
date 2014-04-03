/**
功能:填充并且转发网络报文
版本:0.1
作者:wangzw@yuettak.com
*/
#include "netmail.h"

static rt_mq_t NetMsg_mq = RT_NULL;



static void net_mail_process(NetMsg_Mail_p mail)
{
	switch(mail->MsgType)
	{
		case MSGTYPE_WORK_ALARM:
		{
			msg_mail_alarm(0,mail->data.alarm.LockStatus,mail->time);
			break;
		}
		case MSGTYPE_FAULT_ALARM:
		{
			break;
		}
		case MSGTYPE_NULL_ACK:
		{
			msg_null_ack(mail->data.NullACK.MSGType);
			break;
		}
		default:
		{
			break;
		}
	}
}

/*
功能:线程入口
*/
void net_mail_manage_entry(void* arg)
{
	rt_err_t mq_result;
	NetMsg_Mail mail;
	
	rt_kprintf("mail manage thread run\n");
	while(1)
	{
		mq_result = rt_mq_recv(NetMsg_mq,&mail,sizeof(NetMsg_Mail),100);
		if(mq_result == RT_EOK)
		{
			net_mail_process(&mail);
		}
	}
}

/*
功能:创建邮件管理线程
*/
int net_mail_manage_init(void)
{
	rt_thread_t id;

	NetMsg_mq = rt_mq_create("NetMsg",sizeof(NetMsg_Mail),10,RT_IPC_FLAG_FIFO);
	RT_ASSERT(NetMsg_mq != RT_NULL);
	
  //设置接收处理函数
	Net_Set_MsgRecv_Callback(net_message_recv_process);
	
	id = rt_thread_create("netmail",
                         net_mail_manage_entry, RT_NULL,
                         256, 27, 20);

  if(id == RT_NULL)
  {
    rt_kprintf("netmail thread init fail !\n");

    return 1;
  }

  rt_thread_startup(id);

  return 0;
}
INIT_APP_EXPORT(net_mail_manage_init);



void NetMsg_Mail_Send(NetMsg_Mail_p data)
{
	NetMsg_Mail mail;
	
	if(data == RT_NULL)
	{
		rt_kprintf("NetMsg_Mail Is NULL\N");
	}	

	mail = *data;
	
	rt_mq_send(NetMsg_mq,&mail,sizeof(NetMsg_Mail));
}

