#include "iwdg.h"

#define USE_SYS_IWDG

#ifdef USE_SYS_IWDG
static void IWDG_Init(rt_uint8_t prer,rt_uint16_t rlr) 
{	
 	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);  //使能对寄存器IWDG_PR和IWDG_RLR的写操作
	
	IWDG_SetPrescaler(prer); //设置IWDG预分频值:设置IWDG预分频值为64
	
	IWDG_SetReload(rlr);  	//设置IWDG重装载值
	
	IWDG_ReloadCounter();  	//按照IWDG重装载寄存器的值重装载IWDG计数器
	
	IWDG_Enable();  				//使能IWDG
} 

static void IWDG_Feed(void)
{   
 	IWDG_ReloadCounter();//reload										   
}

static void IWDG_Timer_OutTime(void *arg)
{
	IWDG_Feed();
}

int IWDG_Init_Process(void)
{
	rt_timer_t timer = RT_NULL;

	IWDG_Init(IWDG_Prescaler_64,625);//40k/64 = 625hz 1s钟
	
	timer = rt_timer_create("IWDG",
													IWDG_Timer_OutTime,
													RT_NULL,
													50,
													RT_TIMER_FLAG_PERIODIC);
	if(timer == RT_NULL)
	{
		rt_kprintf("Timer %s create fail!!!\n",timer->parent.name);
		return 1;
	}

	rt_timer_start(timer);
	
	return 0;
}

//INIT_DEVICE_EXPORT(IWDG_Init_Process);
#endif

