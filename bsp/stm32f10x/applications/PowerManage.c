#include "PowerManage.h"
#include "untils.h"


/* test thread low power useing */

typedef struct 
{
	rt_uint32_t WorkFlag;
	rt_uint8_t  IsSleep;
}SystemSleepDef,*SystemSleepDef_p;
static SystemSleepDef SysSleep = 
{
 0,
 1,
};

/*
功能:电源事件管理
参数:mode 模式  type 事件类型
返回: -------------------------
		 |模式 |成功|失败|功能    |
		 |0    |0   |1   |发送事件|
		 |1    |0   |1   |收到事件|
		 |2    |0   |1   |清除事件|
		 --------------------------
*/
rt_uint8_t power_event_process(rt_uint8_t mode,rt_uint32_t type)
{
	rt_uint32_t value;
	rt_err_t    result;
	rt_uint8_t  return_data = 1;
	static rt_event_t power_evt;
	
	//net_evt_mutex_op(RT_TRUE);

	if(power_evt == RT_NULL)
	{
    power_evt = rt_event_create("power",RT_IPC_FLAG_FIFO);
    RT_ASSERT(power_evt != RT_NULL);
	}
	switch(mode)
	{
		case 0:	//set event 
		{
			result = rt_event_send(power_evt,type);
			if(result == RT_EOK)
			{
				return_data = 0;
			}
			break;
		}
		case 1:	//get event 
		{
			result = rt_event_recv(power_evt,
			                       type,
			                       RT_EVENT_FLAG_OR,
			                       RT_WAITING_NO,&value);
			if(result == RT_EOK)
			{
				return_data = 0;
			}
			else if(result == -RT_ETIMEOUT)
			{
				return_data = 1;
			}
			break;
		}
		case 2://clean event
		{
			result = rt_event_recv(power_evt,
			                       type,
			                       RT_EVENT_FLAG_OR | 
			                       RT_EVENT_FLAG_CLEAR,
			                       RT_WAITING_NO,&value);
			if(result == RT_EOK)
			{
				return_data = 0;
			}
			break;
		}
    case 3://clean all event 
    {
      result = rt_event_recv(power_evt,
                             0xffffffff,
                             RT_EVENT_FLAG_OR | 
                             RT_EVENT_FLAG_CLEAR,
                             RT_WAITING_NO,&value);
      if(result == RT_EOK)
      {
        return_data = 0;
      }
      break;
    }
    default:
    {
			break;
    }
	}

	//net_evt_mutex_op(RT_FALSE);
	return return_data;
}

/** 
@brief  关闭stm32所有GPIO
@param  none 
@retval none
*/
static void stm32_gpio_all_close(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  GPIO_StructInit(&GPIO_InitStructure);
  GPIO_Init(GPIOA,&GPIO_InitStructure);
  GPIO_Init(GPIOB,&GPIO_InitStructure);
  GPIO_Init(GPIOC,&GPIO_InitStructure);
  GPIO_Init(GPIOD,&GPIO_InitStructure);
  GPIO_Init(GPIOE,&GPIO_InitStructure);
  GPIO_Init(GPIOF,&GPIO_InitStructure);
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA| RCC_APB2Periph_GPIOB| RCC_APB2Periph_GPIOC | 
  											 RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOF, DISABLE);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC,DISABLE);
}


/**
  * @brief  Configures system clock after wake-up from STOP: enable HSE, PLL
  *   and select PLL as system clock source.
  * @param  None
  * @retval None
  */
void SYSCLKConfig_STOP(void)
{
	ErrorStatus HSEStartUpStatus; 
	
  /* Enable HSE */
  RCC_HSEConfig(RCC_HSE_ON);

  /* Wait till HSE is ready */
  HSEStartUpStatus = RCC_WaitForHSEStartUp();

  if(HSEStartUpStatus == SUCCESS)
  {

#ifdef STM32F10X_CL
    /* Enable PLL2 */ 
    RCC_PLL2Cmd(ENABLE);

    /* Wait till PLL2 is ready */
    while(RCC_GetFlagStatus(RCC_FLAG_PLL2RDY) == RESET)
    {
    }
#endif

    /* Enable PLL */ 
    RCC_PLLCmd(ENABLE);

    /* Wait till PLL is ready */
    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
    {
    }

    /* Select PLL as system clock source */
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

    /* Wait till PLL is used as system clock source */
    while(RCC_GetSYSCLKSource() != 0x08)
    {
    }
  }
}


