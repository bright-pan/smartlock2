/**
功能:实现报文收发协议栈，主要负责将报文描述邮件打包为完整的报文并且发送出去
     同时负责接收来自网络的报文。
版本:0.1v
*/
#include "netprotocol.h"
#include "crc16.h"
#include "des.h"
//#include "untils.h"
//#include "appconfig.h"

//发送结果
#define SEND_OK            0
#define SEND_FAIL          1

rt_mq_t 				net_msgmail_mq = RT_NULL;     //报文邮件
rt_mq_t 				net_datsend_mq = RT_NULL;     //协议层发送给物理网络层
rt_mailbox_t 		net_datrecv_mb = RT_NULL;			//接收邮箱
rt_event_t 			net_event = RT_NULL;       		//网络协议层的事件
rt_mutex_t 			net_wnd_mutex = RT_NULL;			//网络窗口互斥量

//发送窗口
net_sendwnd 		sendwnd_node[NET_WND_MAX_NUM+1];
rt_timer_t  		sendwnd_timer = RT_NULL;//发送窗口的定时器

//序号
net_col 				net_order;	//序号

//ID key0 key1
net_parameter NetParameterConfig = 
{
  {0x99,0x99,0x15,0x10,0x90,0x00,0x01,0x51},
  {0x12,0x34,0x56,0x78,0x90,0x12,0x34,0x51},
  {0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa}
};


void message_ASYN(rt_uint8_t type);

//邮件接收处理函数
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
设置:报文接收的回调函数
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
功能:协议接收到报文的回调函数
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
@brief 协议处理线程回调函数
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
功能:获得最新的序号
参数:flag :RT_TRUE序号更新 
参数:flag :RT_FALSE 获得最新序号
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
功能:发送报文邮件
*/
void net_msg_send_mail(net_msgmail_p mail)
{
	RT_ASSERT(mail != RT_NULL);
	RT_ASSERT(net_msgmail_mq != RT_NULL);
	rt_mq_send(net_msgmail_mq,(void *)mail,sizeof(net_msgmail));
}

