/**
����:ʵ�ֱ����շ�Э��ջ����Ҫ���𽫱��������ʼ����Ϊ�����ı��Ĳ��ҷ��ͳ�ȥ
     ͬʱ���������������ı��ġ�
�汾:0.1v
*/
#include "netprotocol.h"
#include "crc16.h"
#include "des.h"
//#include "untils.h"
//#include "appconfig.h"

//���ͽ��
#define SEND_OK            0
#define SEND_FAIL          1

rt_mq_t 				net_msgmail_mq = RT_NULL;     //�����ʼ�
rt_mq_t 				net_datsend_mq = RT_NULL;     //Э��㷢�͸����������
rt_mailbox_t 		net_datrecv_mb = RT_NULL;			//��������
rt_event_t 			net_event = RT_NULL;       		//����Э�����¼�
rt_mutex_t 			net_wnd_mutex = RT_NULL;			//���細�ڻ�����

//���ʹ���
net_sendwnd 		sendwnd_node[NET_WND_MAX_NUM+1];
rt_timer_t  		sendwnd_timer = RT_NULL;//���ʹ��ڵĶ�ʱ��

//���
net_col 				net_order;	//���

//ID key0 key1
net_parameter NetParameterConfig = 
{
  {0x99,0x99,0x15,0x10,0x90,0x00,0x01,0x51},
  {0x12,0x34,0x56,0x78,0x90,0x12,0x34,0x51},
  {0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa}
};


void message_ASYN(rt_uint8_t type);

//�ʼ����մ�����
////////////////////////////////////////////////////////////////////////////////////////
typedef rt_uint8_t 
				(*msg_callback_type1)(net_recvmsg_p Mail,void *UserData);

typedef void 
				(*msg_callback_type2)(void);

msg_callback_type1	NetMsg_Recv_handle = RT_NULL;
msg_callback_type2	NetMsg_Recv_CallBack_Fun = RT_NULL;
msg_callback_type2	Net_Mail_Heart = RT_NULL;
msg_callback_type2  NetMsg_thread_init = RT_NULL;


void net_config_parameter_set(rt_uint8_t type,rt_uint8_t *data)
{
	RT_ASSERT(data != RT_NULL);
	switch(type)
	{
		case 1:
		{
			rt_memcpy(NetParameterConfig.id,(const void*)data,8);
			break;
		}
		case 2:
		{
			rt_memcpy(NetParameterConfig.key0,(const void*)data,8);
			break;
		}
		case 3:
		{
			rt_memcpy(NetParameterConfig.key1,(const void*)data,8);
			break;
		}
		default:
		{
			rt_kprintf("Net parameter type error\n");
			break;
		}
	}
}

/*
����:���Ľ��յĻص�����
*/
void Net_Set_MsgRecv_Callback(rt_uint8_t (*Callback)(net_recvmsg_p Mail,void *UserData))
{
	if(Callback != RT_NULL)
	{
		NetMsg_Recv_handle = Callback;
	}
}

void Net_NetMsg_thread_callback(void (*Callback)(void))
{
	if(Callback != RT_NULL)
	{
		NetMsg_Recv_CallBack_Fun = Callback;
	}
}

void Net_Mail_Heart_callback(void (*Callback)(void))
{
	if(Callback != RT_NULL)
	{
		Net_Mail_Heart = Callback;
	}
}

void Net_thread_init_callback(void (*Callback)(void))
{
	if(Callback != RT_NULL)
	{
		NetMsg_thread_init = Callback;
	}
}

////////////////////////////////////////////////////////////////////////////////////////

/*
����:Э����յ����ĵĻص�����
*/
static rt_int8_t Net_MsgRecv_handle(net_recvmsg_p Mail,void *UserData)
{
	rt_int8_t result;
	
	if(NetMsg_Recv_handle != RT_NULL)
	{
		result = NetMsg_Recv_handle(Mail,UserData);
	}
	else
	{
		RT_DEBUG_LOG(SHOW_RECV_MSG_INFO,("NetMsg_Recv_handle Is NULL\n"));
	}

	return result;
}

/** 
@brief Э�鴦���̻߳ص�����
@param void
@retval void
*/
static void Net_Msg_thread_callback(void)
{
	if(NetMsg_Recv_CallBack_Fun != RT_NULL)
	{
		NetMsg_Recv_CallBack_Fun();
	}
	/*else
	{
		RT_DEBUG_LOG(SHOW_RECV_MSG_INFO,("NetMsg_Recv_CallBack_Fun Is NULL\n"));
	}*/
}

/** 
@brief net message pack CRC16 verify
@param receive net message mail
@retval RT_TRUE	 :verify succeed
@retval RT_FALSE :verify fail
*/
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
	
	CurCRC16 = net_crc16((unsigned char *)(tmp+2),CRCLength);

	Mail->lenmap.bype = net_rev16(Mail->lenmap.bype);

	RT_DEBUG_LOG(SHOW_CRC16_INIF,("Remote  CRC16 = %04X\n",CRC16Right));
	RT_DEBUG_LOG(SHOW_CRC16_INIF,("Local   CRC16 = %04X\n",CurCRC16));
	if(CurCRC16 == CRC16Right)
	{
		return RT_TRUE;
	}
	
	return RT_FALSE;
}

/*
����:������µ����
����:flag :RT_TRUE��Ÿ��� 
����:flag :RT_FALSE ����������
*/
rt_uint8_t get_msg_new_order(rt_bool_t flag)
{
	rt_uint8_t CurOrder;

	CurOrder = net_order.byte;
	if(flag == RT_TRUE)
	{
    net_order.bit.col++;
	}
	
	return CurOrder;
}

/*
����:���ͱ����ʼ�
*/
void net_msg_send_mail(net_msgmail_p mail)
{
	RT_ASSERT(mail != RT_NULL);
	RT_ASSERT(net_msgmail_mq != RT_NULL);
	rt_mq_send(net_msgmail_mq,(void *)mail,sizeof(net_msgmail));
}

/*
����:��ʾ���ձ��ĵ���Ҫ��Ϣ
����:msg ���յ��ı���ָ��
*/
void show_recvmsg(net_recvmsg_p msg)
{
  RT_DEBUG_LOG(SHOW_RECV_MSG_INFO,
  	("Receive Message key parameter:\nLength:%02X LengthMap:%02X Cmd:%02X Order:%02d Resend:%X\n",
		msg->length,
		msg->lenmap,
		msg->cmd,
		msg->col.bit.col,
		msg->col.bit.resend));
}
/*
����:��ʾ���ͱ��ĵ���Ҫ��Ϣ
����:msg ���͵ı���ָ��
*/
void show_sendmsg(net_encrypt_p msg)
{
  RT_DEBUG_LOG(SHOW_SEND_MSG_INFO,
  	("Send Message Info:\nCmd:%02X Order:%02d Resend:%02d LengthMap:%02X CRC16:%02X%02X\n",
		msg->cmd,
		msg->col.bit.col,
		msg->col.bit.resend,
		msg->lenmap.bype,
		msg->check[0],
		msg->check[1]));
}

/*
����:��ʾ����ӳ������Ϣ
����:map ��ӳ������
*/
void show_lenmap(net_lenmap map)
{
  RT_DEBUG_LOG(SHOW_LENMAP_INFO,
  	("Length Map:0x%02X  Check:%02d Cmd:%02d Order:%02d Data:%02d \n",
    map.bype,
    map.bit.check,
    map.bit.cmd,
    map.bit.col,
    map.bit.data));
}

static void net_evt_mutex_op(rt_bool_t way)
{
	static rt_mutex_t system_evt = RT_NULL;
	
	if(system_evt == RT_NULL)
	{
		system_evt = rt_mutex_create("netevt",RT_IPC_FLAG_FIFO);
	}
	if(way == RT_TRUE)
	{
    rt_mutex_take(system_evt,RT_WAITING_FOREVER);
	}
	else if(way == RT_FALSE)
	{
		rt_mutex_release(system_evt);
	}
}

static void net_wnd_mutex_op(rt_bool_t way)
{
	static rt_mutex_t wnd_mutex = RT_NULL;
	
	if(wnd_mutex == RT_NULL)
	{
		wnd_mutex = rt_mutex_create("netwnd",RT_IPC_FLAG_FIFO);
	}
	if(way == RT_TRUE)
	{
    rt_mutex_take(wnd_mutex,RT_WAITING_FOREVER);
	}
	else if(way == RT_FALSE)
	{
		rt_mutex_release(wnd_mutex);
	}
}


/*
����:��������Э���еĸ����¼�
����:mode ģʽ  type �¼�����
����: -------------------------
		 |ģʽ |�ɹ�|ʧ��|����    |
		 |0    |0   |1   |�����¼�|
		 |1    |0   |1   |�յ��¼�|
		 |2    |0   |1   |����¼�|
		 --------------------------
*/
rt_uint8_t net_event_process(rt_uint8_t mode,rt_uint32_t type)
{
	rt_uint32_t value;
	rt_err_t    result;
	rt_uint8_t  return_data = 1;
	
	net_evt_mutex_op(RT_TRUE);

	if(net_event == RT_NULL)
	{
    net_event = rt_event_create("netevent",RT_IPC_FLAG_FIFO);
    RT_ASSERT(net_event != RT_NULL);
	}
	switch(mode)
	{
		case 0:	//set event 
		{
			result = rt_event_send(net_event,type);
			if(result == RT_EOK)
			{
				return_data = 0;
			}
			break;
		}
		case 1:	//get event 
		{
			result = rt_event_recv(net_event,
			                       type,
			                       RT_EVENT_FLAG_OR,
			                       RT_WAITING_NO,&value);
			if(result == RT_EOK)
			{
				return_data = 0;
			}
			else if(result == -RT_ETIMEOUT)
			{
				return_data = 1;
			}
			break;
		}
		case 2://clean event
		{
			result = rt_event_recv(net_event,
			                       type,
			                       RT_EVENT_FLAG_OR | 
			                       RT_EVENT_FLAG_CLEAR,
			                       RT_WAITING_NO,&value);
			if(result == RT_EOK)
			{
				return_data = 0;
			}
			break;
		}
    case 3://clean all event 
    {
      result = rt_event_recv(net_event,
                             NET_EVENT_ALL,
                             RT_EVENT_FLAG_OR | 
                             RT_EVENT_FLAG_CLEAR,
                             RT_WAITING_NO,&value);
      if(result == RT_EOK)
      {
        return_data = 0;
      }
      break;
    }
    default:
    {
			break;
    }
	}

	net_evt_mutex_op(RT_FALSE);
	return return_data;
}