/** 
@brief  stm32进入睡眠模式
@param  none 
@retval none
*/
static void stm32_sleep_mode_entry(void)
{ 
  __WFI();
}


/** 
@brief  stm32进入停机模式
@param  none 
@retval none
*/
static void stm32_stop_mode_entry(void)
{
	// PWR的使能。
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP | RCC_APB2Periph_AFIO, ENABLE);

  //PWR_WakeUpPinCmd(ENABLE);
  
  // 进入睡眠。
  PWR_EnterSTOPMode( PWR_Regulator_LowPower, PWR_STOPEntry_WFI);  

	// 配置系统停止后时钟
  SYSCLKConfig_STOP();
  rt_thread_idle_sethook(RT_NULL);
}

static void rt_filesytem_unmount(void)
{
	dfs_unmount("/");
}

/** 
@brief  空闲线程处理
@param  none 
@retval none
*/
void rt_thread_idle_process(void)
{
	// 卸载文件系统
	//rt_filesytem_unmount();

	// CPU休眠
	stm32_sleep_mode_entry();
	
	if((SysSleep.IsSleep == 0)&&(SysSleep.WorkFlag == 0))
	{
		// 进入停机模式
		SysSleep.IsSleep = 1;
		stm32_stop_mode_entry();
	}
}



/** 
@brief  设置空闲线程钩子函数
@param  none 
@retval none
*/
int rt_thread_idle_set(void)
{
	rt_thread_idle_sethook(rt_thread_idle_process);
}
INIT_APP_EXPORT(rt_thread_idle_set);



/** 
@brief  thread status manage
@param  thread object
@param  status 0:work or 1:sleep
@retval none 
*/
void rt_thread_status_manage(rt_thread_t thread,rt_uint8_t status)
{
	rt_uint32_t flag;

	flag = SysSleep.WorkFlag;
	if(rt_strcmp(thread->name,"bat") == 0)
	{
	  status ? (flag &= ~(0x01<<0)):(flag |= (0x01<<0));
	}
	else if(rt_strcmp(thread->name,"local") == 0)
	{
	  status ? (flag &= ~(0x01<<1)):(flag |= (0x01<<1));
	}
	else if(rt_strcmp(thread->name,"gprs") == 0)
	{
	  status ? (flag &= ~(0x01<<2)):(flag |= (0x01<<2));
	}
	else if(rt_strcmp(thread->name,"buzzer") == 0)
	{
	  status ? (flag &= ~(0x01<<3)):(flag |= (0x01<<3));
	}
	else if(rt_strcmp(thread->name,"key") == 0)
	{
	  status ? (flag &= ~(0x01<<4)):(flag |= (0x01<<4));
	}

	if(flag != SysSleep.WorkFlag)
	{
	  SysSleep.WorkFlag = flag;
	  SysSleep.IsSleep = 0;
	  rt_kprintf("thread status %x\n",SysSleep.WorkFlag);
	}
}


/** 
@brief  thread entry work 
@param  thread object
@retval none 
*/
void rt_thread_entry_work(rt_thread_t thread)
{	
	rt_thread_status_manage(thread,0);
}


/** 
@brief  thread entry sleep 
@param  thread object
@retval none 
*/
void rt_thread_entry_sleep(rt_thread_t thread)
{
  rt_thread_status_manage(thread,1);
}




#ifdef RT_USING_FINSH
#include <finsh.h>

void idle_set(rt_uint8_t cmd)
{
	if(cmd == 0)
	{
		rt_thread_idle_sethook(rt_thread_idle_process);
	}
	else
	{
		rt_thread_idle_sethook(RT_NULL);
	}
	switch(cmd)
	{
		case 0:
		{
			break;
		}
		case :
	}
}

FINSH_FUNCTION_EXPORT(idle_set,"(cmd)idle set");

#include "usart.h"
void uart_manage(const char *name,rt_bool_t cmd)
{
	rt_device_t dev = device_enable(name);

	if(dev == RT_NULL)
	{
		rt_kprintf("%s device %s is RT_NULL",__FUNCTION__,name);
		return ;
	}
	if(cmd == RT_TRUE)
	{
		rt_device_control(dev,RT_DEVICE_CTRL_SET_TX_GPIO,RT_NULL);
		rt_kprintf("set tx pin");
	}
	else if(cmd == RT_FALSE)
	{
    rt_device_control(dev,RT_DEVICE_CTRL_CLR_TX_GPIO,RT_NULL);
    rt_kprintf("clr tx pin");
	}
}
FINSH_FUNCTION_EXPORT(uart_manage,"(uart,cmd)usart manage");

#endif
