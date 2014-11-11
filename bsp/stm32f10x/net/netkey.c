/**
  ******************************************************************************
  * @file    netkey.c
  * @author  wangzw <wangzw@yuettak.com>
  * @version v0.1
  * @date    2014-4-2
  * @brief   This file provides net message key process functions.
  ******************************************************************************
  * @attention
  *
	*
  ******************************************************************************
  */

#include "netkey.h"
//#include "apppubulic.h"

#define SHOW_NETKEY_INFO      1

//用bit表示周
typedef struct 
{
	rt_uint32_t reserve:24;
	rt_uint32_t week1:1;
	rt_uint32_t week2:1;
	rt_uint32_t week3:1;
	rt_uint32_t week4:1;
	rt_uint32_t week5:1;
	rt_uint32_t week6:1;
	rt_uint32_t week7:1;
}WeekBitMapDef;

typedef struct
{
	rt_uint32_t start_h:8;
	rt_uint32_t start_m:8;
	rt_uint32_t stop_h:8;
	rt_uint32_t stop_m:8;
}TimeBitMapDef;

/** 
@brief Check whether the key data received legally
@param receive net message mail
@retval RT_EOK	 :add ok
@retval RT_ERROR :add false
*/
static rt_bool_t check_net_key_data(rt_uint16_t keypos,rt_uint8_t KeyType)
{
	if(keypos >= KEY_NUMBERS)
	{
		return RT_FALSE;
	}
	if(KeyType > KEY_TYPE_KBOARD)
	{
		return RT_FALSE;
	}
	
	return RT_TRUE;
}

