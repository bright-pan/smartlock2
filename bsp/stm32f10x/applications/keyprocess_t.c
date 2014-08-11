#include "keyprocess_t.h"
#include "keyboard_t.h"
#define KEYPROCESS_DEBUG_INFO				1

#ifndef KEYB_THREAD_PRIORITY
#define KEYB_THREAD_PRIORITY				13
#endif

void keyprocess_thread_entry(void *arg)
{
	rt_device_t dev;

	rt_device_init_processor(&dev,KEYBOARD_DEVICE_NAME);
	while(1)
	{
		rt_uint8_t data;
		
		if(rt_device_read(dev,0,&data,1) == 1)
		{
			rt_kprintf("key data = %X\n",data);
		}
		rt_thread_delay(10);
	}
}

int	keyprocess_thread_init(void)
{
	rt_thread_t thread;
	
	thread = rt_thread_create("key",
	                           keyprocess_thread_entry, RT_NULL,
	                           1024,KEYB_THREAD_PRIORITY,10);
	rt_thread_startup(thread);
	
	return 0;
}
INIT_APP_EXPORT(keyprocess_thread_init);

