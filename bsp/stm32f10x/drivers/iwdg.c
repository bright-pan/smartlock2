#include "iwdg.h"

#define USE_SYS_IWDG

#ifdef USE_SYS_IWDG
static void IWDG_Init(rt_uint8_t prer,rt_uint16_t rlr) 
{	
 	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);  //ʹ�ܶԼĴ���IWDG_PR��IWDG_RLR��д����
	
	IWDG_SetPrescaler(prer); //����IWDGԤ��Ƶֵ:����IWDGԤ��ƵֵΪ64
	
	IWDG_SetReload(rlr);  	//����IWDG��װ��ֵ
	
	IWDG_ReloadCounter();  	//����IWDG��װ�ؼĴ�����ֵ��װ��IWDG������
	
	IWDG_Enable();  				//ʹ��IWDG
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

	IWDG_Init(IWDG_Prescaler_64,625);//40k/64 = 625hz 1s��
	
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

