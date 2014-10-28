#include "bluetooth_device.h"
//#include "sysconfig.h"
#include "rtdevice.h"
#include "netprotocol.h"

#ifndef BTM_THREAD_PRIORITY
#define BTM_THREAD_PRIORITY   			RT_THREAD_PRIORITY_MAX/2+2
#endif
//#define USEING_BT_C_CONN      //使用蓝牙主动连接
#define RT_DRIVE_NAME         "Blooth"
#define RT_USEING_DEBUG				1
#define BT_USEING_UARTX_NAME	"uart1"
#define BT_USEING_WK_NAME			"BT_WK"
#define BT_USEING_LED_NAME		"BT_LED"
#define BT_USEING_RST_NAME		"BT_RST"

#define BT_SYSTEM_ERROR_INFO	"The bluetooth module systematic errors function:%s line:%d"

#define BT_AT_CMD_DATA_BUFFER 64
#define BT_DATA_RCV_BUF_SIZE  4096//接收buffer
#define BT_DATA_PAGE_SIZE     60 	//每页大小
#define BT_CONNECT_OUTTIME    2000//连接超时2s
#define BT_AUTO_SLEEP_TIME		100000
//模块状态
#define BT_MODULE_CMD_MODE     0
#define BT_MODULE_DATA_MODE		 1
#define BT_MODULE_CMD_ABNORMAL 3

//邮件类型
#define BT_MAIL_TYPE_SLEEP    0X01//断开睡眠
#define BT_MAIL_TYPE_CONN     0X02//连接
#define BT_MAIL_TYPE_STATUS   0X03

//连接状态
#define BT_NOW_CONNECT        0
#define BT_NOW_DISCONNECT     !BT_NOW_CONNECT

#define USEING_AUTO_CONN		

//蓝牙设备的MAC
typedef struct 
{
	rt_uint8_t target_mac[12];
	rt_uint8_t local_mac[12];
}
bluetooth_user,*bluetooth_user_p;

//蓝牙模块设备
typedef struct 
{
	rt_device_t uart_dev;
	rt_device_t	wk_dev;
	rt_device_t led_dev;
	rt_device_t rst_dev;
	rt_uint8_t  work_status;
	rt_uint32_t sleep_cnt;
}bluetooth_module,*bluetooth_module_p;

typedef struct
{
	rt_uint8_t    type;
	rt_mailbox_t 	tx_end;
}bluetooth_tx_mq,*bluetooth_tx_mq_p;

static struct rt_ringbuffer bt_ringbuf_rcv;
static rt_uint8_t rcv_buffer[BT_DATA_RCV_BUF_SIZE];

static rt_mq_t BloothRequest_mq = RT_NULL;

static struct rt_device  bluetooth_drive;

static bluetooth_user	BluetoothUerConfig = 
{
	"7C669D9F676C"
};

rt_size_t bluetooth_module_read(rt_uint8_t *buf,rt_size_t size);

//驱动设备初始化
static void device_init_processor(rt_device_t *dev,const char *dev_name)
{
  (*dev) = rt_device_find(dev_name);
	if((*dev) == RT_NULL)
	{
	  rt_kprintf("Device %s none find at %s:%d \n",dev_name, __FUNCTION__, __LINE__);
	  return ;
	}
	if(!((*dev)->open_flag & RT_DEVICE_OFLAG_OPEN))
	{
	  rt_kprintf("open %s device\n",(*dev)->parent.name);
	  rt_device_open((*dev),RT_DEVICE_OFLAG_RDWR);
	}
}

//驱动设备关闭
static void device_close_processor(rt_device_t dev)
{
	if(dev == RT_NULL)
	{
	  rt_kprintf("Device is RT_NULL not close at %s:%d \n", __FUNCTION__, __LINE__);
	  return ;
	}
	rt_device_close(dev);
}

