#include "dataSYNC.h"
#include "gprs.h"


//远程数据同步报文处理 
//数据域为空
rt_err_t remote_data_sync_process(void)
{
	//设置所有更新标志位

	set_all_update_flag(1);
	return RT_EOK;
}

