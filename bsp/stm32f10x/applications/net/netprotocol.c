/**
功能:实现报文收发协议栈，主要负责将报文描述邮件打包为完整的报文并且发送出去
     同时负责接收来自网络的报文。
版本:0.1v
*/
#include "netprotocol.h"
#include "crc16.h"
#include "des.h"

//发送结果
#define SEND_OK            0
#define SEND_FAIL          1

rt_mq_t net_msgmail_mq = RT_NULL;     //报文邮件
rt_mq_t net_datsend_mq = RT_NULL;     //协议层发送给物理网络层
rt_mailbox_t net_datrecv_mb = RT_NULL;//接收邮箱
rt_event_t net_event = RT_NULL;       //网络协议层的事件

//发送窗口
net_sendwnd sendwnd_node[NET_WND_MAX_NUM];
rt_timer_t  sendwnd_timer = RT_NULL;//发送窗口的定时器

//序号
net_col net_order;


void message_ASYN(rt_uint8_t type);
//邮件接收处理函数
rt_uint8_t (*NetMsg_Recv_handle)(net_recvmsg_p Mail,void *UserData);
void (*NetMsg_Recv_CallBack_Fun)(void);

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

/*
功能:协议处理线程回调函数
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
	
	rt_kprintf("CRC16Right = %04X\n",CRC16Right);
	rt_kprintf("CurCRC16   = %04X\n",CurCRC16);
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
  	("Send message key parameter:\nCmd:%02X Order:%02d LengthMap:%02X CRC16:%02X%02X\n",
		msg->cmd,
		msg->col.bit.col,
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
  des_setkey_enc(&ctx_key0_enc, "\x01\x02\x03\x04\x05\x06\x07\x08");
  des_setkey_enc(&ctx_key1_enc, "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa");
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
 RT_DEBUG_LOG(SHOW_RECV_MSG_INFO,("DES ciphertext data length %d\n",message->length));

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
			rt_memcpy(bufp,data->data.filedata.data,data->lenmap.bit.data);
			break;
		}
		case NET_MSGTYPE_KEYADD:
		{
			rt_memcpy(bufp,&data->data.keyadd,16);
			rt_memcpy(bufp + 16,data->data.keyadd.data,data->lenmap.bit.data - 16);
			rt_kprintf("data->lenmap.bit.data = %d\n",data->lenmap.bit.data);
			
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

    //rt_kprintf("Send encrypt message:\n");
    for(i = 0;i < message->length+4;i++)
    {
      rt_kprintf("%02X",*(message->buffer+i));
    }
    rt_kprintf("\n");
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
void net_set_message(net_encrypt_p msg_data,net_msgmail_p MsgMail)
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
        RT_DEBUG_LOG(SHOW_SET_MSG_INOF,("Alarm message user is null\n"));
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
      msg_data->cmd = NET_MSGTYPE_OPENDOOR;
      net_set_lenmap(&msg_data->lenmap,1,1,7,2);
      
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

			RT_ASSERT(MsgMail->user != RT_NULL);
			request = (net_filerequest_user *)MsgMail->user;
			msg_data->data.filerequest = request->file;
    
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
    	msg_data->cmd = NET_MSGTYPE_FILEDATA_ACK;
      net_set_lenmap(&msg_data->lenmap,1,1,5,2);//512byte + 4byte包序号
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
    default:
    {
    	rt_kprintf("Send CMD Nonentity:%02X\n",MsgMail->type);
      break;
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
	if(pos >= 0 && pos < NET_WND_MAX_NUM)
	{
		//如果是异步
		if(sendwnd_node[pos].mail.sendmode == ASYN_MODE)
		{
		  RT_DEBUG_LOG(SHOW_SEND_MODE_INFO,("This is asynchronous sending mode,Result OK\n"));
      if(sendwnd_node[pos].mail.user != RT_NULL)
      {
        rt_kprintf("free user memroy\n");
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
static rt_int8_t get_wnd_mail_pos(message_type type)
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
static message_type get_wnd_pos_cmd(rt_uint8_t pos)
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
  
  rt_kprintf("Send window not find Mail Col:%d\n",col.bit.col);
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
static void sendwnd_add_new_mail(net_msgmail_p msg)
{
	rt_int8_t pos;
	
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
      rt_free(msg->user);
      msg->user = RT_NULL;
    }
  }

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
/*
功能:报文窗口处理 
*/
static void net_send_wnd_process(net_msgmail_p msg)
{
  rt_int8_t pos;

	if(msg->type & 0x80)
	{
		//不需要添加到窗口的报文释放资源
		rt_kprintf("This ACK Message\n");
		net_msg_user_delete(msg);
		return ;
	}
  pos = get_wnd_mail_pos(msg->type);
  if(pos != -1)//have this type mail
  {
		//同种类型的新邮件
		if(get_wnd_order_pos(msg->col) == -1)
		{
		 sendwnd_add_new_mail(msg);
		}
		else
		{
			//释放资源
			//net_msg_user_delete(msg);
		}
  }
  else //新类型的邮件
  {
		sendwnd_add_new_mail(msg);
  }

  
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

  message = rt_calloc(1,sizeof(net_message));
  RT_ASSERT(message != RT_NULL);
  message->sendsem = rt_sem_create("netsend",0,RT_IPC_FLAG_FIFO);
	RT_ASSERT(message->sendsem != RT_NULL);
  net_set_message(&data,msg);//设置报文信息准备打包

  net_pack_data(message,&data);//打包

  net_send_wnd_process(msg);//将数据添加到窗口

  net_send_hardware(message,0,user);//将一个包发送给物理接口

  RT_DEBUG_LOG(SHOW_MEM_INFO,("release memory resource\n"));
  rt_sem_delete(message->sendsem);
  rt_free(message->buffer);
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
  des_setkey_dec(&ctx_key1_enc, "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa");

  //小端转换
  rt_kprintf("length = %X ",msg->length);
  msg->length = net_rev16(msg->length);
  rt_kprintf("length = %X \n",msg->length);

  //判断包长度是否大于buffer长度
  if(msg->length > sizeof(net_recvmsg))
  {
  	rt_kprintf("Array Bounds Write sizeof(net_recvmsg):%d RecvMSG->length%d\n",
  							sizeof(net_recvmsg),msg->length);
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
	rt_free(tmp);
	tmp = RT_NULL;
	rt_kprintf("receive alagn process ok\n");
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
      if(net_des_decode(msg) < 0)
      {
        //解密失败
        rt_kprintf("Message decryption failure\n");
        rt_free(msg);
        return ;
      }
	  }
	  else
	  {
			rt_kprintf("Is NET_MSGTYPE_LANDED_ACK cmd %x\n",msg->cmd);
	  }
	  
	  //大小端转换
		//msg.length = net_rev16(msg.length);
		msg->lenmap.bype = net_rev16(msg->lenmap.bype);

		//如果是响应
    if(msg->cmd & 0x80)
    {
      rt_int8_t pos;
			
      pos = get_wnd_order_pos(msg->col);
      if(pos < 0)
      {
        rt_kprintf("Recv Message ACK Window Couldn't Find msg->col = %d\n",msg->col);
        rt_free(msg);
        return ;
      }
    }
		show_recvmsg(msg);
    rt_timer_stop(sendwnd_timer);
    if(net_event_process(1,NET_ENVET_ONLINE) == 0)
    {
      CRC16Result = net_mail_crc16_check(msg);
      //verify crc16
      if(CRC16Result == RT_FALSE)
      {
        rt_kprintf("Message CRC16 verify Fial\n");
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
          rt_kprintf("clear wnd permission\n");
          Net_MsgRecv_handle(msg,RT_NULL);
				}
				//在线标志
				net_event_process(0,NET_ENVET_ONLINE);
				rt_kprintf("NET_MSGTYPE_LANDED_ACK\n");
				break;
			}
			case NET_MSGTYPE_HEART_ACK:
			{
				rt_kprintf("NET_MSGTYPE_HEART_ACK\n");
				break;
			}
			case NET_MSGTYPE_ALARM_ACK:
			{
				rt_kprintf("NET_MSGTYPE_ALARM_ACK\n");
				break;
			}
			case NET_MSGTYPE_FAULT_ACK:
			{
				rt_kprintf("NET_MSGTYPE_FAULT_ACK\n");
				break;
			}
			case NET_MSGTYPE_OPENDOOR_ACK:
			{
				rt_kprintf("NET_MSGTYPE_OPENDOOR_ACK\n");
				break;
			}
			case NET_MSGTYPE_BATTERY_ACK:
			{
				rt_kprintf("NET_MSGTYPE_BATTERY_ACK\n");
				break;
			}
			case NET_MSGTYPE_FILEREQUEST:
			{
				//文件请求
				rt_kprintf("NET_MSGTYPE_FILEREQUEST\n");
				Net_MsgRecv_handle(msg,RT_NULL);
				break;
			}
			case NET_MSGTYPE_FILEREQUE_ACK:
			{
				//文件请求应答
				rt_kprintf("NET_MSGTYPE_FILEREQUE_ACK\n");
				break;
			}
			case NET_MSGTYPE_FILEDATA_ACK:
			{
				//文件数据包应答
				rt_kprintf("NET_MSGTYPE_FILEDATA_ACK\n");
				break;
			}
			case NET_MSGTYPE_FILEDATA:
			{
				//文件数据包
				rt_kprintf("NET_MSGTYPE_FILEDATA\n");
				Net_MsgRecv_handle(msg,RT_NULL);
				break;
			}
			case NET_MSGTYPE_PHONEADD_ACK:
			{
				rt_kprintf("NET_MSGTYPE_PHONEADD_ACK\n");
				break;
			}
			case NET_MSGTYPE_PHONEDEL_ACK:
			{
				rt_kprintf("NET_MSGTYPE_PHONEDDEL_ACK\n");
				break;
			}
			case NET_MSGTYPE_ALARMARG_ACK:
			{
				rt_kprintf("NET_MSGTYPE_ALARMARG_ACK\n");
				break;
			}
			case NET_MSGTYPE_LINK_ACK:
			{
				rt_kprintf("NET_MSGTYPE_LINK_ACK\n");
				break;
			}
			case NET_MSGTYPE_KEYADD_ACK:
			{
				rt_kprintf("NET_MSGTYPE_KEYADD_ACK\n");
				break;
			}
			case NET_MSGTYPE_KEYDEL_ACK:
			{
				rt_kprintf("NET_MSGTYPE_KEYDEL_ACK\n");
				break;
			}
			case NET_MSGTYPE_UPDATE:
			{
				//远程更新
				rt_kprintf("NET_MSGTYPE_UPDATE\n");
				message_ASYN(NET_MSGTYPE_UPDATE_ACK);
				break;
			}
			case NET_MSGTYPE_TIME_ACK:
			{
				rt_kprintf("NET_MSGTYPE_TIME_ACK\n");
				break;
			}
			case NET_MSGTYPE_SETK0:
			{
				//K0设置
				rt_kprintf("NET_MSGTYPE_SETK0\n");
				message_ASYN(NET_MSGTYPE_SETK0_ACK);
				break;
			}
			case NET_MSGTYPE_HTTPUPDATE:
			{
				//http更新
				rt_kprintf("NET_MSGTYPE_HTTPUPDATE\n");
				message_ASYN(NET_MSGTYPE_HTTPUPDAT_ACK);
				break;
			}
			case NET_MSGTYPE_MOTOR:
			{
				//远程开门
				rt_kprintf("NET_MSGTYPE_MOTOR\n");
				message_ASYN(NET_MSGTYPE_MOTOR_ACK);
				Net_MsgRecv_handle(msg,RT_NULL);
				break;
			}
			case NET_MSGTYPE_DOORMODE:
			{
				//开门方式管理
				rt_kprintf("NET_MSGTYPE_DOORMODE\n");
				message_ASYN(NET_MSGTYPE_DOORMODE_ACK);
				break;
			}
			case NET_MSGTYPE_CAMERA:
			{
				//远程拍照
				rt_kprintf("NET_MSGTYPE_CAMERA\n");
				message_ASYN(NET_MSGTYPE_CAMERA_ACK);
				break;
			}
			case NET_MSGTYPE_TERMINAL:
			{
				//模块查询
				rt_kprintf("NET_MSGTYPE_TERMINAL\n");
				message_ASYN(NET_MSGTYPE_TERMINAL_ACK);
				break;
			}
			case NET_MSGTYPE_DOMAIN:
			{
				//域名设置
				rt_kprintf("NET_MSGTYPE_DOMAIN\n");
				message_ASYN(NET_MSGTYPE_DOMAIN_ACK);
				break;
			}
			case NET_MSGTYPE_PHONEADD:
			{	
				//添加手机白名单
				rt_kprintf("NET_MSGTYPE_PHONEADD\n");
				//message_ASYN(NET_MSGTYPE_PHONEADD_ACK);
				Net_MsgRecv_handle(msg,RT_NULL);
				break;
			}
			case NET_MSGTYPE_PHONEDELETE:
			{	
				//删除手机白名单
				rt_kprintf("NET_MSGTYPE_PHONEDELETE\n");
				message_ASYN(NET_MSGTYPE_PHONEDEL_ACK);
				break;
			}
			case NET_MSGTYPE_ALARMARG:
			{
				//报警参数设置
				rt_kprintf("NET_MSGTYPE_ALARMARG\n");
				message_ASYN(NET_MSGTYPE_ALARMARG_ACK);
				break;
			}
			case NET_MSGTYPE_LINK:
			{
				//终端休眠
				rt_kprintf("NET_MSGTYPE_LINK\n");
				message_ASYN(NET_MSGTYPE_LINK_ACK);
				break;
			}
			case NET_MSGTYPE_KEYADD:
			{
				//钥匙添加
				rt_kprintf("NET_MSGTYPE_KEYADD\n");
				message_ASYN(NET_MSGTYPE_KEYADD_ACK);
				break;
			}
			case NET_MSGTYPE_KEYDELETE:
			{
				//钥匙删除
				rt_kprintf("NET_MSGTYPE_KEYDELETE\n");
				message_ASYN(NET_MSGTYPE_KEYDEL_ACK);
			}
			default:
			{
				break;
			}
		}
		if(msg->cmd & 0x80)
		{
      /* 接收时对窗口的处理 */
      net_recv_wnd_process(msg,SendResult);
		}
		rt_free(msg);
		rt_timer_start(sendwnd_timer);
	}
}