//读取命令应答
static rt_err_t bt_uart_read_ack(rt_device_t uart
																,const char at_ack[]
																,rt_uint8_t rcv_buf[]
																,rt_uint16_t outtime)
{
  rt_uint8_t *uart_buffer;
  rt_uint8_t *uart_buffer1;
  rt_tick_t start_t,outtime_t;
  rt_err_t result = RT_EOK;

  //parameter detection
	RT_ASSERT(uart != RT_NULL);

	uart_buffer1 = uart_buffer = rt_calloc(1,BT_AT_CMD_DATA_BUFFER);
	if(uart_buffer == RT_NULL)
	{
		rt_kprintf("Memroy create failed at %s:%d \n", __FUNCTION__, __LINE__);
		return RT_ERROR;
	}

	start_t = rt_tick_get();
	outtime_t = rt_tick_get();
	while((outtime_t-start_t) < outtime)
	{
		rt_size_t read_size;
		
		outtime_t = rt_tick_get();

		read_size = rt_device_read(uart,0,uart_buffer1,BT_AT_CMD_DATA_BUFFER);
		if(read_size > 0)
		{
			uart_buffer1 += read_size;
			if(rt_strstr((const char *)uart_buffer,"\r\n") != RT_NULL)
			{
				break;
			}
		}
		else
		{
			rt_thread_delay(1);
		}
	}
	RT_DEBUG_LOG(RT_USEING_DEBUG,("UART>>>%s",uart_buffer));
	if((outtime_t-start_t) >= outtime)
	{
		RT_DEBUG_LOG(RT_USEING_DEBUG,("\nUART BUF :%s\n",uart_buffer));
		result = RT_ETIMEOUT;
	}
	else if((at_ack[0] == '\r')
					&& (at_ack[1] == '\n')
					|| (at_ack[0] == 0))
	{
		RT_DEBUG_LOG(RT_USEING_DEBUG,("AT Result EOK\n"));
		result = RT_EOK;
		if(rcv_buf != RT_NULL)
		{
			rt_memcpy((void *)rcv_buf,(const void *)uart_buffer,rt_strlen((const void *)uart_buffer));
		}
	}
	else
	{
		if(rt_strstr((const char *)uart_buffer,(const char *)at_ack) != RT_NULL)
		{
			RT_DEBUG_LOG(RT_USEING_DEBUG,("AT Result Ack EOK\n"));
      result = RT_EOK;
      if(rcv_buf != RT_NULL)
      {
        rt_memcpy((void *)rcv_buf,(const void *)uart_buffer,rt_strlen((const void *)uart_buffer));
      }
		}
		else
		{
			RT_DEBUG_LOG(RT_USEING_DEBUG,("AT Result ERROR\n"));
			result = RT_ERROR;
		}
	}

 	rt_free(uart_buffer);

 	return result;
}

/**
  * @brief  AT指令解析
  * @param  uart 串口设备
  * @param  at_cmd[]: 串口发送命令 为RT_NULL表示只读解析
  * @param  rcv_ack[]: 命令正确的应答字符串，解析后会更新rcv_ack  
  * @param  outtime: 等待应答的超时时间
  * @retval RT_EOK 表示解析正确 
  *					RT_ERROR 表示解析错误或超时
  */
static rt_err_t bt_at_cmd_analysis(rt_device_t uart
													,rt_uint8_t at_cmd[]
													,const char rcv_ack[]
													,rt_uint8_t rcv_buf[]
													,rt_uint16_t outtime)
{
	rt_err_t result = RT_EOK;
	rt_uint8_t run = 2;

	//parameter detection
	RT_ASSERT(uart != RT_NULL);
	RT_ASSERT(rcv_ack != RT_NULL);

	do
	{
		if(at_cmd != RT_NULL)
	  {
	    rt_device_write(uart,0,(const void *)at_cmd,rt_strlen((const void *)at_cmd));
	    result = bt_uart_read_ack(uart,rcv_ack,rcv_buf,outtime);
	    RT_DEBUG_LOG(RT_USEING_DEBUG,("Send: %s ok_recv: %s",at_cmd,rcv_ack));
	  }
	  else
	  {
	    result = bt_uart_read_ack(uart,rcv_ack,rcv_buf,outtime);    
	    RT_DEBUG_LOG(RT_USEING_DEBUG,("recv: %s",rcv_buf));
	  }

	  RT_DEBUG_LOG(RT_USEING_DEBUG,("result %x\n",result));
	  if(result == RT_EOK)
		{
				break;
		}
	}while(run--);
	
	return result;
}


static rt_err_t bt_module_create(bluetooth_module_p *module)
{
	(*module) = rt_calloc(1,sizeof(bluetooth_module));
	RT_ASSERT((*module) != RT_NULL);
	
	device_init_processor(&(*module)->uart_dev,BT_USEING_UARTX_NAME);
	device_init_processor(&(*module)->led_dev,BT_USEING_LED_NAME);
	device_init_processor(&(*module)->wk_dev,BT_USEING_WK_NAME);
	device_init_processor(&(*module)->rst_dev,BT_USEING_RST_NAME);
	(*module)->work_status = BT_MODULE_CMD_MODE;
	(*module)->sleep_cnt = 0;
	
	return RT_EOK;
}

static void bt_module_detele(bluetooth_module_p module)
{
	device_close_processor(module->uart_dev);
	device_close_processor(module->led_dev);
	device_close_processor(module->wk_dev);
	device_close_processor(module->rst_dev);
	rt_free(module);
}