/*
����: �����ݰ�DES����
*/
void net_des_pack(rt_uint8_t *buffer,rt_size_t size,net_encrypt *data)
{
  rt_uint8_t input_buf[8], output_buf[8];
  des_context ctx_key0_enc, ctx_key1_enc;
  
  //����k0 k1
  des_setkey_enc(&ctx_key0_enc, NetParameterConfig.key0);
  des_setkey_enc(&ctx_key1_enc, NetParameterConfig.key1);
 
  if(data->cmd == NET_MSGTYPE_LANDED)
  {
 
   // DES(ID+VERSION,K1)
   rt_memset(input_buf, 0, 8);
   rt_memcpy(input_buf, data->data.landed.id, 8);
   des_crypt_ecb(&ctx_key1_enc, input_buf, output_buf);
   rt_memcpy(buffer+16, output_buf, 8);
   
   rt_memset(input_buf, 0, 8);
   input_buf[0] = data->data.landed.version;
   des_crypt_ecb(&ctx_key1_enc, input_buf, output_buf);
   rt_memcpy(buffer + 24, output_buf, 8);

   //DES((K1+DES(ID+VERSION,K1),K0)
   rt_memset(input_buf, 0, 8);
   rt_memcpy(input_buf, data->data.landed.k1, 8);
   des_crypt_ecb(&ctx_key0_enc, input_buf, output_buf);
   rt_memcpy(buffer+8, output_buf, 8);

   rt_memset(input_buf, 0, 8);
   rt_memcpy(input_buf, buffer+16, 8);
   des_crypt_ecb(&ctx_key0_enc, input_buf, output_buf);
   rt_memcpy(buffer + 16, output_buf, 8);

   rt_memset(input_buf, 0, 8);
   rt_memcpy(input_buf, buffer+24, 8);
   des_crypt_ecb(&ctx_key0_enc, input_buf, output_buf);
   rt_memcpy(buffer + 24, output_buf, 8);
   rt_memcpy(buffer, data->data.landed.id,8);
  }
  else
  {
    rt_uint16_t i;
    rt_uint8_t  j;

   	RT_DEBUG_LOG(SHOW_NONE_ENC_DATA,("Send raw data:\n"));
    for(i = 0; i < size/8; i++)
    {
      rt_memset(input_buf, 0, 8);
      rt_memcpy(input_buf, buffer+(i*8), 8);
      for(j = 0 ; j < 8; j++)
      {
        RT_DEBUG_LOG(SHOW_NONE_ENC_DATA,("%02X",input_buf[j]));
      }
      des_crypt_ecb(&ctx_key1_enc, input_buf, output_buf);
      rt_memcpy(buffer+(i*8), output_buf, 8);
    }
   	RT_DEBUG_LOG(SHOW_NONE_ENC_DATA,("\n"));
  }

}
/*
����:���ɱ��İ�����
����:message :���汨�����ݵĴ洢��ַ
		 data    :���������ṹ
*/
void net_pack_data(net_message *message,net_encrypt *data)
{
  rt_uint16_t data_len;
  rt_uint8_t  *bufp = RT_NULL;
  rt_uint16_t rev;
  rt_uint16_t crc16 = 0;
  rt_uint8_t  *des_start = RT_NULL;
  rt_uint8_t  *crc_start = RT_NULL;

	RT_ASSERT(message != RT_NULL);
  //��ʾ���ĵ�һЩ��Ҫ��Ϣ
  show_sendmsg(data);
  show_lenmap(data->lenmap);
  
  //�������ݳ���
  data_len = data->lenmap.bit.check +
             data->lenmap.bit.cmd +
             data->lenmap.bit.col +   
             data->lenmap.bit.data;
  // ������ܺ�İ��ܳ���
  if(data->cmd == NET_MSGTYPE_LANDED)
  {
    message->length = data->lenmap.bit.check +
                      data->lenmap.bit.cmd +
                      data->lenmap.bit.col +
                      32+2;
    data->lenmap.bit.data = 32;          
  }
  else
  {
    rt_uint16_t num;
    
    message->length = data_len + 2;
    if(message->length > 8)
    {
      num = message->length/8;
      if((message->length %8) != 0)
      {
        num++;
      }
      message->length = num*8;
    }
    else
    {
      message->length = 8;
    }
  }
 RT_DEBUG_LOG(SHOW_RECV_MSG_INFO,("DES data length: %d\n",message->length));

 RT_DEBUG_LOG(SHOW_MEM_INFO,("obtain acquire memory resource\n"));
 message->buffer = rt_calloc(1,message->length+4);//���䱨�ĵ��ڴ�
 RT_ASSERT(message->buffer != RT_NULL);
 bufp = message->buffer;

 rev = net_rev16(message->length);
 rt_memcpy(bufp,&rev,sizeof(message->length));//������
 bufp += sizeof(message->length);

 /* ��Ҫ���ܵ��� */
 if(data->cmd != NET_MSGTYPE_LANDED)
 {
   des_start = bufp;
 }
 crc_start = bufp;

 rev = net_rev16(data->lenmap.bype);
 rt_memcpy(bufp,&rev,sizeof(data->lenmap.bype));//����ӳ����
 bufp += sizeof(data->lenmap.bype);      
 
 rt_memcpy(bufp,&data->cmd,data->lenmap.bit.cmd);//����
 bufp += data->lenmap.bit.cmd;
 
 rt_memcpy(bufp,&data->col.byte,data->lenmap.bit.col);//���
 bufp += data->lenmap.bit.col;

 //����ǵ�½����
 if(data->cmd == NET_MSGTYPE_LANDED)
 {
    des_start = bufp;
    rt_memcpy(bufp,&data->data,data->lenmap.bit.data);//����
    bufp += data->lenmap.bit.data;


    //DES����
    net_des_pack(des_start,32,data);
    des_start += 32;
    bufp = des_start;

    //����CRC 
    crc16 = net_crc16(crc_start,bufp-crc_start);
    data->check[0] = (crc16 & 0xff00) >> 8;
    data->check[1] = (crc16 & 0x00ff);
    RT_DEBUG_LOG(SHOW_RECV_MSG_INFO,("Calculate CRC16 = %02X\n",crc16));
    rt_memcpy(bufp,&data->check,data->lenmap.bit.check);//crc
    bufp += data->lenmap.bit.check; 
 }
 else
 {
 	//������ݳ��Ȳ�ȷ���Ĵ������ݰ����ƴ���
 	switch(data->cmd)
 	{
		case NET_MSGTYPE_FILEDATA:
		{
			//extern void wnd_show(void);
			//rt_kprintf("Msg File Data buffer Length %d\n\n",data->lenmap.bit.data);
			//wnd_show();
			rt_memcpy(bufp,data->data.filedata.data,data->lenmap.bit.data);
			break;
		}
		case NET_MSGTYPE_KEYADD:
		{
			rt_memcpy(bufp,&data->data.keyadd,16);
			rt_memcpy(bufp + 16,data->data.keyadd.data,data->lenmap.bit.data - 16);
			
			break;
		}
		case NET_MSGTYPE_ACCMAPADD:
		{
			//�˻�ӳ�������
			rt_memcpy(bufp,data->data.AccMapAdd.MapByte,data->lenmap.bit.data-4);//����ӳ����
			rt_memcpy(bufp+data->lenmap.bit.data-4,&data->data.AccMapAdd.Date,4);//����ʱ��

			break;
		}
		case NET_MSGTYPE_KEYMAPADD:
		{
			//Կ��ӳ�������
			rt_memcpy(bufp,data->data.KeyMapAdd.MapByte,data->lenmap.bit.data-4);//����ӳ����
			rt_memcpy(bufp+data->lenmap.bit.data-4,&data->data.KeyMapAdd.Date,4);//����ʱ��

			break;
		}
		case NET_MSGTYPE_PHMAPADD:
		{
			//�ֻ�ӳ�������
			rt_memcpy(bufp,data->data.PhMapAdd.MapByte,data->lenmap.bit.data-4);//����ӳ����
			rt_memcpy(bufp+data->lenmap.bit.data-4,&data->data.PhMapAdd.Date,4);//����ʱ��

			break;
		}
		case NET_MSGTYPE_RECMAPADD:
		{
			//��¼ӳ�������
			rt_memcpy(bufp,data->data.RecMapAdd.MapByte,data->lenmap.bit.data-4);//����ӳ����
			rt_memcpy(bufp+data->lenmap.bit.data-4,&data->data.RecMapAdd.Date,4);//����ʱ��

			break;
		}
		case NET_MSGTYPE_HTTPUPDATE:
		{
			break;
		}
		default:
		{	
			//ʵ��������
			rt_memcpy(bufp,&data->data,data->lenmap.bit.data);//����
			break;
		}
 	}
  
  bufp += data->lenmap.bit.data;
  //����CRC 
  crc16 = net_crc16(crc_start,data_len);
  data->check[0] = (crc16 & 0xff00) >> 8;
  data->check[1] = (crc16 & 0x00ff);
  RT_DEBUG_LOG(SHOW_RECV_MSG_INFO,("Calculate CRC16 = %02X\n",crc16));
  rt_memcpy(bufp,&data->check,data->lenmap.bit.check);//crc
  bufp += data->lenmap.bit.check; 
  //DES����
  net_des_pack(des_start,message->length,data);
  des_start += message->length;
  bufp = des_start;
 }
 
 rt_memcpy(bufp,"\x0D\x0A",2);//��β����ʶ

 /*��ӡ���ܺ�ı�������*/
 {
    rt_uint16_t i;

    RT_DEBUG_LOG(SHOW_SEND_MSG_INFO,("Send encrypt message:\n>>>>>>"));
    for(i = 0;i < message->length+4;i++)
    {
      RT_DEBUG_LOG(SHOW_SEND_MSG_INFO,("%02X",*(message->buffer+i)));
    }
    RT_DEBUG_LOG(SHOW_SEND_MSG_INFO,("\n"));
 }
}

/*
*���ó���ӳ����
*
*/
void net_set_lenmap(net_lenmap *lenmap,
                          rt_uint8_t cmd,
                          rt_uint8_t col,
                          rt_uint16_t data,
                          rt_uint8_t check)
{
  lenmap->bit.cmd = cmd;
  lenmap->bit.col = col;
  lenmap->bit.data = data;
  lenmap->bit.check = check;
}

/*
����:��32λ�����ݸ��Ƶ��ĸ��ֽ���
����:str�ַ�����ַ  time 32λ��ʱ������
*/
void net_copy_time(rt_uint8_t str[],rt_uint32_t time)
{
  str[0] = time>>24;
  str[1] = time>>16;
  str[2] = time>>8;
  str[3] = time>>0;
}

