#include "dataSYNC.h"
#include "gprs.h"


//Զ������ͬ�����Ĵ��� 
//������Ϊ��
rt_err_t remote_data_sync_process(void)
{
	//�������и��±�־λ

	set_all_update_flag(1);
	return RT_EOK;
}