static rt_err_t bluetooth_mac_manage(bluetooth_module_p module,rt_bool_t mode)
{	
	rt_uint8_t *buf = RT_NULL;
	rt_err_t	 result = RT_EOK;
	
	buf = rt_calloc(1,BT_AT_CMD_DATA_BUFFER);
	RT_ASSERT(buf != RT_NULL);

	if(mode == RT_TRUE)
	{
		//设置目标mac
		/*rt_memcpy(buf,"AT+CON[",rt_strlen("AT+CON["));
		rt_memcpy(buf+rt_strlen("AT+CON["),BluetoothUerConfig.target_mac,12);
		rt_memcpy(buf+rt_strlen("AT+CON[")+12,"]",1);

		rt_kprintf("\nbuf = %s\n",buf);
		result = bt_at_cmd_analysis(module->uart_dev,buf,"OK+CONN:S",RT_NULL,400);
		if(result == RT_ERROR)
		{
			result = bt_at_cmd_analysis(module->uart_dev,"AT+CONNL","OK+CONN:S",RT_NULL,400);
		}*/
		result = bt_at_cmd_analysis(module->uart_dev,"AT+CONNL","OK+CONN:S",RT_NULL,400);
	}
	else
	{
		//获得本地mac
		result = bt_at_cmd_analysis(module->uart_dev,"AT+MAC","OK+GET:",buf,50);
		if(result == RT_EOK)
		{
			rt_memcpy(BluetoothUerConfig.local_mac,buf+rt_strlen("OK+GET:"),12);
		}
	}

	rt_free(buf);
	return result;
}

static rt_err_t bluetooth_module_reset(bluetooth_module_p module)
{
	rt_uint8_t gpio_status;

	gpio_status = 0;
	rt_device_write(module->rst_dev,0,&gpio_status,1);
	rt_thread_delay(10);
	
  gpio_status = 1;
  rt_device_write(module->rst_dev,0,&gpio_status,1);
  bt_at_cmd_analysis(module->uart_dev,RT_NULL,"SYS START",RT_NULL,50);
  
	return bt_at_cmd_analysis(module->uart_dev,"AT+","OK+",RT_NULL,50);
}
static rt_err_t bluetooth_initiate(bluetooth_module_p module)
{
	rt_uint8_t gpio_status;

	rt_kprintf("Bluetooth Moudlue Reset\n");
	bluetooth_module_reset(module);
	//wk = 1
	gpio_status = 1;
	rt_device_write(module->wk_dev,0,&gpio_status,1);
	//处理连接状态
	rt_device_read(module->led_dev,0,&gpio_status,1);
	if(gpio_status == BT_NOW_CONNECT)
	{
    if(bt_at_cmd_analysis(module->uart_dev,"AT+DISCON","OK+DISCON",RT_NULL,50) != RT_EOK)
    {
      goto BT_INIT_EXIT;
    }
    do
    {
      rt_device_read(module->led_dev,0,&gpio_status,1);
      rt_thread_delay(10);
    }while(gpio_status == BT_NOW_CONNECT);
	}
	//设置LED
	if(bt_at_cmd_analysis(module->uart_dev,"AT+LED[N]","OK+SET:N",RT_NULL,50) != RT_EOK)
	{
    goto BT_INIT_EXIT;
	}
	//配置从机
	if(bt_at_cmd_analysis(module->uart_dev,"AT+ROLE[P]","OK+SET:P",RT_NULL,50) != RT_EOK)
	{
    goto BT_INIT_EXIT;
	}
	rt_thread_delay(RT_TICK_PER_SECOND/4);
	//启动信息
	if(bt_at_cmd_analysis(module->uart_dev,RT_NULL,"SYS START",RT_NULL,50) != RT_EOK)
	{
    goto BT_INIT_EXIT;
	}
	//获得MAC
	bluetooth_mac_manage(module,RT_FALSE);
	//配置通知
	if(bt_at_cmd_analysis(module->uart_dev,"AT+NOTI[Y]","OK+SET:Y",RT_NULL,50) != RT_EOK)
	{
    goto BT_INIT_EXIT;
	}
	//进入休眠
	if(bt_at_cmd_analysis(module->uart_dev,"AT+SLEEP","OK+SLEEP",RT_NULL,50) != RT_EOK)
	{
    goto BT_INIT_EXIT;
	}

	return RT_EOK;
BT_INIT_EXIT:
	return RT_ERROR;
}