/*
����:�����ʼ��������ò�ͬ���ͱ�������
����:
*/
rt_err_t net_set_message(net_encrypt_p msg_data,net_msgmail_p MsgMail)
{
	//�������
	msg_data->col = MsgMail->col;
	
  switch(MsgMail->type)
  {
    case NET_MSGTYPE_LANDED:
    {
    	net_landed *data = RT_NULL;
    	
    	//��½����
      msg_data->cmd = NET_MSGTYPE_LANDED;
      net_set_lenmap(&msg_data->lenmap,1,1,32,2);
      
      data = (net_landed *)MsgMail->user;
      if(data != RT_NULL)
      {
				msg_data->data.landed = *data;
      }
      
      break;
    }
    case NET_MSGTYPE_HEART:
    {
    	//��������
      msg_data->cmd = NET_MSGTYPE_HEART;
      msg_data->data.heart.door_status = 0;//��״̬
      net_set_lenmap(&msg_data->lenmap,1,1,1,2);
      break;
    }
    case NET_MSGTYPE_ALARM:
    {
    	//��������
      net_alarm_user *alarm = (net_alarm_user *)MsgMail->user;

      msg_data->cmd = NET_MSGTYPE_ALARM;
      net_set_lenmap(&msg_data->lenmap,1,1,6,2);

      if(alarm != RT_NULL)
      {
        msg_data->data.alarm = alarm->alarm;
      }
      else
      {
        RT_DEBUG_LOG(SHOW_SET_MSG_INOF,("Alarm NET_MSGTYPE_ALARM user is null\n"));

        return RT_ERROR;
      }
      break;
    }
    case NET_MSGTYPE_FAULT:
    {
    	//Ӳ������
    	net_fault_user *fault;

			fault = MsgMail->user;
    	if(fault != RT_NULL)
    	{
				msg_data->data.fault = fault->fault;
    	}
    	else
    	{
    		
        RT_DEBUG_LOG(SHOW_SET_MSG_INOF,("NET_MSGTYPE_FAULT message user is null\n"));
        
				return RT_ERROR;
    	}
      msg_data->cmd = NET_MSGTYPE_FAULT;
      net_set_lenmap(&msg_data->lenmap,1,1,5,2);

      break;
    }
    case NET_MSGTYPE_OPENDOOR:
    {
    	//����
    	net_opendoor_user *opendoor;

    	opendoor = MsgMail->user;
			if(opendoor != RT_NULL)
			{
				msg_data->data.opendoor = opendoor->opendoor;
			}
			else
			{
				RT_DEBUG_LOG(SHOW_SET_MSG_INOF,("NET_MSGTYPE_OPENDOOR message user is null\n"));
        
				return RT_ERROR;
			}
      msg_data->cmd = NET_MSGTYPE_OPENDOOR;
      net_set_lenmap(&msg_data->lenmap,1,1,9,2);
      
      break;
    }
    case NET_MSGTYPE_BATTERY:
    {
    	//���ݵ��
    	net_battery_user *battery;

    	battery = MsgMail->user;
    	if(battery != RT_NULL)
    	{
				msg_data->data.battery = battery->battery;
    	}
    	else
			{
				RT_DEBUG_LOG(SHOW_SET_MSG_INOF,("NET_MSGTYPE_BATTERY message user is null\n"));
        
				return RT_ERROR;
			}
      msg_data->cmd = NET_MSGTYPE_BATTERY;
      net_set_lenmap(&msg_data->lenmap,1,1,6,2);

      break;
    }
    case NET_MSGTYPE_FILEREQUEST:
    {
    	//�ļ�����
    	net_filerequest_user *request;
    	
      msg_data->cmd = NET_MSGTYPE_FILEREQUEST;
      net_set_lenmap(&msg_data->lenmap,1,1,19,2);

			request = (net_filerequest_user *)MsgMail->user;
			if(request != RT_NULL)
			{
				msg_data->data.filerequest = request->file;
			}
			else
			{
				RT_DEBUG_LOG(SHOW_SET_MSG_INOF,("NET_MSGTYPE_FILEREQUEST message user is null\n"));
        
				return RT_ERROR;
			}
    
      break;
    }
    case NET_MSGTYPE_FILEREQUE_ACK:
    {
    	//�ļ�����Ӧ��
    	net_filereq_ack_user *data;
    	
    	msg_data->cmd = NET_MSGTYPE_FILEREQUE_ACK;
      net_set_lenmap(&msg_data->lenmap,1,1,1,2);

			data = MsgMail->user;
      if(data != RT_NULL)
      {
				msg_data->data.FileReqAck = data->result;
      }
      else
			{
				RT_DEBUG_LOG(SHOW_SET_MSG_INOF,("NET_MSGTYPE_FILEREQUE_ACK message user is null\n"));
        
				return RT_ERROR;
			}
			
			break;
    }
    case NET_MSGTYPE_FILEDATA:
    {
    	//�ļ����ݰ�
      net_filedata_user *file = RT_NULL;

      if(MsgMail->user != RT_NULL)
      {
        file = (net_filedata_user*)MsgMail->user;
        msg_data->data.filedata.data = file->data.data;
      }
      else
			{
				RT_DEBUG_LOG(SHOW_SET_MSG_INOF,("NET_MSGTYPE_FILEDATA message user is null\n"));
        
				return RT_ERROR;
			}
      msg_data->cmd = NET_MSGTYPE_FILEDATA;
      net_set_lenmap(&msg_data->lenmap,1,1,file->length+4,2);//512byte + 4byte�����
      break;
    }
    case NET_MSGTYPE_FILEDATA_ACK:
    {
    	//�ļ����ݰ�Ӧ��
    	net_filedat_ack *file;

    	if(MsgMail->user != RT_NULL)
      {
        file = (net_filedat_ack*)MsgMail->user;
        msg_data->data.FileDatAck= *file;
      } 
      else
			{
				RT_DEBUG_LOG(SHOW_SET_MSG_INOF,("NET_MSGTYPE_FILEDATA_ACK message user is null\n"));
        
				return RT_ERROR;
			}
    	msg_data->cmd = NET_MSGTYPE_FILEDATA_ACK;
      net_set_lenmap(&msg_data->lenmap,1,1,5,2);//512byte + 4byte�����
			break;
    }
    case NET_MSGTYPE_PHONEADD:
    {
    	//����ֻ�����
    	net_phoneadd_user *data = RT_NULL;
    	
    	if(MsgMail->user != RT_NULL)
    	{
				data = (net_phoneadd_user*)MsgMail->user;
				msg_data->data.phoneadd = data->data;

				msg_data->cmd = NET_MSGTYPE_PHONEADD;
        net_set_lenmap(&msg_data->lenmap,1,1,20,2);
    	}
    	else
			{
				RT_DEBUG_LOG(NET_MSGTYPE_PHONEADD_ACK,("NET_MSGTYPE_PHONEADD message user is null\n"));
        
				return RT_ERROR;
			}
			
			break;
    }
    case NET_MSGTYPE_PHONEADD_ACK:
    {
      //�ֻ��������Ӧ��
			net_phoneadd_ack *data;
      
      msg_data->cmd = NET_MSGTYPE_PHONEADD_ACK;
      net_set_lenmap(&msg_data->lenmap,1,1,2,2);

			data = MsgMail->user;
      if(data != RT_NULL)
      {
        msg_data->data.PhoneAddAck = *data;
      }
      else
			{
				RT_DEBUG_LOG(NET_MSGTYPE_PHONEADD_ACK,("NET_MSGTYPE_FILEDATA_ACK message user is null\n"));
        
				return RT_ERROR;
			}
			
      break;
    }
    case NET_MSGTYPE_PHONEDELETE:
    {
    	//�ֻ�����ɾ��
    	net_phonedel_user *data = RT_NULL;
    	
    	if(MsgMail->user != RT_NULL)
    	{
				data = (net_phonedel_user*)MsgMail->user;
				msg_data->data.phonedelete = data->data;

				msg_data->cmd = NET_MSGTYPE_PHONEDELETE;
        net_set_lenmap(&msg_data->lenmap,1,1,6,2);
    	}
    	else
			{
				RT_DEBUG_LOG(NET_MSGTYPE_PHONEADD_ACK,("NET_MSGTYPE_PHONEDELETE message user is null\n"));
        
				return RT_ERROR;
			}
			break;
    }
    case NET_MSGTYPE_PHONEDEL_ACK:
    {
      //�ֻ�����ɾ��Ӧ��
			net_phonedel_ack *data;
      
      msg_data->cmd = NET_MSGTYPE_PHONEDEL_ACK;
      net_set_lenmap(&msg_data->lenmap,1,1,2,2);
      
			data = MsgMail->user;
      if(data != RT_NULL)
      {
				msg_data->data.PhoneDelAck = *data;
      }
      else
			{
				RT_DEBUG_LOG(NET_MSGTYPE_PHONEADD_ACK,("NET_MSGTYPE_PHONEDEL_ACK message user is null\n"));
        
				return RT_ERROR;
			}
      
      break;
    }
    case NET_MSGTYPE_ALARMARG:
    {
      //���͸澯����
      net_alarmarg_user *alarmarg;

      alarmarg = MsgMail->user;
      if(alarmarg != RT_NULL)
      {
				msg_data->data.alarmarg = alarmarg->args;
      }
      else
			{
				RT_DEBUG_LOG(NET_MSGTYPE_PHONEADD_ACK,("NET_MSGTYPE_ALARMARG message user is null\n"));
        
				return RT_ERROR;
			}
      msg_data->cmd = NET_MSGTYPE_ALARMARG ;
      net_set_lenmap(&msg_data->lenmap,1,1,2,2);
      break;
    }
    case NET_MSGTYPE_ALARMARG_ACK:
    {
    	//������������Ӧ��
      msg_data->cmd = NET_MSGTYPE_ALARMARG_ACK ;
      net_set_lenmap(&msg_data->lenmap,1,1,1,2);

      msg_data->data.AlarmArgAck.result = 1;
      break;
    }
    case NET_MSGTYPE_LINK:
    {
      msg_data->cmd = NET_MSGTYPE_LINK ;
      net_set_lenmap(&msg_data->lenmap,1,1,1,2);

      msg_data->data.link.arg = 0;
      
      break;
    }
    case NET_MSGTYPE_LINK_ACK:
    {
			msg_data->cmd = NET_MSGTYPE_LINK_ACK;
			net_set_lenmap(&msg_data->lenmap,1,1,1,2);
			msg_data->data.LinkAck.result = 1;
			break;
    }
    case NET_MSGTYPE_KEYADD:
    {
    	//Կ�����
    	net_keyadd_user *keydata;
    	
    	if(MsgMail->user != RT_NULL)
    	{
				keydata = (net_keyadd_user*)MsgMail->user;
				msg_data->data.keyadd = keydata->data;

				msg_data->cmd = NET_MSGTYPE_KEYADD;
				net_set_lenmap(&msg_data->lenmap,1,1,keydata->DataLen + 16,2);
    	}
    	else
			{
				RT_DEBUG_LOG(NET_MSGTYPE_PHONEADD_ACK,("NET_MSGTYPE_KEYADD message user is null\n"));
        
				return RT_ERROR;
			}
			
      break;
    }
    case NET_MSGTYPE_KEYADD_ACK:
    {
    	//Կ�����Ӧ��
    	net_keyadd_ack *data;
    	
    	msg_data->cmd = NET_MSGTYPE_KEYADD_ACK ;
      net_set_lenmap(&msg_data->lenmap,1,1,3,2);

			data = MsgMail->user;
			if(data != RT_NULL)
			{
				msg_data->data.KeyAddAck = *data;
			}
			else
			{
				RT_DEBUG_LOG(NET_MSGTYPE_PHONEADD_ACK,("NET_MSGTYPE_KEYADD_ACK message user is null\n"));
        
				return RT_ERROR;
			}
      
			break;
    }
    case NET_MSGTYPE_KEYDELETE:
    { 
    	//Կ��ɾ��
    	net_keydelete_user *data;

      msg_data->cmd = NET_MSGTYPE_KEYDELETE ;
      net_set_lenmap(&msg_data->lenmap,1,1,2,2);

			data = MsgMail->user;
      if(data != RT_NULL)
      {
				msg_data->data.keydelete = data->data;
      }
      else
			{
				RT_DEBUG_LOG(NET_MSGTYPE_PHONEADD_ACK,("NET_MSGTYPE_KEYDELETE message user is null\n"));
        
				return RT_ERROR;
			}
      
      break;
    }
    case NET_MSGTYPE_KEYDEL_ACK:
    {
    	//Կ��ɾ��Ӧ��
			net_keydel_ack *data;
  		
    	msg_data->cmd = NET_MSGTYPE_KEYDEL_ACK ;
      net_set_lenmap(&msg_data->lenmap,1,1,3,2);

			data = MsgMail->user;
			if(data != RT_NULL)
			{
				 msg_data->data.KeyDelAck = *data;
			}
			else
			{
				RT_DEBUG_LOG(NET_MSGTYPE_KEYDEL_ACK,("NET_MSGTYPE_KEYDELETE message user is null\n"));
        
				return RT_ERROR;
			}
      
			break;
    }
    case NET_MSGTYPE_UPDATE_ACK:
    {
    	//Զ�̸��±���Ӧ��
    	
      msg_data->cmd = NET_MSGTYPE_UPDATE_ACK ;
      net_set_lenmap(&msg_data->lenmap,1,1,1,2);

      msg_data->data.UpDateAck.result = 1;
      break;
    }
    case NET_MSGTYPE_TIME:
    {
    	net_time_user *date;

    	date = MsgMail->user;
			if(date != RT_NULL)
			{
				msg_data->data.time = date->date;
			}
			else
			{
				RT_DEBUG_LOG(NET_MSGTYPE_KEYDEL_ACK,("NET_MSGTYPE_TIME message user is null\n"));
        
				return RT_ERROR;
			}
			msg_data->cmd = NET_MSGTYPE_TIME ;
			net_set_lenmap(&msg_data->lenmap,1,1,4,2);

      break;
    }
    case NET_MSGTYPE_SETK0:
    {
    	//����k0
      break;
    }
    case NET_MSGTYPE_SETK0_ACK:
    {
    	//����k0Ӧ��
			msg_data->cmd = NET_MSGTYPE_SETK0_ACK ;
			net_set_lenmap(&msg_data->lenmap,1,1,1,2);

			msg_data->data.SetK0Ack.result = 1;
			break;
    }
    case NET_MSGTYPE_HTTPUPDAT_ACK:
    {
    	//http����Ӧ��
    	msg_data->cmd = NET_MSGTYPE_HTTPUPDAT_ACK ;
			net_set_lenmap(&msg_data->lenmap,1,1,1,2);

			msg_data->data.HttpUpDateAck.result = 1;
			break;
    }
    case NET_MSGTYPE_MOTOR_ACK:
    {
    	//���Ӧ��
    	msg_data->cmd = NET_MSGTYPE_MOTOR_ACK ;
			net_set_lenmap(&msg_data->lenmap,1,1,1,2);

			msg_data->data.MotorAck.result = 1;
      break;
    }
    case NET_MSGTYPE_DOORMODE_ACK:
    {
    	//���ŷ�ʽӦ��
    	msg_data->cmd = NET_MSGTYPE_DOORMODE_ACK ;
			net_set_lenmap(&msg_data->lenmap,1,1,1,2);

			msg_data->data.DoorModeAck.result = 1;
      break;
    }
    case NET_MSGTYPE_CAMERA_ACK:
    {
    	//����Ӧ��
    	msg_data->cmd = NET_MSGTYPE_CAMERA_ACK ;
			net_set_lenmap(&msg_data->lenmap,1,1,1,2);

			msg_data->data.CameraAck.result = 1;
      break;
    }
    case NET_MSGTYPE_TERMINAL_ACK:
    {
    	//�ն�״̬��ѯӦ��
    	msg_data->cmd = NET_MSGTYPE_TERMINAL_ACK ;
			net_set_lenmap(&msg_data->lenmap,1,1,1,2);

			msg_data->data.TerminalAck.result = 1;
      break;
    }
    case NET_MSGTYPE_DOMAIN_ACK:
    {
    	//��������Ӧ��
    	msg_data->cmd = NET_MSGTYPE_DOMAIN_ACK ;
			net_set_lenmap(&msg_data->lenmap,1,1,1,2);

			msg_data->data.DomainAck.result = 1;
      break;
    }
    case NET_MSGTYPE_ACCOUNTADD:
    {
    	//�û����
    	net_account_add_user *data;
    	
    	msg_data->cmd = NET_MSGTYPE_ACCOUNTADD;
    	net_set_lenmap(&msg_data->lenmap,1,1,26,2);

    	data = MsgMail->user;
      if(data != RT_NULL)
      {
				msg_data->data.AccountAdd = data->account;
      }
      else
			{
				RT_DEBUG_LOG(NET_MSGTYPE_PHONEADD_ACK,("NET_MSGTYPE_ACCOUNTADD message user is null\n"));
        
				return RT_ERROR;
			}
			break;
    }
    case NET_MSGTYPE_ACCOUNTDEL:
    { 
    	//�û�ɾ��
    	net_account_del_user *data;
    	
    	msg_data->cmd = NET_MSGTYPE_ACCOUNTDEL;
    	net_set_lenmap(&msg_data->lenmap,1,1,6,2);

    	data = MsgMail->user;
      if(data != RT_NULL)
      {
				msg_data->data.AccountDel= data->account;
      }
      else
			{
				RT_DEBUG_LOG(NET_MSGTYPE_PHONEADD_ACK,("NET_MSGTYPE_ACCOUNTDEL message user is null\n"));
        
				return RT_ERROR;
			}
			break;
    }
    case NET_MSGTYPE_ACCOUNTADD_ACK:
    {
    	//�û����Ӧ��
    	net_account_ack_user *ack;

    	if(MsgMail->user != RT_NULL)
      {
        ack = (net_account_ack_user*)MsgMail->user;
        msg_data->data.AccountAddAck = ack->ack;
      } 
      else
			{
				RT_DEBUG_LOG(SHOW_SET_MSG_INOF,("NET_MSGTYPE_ACCOUNTADD_ACK message user is null\n"));
        
				return RT_ERROR;
			}
    	msg_data->cmd = NET_MSGTYPE_ACCOUNTADD_ACK ;
			net_set_lenmap(&msg_data->lenmap,1,1,3,2);
			break;
    }
    case NET_MSGTYPE_ACCOUNTDEL_ACK:
    {	
    	//�û�ɾ��Ӧ��
 			net_account_ack_user *ack;

    	if(MsgMail->user != RT_NULL)
      {
        ack = (net_account_ack_user*)MsgMail->user;
        msg_data->data.AccountDelAck = ack->ack;
      } 
      else
			{
				RT_DEBUG_LOG(SHOW_SET_MSG_INOF,("NET_MSGTYPE_ACCOUNTDEL_ACK message user is null\n"));
        
				return RT_ERROR;
			}
    	msg_data->cmd = NET_MSGTYPE_ACCOUNTDEL_ACK ;
			net_set_lenmap(&msg_data->lenmap,1,1,3,2);
			break;
    }
    case NET_MSGTYPE_KEYBIND:
    {
    	//Կ�װ�
    	net_keybind_user *data;
    	
    	msg_data->cmd = NET_MSGTYPE_KEYBIND;
    	net_set_lenmap(&msg_data->lenmap,1,1,8,2);

    	data = MsgMail->user;
      if(data != RT_NULL)
      {
				msg_data->data.KeyBind = data->data;
      }
      else
			{
				RT_DEBUG_LOG(SHOW_SET_MSG_INOF,("NET_MSGTYPE_KEYBIND message user is null\n"));
        
				return RT_ERROR;
			}
			break;
    }
    case NET_MSGTYPE_KEYBIND_ACK:
    {
    	//Կ�װ�Ӧ��
    	net_keybind_ack_user *data;
    	
    	msg_data->cmd = NET_MSGTYPE_KEYBIND_ACK;
    	net_set_lenmap(&msg_data->lenmap,1,1,3,2);

    	data = MsgMail->user;
      if(data != RT_NULL)
      {
				msg_data->data.KeyBindAck = data->data;
      }
      else
			{
				RT_DEBUG_LOG(SHOW_SET_MSG_INOF,("NET_MSGTYPE_KEYBIND message user is null\n"));
        
				return RT_ERROR;
			}
			break;
    }
    case NET_MSGTYPE_PHONEBIND:
    {
    	//�绰��
    	net_phonebind_user *data;
    	
    	msg_data->cmd = NET_MSGTYPE_PHONEBIND;
    	net_set_lenmap(&msg_data->lenmap,1,1,8,2);

    	data = MsgMail->user;
      if(data != RT_NULL)
      {
				msg_data->data.PhoneBind = data->data;
      }
      else
			{
				RT_DEBUG_LOG(SHOW_SET_MSG_INOF,("NET_MSGTYPE_KEYBIND message user is null\n"));
        
				return RT_ERROR;
			}
			break;
    }
    case NET_MSGTYPE_PHONEBIND_ACK:
    {
    	//�绰��Ӧ��
    	net_keyphone_ack_user *data;
    	
    	msg_data->cmd = NET_MSGTYPE_PHONEBIND_ACK;
    	net_set_lenmap(&msg_data->lenmap,1,1,3,2);

    	data = MsgMail->user;
      if(data != RT_NULL)
      {
				msg_data->data.PhoneBindAck = data->data;
      }
      else
			{
				RT_DEBUG_LOG(SHOW_SET_MSG_INOF,("NET_MSGTYPE_KEYBIND message user is null\n"));
        
				return RT_ERROR;
			}
			break;
    }
    case NET_MSGTYPE_ACCMAPADD:
    {
    	//�û�ӳ�������
    	net_accmapadd_user *data;

			data = MsgMail->user;
			if(data != RT_NULL)
			{
				msg_data->data.AccMapAdd = data->data;
			}
			else
			{
				RT_DEBUG_LOG(SHOW_SET_MSG_INOF,("NET_MSGTYPE_ACCMAPADD message user is null\n"));
				return RT_ERROR;
			}
    	msg_data->cmd = MsgMail->type;
    	net_set_lenmap(&msg_data->lenmap,1,1,data->DataLen+4,2);
    	
			break;
    }
    case NET_MSGTYPE_ACCDATCKS:
    {
    	//�˻�����У��
    	net_accdatcks_user *data;

			msg_data->cmd = MsgMail->type;
    	net_set_lenmap(&msg_data->lenmap,1,1,6,2);
    	
			data = MsgMail->user;
			if(data != RT_NULL)
			{
				msg_data->data.AccDatCks = data->data;
			}
			else
			{
				RT_DEBUG_LOG(SHOW_SET_MSG_INOF,("NET_MSGTYPE_ACCDATCKS message user is null\n"));
				return RT_ERROR;
			}
    	
			break;
    }
    case NET_MSGTYPE_KEYMAPADD:
    {
    	//Կ��ӳ�������
    	net_keymapadd_user *data;

			data = MsgMail->user;
			if(data != RT_NULL)
			{
				msg_data->data.KeyMapAdd = data->data;
			}
			else
			{
				RT_DEBUG_LOG(SHOW_SET_MSG_INOF,("NET_MSGTYPE_KEYMAPADD message user is null\n"));
				return RT_ERROR;
			}
    	msg_data->cmd = MsgMail->type;
    	net_set_lenmap(&msg_data->lenmap,1,1,data->DataLen+4,2);
    	
			break;
    }
    case NET_MSGTYPE_KEYDATCKS:
    {
    	//Կ������У��
    	net_keydatcks_user *data;

			msg_data->cmd = MsgMail->type;
    	net_set_lenmap(&msg_data->lenmap,1,1,6,2);
    	
			data = MsgMail->user;
			if(data != RT_NULL)
			{
				msg_data->data.KeyDatCks = data->data;
			}
			else
			{
				RT_DEBUG_LOG(SHOW_SET_MSG_INOF,("NET_MSGTYPE_KEYDATCKS message user is null\n"));
				return RT_ERROR;
			}
    	
			break;
    }
    case NET_MSGTYPE_PHMAPADD:
    {
    	//�ֻ�ӳ�������
    	net_phmapadd_user *data;

			data = MsgMail->user;
			if(data != RT_NULL)
			{
				msg_data->data.PhMapAdd = data->data;
			}
			else
			{
				RT_DEBUG_LOG(SHOW_SET_MSG_INOF,("NET_MSGTYPE_PHMAPADD message user is null\n"));
				return RT_ERROR;
			}
    	msg_data->cmd = MsgMail->type;
    	net_set_lenmap(&msg_data->lenmap,1,1,data->DataLen+4,2);
    	
			break;
    }
    case NET_MSGTYPE_PHDATCKS:
    {
    	//�ֻ�����У��
    	net_phdatcks_user *data;

			msg_data->cmd = MsgMail->type;
    	net_set_lenmap(&msg_data->lenmap,1,1,6,2);
    	
			data = MsgMail->user;
			if(data != RT_NULL)
			{
				msg_data->data.PhDatCks = data->data;
			}
			else
			{
				RT_DEBUG_LOG(SHOW_SET_MSG_INOF,("NET_MSGTYPE_PHDATCKS message user is null\n"));
				return RT_ERROR;
			}
    	
			break;
    }
    case NET_MSGTYPE_RECMAPADD:
    {
    	//��¼ӳ�������
    	net_recmapadd_user *data;

			data = MsgMail->user;
			if(data != RT_NULL)
			{
				msg_data->data.RecMapAdd = data->data;
			}
			else
			{
				RT_DEBUG_LOG(SHOW_SET_MSG_INOF,("NET_MSGTYPE_RECMAPADD message user is null\n"));
				return RT_ERROR;
			}
    	msg_data->cmd = MsgMail->type;
    	net_set_lenmap(&msg_data->lenmap,1,1,data->DataLen+4,2);
    	
			break;
    }
    case NET_MSGTYPE_RECDATCKS:
    {
    	//��¼����У��
    	net_recdatcks_user *data;

			msg_data->cmd = MsgMail->type;
    	net_set_lenmap(&msg_data->lenmap,1,1,6,2);
    	
			data = MsgMail->user;
			if(data != RT_NULL)
			{
				msg_data->data.RecDatCks = data->data;
			}
			else
			{
				RT_DEBUG_LOG(SHOW_SET_MSG_INOF,("NET_MSGTYPE_RECDATCKS message user is null\n"));
				return RT_ERROR;
			}
    	
			break;
    }
    case NET_MSGTYPE_DATA_SYNC_ACK:
    {
    	//����ͬ��
    	net_datasync_ack_user *data;

    	msg_data->cmd = NET_MSGTYPE_DATA_SYNC_ACK;
     	net_set_lenmap(&msg_data->lenmap,1,1,1,2);

     	data = MsgMail->user;
      if(data != RT_NULL)
      {
				msg_data->data.DataSYNCAck = data->data;
      }
      else
			{
				RT_DEBUG_LOG(SHOW_SET_MSG_INOF,("NET_MSGTYPE_DATA_SYNC message user is null\n"));
        
				return RT_ERROR;
			}
			break;
    }
    default:
    {
    	rt_kprintf("Send CMD Nonentity:%02X!!!!!!!!!!!!\n\n",MsgMail->type);

    	return RT_ERROR;
    }
  }
  return RT_EOK;
}

