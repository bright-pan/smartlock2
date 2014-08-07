#include "commfun_t.h"

void rt_device_init_processor(rt_device_t *dev,const char *dev_name)
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