/** 
@brief add key process
@param receive net message mail
@retval RT_EOK	 :add ok
@retval RT_ERROR :add false
*/
rt_err_t net_key_add_process(net_recvmsg_p mail)
{
	net_recv_keyadd *remote;
  struct key			*keydat;
	rt_uint16_t     KeyID;
	rt_int32_t			KeyOpResult;
	TimeBitMapDef   OpenDoorTime;
	ALIGN(RT_ALIGN_SIZE)
	WeekBitMapDef   OpenDoorWeek;
	
	RT_ASSERT(remote != RT_NULL);
  remote = &(mail->data.keyadd);

	keydat = rt_calloc(1,sizeof(*keydat));

	net_string_copy_uint16(&KeyID,remote->col);
	keydat->head.key_type = remote->type+1;//1//1.指纹 2.RFID 3.密码
	keydat->head.operation_type = remote->accredit+1;
	net_string_copy_uint32((rt_uint32_t *)&keydat->head.updated_time,remote->createt);
	net_string_copy_uint32((rt_uint32_t *)&keydat->head.start_time,remote->start_t);
	net_string_copy_uint32((rt_uint32_t *)&keydat->head.end_time,remote->stop_t);

	rt_kprintf("%02x%02x%02x%02x\n",remote->start_t[0],remote->start_t[1],remote->start_t[2],remote->start_t[3]);
	{
		rt_uint32_t test;

		test = keydat->head.start_time;
		rt_kprintf("%x",test>>24&0x000000ff);
		rt_kprintf("%x",test>>16&0x000000ff);
		rt_kprintf("%x",test>>8&0x000000ff);
		rt_kprintf("%x\n",test>>0&0x000000ff);
		rt_kprintf("OpenDoorWeek sizeof() = %d  \n",sizeof(OpenDoorWeek));
	}
	rt_memcpy(&OpenDoorWeek,remote->start_t,4);
	rt_memcpy(&OpenDoorTime,remote->stop_t,4);
	
	RT_DEBUG_LOG(SHOW_NETKEY_INFO,("Key add data info:>>>>>>\n"));
	RT_DEBUG_LOG(SHOW_NETKEY_INFO,("KeyID                       = %x\n",KeyID));
	RT_DEBUG_LOG(SHOW_NETKEY_INFO,("keydat->head.key_type       = %x\n",keydat->head.key_type-1));
	RT_DEBUG_LOG(SHOW_NETKEY_INFO,("keydat->head.operation_type = %x\n",keydat->head.operation_type-1));
	RT_DEBUG_LOG(SHOW_NETKEY_INFO,("keydat->head.is_updated     = %x\n",keydat->head.is_updated));
	RT_DEBUG_LOG(SHOW_NETKEY_INFO,("keydat->head.updated_time   = %x\n",keydat->head.updated_time));
	RT_DEBUG_LOG(SHOW_NETKEY_INFO,("keydat->head.start_time     = %x\n",keydat->head.start_time));
	RT_DEBUG_LOG(SHOW_NETKEY_INFO,("keydat->head.end_time       = %x\n",keydat->head.end_time));
	
	if(keydat->head.key_type == KEY_TYPE_KBOARD)
	{
    rt_memcpy((void *)keydat->data.kboard.code,(const void *)remote->data,6);
    RT_DEBUG_LOG(SHOW_NETKEY_INFO,("keydat->data.kboard       = %s\n",keydat->data.kboard.code));
	}
	else if(keydat->head.key_type == KEY_TYPE_RFID)
	{
		//由于RF433没有对应的报文这里做了一个不合理的处理
		keydat->head.key_type = KEY_TYPE_RF433;
		rt_memcpy((void *)keydat->data.rfid.code,(const void *)remote->data,4);
    RT_DEBUG_LOG(SHOW_NETKEY_INFO,("keydat->data.rfid         = %s\n",keydat->data.rfid.code));
	}
	else if(keydat->head.key_type == KEY_TYPE_FPRINT)
	{
    rt_memcpy((void *)keydat->data.fprint.code,(const void *)remote->data,512);
    RT_DEBUG_LOG(SHOW_NETKEY_INFO,("Fprintf type\n"));
	}
	else if(keydat->head.key_type == KEY_TYPE_RF433)
	{
    rt_memcpy((void *)keydat->data.rf433.code,(const void *)remote->data,4);
    RT_DEBUG_LOG(SHOW_NETKEY_INFO,("keydat->data.rf433         = %s\n",keydat->data.rf433.code));
	}
	else 
	{
    RT_DEBUG_LOG(SHOW_NETKEY_INFO,("error type key!!!\n\n"));
	}
	
	KeyOpResult = device_config_key_set(KeyID,keydat,keydat->head.updated_time);

	
	RT_DEBUG_LOG(SHOW_NETKEY_INFO,("OpenDoorTime.start_h   = %d\n",OpenDoorTime.start_h));
	RT_DEBUG_LOG(SHOW_NETKEY_INFO,("OpenDoorTime.start_m   = %d\n",OpenDoorTime.start_m));
	RT_DEBUG_LOG(SHOW_NETKEY_INFO,("OpenDoorTime.stop_h    = %d\n",OpenDoorTime.stop_h));
	RT_DEBUG_LOG(SHOW_NETKEY_INFO,("OpenDoorTime.stop_m    = %d\n",OpenDoorTime.stop_m));

	//RT_DEBUG_LOG(SHOW_NETKEY_INFO,("OpenDoorWeek.week0     = %x\n",OpenDoorWeek.week0));
	RT_DEBUG_LOG(SHOW_NETKEY_INFO,("OpenDoorWeek.week1     = %x\n",OpenDoorWeek.week1));
	RT_DEBUG_LOG(SHOW_NETKEY_INFO,("OpenDoorWeek.week2     = %x\n",OpenDoorWeek.week2));
	RT_DEBUG_LOG(SHOW_NETKEY_INFO,("OpenDoorWeek.week3     = %x\n",OpenDoorWeek.week3));
	RT_DEBUG_LOG(SHOW_NETKEY_INFO,("OpenDoorWeek.week4     = %x\n",OpenDoorWeek.week4));
	RT_DEBUG_LOG(SHOW_NETKEY_INFO,("OpenDoorWeek.week5     = %x\n",OpenDoorWeek.week5));
	RT_DEBUG_LOG(SHOW_NETKEY_INFO,("OpenDoorWeek.week6     = %x\n",OpenDoorWeek.week6));
	RT_DEBUG_LOG(SHOW_NETKEY_INFO,("OpenDoorWeek.week7     = %x\n",OpenDoorWeek.week7));
	rt_free(keydat);

	if(KeyOpResult < 0)
	{
    RT_DEBUG_LOG(SHOW_NETKEY_INFO,("Remote add keys to fail!!!\n"));
		return RT_ERROR;
	}
  RT_DEBUG_LOG(SHOW_NETKEY_INFO,("Remote add keys to success!!!\n"));
	return RT_EOK;
}

/** 
@brief delete key process
@param receive net message mail
@retval RT_EOK	 :add ok
@retval RT_ERROR :add false
*/
rt_err_t net_key_del_process(net_recvmsg_p mail)
{
	net_keydelete *remote;
	rt_uint16_t 	keypos;
	rt_uint32_t 	date;
	rt_int32_t		KeyOpResult;
	
	remote = &(mail->data.keydel.key);

	net_string_copy_uint16(&keypos,remote->pos);
  net_string_copy_uint32(&date,remote->date);

	if(keypos == 0)
	{
    RT_DEBUG_LOG(SHOW_NETKEY_INFO,("Not del admin key\n"));
    return RT_ERROR;
	}
	RT_DEBUG_LOG(SHOW_NETKEY_INFO,("key delete data info:>>>>>>\n"));
	RT_DEBUG_LOG(SHOW_NETKEY_INFO,("keypos   = %d\n",keypos));
	RT_DEBUG_LOG(SHOW_NETKEY_INFO,("date     = %x\n",date));
	KeyOpResult = device_config_key_delete(keypos,date,1);
	  
	if(KeyOpResult < 0)
	{
  	RT_DEBUG_LOG(SHOW_NETKEY_INFO,("Remote delete keys to fail!!!\n"));
		return RT_ERROR;
	}
	
  RT_DEBUG_LOG(SHOW_NETKEY_INFO,("Remote delete keys to success!!!\n"));
	return RT_EOK;
}

