#include "testcom.h"
#include "comm.h"

void com_test_entry(void *arg)
{
	rt_kprintf("entry com thread \n");
	while(1)
	{
		rt_err_t  result;

		result = send_ctx_mail(COMM_TYPE_GSM_CTRL_CLOSE,0,500,"\x01",1);
		if(result ==  CW_STATUS_OK)
		{
			rt_kprintf("close GSM\n");
		}
		
		result = send_ctx_mail(COMM_TYPE_GSM_CTRL_OPEN,0,500,"\x01",1);
		if(result ==  CW_STATUS_OK)
		{
			rt_kprintf("GSM Open\n");

			{
				rt_uint8_t *buf = rt_malloc(512);
				*buf = 1;
				rt_memcpy(buf+1, &(device_parameters.tcp_domain[0]), sizeof(device_parameters.tcp_domain[0]));
				result = send_ctx_mail(COMM_TYPE_GSM_CTRL_DIALING, 0, 1000, buf, sizeof(device_parameters.tcp_domain[0])+1);
				rt_free(buf);
				if(result == CW_STATUS_OK)
				{
					rt_kprintf("Set IP And Port\n");
				}
			}
			result =  send_ctx_mail(COMM_TYPE_GSM_CTRL_SWITCH_TO_GPRS,0,1000,"\x01",1);
			if(result == CW_STATUS_OK)
			{
				rt_kprintf("SWTICH GPRS Mode\n");
			}
			result = send_ctx_mail(COMM_TYPE_GPRS,0,1000,"\x00\x12\x34\x56\x78",sizeof("\x00\x12\x34\x56\x78"));
			if(result == CW_STATUS_OK)
			{
				rt_kprintf("Send Data\n");
			}

			result = send_ctx_mail(COMM_TYPE_GPRS,0,1000,"\x00\xab\xcd\xef\xff",sizeof("\x00\xab\xcd\xef\xff"));
			if(result == CW_STATUS_OK)
			{
				rt_kprintf("Send Data\n");
			}
		}
		return ;
	}
}

int com_test_init(void)
{
	rt_thread_t id;
	
	id = rt_thread_create("ComTest",
												com_test_entry,
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
INIT_APP_EXPORT(com_test_init);