static void clear_wnd_resend_all(void)
{
	rt_uint8_t i;

	for(i = 0 ;i < NET_WND_MAX_NUM;i++)
	{
		if(sendwnd_node[i].mail.type == NET_MSGTYPE_NULL)
		{
      sendwnd_node[i].mail.col.bit.resend = 0;
		}
	}


}
/*static void net_mail_result_process(void)
{

}*/

/*���posλ�õ��ʼ� 
*pos      Ҫ����Ĵ���λ��
*result   ���ͽ��
*
*/
static void clear_wnd_mail_pos(rt_int8_t pos,rt_int8_t result)
{
	if(pos >= 0 && pos <= NET_WND_MAX_NUM)
	{
		//������첽
		if(sendwnd_node[pos].mail.sendmode == ASYN_MODE)
		{
		  RT_DEBUG_LOG(SHOW_SEND_MODE_INFO,("This is asynchronous sending mode,Result OK\n"));
      if(sendwnd_node[pos].mail.user != RT_NULL)
      {
        RT_DEBUG_LOG(SHOW_MEM_INFO,("free user memroy\n"));
        
        RT_ASSERT(sendwnd_node[pos].mail.user != RT_NULL);
        rt_free(sendwnd_node[pos].mail.user);
        sendwnd_node[pos].mail.user = RT_NULL;
      }

		}
		else if(sendwnd_node[pos].mail.sendmode == SYNC_MODE)
		{
      //ͬ�����ؽ��
      net_send_result *send_result = RT_NULL;

      if(sendwnd_node[pos].mail.user != RT_NULL)
      {
        RT_DEBUG_LOG(SHOW_SEND_MODE_INFO,("This is synchronization sending mode,Result OK\n"));
        send_result = (net_send_result *)sendwnd_node[pos].mail.user;

        send_result->result = result;
        rt_sem_release(send_result->complete);
        RT_DEBUG_LOG(SHOW_SEND_MODE_INFO,("release mail of wait sem\n"));
        //�������ָ��
        sendwnd_node[pos].mail.user = RT_NULL;
      }
      else
      {
        RT_DEBUG_LOG(SHOW_SEND_MODE_INFO,
                     ("Send mode set error mode:SYNC_MODE but mail.user=RT_NULL,This is unreasonable\n"));
      }
		}
		else if(sendwnd_node[pos].mail.sendmode == INIT_MODE)
		{
      
		}
		else
		{
      RT_DEBUG_LOG(SHOW_SEND_MODE_INFO,("Send mode value is error!!!\n"));
		}
		sendwnd_node[pos].curtime = 0;
		sendwnd_node[pos].permission = -1;
		sendwnd_node[pos].mail.col.byte = 0;
		sendwnd_node[pos].mail.outtime = 0;
		sendwnd_node[pos].mail.sendmode = INIT_MODE;
		sendwnd_node[pos].mail.time = 0;
		sendwnd_node[pos].mail.type = NET_MSGTYPE_NULL;
		sendwnd_node[pos].mail.resend = 3;
	}
}

