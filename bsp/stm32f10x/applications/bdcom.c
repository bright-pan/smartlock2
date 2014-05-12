#include "bdcom.h"
#include "comm.h"
#include "untils.h"

#define SHOW_GSM_OP_INFO   1
#define GPRS_PACK_RESEND_T 200
#define GSM_MODE_SWITCH_T  100
#define GSM_OPEN_WAIT_T    100
#define GSM_CLOSE_WAIT_T   100
#define GSM_LINK_WAIT_T    300


rt_mailbox_t GSM_Mail_mb = RT_NULL; //GSM模块邮件

typedef struct
{
	rt_uint32_t WorkStatus:1; //开关 关0 开1
	rt_uint32_t WorkMode:1;   //cmd模式0 gprs模式1
	rt_uint32_t LinkStatus:1; //链接状态正常1 断线0
}GSM_Module,*GSM_Module_p;

static GSM_Module GSMModule;//gsm模块对象
static rt_mutex_t GSMMutex = RT_NULL;



/** 
@brief 	close gsm module
@param 	void
@retval RT_TRUE :succeed,RT_FALSE:fail
*/
rt_bool_t gsm_close(void)
{
  rt_err_t  result;

	result = send_ctx_mail(COMM_TYPE_GSM_CTRL_CLOSE,0,GSM_CLOSE_WAIT_T,"\x01",1);
	if(result ==  CTW_STATUS_OK)
	{
		RT_DEBUG_LOG(SHOW_GSM_OP_INFO,("Close GSM Module\n"));
		return RT_TRUE;
	}

	return RT_FALSE;
}

/** 
@brief	Open GSM module
@param  void 
@retval RT_TRUE :succeed,RT_FALSE:fail
*/

rt_bool_t gsm_open(void)
{
  rt_err_t  result;
  
	result = send_ctx_mail(COMM_TYPE_GSM_CTRL_OPEN,0,GSM_OPEN_WAIT_T,"\x01",1);
	if(result ==  CTW_STATUS_OK)
	{
	  RT_DEBUG_LOG(SHOW_GSM_OP_INFO,("Open GSM Module\n"));
	  return RT_TRUE;
	}

	return RT_FALSE;
}

/** 
@brief 
@param 
@retval 
*/
rt_bool_t gsm_setup(GSM_Module_p gsm)
{	
	if(gsm_close() == RT_TRUE)
	{
		if(gsm_open() == RT_TRUE)
		{
			gsm->WorkStatus = 1;
			gsm->LinkStatus = 0;
			gsm->WorkMode = 0;

			return RT_TRUE;
		}
	}

	return RT_FALSE;
}

/** 
@brief 
@param 
@retval 
*/
rt_bool_t gsm_switch_mode(GSM_Module_p gsm,rt_uint8_t mode)
{
	rt_err_t  result;
		
	if(gsm->WorkMode != mode)
	{
		if(mode == 0)//cmd mdoe
		{
      result =  send_ctx_mail(COMM_TYPE_GSM_CTRL_SWITCH_TO_CMD,0,GSM_MODE_SWITCH_T,"\x01",1);
	    if(result == CTW_STATUS_OK)
	    {
	      RT_DEBUG_LOG(SHOW_GSM_OP_INFO,("SWTICH AT CMD Mode\n"));
	      gsm->WorkMode = 0;
	      return RT_TRUE;
	    }
			return RT_FALSE;
		}
		else //gprs mode
		{
			result =  send_ctx_mail(COMM_TYPE_GSM_CTRL_SWITCH_TO_GPRS,0,GSM_MODE_SWITCH_T,"\x01",1);
			if(result == CTW_STATUS_OK)
			{
			  RT_DEBUG_LOG(SHOW_GSM_OP_INFO,("SWTICH GPRS Mode\n"));
			  gsm->WorkMode = 1;
			  return RT_TRUE;
			}
			return RT_FALSE;
		}
	}
	return RT_TRUE;
}

/** 
@brief 
@param 
@retval 
*/
rt_bool_t gsm_link(GSM_Module_p gsm)
{
	rt_err_t  result;
	rt_uint8_t *buf = rt_malloc(512);

	gsm_switch_mode(gsm,0);
	*buf = 1;
  RT_DEBUG_LOG(SHOW_GSM_OP_INFO,("Connect IP :%s ...\n",device_config.param.tcp_domain[0].domain));
	rt_memcpy(buf+1, &(device_config.param.tcp_domain[0]), sizeof(device_config.param.tcp_domain[0]));
	
	result = send_ctx_mail(COMM_TYPE_GSM_CTRL_DIALING, 
												0, 
												GSM_LINK_WAIT_T, 
												buf, 
												sizeof(device_config.param.tcp_domain[0])+1);
	rt_free(buf);
	if(result == CTW_STATUS_OK)
	{
		RT_DEBUG_LOG(SHOW_GSM_OP_INFO,("GSM Link IP\n"));
		gsm->LinkStatus = 1;
		gsm->WorkMode = 1;
		return RT_TRUE;
	}
	gsm->LinkStatus = 0;
	
	return RT_FALSE;
}

/** 
@brief 
@param 
@retval 
*/
void mail_gprs_process(GSM_Mail_p Mail)
{
	rt_err_t  result;
	rt_uint8_t *buf;
	
	buf = rt_calloc(1,Mail->BufSize+1);
	*buf = Mail->SendMode;
	rt_memcpy(buf+1,Mail->buf,Mail->BufSize);
	result = send_ctx_mail(COMM_TYPE_GPRS,0,GPRS_PACK_RESEND_T,buf,Mail->BufSize+1);
  if(result == CTW_STATUS_OK)
  {
    RT_DEBUG_LOG(SHOW_GSM_OP_INFO,("COM Send Data OK\n"));
  }
  else
  {
    RT_DEBUG_LOG(SHOW_GSM_OP_INFO,("COM send Data Fail\n"));
    GSMModule.LinkStatus = 0;
    GSMModule.WorkMode = 0;
  }
  rt_free(buf);
}