/*
功能:定时器处理函数
*/
static void net_wnd_timer_process(void)
{
  rt_int8_t pos;
  
	pos = get_wnd_mail_pos(NET_MSGTYPE_LANDED);
	if(pos != -1)
	{
		//如果有登陆邮件
		if(sendwnd_node[pos].curtime >= sendwnd_node[pos].mail.outtime)
		{
		  sendwnd_node[pos].curtime = 0;
		  sendwnd_node[pos].mail.col.bit.resend++;
			if(sendwnd_node[pos].mail.col.bit.resend < sendwnd_node[pos].mail.resend)
			{
				//小于重发次数
				net_msg_send_mail(&sendwnd_node[pos].mail);
			}        
			else
			{
				//清除窗口的邮件信息不再重发
				clear_wnd_mail_pos(pos,SEND_FAIL);
				//标志断线 申请重新登陆
				net_event_process(2,NET_ENVET_ONLINE);
				net_event_process(0,NET_ENVET_RELINK);
				set_wnd_allmail_permission(-1);
			}
			RT_DEBUG_LOG(SHOW_WND_INFO,("Try Connection Again Server\n"));
		}
		
		return;
	}
	//如果是断线状态
	if(net_event_process(1,NET_ENVET_ONLINE) == 1)
	{
		return ;
	}
  for(pos = 0 ;pos < NET_WND_MAX_NUM; pos++)
  {
    if(sendwnd_node[pos].permission != -1)
    {
      if(sendwnd_node[pos].curtime >= sendwnd_node[pos].mail.outtime)
      {
        RT_DEBUG_LOG(SHOW_WND_INFO,("Window Mail %d Out Time\n",pos));
        sendwnd_node[pos].curtime = 0;
        sendwnd_node[pos].mail.col.bit.resend++;
        if(sendwnd_node[pos].mail.col.bit.resend < sendwnd_node[pos].mail.resend)
        {
        	//小于重发次数
          net_msg_send_mail(&sendwnd_node[pos].mail);
          break;
        }        
        else
        {
        	//发送失败
					clear_wnd_mail_pos(pos,SEND_FAIL);
					set_wnd_allmail_permission(-1);
					//标志断线 申请重新登陆
					net_event_process(2,NET_ENVET_ONLINE);
					net_event_process(0,NET_ENVET_RELINK);
					break;
        }
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
  rt_kprintf("message thread run\n");
  while(1)
  {
    rt_err_t result;
    net_msgmail msg_mail;

    result = rt_mq_recv(net_msgmail_mq,(void *)&msg_mail,sizeof(net_msgmail),1);
    if(result == RT_EOK)
    {
      rt_kprintf("rev net_msgmail_mq \n");
      rt_kprintf("send mode %d\n",msg_mail.sendmode);
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
    if(HearTime >= 100*60)
    {
			HearTime = 0;
			//如果已经登陆
			if(net_event_process(1,NET_ENVET_ONLINE) == 0)
			{
				rt_kprintf("send heart\n");
				//message_ASYN(NET_MSGTYPE_HEART);
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
                                   1,
                                   RT_TIMER_FLAG_PERIODIC);
  net_event = rt_event_create("netevent",RT_IPC_FLAG_FIFO);
  
  rt_timer_start(sendwnd_timer);
  rt_hw_interrupt_enable(level);

  id = rt_thread_create("msg",
                         netmsg_thread_entry, RT_NULL,
                         1024,110, 20);

  if(id == RT_NULL)
  {
    rt_kprintf("msg thread init fail !\n");

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
  rt_free(file->data.data);
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
		rt_kprintf("[%2d] permission=%d order=%03d resend=%d outtime=%3d curtime=%3d resend=0x%X type=0x%2X *user=%X\n"
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

#endif

