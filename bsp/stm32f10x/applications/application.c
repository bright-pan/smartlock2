/*
 * File      : application.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 * 2013-07-12     aozima       update for auto initial.
 */

/**
 * @addtogroup STM32
 */
/*@{*/

#include <board.h>
#include <rtthread.h>

#ifdef  RT_USING_COMPONENTS_INIT
#include <components.h>
#endif  /* RT_USING_COMPONENTS_INIT */

#ifdef RT_USING_DFS
/* dfs filesystem:ELM filesystem init */
#include <dfs_elm.h>
/* dfs Filesystem APIs */
#include <dfs_fs.h>
#endif

#ifdef RT_USING_RTGUI
#include <rtgui/rtgui.h>
#include <rtgui/rtgui_server.h>
#include <rtgui/rtgui_system.h>
#include <rtgui/driver.h>
#include <rtgui/calibration.h>
#endif

#include "board.h"
#include "alarm.h"
#include "local.h"
#include "sms.h"
#include "untils.h"
#include "comm.h"
#include "fprint.h"
#include "gpio_pin.h"
#include "Databases.h"
#include "menu.h"


#ifdef RT_USING_RTGUI
rt_bool_t cali_setup(void)
{
    rt_kprintf("cali setup entered\n");
    return RT_FALSE;
}

void cali_store(struct calibration_data *data)
{
    rt_kprintf("cali finished (%d, %d), (%d, %d)\n",
               data->min_x,
               data->max_x,
               data->min_y,
               data->max_y);
}
#endif /* RT_USING_RTGUI */

#if defined(RT_USING_DFS) && defined(RT_USING_DFS_ELMFAT)
#define FILE_SYSTEM_DEVICE_NAME	"flash1"
#define FILE_SYSTEM_TYPE_NAME		"elm"

void dfs_mount_processing(void)
{
	/* mount sd card fat partition 1 as root directory */
	if (dfs_mount(FILE_SYSTEM_DEVICE_NAME, "/", FILE_SYSTEM_TYPE_NAME, 0, 0) == 0)
	{
	rt_kprintf("\nFile System initialized ok!\n");
	}
	else
	{
		extern int dfs_mkfs(const char *fs_name, const char *device_name);
		rt_kprintf("mount fatfs fail\n");
		if(dfs_mount(FILE_SYSTEM_DEVICE_NAME, "/", FILE_SYSTEM_TYPE_NAME,0, 0) != 0)
		{
			if(dfs_mkfs(FILE_SYSTEM_TYPE_NAME,FILE_SYSTEM_DEVICE_NAME) == 0)
			{
				if (dfs_mount("flash1", "/", "elm", 0, 0) == 0)
				{
				  rt_kprintf("\nFile System initialized ok!\n");
				}
			}
			else
			{
				rt_kprintf("\nFile System initialzation failed!\n");
			} 
		}
		else
		{
			rt_kprintf("\nFile System initialized ok!\n");
		}
	}
}
#endif

static void system_power_Insufficient_process(void)
{
	//��ѹ����
	if(system_power_Insufficient() == RT_TRUE)
	{
		gui_open_lcd_show();
		battery_low_alarm_show();
		gui_close_lcd_show();
		rt_thread_entry_sleep(rt_thread_self());
		while(1)
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
	}
}
void rt_init_thread_entry(void* parameter)
{
#ifdef RT_USING_COMPONENTS_INIT
    /* initialization RT-Thread Components */
    rt_components_init();
#endif

#ifdef  RT_USING_FINSH
    finsh_set_device(RT_CONSOLE_DEVICE_NAME);
#endif  /* RT_USING_FINSH */
		rt_thread_entry_work(rt_thread_self());
		system_power_Insufficient_process();
    /* Filesystem Initialization */
#if defined(RT_USING_DFS) && defined(RT_USING_DFS_ELMFAT)
    /* mount sd card fat partition 1 as root directory */
    dfs_mount_processing();
    //databases_init();
#endif  /* RT_USING_DFS */

#ifdef RT_USING_RTGUI
    {
        extern void rt_hw_lcd_init();
        extern void rtgui_touch_hw_init(void);

        rt_device_t lcd;

        /* init lcd */
        rt_hw_lcd_init();

        /* init touch panel */
        rtgui_touch_hw_init();

        /* find lcd device */
        lcd = rt_device_find("lcd");

        /* set lcd device as rtgui graphic driver */
        rtgui_graphic_set_device(lcd);

#ifndef RT_USING_COMPONENTS_INIT
        /* init rtgui system server */
        rtgui_system_server_init();
#endif

        calibration_set_restore(cali_setup);
        calibration_set_after(cali_store);
        calibration_init();
    }
#endif /* #ifdef RT_USING_RTGUI */
		rt_thread_entry_sleep(rt_thread_self());
}

int rt_application_init(void)
{
    rt_thread_t init_thread;

#if (RT_THREAD_PRIORITY_MAX == 32)
    init_thread = rt_thread_create("init",
                                   rt_init_thread_entry, RT_NULL,
                                   2048, 8, 20);
#else
    init_thread = rt_thread_create("init",
                                   rt_init_thread_entry, RT_NULL,
                                   2048, 80, 20);
#endif

	if (init_thread != RT_NULL)
        rt_thread_startup(init_thread);

	return 0;
}

/*@}*/