/*
*���type���͵��ʼ��ڴ����е�λ��
*���� -1 ��ʾ�����ڸ����� 
*/
static rt_int8_t get_wnd_mail_pos(rt_uint8_t type)
{
  rt_uint8_t i;

  for(i = 0; i < NET_WND_MAX_NUM; i++)
  {
    if(sendwnd_node[i].mail.type == type)
    {
      return i;
    }
  }
  return 0xff;
}

void clear_wnd_cmd_all(rt_uint8_t cmd)
{
	rt_uint8_t i;
	rt_int8_t  pos;

	net_wnd_mutex_op(RT_TRUE);
	for(i = 0 ;i < NET_WND_MAX_NUM;i++)
	{
		pos = get_wnd_mail_pos(cmd);
		if(pos != -1 )
		{
      clear_wnd_mail_pos(pos,SEND_FAIL);
		}
	}
	net_wnd_mutex_op(RT_FALSE);
}

/*
*��ô����е�һ���յ�λ��
*
*/
static rt_int8_t get_wnd_space_pos(void)
{
  rt_uint8_t i;  

  for(i = 0; i < NET_WND_MAX_NUM; i++)
  {
		if(sendwnd_node[i].mail.type == NET_MSGTYPE_NULL)
		{
			//���λ��ʱ�յ�
			return i;
		}
  }
  
  return -1; 
}

/*
����:��ȡ����ĳ��λ�õ��ʼ�����
*/
static rt_uint8_t get_wnd_pos_cmd(rt_uint8_t pos)
{
	return sendwnd_node[pos].mail.type;
}

/*
����:��������ڴ����л�ȡ�����ʼ���λ��
*/
static rt_int8_t get_wnd_order_pos(net_col col)
{
  rt_uint8_t i;  

  for(i = 0; i < NET_WND_MAX_NUM; i++)
  {
    if(sendwnd_node[i].mail.type != NET_MSGTYPE_NULL)
    {
      if(sendwnd_node[i].mail.col.bit.col == col.bit.col)
  		{
  			return i;
  		}
    }
  }
  /*{
		extern void wnd_show(void);
		wnd_show();
  }*/
  
  RT_DEBUG_LOG(SHOW_WND_INFO,("Send window not find Mail Col:%d\n",col.bit.col));
  return -1; 
}

static rt_int8_t get_wnd_mail_resend_pos(void)
{
  rt_uint8_t i;  

  for(i = 0; i < NET_WND_MAX_NUM; i++)
  {
    if(sendwnd_node[i].mail.type != NET_MSGTYPE_NULL)
    {
      if(sendwnd_node[i].mail.col.bit.resend > 0)
  		{
  			return i;
  		}
    }
  }
    
  return -1;
}

/*static rt_int8_t get_wnd_space_num(void)
{
  rt_int8_t pos;
  rt_int8_t i;
  rt_int8_t num = 0;
  
  for(i = 0; i < NET_WND_MAX_NUM; i++)
  {
    pos = get_wnd_space_pos();
    if(pos != -1)
    {
      num++;
    }
  }

  return num;
}*/

/*
����:���±�����ӵ�������
*/ 
static rt_err_t sendwnd_add_new_mail(net_msgmail_p msg)
{
	rt_int8_t pos;
	rt_err_t  result = RT_EOK;

	if(msg == RT_NULL)
	{
    RT_DEBUG_LOG(SHOW_WND_INFO,("Window add a mail is NULL\n"));

    return RT_ERROR;
	}
  pos = get_wnd_space_pos();
  if(pos != -1)
  {
    //�пյĴ���
    sendwnd_node[pos].mail = *msg;
    sendwnd_node[pos].curtime = 0;
    sendwnd_node[pos].permission = NET_WND_MAX_NUM-1;
    RT_DEBUG_LOG(SHOW_WND_INFO,("Window add a mail\n"));
  }
  else
  {
    //�����Ѿ�����
    //net_msg_send_mail(msg);
    if(msg->user != RT_NULL)
    {
      sendwnd_node[NET_WND_MAX_NUM].mail = *msg;
      clear_wnd_mail_pos(NET_WND_MAX_NUM,SEND_FAIL);
    }
    rt_kprintf("\n\nNet window is Full!!!!!\n\n\n");
    result = RT_ERROR;
  }

	return result;
}

/*
*���ô������ʼ���Ȩ��
*/
static void set_wnd_allmail_permission(rt_int8_t permission)
{
	rt_uint8_t pos;

  for(pos = 0; pos < NET_WND_MAX_NUM; pos++)
  {
		if(sendwnd_node[pos].mail.type != NET_MSGTYPE_NULL)
		{
			sendwnd_node[pos].permission =  permission;
		}
  }
}

static void set_wnd_mail_permission(net_col order,rt_int8_t permission)
{
  rt_int8_t pos;

  pos = get_wnd_order_pos(order);
  if(pos != -1)
  {
    sendwnd_node[pos].permission =  permission;
  }
}
/*
����:ɾ�������ʼ���˽�����ݵ��ڴ�ռ�
*/
static void net_msg_user_delete(net_msgmail_p msg)
{
	if(msg->user != RT_NULL)
	{
		RT_ASSERT(msg->user != RT_NULL);
		rt_free(msg->user);
	}
}
/*
*���ô�������λ�õ�Ȩ��
*/
/*static void set_wnd_all_permission(rt_int8_t permission)
{
	rt_uint8_t pos;

  for(pos = 0; pos < NET_WND_MAX_NUM; pos++)
  {
		sendwnd_node[pos].permission =  permission;
  }
}*/

/** 
@brief ���ͱ��Ĵ��ڴ�����
@param msg :���͵ı����ʼ�
@retval RT_EOK   :����ɹ�
@retval RT_ERROR :����ʧ��
*/
static rt_err_t net_send_wnd_process(net_msgmail_p msg)
{
  rt_int8_t pos;
  rt_err_t  result = RT_EOK;

	if(msg->type & 0x80)
	{
		//����Ҫ��ӵ����ڵı����ͷ���Դ
		RT_DEBUG_LOG(SHOW_WND_INFO,("This ACK Message\n"));
		net_msg_user_delete(msg);
		
		return RT_EOK;
	}
  pos = get_wnd_mail_pos(msg->type);
  if(pos != -1)//have this type mail
  {
		//ͬ�����͵����ʼ�
		if(get_wnd_order_pos(msg->col) == -1)
		{
		 result = sendwnd_add_new_mail(msg);
		}
		else
		{
			//�ͷ���Դ
			//net_msg_user_delete(msg);
		}
  }
  else //�����͵��ʼ�
  {
		result = sendwnd_add_new_mail(msg);
  }

	return result;  
}

/*
����:��ȡ�յ���Ӧ���Ķ�Ӧ���ĵ�˽�����ݵ�ַ
����:msg ���յ����ʼ�
*/
rt_uint32_t net_get_wnd_user(net_recvmsg_p msg)
{
	rt_int8_t pos;	
	rt_uint8_t cmd = NET_MSGTYPE_NULL;

	pos = get_wnd_order_pos(msg->col);
	if(pos != -1)
	{
	  if(msg->cmd & 0x80)
	  {
      cmd = (message_type)msg->cmd - 0x80;
	  }
	  //�����յ����ʼ���������ڷ��ʹ�����
	  //�ҵ��İ�����Ƚ�
		if(cmd == get_wnd_pos_cmd(pos))
		{
		  return (rt_uint32_t)sendwnd_node[pos].mail.user;
		}
	}

	return RT_NULL;
}
/*
����:���յ�һ���ʼ���Դ��ڵĴ���
����:msg ���յ����ʼ�
*/
static void net_recv_wnd_process(net_recvmsg_p msg,rt_int8_t result)
{
	rt_int8_t pos;	
	rt_uint8_t cmd = NET_MSGTYPE_NULL;
  
	pos = get_wnd_order_pos(msg->col);
	if(pos != -1)//�ҵ������ŵİ�
	{
	  if(msg->cmd & 0x80)
	  {
      cmd = (message_type)msg->cmd - 0x80;
	  }
	  //�����յ����ʼ���������ڷ��ʹ�����
	  //�ҵ��İ�����Ƚ�
		if(cmd == get_wnd_pos_cmd(pos))
		{
		  RT_DEBUG_LOG(SHOW_WND_INFO,("clear window %d Col %d\n",pos,msg->col.bit.col));
      clear_wnd_mail_pos(pos,result);
		}
	}
}

