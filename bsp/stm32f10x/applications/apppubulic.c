#include "apppubulic.h"


/*
@brief 求平均数
*/
rt_uint16_t get_average_value(rt_uint16_t dat[],rt_uint8_t num)
{
	rt_uint8_t  i = 0,j = 0;
	rt_uint16_t temp;
	rt_uint32_t	result = 0;

	for(j = 0; j < num-1; j++)
	for(i = 0; i < num-1; i++)
	{
		if(dat[i] > dat[i+1])
		{
			temp = dat[i];
			dat[i] = dat[i+1];
			dat[i + 1] = temp;
		}
	}
	for(i = 2;i< num-2 ;i++)
	{
		result += dat[i];
	}
	result /= (num-4);

	return result;
}


/*
功能:获得当前时间
*/
rt_uint32_t sys_cur_date(void)
{
  rt_device_t device;
  rt_uint32_t time=0;

  device = rt_device_find("rtc");
  if (device != RT_NULL)
  {
      rt_device_control(device, RT_DEVICE_CTRL_RTC_GET_TIME, &time);
  }

	rt_kprintf("Current System Time: 0x%X\n",time);
  return time;
}
RTM_EXPORT(sys_cur_date);