/*
功能:显示接收报文的重要信息
参数:msg 接收到的报文指针
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
功能:显示发送报文的重要信息
参数:msg 发送的报文指针
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
功能:显示长度映射域信息
参数:map 重映射数据
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
功能:操作网络协议中的各种事件
参数:mode 模式  type 事件类型
返回: -------------------------
		 |模式 |成功|失败|功能    |
		 |0    |0   |1   |发送事件|
		 |1    |0   |1   |收到事件|
		 |2    |0   |1   |清除事件|
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
功能: 将数据包DES加密
*/
void net_des_pack(rt_uint8_t *buffer,rt_size_t size,net_encrypt *data)
{
  rt_uint8_t input_buf[8], output_buf[8];
  des_context ctx_key0_enc, ctx_key1_enc;
  
  //设置k0 k1
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
功能:生成报文包数据
参数:message :保存报文数据的存储地址
		 data    :报文描述结构
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
  //显示报文的一些重要信息
  show_sendmsg(data);
  show_lenmap(data->lenmap);
  
  //明文数据长度
  data_len = data->lenmap.bit.check +
             data->lenmap.bit.cmd +
             data->lenmap.bit.col +   
             data->lenmap.bit.data;
  // 计算加密后的包总长度
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
 message->buffer = rt_calloc(1,message->length+4);//分配报文的内存
 RT_ASSERT(message->buffer != RT_NULL);
 bufp = message->buffer;

 rev = net_rev16(message->length);
 rt_memcpy(bufp,&rev,sizeof(message->length));//包长度
 bufp += sizeof(message->length);

 /* 需要加密的数 */
 if(data->cmd != NET_MSGTYPE_LANDED)
 {
   des_start = bufp;
 }
 crc_start = bufp;

 rev = net_rev16(data->lenmap.bype);
 rt_memcpy(bufp,&rev,sizeof(data->lenmap.bype));//长度映射域
 bufp += sizeof(data->lenmap.bype);      
 
 rt_memcpy(bufp,&data->cmd,data->lenmap.bit.cmd);//命令
 bufp += data->lenmap.bit.cmd;
 
 rt_memcpy(bufp,&data->col.byte,data->lenmap.bit.col);//序号
 bufp += data->lenmap.bit.col;

 //如果是登陆报文
 if(data->cmd == NET_MSGTYPE_LANDED)
 {
    des_start = bufp;
    rt_memcpy(bufp,&data->data,data->lenmap.bit.data);//数据
    bufp += data->lenmap.bit.data;


    //DES加密
    net_des_pack(des_start,32,data);
    des_start += 32;
    bufp = des_start;

    //计算CRC 
    crc16 = net_crc16(crc_start,bufp-crc_start);
    data->check[0] = (crc16 & 0xff00) >> 8;
    data->check[1] = (crc16 & 0x00ff);
    RT_DEBUG_LOG(SHOW_RECV_MSG_INFO,("Calculate CRC16 = %02X\n",crc16));
    rt_memcpy(bufp,&data->check,data->lenmap.bit.check);//crc
    bufp += data->lenmap.bit.check; 
 }
 else
 {
 	//针对数据长度不确定的大型数据包复制处理
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
			//账户映射域添加
			rt_memcpy(bufp,data->data.AccMapAdd.MapByte,data->lenmap.bit.data-4);//拷贝映射域
			rt_memcpy(bufp+data->lenmap.bit.data-4,&data->data.AccMapAdd.Date,4);//拷贝时间

			break;
		}
		case NET_MSGTYPE_KEYMAPADD:
		{
			//钥匙映射域添加
			rt_memcpy(bufp,data->data.KeyMapAdd.MapByte,data->lenmap.bit.data-4);//拷贝映射域
			rt_memcpy(bufp+data->lenmap.bit.data-4,&data->data.KeyMapAdd.Date,4);//拷贝时间

			break;
		}
		case NET_MSGTYPE_PHMAPADD:
		{
			//手机映射域添加
			rt_memcpy(bufp,data->data.PhMapAdd.MapByte,data->lenmap.bit.data-4);//拷贝映射域
			rt_memcpy(bufp+data->lenmap.bit.data-4,&data->data.PhMapAdd.Date,4);//拷贝时间

			break;
		}
		case NET_MSGTYPE_RECMAPADD:
		{
			//记录映射域添加
			rt_memcpy(bufp,data->data.RecMapAdd.MapByte,data->lenmap.bit.data-4);//拷贝映射域
			rt_memcpy(bufp+data->lenmap.bit.data-4,&data->data.RecMapAdd.Date,4);//拷贝时间

			break;
		}
		case NET_MSGTYPE_HTTPUPDATE:
		{
			break;
		}
		default:
		{	
			//实体数据域
			rt_memcpy(bufp,&data->data,data->lenmap.bit.data);//数据
			break;
		}
 	}
  
  bufp += data->lenmap.bit.data;
  //计算CRC 
  crc16 = net_crc16(crc_start,data_len);
  data->check[0] = (crc16 & 0xff00) >> 8;
  data->check[1] = (crc16 & 0x00ff);
  RT_DEBUG_LOG(SHOW_RECV_MSG_INFO,("Calculate CRC16 = %02X\n",crc16));
  rt_memcpy(bufp,&data->check,data->lenmap.bit.check);//crc
  bufp += data->lenmap.bit.check; 
  //DES加密
  net_des_pack(des_start,message->length,data);
  des_start += message->length;
  bufp = des_start;
 }
 
 rt_memcpy(bufp,"\x0D\x0A",2);//包尾部标识

 /*打印加密后的报文数据*/
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
*设置长度映射域
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
功能:将32位的数据复制到四个字节中
参数:str字符串地址  time 32位的时间数据
*/
void net_copy_time(rt_uint8_t str[],rt_uint32_t time)
{
  str[0] = time>>24;
  str[1] = time>>16;
  str[2] = time>>8;
  str[3] = time>>0;
}