static rt_err_t bluetooth_conn_status(rt_uint8_t *status)
{
	bluetooth_tx_mq tx_mq;
	rt_err_t 				result;
	rt_uint32_t     mail_result; 
	
	tx_mq.tx_end = rt_mb_create("bt_sleep",1,RT_IPC_FLAG_FIFO);
	RT_ASSERT(tx_mq.tx_end != RT_NULL);
	RT_ASSERT(BloothRequest_mq != RT_NULL);

	
  tx_mq.type = BT_MAIL_TYPE_STATUS;
	result = rt_mq_send(BloothRequest_mq,&tx_mq,sizeof(bluetooth_tx_mq));
	if(result != RT_EOK)
	{
	  rt_mb_delete(tx_mq.tx_end);
	  return RT_ERROR;
	}
	result = rt_mb_recv(tx_mq.tx_end,&mail_result,RT_WAITING_FOREVER);
	if(result != RT_EOK)
	{
	  rt_mb_delete(tx_mq.tx_end);
	  return RT_ERROR;
	}

 	*status = mail_result;

  rt_mb_delete(tx_mq.tx_end);
	return RT_EOK;

}
static rt_err_t bluetooth_sleep(void)
{
	bluetooth_tx_mq tx_mq;
	rt_err_t 				result;
	rt_uint32_t     mail_result; 
	
	tx_mq.tx_end = rt_mb_create("bt_sleep",1,RT_IPC_FLAG_FIFO);
	RT_ASSERT(tx_mq.tx_end != RT_NULL);
	RT_ASSERT(BloothRequest_mq != RT_NULL);

	
  tx_mq.type = BT_MAIL_TYPE_SLEEP;
	result = rt_mq_send(BloothRequest_mq,&tx_mq,sizeof(bluetooth_tx_mq));
	if(result != RT_EOK)
	{
	  rt_mb_delete(tx_mq.tx_end);
	  return RT_ERROR;
	}
	result = rt_mb_recv(tx_mq.tx_end,&mail_result,RT_WAITING_FOREVER);
	if(result != RT_EOK)
	{
	  rt_mb_delete(tx_mq.tx_end);
	  return RT_ERROR;
	}

  rt_mb_delete(tx_mq.tx_end);
	return RT_EOK;
}

static void bluetooth_uart_read_data(bluetooth_module_p module)
{
  rt_uint8_t *data_buf = RT_NULL;
  rt_size_t  read_len;
 
	data_buf = rt_calloc(1,BT_DATA_PAGE_SIZE);
	RT_ASSERT(data_buf != RT_NULL);

	read_len = rt_device_read(module->uart_dev,0,(void *)data_buf,BT_DATA_PAGE_SIZE);
	if(read_len > 0)
	{
		rt_ringbuffer_put(&bt_ringbuf_rcv,data_buf,read_len);
		module->sleep_cnt = 0;

		if(RT_USEING_DEBUG == 1)
		{
			rt_size_t  i;
  		rt_uint8_t *buf_p;

  		buf_p = data_buf;
			rt_kprintf("\nBluetooth received data:\n");
			for(i=0;i<read_len;i++)
			{
				rt_kprintf("%02X",*(buf_p+i));
			}
			rt_kprintf("\n");
		}
	}
	rt_free(data_buf);
}

static rt_err_t bluetooth_connect(void)
{
	bluetooth_tx_mq tx_mq;
	rt_err_t 				result;
	rt_uint32_t     mail_result;
	
	tx_mq.tx_end = rt_mb_create("bt_conn",1,RT_IPC_FLAG_FIFO);
	RT_ASSERT(tx_mq.tx_end != RT_NULL);
	RT_ASSERT(BloothRequest_mq != RT_NULL);

	
  tx_mq.type = BT_MAIL_TYPE_CONN;
	result = rt_mq_send(BloothRequest_mq,&tx_mq,sizeof(bluetooth_tx_mq));
	if(result != RT_EOK)
	{
	  rt_mb_delete(tx_mq.tx_end);
	  return RT_ERROR;
	}
	result = rt_mb_recv(tx_mq.tx_end,&mail_result,RT_WAITING_FOREVER);
	if(result != RT_EOK)
	{
	  rt_mb_delete(tx_mq.tx_end);
	  return RT_ERROR;
	}

  rt_mb_delete(tx_mq.tx_end);
	return RT_EOK;
}


static void bluetooth_cmd_switch(bluetooth_module_p bluetooth,rt_bool_t mode)
{
	rt_uint8_t gpio_status;

	if(RT_TRUE == mode)
	{
    bluetooth->work_status = BT_MODULE_CMD_MODE;
		gpio_status = 1;
	}
	else
	{
    bluetooth->work_status = BT_MODULE_DATA_MODE;
    gpio_status = 0;
	}
	rt_device_write(bluetooth->wk_dev,0,&gpio_status,1);
	rt_thread_delay(1);
}

