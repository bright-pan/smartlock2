#include "keyprocess_t.h"
#include "keyboard_t.h"
#include "unlock_ui.h"
#include "menu.h"

#define KEYPROCESS_DEBUG_INFO				1

#ifndef KEYB_THREAD_PRIORITY
#define KEYB_THREAD_PRIORITY				RT_THREAD_PRIORITY_MAX/2
#endif

void keyprocess_thread_entry(void *arg)
{
	// 电量不足
	if(system_power_Insufficient() == RT_TRUE)
	{
	  return ;
	}
	
  rt_thread_entry_work(rt_thread_self());
	// 创建管理员
  admin_account_init();

  key_input_processing_init();
	  
  // ui线程进入工作状态
	rt_thread_entry_work(rt_thread_self());
	while(1)
	{
		if(system_power_Insufficient() == RT_TRUE)
		{
      rt_err_t result;
	    rt_uint8_t data;
	    
	    result = gui_key_input(&data);
	    if(result == RT_EOK)
	    {
	      rt_thread_entry_work(rt_thread_self());
	      menu_event_process(0,MENU_EVT_BAT_LOW);
	      gui_open_lcd_show();
	      battery_low_alarm_show();
	      gui_close_lcd_show();
	      rt_thread_entry_sleep(rt_thread_self());
	    }
		}
		else
		{
      key_input_processing();
		}
	}
}

int	keyprocess_thread_init(void)
{
	rt_thread_t thread;
	
	thread = rt_thread_create("key",
	                           keyprocess_thread_entry, RT_NULL,
	                           1024*2,KEYB_THREAD_PRIORITY,10);
	rt_thread_startup(thread);
	
	return 0;
}
INIT_APP_EXPORT(keyprocess_thread_init);