/*
功能:根据邮件类型设置不同类型报文类型
参数:
*/
rt_err_t net_set_message(net_encrypt_p msg_data,net_msgmail_p MsgMail)
{
	//设置序号
	msg_data->col = MsgMail->col;
	
  switch(MsgMail->type)
  {
    case NET_MSGTYPE_LANDED:
    {
    	net_landed *data = RT_NULL;
    	
    	//登陆报文
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
    	//心跳报文
      msg_data->cmd = NET_MSGTYPE_HEART;
      msg_data->data.heart.door_status = 0;//门状态
      net_set_lenmap(&msg_data->lenmap,1,1,1,2);
      break;
    }
    case NET_MSGTYPE_ALARM:
    {
    	//工作报警
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
    	//硬件故障
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
    	//开门
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
    	//备份电池
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
    	//文件请求
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
    	//文件请求应答
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
    	//文件数据包
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
      net_set_lenmap(&msg_data->lenmap,1,1,file->length+4,2);//512byte + 4byte包序号
      break;
    }
    case NET_MSGTYPE_FILEDATA_ACK:
    {
    	//文件数据包应答
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
      net_set_lenmap(&msg_data->lenmap,1,1,5,2);//512byte + 4byte包序号
			break;
    }
    case NET_MSGTYPE_PHONEADD:
    {
    	//添加手机号码
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
      //手机号码添加应答
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
    	//手机号码删除
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
      //手机号码删除应答
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
      //发送告警参数
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
    	//报警参数报文应答
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
    	//钥匙添加
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
    	//钥匙添加应答
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
    	//钥匙删除
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
    	//钥匙删除应答
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
    	//远程更新报文应答
    	
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
    	//设置k0
      break;
    }
    case NET_MSGTYPE_SETK0_ACK:
    {
    	//设置k0应答
			msg_data->cmd = NET_MSGTYPE_SETK0_ACK ;
			net_set_lenmap(&msg_data->lenmap,1,1,1,2);

			msg_data->data.SetK0Ack.result = 1;
			break;
    }
    case NET_MSGTYPE_HTTPUPDAT_ACK:
    {
    	//http更新应答
    	msg_data->cmd = NET_MSGTYPE_HTTPUPDAT_ACK ;
			net_set_lenmap(&msg_data->lenmap,1,1,1,2);

			msg_data->data.HttpUpDateAck.result = 1;
			break;
    }
    case NET_MSGTYPE_MOTOR_ACK:
    {
    	//电机应答
    	msg_data->cmd = NET_MSGTYPE_MOTOR_ACK ;
			net_set_lenmap(&msg_data->lenmap,1,1,1,2);

			msg_data->data.MotorAck.result = 1;
      break;
    }
    case NET_MSGTYPE_DOORMODE_ACK:
    {
    	//开门方式应答
    	msg_data->cmd = NET_MSGTYPE_DOORMODE_ACK ;
			net_set_lenmap(&msg_data->lenmap,1,1,1,2);

			msg_data->data.DoorModeAck.result = 1;
      break;
    }
    case NET_MSGTYPE_CAMERA_ACK:
    {
    	//拍照应答
    	msg_data->cmd = NET_MSGTYPE_CAMERA_ACK ;
			net_set_lenmap(&msg_data->lenmap,1,1,1,2);

			msg_data->data.CameraAck.result = 1;
      break;
    }
    case NET_MSGTYPE_TERMINAL_ACK:
    {
    	//终端状态查询应答
    	msg_data->cmd = NET_MSGTYPE_TERMINAL_ACK ;
			net_set_lenmap(&msg_data->lenmap,1,1,1,2);

			msg_data->data.TerminalAck.result = 1;
      break;
    }
    case NET_MSGTYPE_DOMAIN_ACK:
    {
    	//域名设置应答
    	msg_data->cmd = NET_MSGTYPE_DOMAIN_ACK ;
			net_set_lenmap(&msg_data->lenmap,1,1,1,2);

			msg_data->data.DomainAck.result = 1;
      break;
    }
    case NET_MSGTYPE_ACCOUNTADD:
    {
    	//用户添加
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
    	//用户删除
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
    	//用户添加应答
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
    	//用户删除应答
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
    	//钥匙绑定
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
    	//钥匙绑定应答
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
    	//电话绑定
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
    	//电话绑定应答
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
    	//用户映射域添加
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
    	//账户数据校验
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
    	//钥匙映射域添加
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
    	//钥匙数据校验
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
    	//手机映射域添加
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
    	//手机数据校验
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
    	//记录映射域添加
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
    	//记录数据校验
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
    	//数据同步
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

/*清除pos位置的邮件 
*pos      要清除的窗口位置
*result   发送结果
*
*/
static void clear_wnd_mail_pos(rt_int8_t pos,rt_int8_t result)
{
	if(pos >= 0 && pos <= NET_WND_MAX_NUM)
	{
		//如果是异步
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
      //同步返回结果
      net_send_result *send_result = RT_NULL;

      if(sendwnd_node[pos].mail.user != RT_NULL)
      {
        RT_DEBUG_LOG(SHOW_SEND_MODE_INFO,("This is synchronization sending mode,Result OK\n"));
        send_result = (net_send_result *)sendwnd_node[pos].mail.user;

        send_result->result = result;
        rt_sem_release(send_result->complete);
        RT_DEBUG_LOG(SHOW_SEND_MODE_INFO,("release mail of wait sem\n"));
        //清除窗口指针
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
*获得type类型的邮件在窗口中的位置
*返回 -1 表示不存在该类型 
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
*获得窗口中的一个空的位置
*
*/
static rt_int8_t get_wnd_space_pos(void)
{
  rt_uint8_t i;  

  for(i = 0; i < NET_WND_MAX_NUM; i++)
  {
		if(sendwnd_node[i].mail.type == NET_MSGTYPE_NULL)
		{
			//这个位置时空的
			return i;
		}
  }
  
  return -1; 
}

/*
功能:获取窗口某个位置的邮件命令
*/
static rt_uint8_t get_wnd_pos_cmd(rt_uint8_t pos)
{
	return sendwnd_node[pos].mail.type;
}

/*
功能:根据序号在窗口中获取发送邮件的位置
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
功能:将新报文添加到窗口中
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
    //有空的窗口
    sendwnd_node[pos].mail = *msg;
    sendwnd_node[pos].curtime = 0;
    sendwnd_node[pos].permission = NET_WND_MAX_NUM-1;
    RT_DEBUG_LOG(SHOW_WND_INFO,("Window add a mail\n"));
  }
  else
  {
    //窗口已经满了
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
*设置窗口中邮件的权限
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
功能:删除报文邮件中私有数据的内存空间
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
*设置窗口所以位置的权限
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
@brief 发送报文窗口处理函数
@param msg :发送的报文邮件
@retval RT_EOK   :处理成功
@retval RT_ERROR :处理失败
*/
static rt_err_t net_send_wnd_process(net_msgmail_p msg)
{
  rt_int8_t pos;
  rt_err_t  result = RT_EOK;

	if(msg->type & 0x80)
	{
		//不需要添加到窗口的报文释放资源
		RT_DEBUG_LOG(SHOW_WND_INFO,("This ACK Message\n"));
		net_msg_user_delete(msg);
		
		return RT_EOK;
	}
  pos = get_wnd_mail_pos(msg->type);
  if(pos != -1)//have this type mail
  {
		//同种类型的新邮件
		if(get_wnd_order_pos(msg->col) == -1)
		{
		 result = sendwnd_add_new_mail(msg);
		}
		else
		{
			//释放资源
			//net_msg_user_delete(msg);
		}
  }
  else //新类型的邮件
  {
		result = sendwnd_add_new_mail(msg);
  }

	return result;  
}

/*
功能:获取收到的应答报文对应报文的私有数据地址
参数:msg 接收到的邮件
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
	  //将接收到的邮件的命令和在发送窗口中
	  //找到的包命令比较
		if(cmd == get_wnd_pos_cmd(pos))
		{
		  return (rt_uint32_t)sendwnd_node[pos].mail.user;
		}
	}

	return RT_NULL;
}
/*
功能:接收到一份邮件后对窗口的处理
参数:msg 接收到的邮件
*/
static void net_recv_wnd_process(net_recvmsg_p msg,rt_int8_t result)
{
	rt_int8_t pos;	
	rt_uint8_t cmd = NET_MSGTYPE_NULL;
  
	pos = get_wnd_order_pos(msg->col);
	if(pos != -1)//找到这个序号的包
	{
	  if(msg->cmd & 0x80)
	  {
      cmd = (message_type)msg->cmd - 0x80;
	  }
	  //将接收到的邮件的命令和在发送窗口中
	  //找到的包命令比较
		if(cmd == get_wnd_pos_cmd(pos))
		{
		  RT_DEBUG_LOG(SHOW_WND_INFO,("clear window %d Col %d\n",pos,msg->col.bit.col));
      clear_wnd_mail_pos(pos,result);
		}
	}
}

/*
  *data      发送的数据地址
  *len        发送的长度
  *mode    发送模式
  *user   针对不同硬件留下的私有数据
  *          接口
*/
static void net_send_hardware(net_message_p msg,
                                   rt_uint8_t mode,
                                   void *user)
{
  rt_mq_send(net_datsend_mq,(void *)msg,sizeof(net_message));
  rt_sem_take(msg->sendsem,RT_WAITING_FOREVER);
}

/*
功能:发送报文信息
参数:msg :接收到的邮件 *user报文的私有参数
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
		//设置报文信息准备打包
    if(net_set_message(&data,msg) == RT_ERROR){break;}

    //打包
		net_pack_data(message,&data);
		
    //将数据添加到窗口
		if(net_send_wnd_process(msg) == RT_ERROR){break;}

		//将一个包发送给物理接口
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
功能:DES解密来自网络的报文
返回值:-1 解密失败 0 解密成功
*/
static rt_int8_t net_des_decode(net_recvmsg_p msg)
{
  rt_uint8_t input_buf[8], output_buf[8];
  des_context ctx_key1_enc;
  rt_uint8_t enc_num = 0;
  rt_uint16_t i,j;
  rt_uint8_t *buffer = RT_NULL;

  buffer = (rt_uint8_t *)msg+2;

  //设置解密钥匙K1
  des_setkey_dec(&ctx_key1_enc, NetParameterConfig.key1);
  
  //小端转换
  msg->length = net_rev16(msg->length);
  RT_DEBUG_LOG(SHOW_RECV_MSG_INFO,("Receive data length = %d \n",msg->length));

  //判断包长度是否大于buffer长度
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
    //显示解密后的数据
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

	  //如果已经登陆在线收到的所有报文都解密
	  if(net_event_process(1,NET_ENVET_ONLINE) == 0)
	  {	
	  	RT_DEBUG_LOG(SHOW_RECV_MSG_INFO,("I have landed ^_^\n"));
      if(net_des_decode(msg) < 0)
      {
        //解密失败
        RT_DEBUG_LOG(SHOW_RECV_MSG_INFO,("Message decryption failure\n"));
        RT_ASSERT(msg != RT_NULL);
        rt_free(msg);
        return ;
      }
	  }
	  else
	  {
			//没有登录收到的报文
			RT_DEBUG_LOG(SHOW_RECV_MSG_INFO,("Not login !!!\n"));

			//大小端转换
			msg->length = net_rev16(msg->length);
			//如果是响应
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
				//收到正确应答恢复窗口中的邮件发送权限
				if(get_wnd_order_pos(msg->col) != -1)
				{
				  set_wnd_mail_permission(msg->col,-1);
          set_wnd_allmail_permission(NET_WND_MAX_NUM-1);
          RT_DEBUG_LOG(SHOW_WND_INFO,("clear wnd permission\n"));
          Net_MsgRecv_handle(msg,RT_NULL);
				}
				//在线标志
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
				//文件请求
				RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_FILEREQUEST\n"));
				Net_MsgRecv_handle(msg,RT_NULL);
				break;
			}
			case NET_MSGTYPE_FILEREQUE_ACK:
			{
				//文件请求应答
				RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_FILEREQUE_ACK\n"));
				break;
			}
			case NET_MSGTYPE_FILEDATA_ACK:
			{
				//文件数据包应答
				RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_FILEDATA_ACK\n"));
				Net_MsgRecv_handle(msg,RT_NULL);
				break;
			}
			case NET_MSGTYPE_FILEDATA:
			{
				//文件数据包
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
				//远程更新
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
				//K0设置
				RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_SETK0\n"));
				//message_ASYN(NET_MSGTYPE_SETK0_ACK);
				Net_MsgRecv_handle(msg,RT_NULL);
				break;
			}
			case NET_MSGTYPE_HTTPUPDATE:
			{
				//http更新
				RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_HTTPUPDATE\n"));
				message_ASYN(NET_MSGTYPE_HTTPUPDAT_ACK);
				break;
			}
			case NET_MSGTYPE_MOTOR:
			{
				//远程开门
				RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_MOTOR\n"));
				//message_ASYN(NET_MSGTYPE_MOTOR_ACK);
				Net_MsgRecv_handle(msg,RT_NULL);
				break;
			}
			case NET_MSGTYPE_DOORMODE:
			{
				//开门方式管理
				RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_DOORMODE\n"));
				message_ASYN(NET_MSGTYPE_DOORMODE_ACK);
				break;
			}
			case NET_MSGTYPE_CAMERA:
			{
				//远程拍照
				RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_CAMERA\n"));
				Net_MsgRecv_handle(msg,RT_NULL);
				break;
			}
			case NET_MSGTYPE_TERMINAL:
			{
				//模块查询
				RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_TERMINAL\n"));
				message_ASYN(NET_MSGTYPE_TERMINAL_ACK);
				break;
			}
			case NET_MSGTYPE_DOMAIN:
			{
				//域名设置
				RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_DOMAIN\n"));
				//message_ASYN(NET_MSGTYPE_DOMAIN_ACK);
				Net_MsgRecv_handle(msg,RT_NULL);
				break;
			}
			case NET_MSGTYPE_PHONEADD:
			{	
				//添加手机白名单
				RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_PHONEADD\n"));
				//message_ASYN(NET_MSGTYPE_PHONEADD_ACK);
				Net_MsgRecv_handle(msg,RT_NULL);
				break;
			}
			case NET_MSGTYPE_PHONEDELETE:
			{	
				//删除手机白名单
				RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_PHONEDELETE\n"));
				//message_ASYN(NET_MSGTYPE_PHONEDEL_ACK);
				Net_MsgRecv_handle(msg,RT_NULL);
				break;
			}
			case NET_MSGTYPE_ALARMARG:
			{
				//报警参数设置
				RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_ALARMARG\n"));
				message_ASYN(NET_MSGTYPE_ALARMARG_ACK);
				break;
			}
			case NET_MSGTYPE_LINK:
			{
				//终端休眠
				RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_LINK\n"));
				message_ASYN(NET_MSGTYPE_LINK_ACK);
				break;
			}
			case NET_MSGTYPE_KEYADD:
			{
				//钥匙添加
				RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_KEYADD\n"));
				Net_MsgRecv_handle(msg,RT_NULL);
				//message_ASYN(NET_MSGTYPE_KEYADD_ACK);
				break;
			}
			case NET_MSGTYPE_KEYDELETE:
			{
				//钥匙删除
				RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_KEYDELETE\n"));
				Net_MsgRecv_handle(msg,RT_NULL);
				break;
				//message_ASYN(NET_MSGTYPE_KEYDEL_ACK);
			}
			case NET_MSGTYPE_ACCOUNTADD:
			{
				//账户添加
        RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_ACCOUNTADD\n"));
        Net_MsgRecv_handle(msg,RT_NULL);
				break;
			}	
			case NET_MSGTYPE_ACCOUNTADD_ACK:
			{
				//账户添加应答
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
				//钥匙绑定
        RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_KEYBIND\n"));
        Net_MsgRecv_handle(msg,RT_NULL);
				break;
			}	
			case NET_MSGTYPE_KEYBIND_ACK:
			{
				//钥匙绑定应答
        RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_KEYBIND_ACK\n"));
        Net_MsgRecv_handle(msg,RT_NULL);
				break;
			}	
			case NET_MSGTYPE_PHONEBIND:
			{
				//手机绑定
        RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_PHONEBIND\n"));
        Net_MsgRecv_handle(msg,RT_NULL);
				break;
			}	
			case NET_MSGTYPE_PHONEBIND_ACK:
			{
				//电话绑定应答
        RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_PHONEBIND_ACK\n"));
        Net_MsgRecv_handle(msg,RT_NULL);
				break;
			}	
			case NET_MSGTYPE_ACCMAPADD_ACK:
			{
				//账户映射域应答
        RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_ACCMAPADD_ACK\n"));
        Net_MsgRecv_handle(msg,RT_NULL);
				break;
			}
			case NET_MSGTYPE_ACCDATCKS_ACK:
			{
				//账户数据校验应答
        RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_ACCDATCKS_ACK\n"));
        Net_MsgRecv_handle(msg,RT_NULL);
				break;
			}
			case NET_MSGTYPE_KEYMAPADD_ACK:
			{
				//钥匙映射域应答
        RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_KEYMAPADD_ACK\n"));
        Net_MsgRecv_handle(msg,RT_NULL);
				break;
			}
			case NET_MSGTYPE_KEYDATCKS_ACK:
			{
				//钥匙数据校验应答
        RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_KEYDATCKS_ACK\n"));
        Net_MsgRecv_handle(msg,RT_NULL);
				break;
			}
			case NET_MSGTYPE_PHMAPADD_ACK:
			{
				//手机映射域应答
        RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_PHMAPADD_ACK\n"));
        Net_MsgRecv_handle(msg,RT_NULL);
				break;
			}
			case NET_MSGTYPE_PHDATCKS_ACK:
			{
				//手机数据校验应答
        RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_PHDATCKS_ACK\n"));
        Net_MsgRecv_handle(msg,RT_NULL);
				break;
			}
			case NET_MSGTYPE_RECMAPADD_ACK:
			{
				//记录映射域应答
        RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_RECMAPADD_ACK\n"));
        Net_MsgRecv_handle(msg,RT_NULL);
				break;
			}
			case NET_MSGTYPE_RECDATCKS_ACK:
			{
				//记录数据校验应答
        RT_DEBUG_LOG(SHOW_RECV_GSM_RST,("NET_MSGTYPE_RECDATCKS_ACK\n"));
        Net_MsgRecv_handle(msg,RT_NULL);
				break;
			}
			case NET_MSGTYPE_DATA_SYNC:
			{
				//数据同步
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
      /* 接收时对窗口的处理 */
      net_recv_wnd_process(msg,SendResult);
      clear_wnd_resend_all();
		}
		rt_free(msg);
		rt_timer_start(sendwnd_timer);
	}
}

/** 
@brief 窗口邮件在定时器中重发处理
@param pos: 重发邮件在窗口中的位置
@retval 0 没有可以重发邮件
@retval 1 重发
@retval 2 重发三次失败
@retval 0xff 操作失败
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
		  //小于重发次数
		  net_msg_send_mail(&sendwnd_node[pos].mail);
		  
		  return 1;
		}        
		else
		{
		  //清除窗口的邮件信息不再重发
		  clear_wnd_mail_pos(pos,SEND_FAIL);
		  //标志断线 申请重新登陆
		  net_event_process(2,NET_ENVET_ONLINE);
		  //net_event_process(0,NET_ENVET_RELINK);
		  set_wnd_allmail_permission(-1);
		  return 2;
		}	
	}
	
	return 0;
}

/*
功能:定时器处理函数
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
	//如果是断线状态
	if(net_event_process(1,NET_ENVET_ONLINE) == 1)
	{
		return ;
	}
	//有心跳包
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
功能:处理消息报文的线程
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

      net_send_message(&msg_mail,RT_NULL);//发送数据
      if(msg_mail.sendmode == SYNC_MODE)
      {
				net_recv_message(&msg_mail);
      }
    }
    net_recv_message(&msg_mail);
    net_wnd_timer_process();		//窗口定时器处理
    Net_Msg_thread_callback();  //执行回调函数
    //发送心跳
    HearTime++;
    //rt_kprintf("HearTime = %d\n",HearTime);
    if(HearTime >= 1000*60)
    {
			HearTime = 0;
			//如果已经登陆
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
/*当超时时间为零是表示该窗口空闲
  *当超时时重新发送报文
  *当发送后马上收到应答时
  */
static void net_sendwnd_timer(void *parameters)
{
  rt_int8_t pos;
  
	pos = get_wnd_mail_pos(NET_MSGTYPE_LANDED);
	if(pos != -1)
	{
	  //登陆报文优先发送
		net_wnd_curconut_process(pos);
	}
	else if(get_wnd_mail_pos(NET_MSGTYPE_HEART) != -1)
	{
		pos = get_wnd_mail_pos(NET_MSGTYPE_HEART);
		net_wnd_curconut_process(pos);
	}
	else if(get_wnd_mail_resend_pos() != -1)
	{
    //如果有报文超时
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
  mail.sendmode = SYNC_MODE;//同步
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