//主动连接
static rt_err_t bluetooth_auto_connect(bluetooth_module_p bluetooth)
{
	rt_uint8_t 	gpio_status;
	rt_tick_t		start_t;
	rt_tick_t		outtime_t;
	 
  //唤醒
	if(bt_at_cmd_analysis(bluetooth->uart_dev,"AT+WAKE","OK+WUAKE",RT_NULL,50) != RT_EOK)
	{
	  return RT_ERROR;
	}
	rt_thread_delay(200);
	gpio_status = 1;
	rt_device_write(bluetooth->wk_dev,0,&gpio_status,1);
	//配置成主机
	if(bt_at_cmd_analysis(bluetooth->uart_dev,"AT+ROLE[C]","OK+SET:C",RT_NULL,50) != RT_EOK)
	{
	  return RT_ERROR;
	}
	rt_thread_delay(RT_TICK_PER_SECOND/4);
	//启动信息
	if(bt_at_cmd_analysis(bluetooth->uart_dev,RT_NULL,"SYS START",RT_NULL,50) != RT_EOK)
	{
	  return RT_ERROR;
	}
	rt_thread_delay(100);

	//MAC管理连接
	if(bluetooth_mac_manage(bluetooth,RT_TRUE) != RT_EOK)
	{
		return RT_ERROR;
	}
	
	//连接
	/*if(bt_at_cmd_analysis(bluetooth->uart_dev,"AT+CONNL","OK+CONN:S",RT_NULL,400) != RT_EOK)
	{
	  return RT_ERROR;
	}*/
	
	rt_thread_delay(300);
	//关闭远程功能
	gpio_status = 0;
	rt_device_write(bluetooth->wk_dev,0,&gpio_status,1);
	//等待连接成功

	outtime_t = start_t = rt_tick_get();
	do
	{
	  rt_device_read(bluetooth->led_dev,0,&gpio_status,1);
	  if(gpio_status == BT_NOW_CONNECT)
	  {
			break;
	  }
	  rt_thread_delay(1);
	  outtime_t = rt_tick_get();
	}
	while((outtime_t - start_t) < BT_CONNECT_OUTTIME);
  bluetooth->work_status = BT_MODULE_DATA_MODE;

	//握手
  return RT_EOK;

}

//主动断开连接
static rt_err_t bluetooth_auto_disconnect(bluetooth_module_p bluetooth)
{
	rt_uint8_t 	gpio_status;
	rt_tick_t		start_t;
	rt_tick_t		outtime_t;
	
  //开启远程控制
	bluetooth_cmd_switch(bluetooth,RT_TRUE);
	
  //断开连接
  if(bt_at_cmd_analysis(bluetooth->uart_dev,"AT+DISCON","OK+DISCON",RT_NULL,50) != RT_EOK)
  {
    //bluetooth_cmd_switch(bluetooth,RT_TRUE);
    rt_device_read(bluetooth->led_dev,0,&gpio_status,1);
    if(gpio_status == BT_NOW_CONNECT)
    {
    	bluetooth_cmd_switch(bluetooth,RT_FALSE);
    	return RT_ERROR;
    }
  }
  start_t = outtime_t = rt_tick_get();
  //等待断开
  do
  {
    rt_device_read(bluetooth->led_dev,0,&gpio_status,1);
    if(gpio_status != BT_NOW_CONNECT)
    {
    	break;
    }
    rt_thread_delay(1);
    outtime_t = rt_tick_get();
   	if((outtime_t - start_t) > BT_CONNECT_OUTTIME)
   	{
   		//不能断开
   		bluetooth_cmd_switch(bluetooth,RT_FALSE);
			return RT_ERROR;
   	}
  }while(1);
  //休眠
  if(bt_at_cmd_analysis(bluetooth->uart_dev,"AT+SLEEP","OK+SLEEP",RT_NULL,50) != RT_EOK)
  {
  	bluetooth_cmd_switch(bluetooth,RT_TRUE);
    return RT_ERROR;
  }

	//进入命令模式
 	bluetooth_cmd_switch(bluetooth,RT_TRUE);
  
	return RT_EOK;
}

//被动连接
static void bluetooth_passivity_connect(bluetooth_module_p bluetooth)
{
	rt_uint8_t 	gpio_status;
	
	rt_device_read(bluetooth->led_dev,0,&gpio_status,1);
	if(gpio_status == BT_NOW_CONNECT)
	{
		//唤醒脉冲
		gpio_status = 0;
		rt_device_write(bluetooth->wk_dev,0,&gpio_status,1);
		rt_thread_delay(1);
		gpio_status = 1;
		rt_device_write(bluetooth->wk_dev,0,&gpio_status,1);
		rt_thread_delay(1);
		gpio_status = 0;
		rt_device_write(bluetooth->wk_dev,0,&gpio_status,1);
		bluetooth->work_status = BT_MODULE_DATA_MODE;

		//接收提示信息
		bt_at_cmd_analysis(bluetooth->uart_dev,RT_NULL,"OK+CONN:S",RT_NULL,500);
		//bt_at_cmd_analysis(bluetooth->uart_dev,RT_NULL,"OK+WUAKE",RT_NULL,500);
		//握手信息
		rt_device_write(bluetooth->uart_dev,0,"Bluetooth\n",rt_strlen("Bluetooth\n"));
		bluetooth_cmd_switch(bluetooth,RT_FALSE);
	}
}