/*
  *data      ���͵����ݵ�ַ
  *len        ���͵ĳ���
  *mode    ����ģʽ
  *user   ��Բ�ͬӲ�����µ�˽������
  *          �ӿ�
*/
static void net_send_hardware(net_message_p msg,
                                   rt_uint8_t mode,
                                   void *user)
{
  rt_mq_send(net_datsend_mq,(void *)msg,sizeof(net_message));
  rt_sem_take(msg->sendsem,RT_WAITING_FOREVER);
}

/*
����:���ͱ�����Ϣ
����:msg :���յ����ʼ� *user���ĵ�˽�в���
*/
static void net_send_message(net_msgmail_p msg,void *user)
{
  net_message_p message = RT_NULL;
  net_encrypt data;
  rt_uint8_t  run = 1;

  message = rt_calloc(1,sizeof(net_message));
  RT_ASSERT(message != RT_NULL);
  message->sendsem = rt_sem_create("netsend",0,RT_IPC_FLAG_FIFO);
	RT_ASSERT(message->sendsem != RT_NULL);

	while(run--)
	{
		//���ñ�����Ϣ׼�����
    if(net_set_message(&data,msg) == RT_ERROR){break;}

    //���
		net_pack_data(message,&data);
		
    //��������ӵ�����
		if(net_send_wnd_process(msg) == RT_ERROR){break;}

		//��һ�������͸�����ӿ�
		net_send_hardware(message,0,user);

		break;
	}

  RT_DEBUG_LOG(SHOW_MEM_INFO,("release memory resource\n"));
  rt_sem_delete(message->sendsem);
  
  RT_ASSERT(message->buffer != RT_NULL);
  rt_free(message->buffer);
  
  RT_ASSERT(message != RT_NULL);
  rt_free(message);
}

/*
����:DES������������ı���
����ֵ:-1 ����ʧ�� 0 ���ܳɹ�
*/
static rt_int8_t net_des_decode(net_recvmsg_p msg)
{
  rt_uint8_t input_buf[8], output_buf[8];
  des_context ctx_key1_enc;
  rt_uint8_t enc_num = 0;
  rt_uint16_t i,j;
  rt_uint8_t *buffer = RT_NULL;

  buffer = (rt_uint8_t *)msg+2;

  //���ý���Կ��K1
  des_setkey_dec(&ctx_key1_enc, NetParameterConfig.key1);
  
  //С��ת��
  msg->length = net_rev16(msg->length);
  RT_DEBUG_LOG(SHOW_RECV_MSG_INFO,("Receive data length = %d \n",msg->length));

  //�жϰ������Ƿ����buffer����
  if(msg->length > sizeof(net_recvmsg))
  {
  	RT_DEBUG_LOG(SHOW_RECV_MSG_INFO,
  							("Array Bounds Write sizeof(net_recvmsg):%d RecvMSG->length:%d!!!!\n",
  							sizeof(net_recvmsg),msg->length));
    return -1;
  } 
  enc_num  = msg->length/8;
  if(enc_num == 0)
  {
    if(msg->length < 8)
    {
      enc_num = 1;
    }
  }
  RT_DEBUG_LOG(SHOW_NONE_ENC_DATA,("Receive to decrypt the data:\n"));
  for(i = 0; i < enc_num;i++)
  {
    rt_memset(input_buf, 0, 8);
    rt_memcpy(input_buf, buffer+(i*8), 8);
    des_crypt_ecb(&ctx_key1_enc, input_buf, output_buf);
    rt_memcpy(buffer+(i*8), output_buf, 8);
    //��ʾ���ܺ������
    for(j = 0; j < 8; j++)
    {
			RT_DEBUG_LOG(SHOW_NONE_ENC_DATA,("%02X",output_buf[j]));
    }
  }
  RT_DEBUG_LOG(SHOW_NONE_ENC_DATA,("\n"));
  show_recvmsg(msg);
  return 0;
}

/** 
@brief net recvice data alagn process
@param receive net message mail
@retval void
*/
static void net_recv_alagn_process(net_recvmsg_p msg)
{
	net_recvmsg_p tmp = RT_NULL;

	tmp = (net_recvmsg_p)rt_calloc(1,sizeof(net_recvmsg));
	RT_ASSERT(tmp != RT_NULL);
	
	rt_memcpy((void *)tmp,(void *)msg,sizeof(net_recvmsg));
	rt_memcpy((void *)&msg->data,(void *)&tmp->reserve,sizeof(net_recvmsg)-8);

  RT_ASSERT(tmp != RT_NULL);
	rt_free(tmp);
	tmp = RT_NULL;
	RT_DEBUG_LOG(SHOW_RECV_MSG_INFO,("Receive alagn process ok\n"));
}

