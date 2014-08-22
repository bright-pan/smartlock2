#include "keyprocess_t.h"
#include "keyboard_t.h"
#include "menu.h"

#define KEYPROCESS_DEBUG_INFO				1

#ifndef KEYB_THREAD_PRIORITY
#define KEYB_THREAD_PRIORITY				RT_THREAD_PRIORITY_MAX/2
#endif

void keyprocess_thread_entry(void *arg)
{
	//rt_device_t dev;

	while(1)
	{
    key_input_processing();

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