//被动断开连接
static rt_err_t bluetooth_passivity_disconnect(bluetooth_module_p bluetooth)
{
	//进入命令模式
	rt_uint8_t *buf;

	buf = rt_calloc(1,16);
	RT_ASSERT(buf != RT_NULL);
	
	bluetooth_cmd_switch(bluetooth,RT_TRUE);

	bluetooth_module_read(buf,12);

	if(rt_strstr((const char *)buf,"OK+CONN:L") == RT_NULL)
	{
    if(bt_at_cmd_analysis(bluetooth->uart_dev,RT_NULL,"OK+CONN:L",RT_NULL,50) != RT_EOK)
    {
      //return RT_ERROR;
    }
	}
	rt_free(buf);
	
	//配置从机
	if(bt_at_cmd_analysis(bluetooth->uart_dev,"AT+ROLE[P]","OK+SET:P",RT_NULL,50) == RT_EOK)
	{
		rt_thread_delay(RT_TICK_PER_SECOND/4);
		//获取启动信息
		if(bt_at_cmd_analysis(bluetooth->uart_dev,RT_NULL,"SYS START",RT_NULL,50) != RT_EOK)
		{
			return RT_ERROR;
		}
	}
	
	//进入休眠
	if(bt_at_cmd_analysis(bluetooth->uart_dev,"AT+SLEEP","OK+SLEEP",RT_NULL,50) != RT_EOK)
	{
		return RT_ERROR;
	}
	rt_thread_delay(300);
	
	return RT_EOK;
}

//透传写
rt_size_t bluetooth_module_wirte(rt_uint8_t *buf,rt_size_t size)
{
	//发送邮件
	//等待进入透传模式
	//发送数据
	//返回结果
	bluetooth_tx_mq tx_mq;
	rt_err_t 				result;
	rt_device_t			uart;
	rt_size_t       i;
	rt_uint32_t     page;
	rt_uint8_t      *bufp = buf;
	rt_size_t       send_num = 0;

	tx_mq.tx_end = rt_mb_create("bt_tx",1,RT_IPC_FLAG_FIFO);
	RT_ASSERT(tx_mq.tx_end != RT_NULL);
	RT_ASSERT(BloothRequest_mq != RT_NULL);

	tx_mq.type = BT_MAIL_TYPE_CONN;
	result = rt_mq_send(BloothRequest_mq,&tx_mq,sizeof(bluetooth_tx_mq));
	if(result != RT_EOK)
	{
		rt_mb_delete(tx_mq.tx_end);
		return 0;
	}
	result = rt_mb_recv(tx_mq.tx_end,(void *)&uart,RT_TICK_PER_SECOND*30);
	if(result != RT_EOK)
	{
		rt_mb_delete(tx_mq.tx_end);
		return 0;
	}

	RT_DEBUG_LOG(RT_USEING_DEBUG,("Start Send Data:\n",i));
	page = size / BT_DATA_PAGE_SIZE;
	send_num = 0;
	for(i=0; i<page; i++)
	{
    send_num += rt_device_write(uart,0,bufp,BT_DATA_PAGE_SIZE);
    bufp += BT_DATA_PAGE_SIZE;
    rt_thread_delay(30);
	}
	send_num += rt_device_write(uart,0,bufp,(size - (BT_DATA_PAGE_SIZE*page)));
	rt_thread_delay(30);

	//tx_mq.type = BT_MAIL_TYPE_SLEEP;
	//result = rt_mq_send(BloothRequest_mq,&tx_mq,sizeof(bluetooth_tx_mq));

	
	RT_DEBUG_LOG(RT_USEING_DEBUG,("%s send_num:%d Send Data end!!!\n",uart->parent.name,send_num));
	rt_mb_delete(tx_mq.tx_end);
	
	return send_num;
}

//透传读
rt_size_t bluetooth_module_read(rt_uint8_t *buf,rt_size_t size)
{
	//检测是否为透传模式
	//读取数据
	return rt_ringbuffer_get(&bt_ringbuf_rcv,buf,size);
}

