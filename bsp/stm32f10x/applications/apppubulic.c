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