/** 
@brief window recvice mail process 
@param receive net message mail
@retval void
*/
static void net_recv_message(net_msgmail_p mail)
{
	net_recvmsg_p msg;
	rt_err_t    result;
	rt_int8_t   SendResult = SEND_OK;
	rt_bool_t   CRC16Result;

	result = rt_mb_recv(net_datrecv_mb,(rt_uint32_t *)&msg,1);
	if(result == RT_EOK)
	{
	  RT_DEBUG_LOG(SHOW_RECV_MAIL_ADDR,("Recv mailbox addr %X\n",msg));

	  //����Ѿ���½�����յ������б��Ķ�����
	  if(net_event_process(1,NET_ENVET_ONLINE) == 0)
	  {	
	  	RT_DEBUG_LOG(SHOW_RECV_MSG_INFO,("I have landed ^_^\n"));
      if(net_des_decode(msg) < 0)
      {
        //����ʧ��
        RT_DEBUG_LOG(SHOW_RECV_MSG_INFO,("Message decryption failure\n"));
        RT_ASSERT(msg != RT_NULL);
        rt_free(msg);
        return ;
      }
	  }
	  else
	  {
			//û�е�¼�յ��ı���
			RT_DEBUG_LOG(SHOW_RECV_MSG_INFO,("Not login !!!\n"));

			//��С��ת��
			msg->length = net_rev16(msg->length);
			//�������Ӧ
			show_recvmsg(msg);
	  }
		msg->lenmap.bype = net_rev16(msg->lenmap.bype);

		
    if(msg->cmd & 0x80)
    {
      rt_int8_t pos;
			
      pos = get_wnd_order_pos(msg->col);
      if(pos < 0)
      {
      	RT_ASSERT(msg != RT_NULL);
        rt_free(msg);

        RT_DEBUG_LOG(SHOW_RECV_MSG_INFO,("Message Order Error !!!\n"));
        return ;
      }
    }
    rt_timer_stop(sendwnd_timer);
    if(net_event_process(1,NET_ENVET_ONLINE) == 0)
    {
      CRC16Result = net_mail_crc16_check(msg);
      //verify crc16
      if(CRC16Result == RT_FALSE)
      {
        RT_DEBUG_LOG(SHOW_RECV_MSG_INFO,("Message CRC16 verify Fial !!!\n"));
        
      	RT_ASSERT(msg != RT_NULL);
        rt_free(msg);
        rt_timer_start(sendwnd_timer);
        return ;
      }
    }
    net_recv_alagn_process(msg);
		switch(msg->cmd)
		{
			case NET_MSGTYPE_LANDED_ACK:
			{
				//�յ���ȷӦ��ָ������е��ʼ�����Ȩ��
				if(get_wnd_order_pos(msg->col) != -1)
				{
				  set_wnd_mail_permission(msg->col,-1);
          set_wnd_allmail_permission(NET_WND_MAX_NUM-1);
          RT_DEBUG_LOG(SHOW_WND_INFO,("clear wnd permission\n"));
          Net_MsgRecv_handle(msg,RT_NULL);
				}
				//���߱�־
				net_event_process(0,NET_ENVET_ONLINE);
				RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_LANDED_ACK\n"));
				break;
			}
			case NET_MSGTYPE_HEART_ACK:
			{
				RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_HEART_ACK\n"));
				break;
			}
			case NET_MSGTYPE_ALARM_ACK:
			{
				RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_ALARM_ACK\n"));
				break;
			}
			case NET_MSGTYPE_FAULT_ACK:
			{
				RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_FAULT_ACK\n"));
				break;
			}
			case NET_MSGTYPE_OPENDOOR_ACK:
			{
				RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_OPENDOOR_ACK\n"));
				break;
			}
			case NET_MSGTYPE_BATTERY_ACK:
			{
				RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_BATTERY_ACK\n"));
				break;
			}
			case NET_MSGTYPE_FILEREQUEST:
			{
				//�ļ�����
				RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_FILEREQUEST\n"));
				Net_MsgRecv_handle(msg,RT_NULL);
				break;
			}
			case NET_MSGTYPE_FILEREQUE_ACK:
			{
				//�ļ�����Ӧ��
				RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_FILEREQUE_ACK\n"));
				break;
			}
			case NET_MSGTYPE_FILEDATA_ACK:
			{
				//�ļ����ݰ�Ӧ��
				RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_FILEDATA_ACK\n"));
				Net_MsgRecv_handle(msg,RT_NULL);
				break;
			}
			case NET_MSGTYPE_FILEDATA:
			{
				//�ļ����ݰ�
				RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_FILEDATA\n"));
				Net_MsgRecv_handle(msg,RT_NULL);
				break;
			}
			case NET_MSGTYPE_PHONEADD_ACK:
			{
				RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_PHONEADD_ACK\n"));
				break;
			}
			case NET_MSGTYPE_PHONEDEL_ACK:
			{
				RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_PHONEDDEL_ACK\n"));
				break;
			}
			case NET_MSGTYPE_ALARMARG_ACK:
			{
				RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_ALARMARG_ACK\n"));
				break;
			}
			case NET_MSGTYPE_LINK_ACK:
			{
				RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_LINK_ACK\n"));
				break;
			}
			case NET_MSGTYPE_KEYADD_ACK:
			{
				RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_KEYADD_ACK\n"));
				SendResult = Net_MsgRecv_handle(msg,RT_NULL);
				break;
			}
			case NET_MSGTYPE_KEYDEL_ACK:
			{
				RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_KEYDEL_ACK\n"));
				break;
			}
			case NET_MSGTYPE_UPDATE:
			{
				//Զ�̸���
				RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_UPDATE\n"));
				//message_ASYN(NET_MSGTYPE_UPDATE_ACK);
				Net_MsgRecv_handle(msg,RT_NULL);
				
				break;
			}
			case NET_MSGTYPE_TIME_ACK:
			{
				RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_TIME_ACK\n"));
				Net_MsgRecv_handle(msg,RT_NULL);
				break;
			}
			case NET_MSGTYPE_SETK0:
			{
				//K0����
				RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_SETK0\n"));
				//message_ASYN(NET_MSGTYPE_SETK0_ACK);
				Net_MsgRecv_handle(msg,RT_NULL);
				break;
			}
			case NET_MSGTYPE_HTTPUPDATE:
			{
				//http����
				RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_HTTPUPDATE\n"));
				message_ASYN(NET_MSGTYPE_HTTPUPDAT_ACK);
				break;
			}
			case NET_MSGTYPE_MOTOR:
			{
				//Զ�̿���
				RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_MOTOR\n"));
				//message_ASYN(NET_MSGTYPE_MOTOR_ACK);
				Net_MsgRecv_handle(msg,RT_NULL);
				break;
			}
			case NET_MSGTYPE_DOORMODE:
			{
				//���ŷ�ʽ����
				RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_DOORMODE\n"));
				message_ASYN(NET_MSGTYPE_DOORMODE_ACK);
				break;
			}
			case NET_MSGTYPE_CAMERA:
			{
				//Զ������
				RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_CAMERA\n"));
				Net_MsgRecv_handle(msg,RT_NULL);
				break;
			}
			case NET_MSGTYPE_TERMINAL:
			{
				//ģ���ѯ
				RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_TERMINAL\n"));
				message_ASYN(NET_MSGTYPE_TERMINAL_ACK);
				break;
			}
			case NET_MSGTYPE_DOMAIN:
			{
				//��������
				RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_DOMAIN\n"));
				//message_ASYN(NET_MSGTYPE_DOMAIN_ACK);
				Net_MsgRecv_handle(msg,RT_NULL);
				break;
			}
			case NET_MSGTYPE_PHONEADD:
			{	
				//����ֻ�������
				RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_PHONEADD\n"));
				//message_ASYN(NET_MSGTYPE_PHONEADD_ACK);
				Net_MsgRecv_handle(msg,RT_NULL);
				break;
			}
			case NET_MSGTYPE_PHONEDELETE:
			{	
				//ɾ���ֻ�������
				RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_PHONEDELETE\n"));
				//message_ASYN(NET_MSGTYPE_PHONEDEL_ACK);
				Net_MsgRecv_handle(msg,RT_NULL);
				break;
			}
			case NET_MSGTYPE_ALARMARG:
			{
				//������������
				RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_ALARMARG\n"));
				message_ASYN(NET_MSGTYPE_ALARMARG_ACK);
				break;
			}
			case NET_MSGTYPE_LINK:
			{
				//�ն�����
				RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_LINK\n"));
				message_ASYN(NET_MSGTYPE_LINK_ACK);
				break;
			}
			case NET_MSGTYPE_KEYADD:
			{
				//Կ�����
				RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_KEYADD\n"));
				Net_MsgRecv_handle(msg,RT_NULL);
				//message_ASYN(NET_MSGTYPE_KEYADD_ACK);
				break;
			}
			case NET_MSGTYPE_KEYDELETE:
			{
				//Կ��ɾ��
				RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_KEYDELETE\n"));
				Net_MsgRecv_handle(msg,RT_NULL);
				break;
				//message_ASYN(NET_MSGTYPE_KEYDEL_ACK);
			}
			case NET_MSGTYPE_ACCOUNTADD:
			{
				//�˻����
        RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_ACCOUNTADD\n"));
        Net_MsgRecv_handle(msg,RT_NULL);
				break;
			}	
			case NET_MSGTYPE_ACCOUNTADD_ACK:
			{
				//�˻����Ӧ��
        RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_ACCOUNTADD_ACK\n"));
        Net_MsgRecv_handle(msg,RT_NULL);
				break;
			}	
			case NET_MSGTYPE_ACCOUNTDEL:
			{
				RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_ACCOUNTDEL\n"));
        Net_MsgRecv_handle(msg,RT_NULL);
				break;
			}
			case NET_MSGTYPE_ACCOUNTDEL_ACK:
			{
				RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_ACCOUNTDEL_ACK\n"));
        Net_MsgRecv_handle(msg,RT_NULL);
				break;
			}
			case NET_MSGTYPE_KEYBIND:
			{
				//Կ�װ�
        RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_KEYBIND\n"));
        Net_MsgRecv_handle(msg,RT_NULL);
				break;
			}	
			case NET_MSGTYPE_KEYBIND_ACK:
			{
				//Կ�װ�Ӧ��
        RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_KEYBIND_ACK\n"));
        Net_MsgRecv_handle(msg,RT_NULL);
				break;
			}	
			case NET_MSGTYPE_PHONEBIND:
			{
				//�ֻ���
        RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_PHONEBIND\n"));
        Net_MsgRecv_handle(msg,RT_NULL);
				break;
			}	
			case NET_MSGTYPE_PHONEBIND_ACK:
			{
				//�绰��Ӧ��
        RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_PHONEBIND_ACK\n"));
        Net_MsgRecv_handle(msg,RT_NULL);
				break;
			}	
			case NET_MSGTYPE_ACCMAPADD_ACK:
			{
				//�˻�ӳ����Ӧ��
        RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_ACCMAPADD_ACK\n"));
        Net_MsgRecv_handle(msg,RT_NULL);
				break;
			}
			case NET_MSGTYPE_ACCDATCKS_ACK:
			{
				//�˻�����У��Ӧ��
        RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_ACCDATCKS_ACK\n"));
        Net_MsgRecv_handle(msg,RT_NULL);
				break;
			}
			case NET_MSGTYPE_KEYMAPADD_ACK:
			{
				//Կ��ӳ����Ӧ��
        RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_KEYMAPADD_ACK\n"));
        Net_MsgRecv_handle(msg,RT_NULL);
				break;
			}
			case NET_MSGTYPE_KEYDATCKS_ACK:
			{
				//Կ������У��Ӧ��
        RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_KEYDATCKS_ACK\n"));
        Net_MsgRecv_handle(msg,RT_NULL);
				break;
			}
			case NET_MSGTYPE_PHMAPADD_ACK:
			{
				//�ֻ�ӳ����Ӧ��
        RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_PHMAPADD_ACK\n"));
        Net_MsgRecv_handle(msg,RT_NULL);
				break;
			}
			case NET_MSGTYPE_PHDATCKS_ACK:
			{
				//�ֻ�����У��Ӧ��
        RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_PHDATCKS_ACK\n"));
        Net_MsgRecv_handle(msg,RT_NULL);
				break;
			}
			case NET_MSGTYPE_RECMAPADD_ACK:
			{
				//��¼ӳ����Ӧ��
        RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_RECMAPADD_ACK\n"));
        Net_MsgRecv_handle(msg,RT_NULL);
				break;
			}
			case NET_MSGTYPE_RECDATCKS_ACK:
			{
				//��¼����У��Ӧ��
        RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_RECDATCKS_ACK\n"));
        Net_MsgRecv_handle(msg,RT_NULL);
				break;
			}
			case NET_MSGTYPE_DATA_SYNC:
			{
				//����ͬ��
        RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_DATA_SYNC\n"));
        Net_MsgRecv_handle(msg,RT_NULL);
				break;
			}
			default:
			{
				RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("Receive None identify the message!!!\n\n"));
				break;
			}
		}
		if(msg->cmd & 0x80)
		{
      /* ����ʱ�Դ��ڵĴ��� */
      net_recv_wnd_process(msg,SendResult);
      clear_wnd_resend_all();
		}
		rt_free(msg);
		rt_timer_start(sendwnd_timer);
	}
}

/** 
@brief �����ʼ��ڶ�ʱ�����ط�����
@param pos: �ط��ʼ��ڴ����е�λ��
@retval 0 û�п����ط��ʼ�
@retval 1 �ط�
@retval 2 �ط�����ʧ��
@retval 0xff ����ʧ��
*/
rt_uint8_t net_wnd_resend_mail(rt_int8_t pos)
{
	if((pos < 0) ||(pos >= NET_WND_MAX_NUM))
	{
		rt_kprintf("The position of the window found is illegal function:%s line:%d\n", 
								__FUNCTION__, __LINE__);
		return 0xff;
	}
  if(sendwnd_node[pos].curtime >= sendwnd_node[pos].mail.outtime)
	{
		RT_DEBUG_LOG(SHOW_WND_INFO,("Window Mail %d Out Time\n",pos));
		sendwnd_node[pos].curtime = 0;
		sendwnd_node[pos].mail.col.bit.resend++;
		if(sendwnd_node[pos].mail.col.bit.resend < sendwnd_node[pos].mail.resend)
		{
		  //С���ط�����
		  net_msg_send_mail(&sendwnd_node[pos].mail);
		  
		  return 1;
		}        
		else
		{
		  //������ڵ��ʼ���Ϣ�����ط�
		  clear_wnd_mail_pos(pos,SEND_FAIL);
		  //��־���� �������µ�½
		  net_event_process(2,NET_ENVET_ONLINE);
		  //net_event_process(0,NET_ENVET_RELINK);
		  set_wnd_allmail_permission(-1);
		  return 2;
		}	
	}
	
	return 0;
}

/*
����:��ʱ��������
*/
static void net_wnd_timer_process(void)
{
  rt_int8_t pos;
  rt_int8_t result;
  
  if(net_event_process(1,NET_ENVET_CONNECT) == 0)
  {
		return ;
  }
  
	pos = get_wnd_mail_pos(NET_MSGTYPE_LANDED);
	if(pos != -1)
	{
		result = net_wnd_resend_mail(pos);
		if(result == 2)
		{
			net_event_process(0,NET_ENVET_LOGINFAIL);
			RT_DEBUG_LOG(SHOW_WND_INFO,("Try Connection Again Server\n"));
		}

		return;
	}
	//����Ƕ���״̬
	if(net_event_process(1,NET_ENVET_ONLINE) == 1)
	{
		return ;
	}
	//��������
	pos = get_wnd_mail_pos(NET_MSGTYPE_HEART);
	if(pos != -1)
	{
    net_wnd_resend_mail(pos);
    
		return ;
	}
  for(pos = 0 ;pos < NET_WND_MAX_NUM; pos++)
  {
    if(sendwnd_node[pos].permission != -1)
    {
      result = net_wnd_resend_mail(pos);
      if(result == 1)
      {
				if(Net_Mail_Heart != RT_NULL)
				{
					Net_Mail_Heart();
				}
      }
      if(result != 0)
      {
				break;
      }
    }
  }
}

/*
����:������Ϣ���ĵ��߳�
*/
void netmsg_thread_entry(void *arg)
{
	rt_uint32_t  HearTime = 0;

	if(NetMsg_thread_init != RT_NULL)
  {
		NetMsg_thread_init();
  }
  while(1)
  {
    rt_err_t result;
    net_msgmail msg_mail;

    result = rt_mq_recv(net_msgmail_mq,(void *)&msg_mail,sizeof(net_msgmail),1);
    if(result == RT_EOK)
    {
    	HearTime =  0;
    	
      RT_DEBUG_LOG(SHOW_MSG_THREAD,("MSG Thread recv net_msgmail mail\n"));

      net_send_message(&msg_mail,RT_NULL);//��������
      if(msg_mail.sendmode == SYNC_MODE)
      {
				net_recv_message(&msg_mail);
      }
    }
    net_recv_message(&msg_mail);
    net_wnd_timer_process();		//���ڶ�ʱ������
    Net_Msg_thread_callback();  //ִ�лص�����
    //��������
    HearTime++;
    //rt_kprintf("HearTime = %d\n",HearTime);
    if(HearTime >= 1000*60)
    {
			HearTime = 0;
			//����Ѿ���½
			if(net_event_process(1,NET_ENVET_ONLINE) == 0)
			{
				RT_DEBUG_LOG(SHOW_SEND_MSG_INFO,("send heart\n"));
				if(Net_Mail_Heart != RT_NULL)
				{
					Net_Mail_Heart();
				}
			}
    }
  }
}

void net_wnd_curconut_process(rt_int8_t pos)
{
  if(sendwnd_node[pos].curtime <= sendwnd_node[pos].mail.outtime)
  {    
    sendwnd_node[pos].curtime++;
  }
}
/*����ʱʱ��Ϊ���Ǳ�ʾ�ô��ڿ���
  *����ʱʱ���·��ͱ���
  *�����ͺ������յ�Ӧ��ʱ
  */