void bluetooth_thread_entry(void *arg)
{
	bluetooth_module_p bluetooth;
	rt_kprintf("bluetooth thread statrt\n");

	bt_module_create(&bluetooth);
	while(1)
	{
	  bluetooth_initiate(bluetooth);
	  rt_thread_delay(1000);
	  while(1)
	  { 
	    rt_uint8_t mb_result;
	    bluetooth_tx_mq mail;
	    rt_uint8_t gpio_status;

	    if(bluetooth->work_status == BT_MODULE_CMD_MODE)
	    {
	      mb_result = rt_mq_recv(BloothRequest_mq,&mail,sizeof(bluetooth_tx_mq),1);
	      if(mb_result == RT_EOK)
	      {
	        bluetooth->sleep_cnt = 0;
	        //主动连接
	        switch(mail.type)
	        {
	          case BT_MAIL_TYPE_CONN:
	          {
	          	rt_kprintf("Bluetooth C Client-initialized\n");
	          	#ifdef USEING_BT_C_CONN
	            if(bluetooth_auto_connect(bluetooth) != RT_EOK)
	            {
	              goto LINK_ON_AT_CMD_ABNORMAL;
	            }
	            #endif
	            rt_mb_send(mail.tx_end,(rt_uint32_t )bluetooth->uart_dev);
	            break;
	          }
	          case BT_MAIL_TYPE_SLEEP:
	          {
	          	rt_kprintf("Bluetooth C  Sleep Preventer\n");
	            rt_mb_send(mail.tx_end,RT_NULL);
	            break;
	          }
	          case BT_MAIL_TYPE_STATUS:
	          { 
	            rt_mb_send(mail.tx_end,(rt_uint32_t )bluetooth->work_status);
	            break;
	          }
	          default:
	          {
	            break;
	          }
	        }
	      }
	      else
	      {
	        //被动连接
					rt_device_read(bluetooth->led_dev,0,&gpio_status,1);
					if(gpio_status == BT_NOW_CONNECT)
					{
						bluetooth_passivity_connect(bluetooth);
						net_event_process(0,NET_ENVET_RELINK);
					}
	      }
	    }
	    else
	    {
	      //已经连接
	      rt_device_read(bluetooth->led_dev,0,&gpio_status,1);
	      if(gpio_status == BT_NOW_CONNECT)
	      {
	        //在数据模式下收到发送处理
	        mb_result = rt_mq_recv(BloothRequest_mq,&mail,sizeof(bluetooth_tx_mq),1);
	        if(mb_result == RT_EOK)
	        {
	          bluetooth->sleep_cnt = 0;
	          switch(mail.type)
	          {
	            case BT_MAIL_TYPE_CONN:
	            {
	              //主动连接请求
	              rt_mb_send(mail.tx_end,(rt_uint32_t )bluetooth->uart_dev);
	              break;
	            }
	            case BT_MAIL_TYPE_SLEEP:
	            {
	              //主动断开
	              bluetooth_auto_disconnect(bluetooth);
	              rt_mb_send(mail.tx_end,RT_NULL);
	              break;
	            }
	            case BT_MAIL_TYPE_STATUS:
	            { 
	              rt_mb_send(mail.tx_end,(rt_uint32_t )bluetooth->work_status);
	              break;
	            }
	            default:
	            {
	              rt_kprintf(BT_SYSTEM_ERROR_INFO,__FUNCTION__, __LINE__);
	              break;
	            }
	          }
	        }
	        else
	        {
	          bluetooth->sleep_cnt++;
	          if(bluetooth->sleep_cnt > BT_AUTO_SLEEP_TIME)
	          {
	            rt_kprintf("The bluetooth module into sleep mode automatically");
	            bluetooth_auto_disconnect(bluetooth);
	          }
	        }
	        
	        //读取数据
	        bluetooth_uart_read_data(bluetooth);
	      }
	      else
	      {
	        //连接被动断开
          rt_uint8_t status = 0;
					rt_uint8_t run = 10;
	        while(run--)
	        {
            rt_device_read(bluetooth->led_dev,0,&gpio_status,1);
            if(gpio_status != BT_NOW_CONNECT)
						{	
							status++;
						}
						rt_thread_delay(1);
	        }
	        if((bluetooth->work_status == BT_MODULE_DATA_MODE) && (status > 5))
	        {
	        	rt_kprintf("Bluetooth Server Disconnected \n");
						if(bluetooth_passivity_disconnect(bluetooth) != RT_EOK)
						{
							//break;
						}
						rt_kprintf("bluetooth->work_status = %d\n",bluetooth->work_status);
	        }
	        else
	        {
	        	//数据模式信号抖动
	        	if(bluetooth->work_status == BT_MODULE_DATA_MODE)
	        	{
              rt_kprintf("Bluetooth Status Pin instability \n");
	        	}
	        }
	      }
	      rt_thread_delay(1);
	    }
	    
	    if(bluetooth->work_status == BT_MODULE_CMD_ABNORMAL)
	    {
LINK_ON_AT_CMD_ABNORMAL:
				break;
	    }
	    rt_thread_delay(1);
	  }

	}

	bt_module_detele(bluetooth);
}

int bluetooth_thread_init(void)
{
	rt_thread_t thread;

	rt_ringbuffer_init(&bt_ringbuf_rcv,rcv_buffer,BT_DATA_RCV_BUF_SIZE);
	
	BloothRequest_mq = rt_mq_create("tx"
													,sizeof(bluetooth_tx_mq)
													,2
													,RT_IPC_FLAG_FIFO);

	thread = rt_thread_create("BT_M",
	                           bluetooth_thread_entry, RT_NULL,
	                           1024,BTM_THREAD_PRIORITY,10);
	if(thread != RT_NULL)
	{
    rt_thread_startup(thread);
	}
	
	return 0;
}
INIT_APP_EXPORT(bluetooth_thread_init);


