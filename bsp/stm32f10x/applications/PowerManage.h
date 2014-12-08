#ifndef __POWERMANAGE_H__
#define __POWERMANAGE_H__
#include <rthw.h>
#include <rtthread.h>
#include "stm32f10x.h"


//系统可以进入睡眠
void rt_thread_entry_sleep(rt_thread_t thread);

//系统恢复正常工作
void rt_thread_entry_work(rt_thread_t thread);

void uart_manage(const char *name,rt_bool_t cmd);
#endif