/** 
@brief 
@param 
@retval 
*/
void mail_sms_process(GSM_Mail_p Mail)
{
	rt_err_t  result;
	rt_uint8_t *buf;

	buf = rt_calloc(1,Mail->BufSize+1);
	*buf = Mail->SendMode;
	rt_memcpy(buf+1,Mail->buf,Mail->BufSize);
	result = send_ctx_mail(COMM_TYPE_SMS,0,GPRS_PACK_RESEND_T,buf,Mail->BufSize+1);
	if(result == CTW_STATUS_OK)
	{
		RT_DEBUG_LOG(SHOW_GSM_OP_INFO,("COM Send Data OK\n"));
	}
	else
	{
		RT_DEBUG_LOG(SHOW_GSM_OP_INFO,("COM send Data Fail\n"));
		GSMModule.LinkStatus = 0;
	}
	rt_free(buf);
}

/** 
@brief 
@param 
@retval 
*/
void gsm_entry_init(void)
{
  rt_uint8_t retry; 
  rt_uint8_t linkcnt;

  linkcnt = 5;
	while(linkcnt--)
	{
		retry = 3;
		while((!GSMModule.WorkStatus) & retry)
		{
		  if(gsm_setup(&GSMModule) == RT_TRUE)
		  {
		    break;
		  }
		  retry--;
		  if(retry == 0)
		  {
		    rt_thread_delay(1000);
		    retry = 3;
		  }
		}

		retry = 3;
		while((!GSMModule.LinkStatus) & retry)
		{
		  if(gsm_link(&GSMModule) == RT_TRUE)
		  {
		    return;
		  }
		  retry--;
		}
	}
}

/** 
@brief 
@param 
@retval 
*/
void gsm_mail_process(GSM_Mail_p Mail)
{
	
  gsm_entry_init();
  
	switch(Mail->type)
	{
		case GSM_MAIL_SMS:
		{
			gsm_switch_mode(&GSMModule,0);
			mail_sms_process(Mail);
			break;
		}
		case GSM_MAIL_MMS:
		{
			gsm_switch_mode(&GSMModule,0);
			break;
		}
		case GSM_MAIL_GPRS:
		{
			gsm_switch_mode(&GSMModule,1);
			mail_gprs_process(Mail);
			break;
		}
		case GSM_MAIL_LINK:
		{
			gsm_switch_mode(&GSMModule,1);
			break;
		}
		default:
		{
			break;
		}
	}
}

/** 
@brief 
@param 
@retval 
*/
void GSM_manage_thread_entry(void *arg)
{
	rt_kprintf("entry com thread \n");
	rt_memset(&GSMModule,0,sizeof(GSM_Module));
	gsm_entry_init();
	while(1)
	{
		rt_err_t   result;
		GSM_Mail_p GsmMail;

		result = rt_mb_recv(GSM_Mail_mb,(rt_uint32_t *)&GsmMail,RT_WAITING_FOREVER);
		if(result == RT_EOK)
		{
			gsm_mail_process(GsmMail);
			rt_sem_release(GsmMail->ResultSem);
		}
	}
}

/** 
@brief 
@param 
@retval 
*/
int GSM_manage_thread_init(void)
{
	rt_thread_t id;

	GSM_Mail_mb = rt_mb_create("GSMmb",5,RT_IPC_FLAG_FIFO);

	RT_ASSERT(GSM_Mail_mb != RT_NULL);
	
	id = rt_thread_create("DBCOM",
												GSM_manage_thread_entry,
												RT_NULL,
												512,
												102,
												100);
	if(RT_NULL == id )
	{
		rt_kprintf("camera thread create fail\n");
		return 1;
	}
	rt_thread_startup(id);
	
	return 0;
}
INIT_APP_EXPORT(GSM_manage_thread_init);

void gsm_set_link(rt_uint8_t status)
{
	GSMModule.LinkStatus = status;
}

rt_bool_t gsm_is_setup(void)
{
	if(GSMModule.WorkStatus) 
	{
		return RT_TRUE;
	}
	return RT_FALSE;
}
rt_bool_t gsm_is_link(void)
{
	if(GSMModule.LinkStatus)
	{
		return RT_TRUE;
	}
	return RT_FALSE;
}
void gsm_mail_send(GSM_Mail_p mail)
{
	if(GSM_Mail_mb == RT_NULL)
	{
		rt_kprintf("GSM_Mail_mb is RT_NULL\n");
		return ;
	}
	if(mail->type == GSM_MAIL_LINK)
	{
		GSMModule.LinkStatus = 0;
	}
	rt_mb_send(GSM_Mail_mb,(rt_uint32_t)mail);
}

/** 
@brief GSM Module Work mode Mutex
@param flag : RT_TRUE take RT_FALSE release
@retval void
*/
void gsm_mutex_operation(rt_bool_t flag)
{
	if(GSMMutex == RT_NULL)
	{
		GSMMutex = rt_mutex_create("GSMMode",RT_IPC_FLAG_FIFO);
		RT_ASSERT(GSMMutex);
	}
	if(flag == RT_TRUE)
	{
		rt_mutex_take(GSMMutex,RT_WAITING_FOREVER);
	}
	else
	{
		rt_mutex_release(GSMMutex);
	}
}