static rt_size_t rt_bluetooth_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
	return bluetooth_module_read(buffer,size);
}

static rt_size_t rt_bluetooth_write(rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size)
{

  return bluetooth_module_wirte((rt_uint8_t *)buffer,size);

}

static rt_err_t  rt_bluetooth_control(rt_device_t dev, rt_uint8_t cmd, void *args)
{
	switch(cmd)
	{
		case 1:
		{
			//自动连接
			bluetooth_connect();
			break;
		}
		case 2:
		{
			//断开进入睡眠
			bluetooth_sleep();
			break;
		}
		case 3:
		{
			//连接状态
			if(args != RT_NULL)
			{
				//rt_kprintf("Get the bluetooth MAC\n");
				bluetooth_conn_status((rt_uint8_t *)args);
			}
			break;
		}
		case 4:
		{
			//获取MAC
			if(args != RT_NULL)
			{
				rt_memcpy(args,BluetoothUerConfig.local_mac,12);
			}
			break;
		}
		default :
		{
			break;
		}
	}

	return RT_EOK;
}

rt_err_t rt_hw_bluetooth_register(void)
{
    struct rt_device *device;

		device = &bluetooth_drive;
    device->type        = RT_Device_Class_Char;
    device->rx_indicate = RT_NULL;
    device->tx_complete = RT_NULL;

    device->init        = RT_NULL;
    device->open        = RT_NULL;
    device->close       = RT_NULL;
    device->read        = rt_bluetooth_read;
    device->write       = rt_bluetooth_write;
    device->control     = rt_bluetooth_control;
    device->user_data   = RT_NULL;

    /* register a character device */
    return rt_device_register(device, RT_DRIVE_NAME, RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STANDALONE);
}

int rt_bluetooth_drive_init(void)
{
  rt_hw_bluetooth_register();

  return 0;
}
INIT_DEVICE_EXPORT(rt_bluetooth_drive_init);

#ifdef RT_USING_FINSH
#include <finsh.h>

void at_test(const char cmd,const char *ack)
{
	rt_device_t uart;
	rt_uint8_t  *buf = RT_NULL;
	rt_err_t result;

	buf = rt_calloc(1,128);

	rt_memcpy((void *)buf,(const void*)ack,rt_strlen((const void*)ack));

	uart = rt_device_find("uart3");
	if(uart == RT_NULL)
	{
		rt_kprintf("This Name UART device none find!\n");
		return ;
	}
	rt_device_open(uart,RT_DEVICE_OFLAG_RDWR);

	result = bt_at_cmd_analysis(uart,(rt_uint8_t *)cmd,ack,buf,50);
	
	rt_kprintf("AT CMD result %x, -- %s\n",result,buf);
	rt_free(buf);
}
FINSH_FUNCTION_EXPORT(at_test, at_test);


void bt_write_test(void)
{
	rt_uint8_t buf[1024];
	rt_uint32_t i,j;
	for(j = 0; j < 1024/BT_DATA_PAGE_SIZE;  j++)
	for(i = 0; i < BT_DATA_PAGE_SIZE;  i++)
	{
		buf[j*BT_DATA_PAGE_SIZE+i] = 'A'+j;
	}
	
	bluetooth_module_wirte(buf,1024);
}
FINSH_FUNCTION_EXPORT(bt_write_test, bt_write_test);

void bt_read_test(void)
{
	rt_uint8_t buf[1024];
	rt_size_t  len;
	rt_uint32_t i;

	rt_memset(buf,0,1024);
	len = bluetooth_module_read(buf,1024);
	rt_kprintf("len:%d:data>>>\n",len);	
	for(i=0; i<1024; i++)
	{
  	rt_kprintf("%c",buf[i]);
	}
}
FINSH_FUNCTION_EXPORT(bt_read_test, bt_read_test);

void bt_sleep_test(void)
{
	bluetooth_sleep();
}
FINSH_FUNCTION_EXPORT(bt_sleep_test, bt_sleep_test);

void bt_info(void)
{
	rt_device_t dev;
	rt_uint8_t	*mac;
	rt_uint8_t status;
	
	mac = rt_calloc(1,13);
	dev = rt_device_find("Blooth");
	if(!(dev->open_flag & RT_DEVICE_OFLAG_OPEN))
	{
		rt_kprintf("open blooth module\n");
		rt_device_open(dev,RT_DEVICE_OFLAG_OPEN);
	}
	rt_device_control(dev,4,mac);
	rt_kprintf("MAC:%s\n",mac);
  rt_free(mac);

	rt_device_control(dev,3,&status);
	rt_kprintf("status:%d\n",status);
}
FINSH_FUNCTION_EXPORT(bt_info,show blooth module information);

#endif 




