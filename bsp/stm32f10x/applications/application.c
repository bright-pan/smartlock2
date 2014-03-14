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

void rt_init_thread_entry(void* parameter)
{
#ifdef RT_USING_COMPONENTS_INIT
    /* initialization RT-Thread Components */
    rt_components_init();
#endif

#ifdef  RT_USING_FINSH
    finsh_set_device(RT_CONSOLE_DEVICE_NAME);
#endif  /* RT_USING_FINSH */

    /* Filesystem Initialization */
#if defined(RT_USING_DFS) && defined(RT_USING_DFS_ELMFAT)
    /* mount sd card fat partition 1 as root directory */
    if (dfs_mount("flash1", "/", "elm", 0, 0) == 0)
    {
        rt_kprintf("File System initialized!\n");
    }
    else
        rt_kprintf("File System initialzation failed!\n");
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

        /* re-init device driver */
        rt_device_init_all();

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
}

int rt_application_init(void)
{
    rt_thread_t init_thread;
	rt_thread_t alarm_thread;
	rt_thread_t local_thread;
	rt_thread_t comm_thread;
	rt_thread_t sms_thread;

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

	// initial alarm msg queue
	alarm_mq = rt_mq_create("alarm", sizeof(ALARM_MAIL_TYPEDEF),
							ALARM_MAIL_MAX_MSGS,
							RT_IPC_FLAG_FIFO);

    // init alarm thread
    alarm_thread = rt_thread_create("alarm",
									alarm_thread_entry, RT_NULL,
									512, 100, 5);
    if (alarm_thread != RT_NULL)
    {
        rt_thread_startup(alarm_thread);
    }

	// initial local msg queue
	local_mq = rt_mq_create("local", sizeof(LOCAL_MAIL_TYPEDEF),
							LOCAL_MAIL_MAX_MSGS, RT_IPC_FLAG_FIFO);

    // init local thread
    local_thread = rt_thread_create("local",
									local_thread_entry, RT_NULL,
									512, 102, 5);
    if (local_thread != RT_NULL)
    {
        rt_thread_startup(local_thread);
    }

	// initial comm msg queue
	comm_mq = rt_mq_create("comm", sizeof(COMM_MAIL_TYPEDEF),
						   COMM_MAIL_MAX_MSGS, RT_IPC_FLAG_FIFO);
	// initial comm thread
	comm_thread = rt_thread_create("comm",
							   comm_thread_entry, RT_NULL,
							   512, 101, 5);
	if (comm_thread != RT_NULL)
	{
		rt_thread_startup(comm_thread);
	}
	// initial sms msg queue
	sms_mq = rt_mq_create("sms", sizeof(SMS_MAIL_TYPEDEF),
						  SMS_MAIL_MAX_MSGS, RT_IPC_FLAG_FIFO);
	// initial sms thread
	sms_thread = rt_thread_create("sms",
								  sms_thread_entry, RT_NULL,
								  1024, 102, 5);
	if (sms_thread != RT_NULL)
	{
		rt_thread_startup(sms_thread);
	}

	return 0;
}

/*@}*/