static void net_sendwnd_timer(void *parameters)
{
  rt_int8_t pos;
  
	pos = get_wnd_mail_pos(NET_MSGTYPE_LANDED);
	if(pos != -1)
	{
	  //��½�������ȷ���
		net_wnd_curconut_process(pos);
	}
	else if(get_wnd_mail_pos(NET_MSGTYPE_HEART) != -1)
	{
		pos = get_wnd_mail_pos(NET_MSGTYPE_HEART);
		net_wnd_curconut_process(pos);
	}
	else if(get_wnd_mail_resend_pos() != -1)
	{
    //����б��ĳ�ʱ
    pos = get_wnd_mail_resend_pos();
    net_wnd_curconut_process(pos);
	}
	else
	{
    for(pos = 0 ;pos < NET_WND_MAX_NUM; pos++)
    {
      if(sendwnd_node[pos].permission != -1)
      {
        net_wnd_curconut_process(pos);
      }
    }
	}
  
}

void net_sendwnd_init(void)
{
	rt_uint8_t i;

	for(i = 0 ; i < NET_WND_MAX_NUM ; i++)
	{
	  sendwnd_node[i].mail.user = RT_NULL;
	  sendwnd_node[i].mail.sendmode = INIT_MODE;
		clear_wnd_mail_pos(i,1);
	}
}


int netmsg_thread_init(void)
{
  rt_thread_t id = RT_NULL;
  rt_base_t   level;

  level = rt_hw_interrupt_disable();

  net_sendwnd_init();

  net_order.byte = 0;
  
  net_msgmail_mq = rt_mq_create("msgmail", sizeof(net_msgmail),
                          10,
                          RT_IPC_FLAG_FIFO);
	RT_ASSERT(net_msgmail_mq != RT_NULL);

  net_datsend_mq = rt_mq_create("datsend", sizeof(net_message),
                          10,
                          RT_IPC_FLAG_FIFO);
  RT_ASSERT(net_datsend_mq != RT_NULL);
  
  net_datrecv_mb = rt_mb_create("datrecv",
                          NET_RECV_MSG_MAX,
                          RT_IPC_FLAG_FIFO);
  RT_ASSERT(net_datrecv_mb != RT_NULL);

  sendwnd_timer = rt_timer_create("sendwnd",
                                   net_sendwnd_timer,
                                   RT_NULL,
                                   10,
                                   RT_TIMER_FLAG_PERIODIC);  
  rt_timer_start(sendwnd_timer);
  rt_hw_interrupt_enable(level);

  id = rt_thread_create("message",
                         netmsg_thread_entry, RT_NULL,
                         2048,MSG_THREAD_PRI_IS, 20);

  if(id == RT_NULL)
  {
    rt_kprintf("%S Thread init fail !\n",id->name);

    return 1;
  }
  
  rt_thread_startup(id);
  return 0;
}
INIT_APP_EXPORT(netmsg_thread_init);




#ifdef RT_USING_FINSH
#include <finsh.h>

void message_SYNC(rt_uint8_t type)
{
  net_msgmail mail;
  net_send_result_p result;

  mail.user = RT_NULL;
  mail.time = 0;
  mail.type = (message_type)type;
  mail.resend = 3;
  mail.outtime = 600;
  mail.sendmode = SYNC_MODE;
  mail.col.byte = net_order.byte;
  net_order.bit.col++;

  mail.user = result = rt_calloc(1,sizeof(net_send_result));
  result->complete = rt_sem_create("netsend",0,RT_IPC_FLAG_FIFO);
  rt_mq_send(net_msgmail_mq,(void*)&mail,sizeof(net_msgmail));
  rt_sem_take(result->complete,300);
  rt_sem_delete(result->complete);
  
  RT_ASSERT(result != RT_NULL);
  rt_free(result);
}
void message_ASYN(rt_uint8_t type)
{
  net_msgmail mail;

  mail.user = RT_NULL;
  mail.time = 0;
  mail.type = (message_type)type;
  mail.resend = 3;
  mail.outtime = 600;
  mail.sendmode = ASYN_MODE;
  mail.col.byte = net_order.byte;
  net_order.bit.col++;

	mail.user = RT_NULL;
	rt_mq_send(net_msgmail_mq,(void*)&mail,sizeof(net_msgmail));
}
FINSH_FUNCTION_EXPORT(message_SYNC,"(type) have wait send message cmd");
FINSH_FUNCTION_EXPORT(message_ASYN,"(type) none wait send message cmd");

/*
void alarm_net(rt_uint8_t type,rt_uint8_t lock)
{
  net_msgmail mail;
  net_alarm_user *alarm;
  
  mail.type = NET_MSGTYPE_ALARM;
  mail.time = 0;
  mail.resend = 3;
  mail.outtime = 600;
  mail.sendmode = SYNC_MODE;//ͬ��
  mail.col.byte = net_order.byte;
  net_order.bit.col++;
  
  mail.user = alarm= (net_alarm_user *)rt_calloc(1,sizeof(net_alarm_user));
  RT_ASSERT(alarm!=RT_NULL);
  alarm->type = type;
  alarm->lock_status = lock;
  alarm->time = net_get_date();
  alarm->result.complete = rt_sem_create("netsend",0,RT_IPC_FLAG_FIFO);
  net_msg_send_mail(&mail);
  rt_sem_take(alarm->result.complete,RT_WAITING_FOREVER);
  rt_sem_delete(alarm->result.complete);
  rt_kprintf("send result = %d\n",alarm->result.result);
  rt_free(alarm);
}
FINSH_FUNCTION_EXPORT(alarm_net,"(type,lock_status)send alarm net msg");
*/


void file_net(void)
{
  net_msgmail mail;
  net_filedata_user *file;
  
  mail.time = 0;
  mail.type = NET_MSGTYPE_FILEDATA;
  mail.resend = 3;
  mail.outtime = 600;
  mail.sendmode = SYNC_MODE;
  mail.col.byte = net_order.byte;
  mail.user = file = (net_filedata_user *)rt_calloc(1,sizeof(net_filedata_user));
  RT_ASSERT(file!=RT_NULL);
  file->data.data  = (rt_uint8_t *)rt_calloc(1,NET_FILE_BUF_SIZE);
  RT_ASSERT(file->data.data!=RT_NULL);
  
  file->result.complete = rt_sem_create("netsend",0,RT_IPC_FLAG_FIFO);
  net_order.bit.col++;
  net_msg_send_mail(&mail);
  rt_sem_take(file->result.complete,RT_WAITING_FOREVER);
  rt_sem_delete(file->result.complete);
  rt_kprintf("send result = %d\n",file->result.result);

  
  RT_ASSERT(file->data.data != RT_NULL);
  rt_free(file->data.data);
  
  RT_ASSERT(file != RT_NULL);
  rt_free(file);
}
FINSH_FUNCTION_EXPORT(file_net,"send file test cmd");

void heart_net(rt_uint8_t DoorStatus)
{
	
}
FINSH_FUNCTION_EXPORT(heart_net,"(DoorStatus)Send heart message");

void wnd_show(void)
{
	rt_uint8_t i;
	rt_base_t  level;

	level = rt_hw_interrupt_disable();
	for(i = 0;i < NET_WND_MAX_NUM; i++)
	{
		rt_kprintf("[%2d] permission=%02d order=%03d resend=%d outtime=%3d curtime=%3d resend=0x%X type=0x%2X *user=%X\n"
		,i
		,sendwnd_node[i].permission
		,sendwnd_node[i].mail.col.bit.col
		,sendwnd_node[i].mail.col.bit.resend
		,sendwnd_node[i].mail.outtime
		,sendwnd_node[i].curtime
		,sendwnd_node[i].mail.resend
		,sendwnd_node[i].mail.type
		,sendwnd_node[i].mail.user);
	}
	rt_hw_interrupt_enable(level);
}
FINSH_FUNCTION_EXPORT(wnd_show,"show send window status");

void bit_map(rt_uint8_t cmd,rt_uint8_t col,
                rt_uint8_t data,rt_uint8_t check)
{
  net_lenmap test;

  test.bit.check = check;
  test.bit.cmd = cmd;
  test.bit.col = col;
  test.bit.data = data;

  rt_kprintf("%X\n",test.bype);
}
FINSH_FUNCTION_EXPORT(bit_map,"(cmd,col,data,check)bit length map");

void bit_showmap(rt_uint16_t map)
{
	net_lenmap test;

	test.bype = map;

	rt_kprintf("cmd%d  col%d data%d check%d\n",test.bit.cmd,test.bit.col,test.bit.data,test.bit.check);
}
FINSH_FUNCTION_EXPORT(bit_showmap,"(map)show map bit");

void order_get(rt_uint8_t order,rt_uint8_t resend)
{
  net_col col = {0,0};

  col.bit.col = order;
  col.bit.resend = resend;
  rt_kprintf("%X\n",col.byte);
}
FINSH_FUNCTION_EXPORT(order_get,"(order,resend) printf order hex");

#include <time.h>

void time_to_str(rt_uint32_t time)
{
    time_t now;

    now = time;
    rt_kprintf("%s\n", ctime(&now));  
}
FINSH_FUNCTION_EXPORT(time_to_str,"(time) time int to string");


/*void crc_test(void)
{
	rt_uint16_t crc16 ;
	rt_uint32_t crc32;
	rt_uint8_t  i;
	rt_uint8_t  data[3] = "\x12\x34\x56";

	rt_kprintf("Checkout data :0x12 0x34 0x56\n");
	crc16 = net_crc16("\x12\x34\x56",3);
	rt_kprintf("crc16 %X\n",crc16);

	crc32 = cyg_ether_crc32("\x12\x34\x56",3);
	rt_kprintf("crc32 %X\n",crc32);
	crc32 = 0;
	for(i = 0 ; i < 3;i++)
	{
		crc32 = cyg_ether_crc32_accumulate(crc32,&data[i],1);
	}
	rt_kprintf("crc32 %X\n",crc32);
}
FINSH_FUNCTION_EXPORT(crc_test,"test crc16 and crc32");*/

void fileopentest(void)
{
	rt_uint32_t cnt = 65536;
	int fileid;
	
	while(cnt--)
	{
		fileid = open("/app.bin",O_RDWR,0x777);
		close(fileid);
	}
}
FINSH_FUNCTION_EXPORT(fileopentest,"FileOpenTest");

static void hex_to_string(rt_uint8_t data[],rt_uint8_t hex[],rt_uint8_t num)
{	
	const rt_uint8_t asiic[] = "0123456789ABCDEF";
	rt_uint8_t i;

	for(i = 0;i < num*2;i++)
	{
		if(i % 2 == 0)
		{
      data[i] = asiic[hex[i/2]/16];
		}
		else
		{
      data[i] =	asiic[hex[i/2]%16];
		}
	}
}

void net_info(void)
{
	rt_uint8_t *data = RT_NULL;

	data = rt_calloc(1,20);
	RT_ASSERT(data != RT_NULL);
	
	rt_kprintf("|----------net-information---------------|\n");

	hex_to_string(data,NetParameterConfig.id,8);
	rt_kprintf("Net device ID   %s\n",data);
	
	hex_to_string(data,NetParameterConfig.key0,8);
	rt_kprintf("Net device key0 %s\n",data);
	
	hex_to_string(data,NetParameterConfig.key1,8);
	rt_kprintf("Net device key0 %s\n",data);

	rt_free(data);
	if(net_event_process(1,NET_ENVET_ONLINE) == 0)
	{
		rt_kprintf("on-line\n");
	}
	if(net_event_process(1,NET_ENVET_CONNECT) == 0)
	{
		rt_kprintf("connecting\n");
	}
	if(net_event_process(1,NET_ENVET_FILE_ON) == 0)
	{
		rt_kprintf("file send ok\n");
	}
	if(net_event_process(1,NET_ENVET_RELINK) == 0)
	{
		rt_kprintf("reconnection\n");
	}
}
FINSH_FUNCTION_EXPORT(net_info,"net current information");

void date_hex(void)
{
	rt_uint32_t date;

	date = net_get_date();
	
  rt_kprintf("date hex :%08x\n",date);
}
FINSH_FUNCTION_EXPORT(date_hex,"date hex show");

#endif

