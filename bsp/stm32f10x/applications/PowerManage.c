#include "PowerManage.h"
#include "untils.h"
#include "gpio_pin.h"


/***************************************************************************************************/
#define POWER_MANAGE_DEBUG  31   //电源管理调试开关

/***************************************************************************************************/
/* test thread low power useing */

typedef struct 
{
	rt_uint32_t WorkFlag;			// 工作标志每一个位代表一个线程
	rt_uint8_t  IsSleep;			// 是否需要进行休眠判断
}SystemSleepDef,*SystemSleepDef_p;
static SystemSleepDef SysSleep = 
{
 0,
 1, //没有一个线程开始工作时不判断休眠
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
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  //GPIO_Init(GPIOA,&GPIO_InitStructure);
  //GPIO_Init(GPIOB,&GPIO_InitStructure);
  //GPIO_Init(GPIOC,&GPIO_InitStructure);
  //GPIO_Init(GPIOD,&GPIO_InitStructure);
  //GPIO_Init(GPIOE,&GPIO_InitStructure);
  //GPIO_Init(GPIOF,&GPIO_InitStructure);
  
}

/** 
@brief  进入低功耗时时钟控制
@param  none 
@retval none
*/
static void stm32_sleep_rcc_manage(FunctionalState NewState)
{
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO|
 												 RCC_APB2Periph_GPIOA|
 												 RCC_APB2Periph_GPIOB|
 												 RCC_APB2Periph_GPIOC|
 												 RCC_APB2Periph_GPIOD|
 												 RCC_APB2Periph_GPIOE|
 												 RCC_APB2Periph_GPIOF|
 												 RCC_APB2Periph_USART1|
 												 RCC_APB2Periph_SPI1, NewState);

 	RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP|
 												 RCC_APB1Periph_SPI2|
 												 RCC_APB1Periph_USART2|
 												 RCC_APB1Periph_USART3|
 												 RCC_APB1Periph_UART4|
 												 RCC_APB1Periph_PWR|
 												 RCC_APB1Periph_TIM3|
 												 RCC_APB1Periph_TIM4|
 												 RCC_APB1Periph_TIM5,NewState);		
  GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, (NewState == ENABLE)?DISABLE:ENABLE);
  GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, (NewState == ENABLE)?DISABLE:ENABLE);
  SPI_Cmd(SPI1,NewState);
  SPI_Cmd(SPI2,NewState);
  
  USART_Cmd(USART1,NewState);
  USART_Cmd(USART2,NewState);
  USART_Cmd(USART3,NewState);
  USART_Cmd(UART4,NewState);
}


/** 
@brief  备份和还原STM32寄存器
@param  cmd  
				@arg RT_TRUE 备份
@retval none
*/
static void stm32_backup_restore(rt_bool_t cmd)
{
	static GPIO_TypeDef *gpio;
	//static EXTI_TypeDef *exti;
	//static AFIO_TypeDef *afio;
	
	if(cmd == RT_TRUE)
	{
    gpio = rt_calloc(6,sizeof(GPIO_TypeDef));
		RT_ASSERT(gpio != RT_NULL);

		/*exti = rt_calloc(1,sizeof(EXTI_TypeDef));
		RT_ASSERT(exti != RT_NULL);

		afio = rt_calloc(1,sizeof(AFIO_TypeDef));
		RT_ASSERT(afio != RT_NULL);*/
		
		// 备份GPIO寄存器
		gpio[0] = *GPIOA;
		gpio[1] = *GPIOB;
		gpio[2] = *GPIOC;
		gpio[3] = *GPIOD;
		gpio[4] = *GPIOE;
		gpio[5] = *GPIOF;

		// 备份中断寄存器
		//*exti = *EXTI;

		// 备份AFIO
		//*afio =	*AFIO;
	}
	else
	{
		// 还原GPIO寄存器
		*GPIOA = gpio[0];
		*GPIOB = gpio[1];
		*GPIOC = gpio[2];
		*GPIOD = gpio[3];
		*GPIOE = gpio[4];
		*GPIOF = gpio[5];
		
		// 还原中断寄存器
		//*EXTI = *exti;
		
		// 还原AFIO
		//*AFIO = *afio;
		
		
		rt_free(gpio);
		gpio = RT_NULL;
		
		/*rt_free(exti);
		exti = RT_NULL;

		rt_free(afio);
		afio = RT_NULL;*/

	}
	

	
}

void min_power_config()
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* HCLK = SYSCLK */
	RCC_HCLKConfig(RCC_SYSCLK_Div1); 
	RCC_PCLK2Config(RCC_HCLK_Div2);       
	RCC_PCLK1Config(RCC_HCLK_Div4);

	/* Select HSI as system clock source */

	RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI);

	while(RCC_GetSYSCLKSource() != 0x00)
	{
	}
	FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
	FLASH_SetLatency(FLASH_Latency_0);
	  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, DISABLE);
	RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE, ENABLE);
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_All;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AIN;
	GPIO_Init(GPIOC,&GPIO_InitStructure);
	GPIO_Init(GPIOD,&GPIO_InitStructure);
	GPIO_Init(GPIOE,&GPIO_InitStructure);
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE, DISABLE);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_Init(GPIOD,&GPIO_InitStructure);
	GPIO_SetBits(GPIOD, GPIO_Pin_1);

}


/**
  * @brief  This function handles RTC Alarm interrupt request.
  * @param  None
  * @retval None
  */
int RTC_Alarm_Config(void)
{
  EXTI_InitTypeDef EXTI_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
	u32 count=0x200000;
	
  /* Configure EXTI Line17(RTC Alarm) to generate an interrupt on rising edge */
  EXTI_ClearITPendingBit(EXTI_Line17);
  EXTI_InitStructure.EXTI_Line = EXTI_Line17;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

  NVIC_InitStructure.NVIC_IRQChannel = RTCAlarm_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* RTC clock source configuration ------------------------------------------*/
  /* Allow access to BKP Domain */
  PWR_BackupAccessCmd(ENABLE);

  /* Reset Backup Domain */
  BKP_DeInit();
  
  /* Enable the LSE OSC */
  RCC_LSEConfig(RCC_LSE_ON);
  /* Wait till LSE is ready */
  while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET && count)
  {
  	count--;
  }

  /* Select the RTC Clock Source */
  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

  /* Enable the RTC Clock */
  RCC_RTCCLKCmd(ENABLE);

  /* RTC configuration -------------------------------------------------------*/
  /* Wait for RTC APB registers synchronisation */
  RTC_WaitForSynchro();

  /* Set the RTC time base to 1s */
  RTC_SetPrescaler(32767);  
  /* Wait until last write operation on RTC registers has finished */
  RTC_WaitForLastTask();

  /* Enable the RTC Alarm interrupt */
  RTC_ITConfig(RTC_IT_ALR, ENABLE);
  /* Wait until last write operation on RTC registers has finished */
  RTC_WaitForLastTask();

  return 0;
}
INIT_APP_EXPORT(RTC_Alarm_Config);


/**
  * @brief  This function handles RTC Alarm interrupt request.
  * @param  None
  * @retval None
  */
void RTCAlarm_IRQHandler(void)
{
  if(RTC_GetITStatus(RTC_IT_ALR) != RESET)
  {
    /* Clear EXTI line17 pending bit */
    EXTI_ClearITPendingBit(EXTI_Line17);

    /* Check if the Wake-Up flag is set */
    if(PWR_GetFlagStatus(PWR_FLAG_WU) != RESET)
    {
      /* Clear Wake Up flag */
      PWR_ClearFlag(PWR_FLAG_WU);
    }

    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();   
    /* Clear RTC Alarm interrupt pending bit */
    RTC_ClearITPendingBit(RTC_IT_ALR);
    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();
    battery_energy_check_20p();
  }
}


/** 
@brief  设置RTC报警事件的时间
@param  none 
@retval none
*/
void RTC_Alarm_Wakeup_Time(rt_uint32_t sec)
{
	/* Wait till RTC Second event occurs */
	RTC_ClearFlag(RTC_FLAG_SEC);
	while(RTC_GetFlagStatus(RTC_FLAG_SEC) == RESET);
	
	/* Alarm in 3 second */
	RTC_SetAlarm(RTC_GetCounter()+ sec);
	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();
}

/** 
@brief  休眠之前引脚处理
@param  none 
@retval none
*/
static void stm32_sleep_gpio_config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  GPIO_StructInit(&GPIO_InitStructure);
  // 设置空闲管脚
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_5|GPIO_Pin_12|GPIO_Pin_13;
 	GPIO_Init(GPIOC,&GPIO_InitStructure);

 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5;
 	GPIO_Init(GPIOD,&GPIO_InitStructure);
 	
 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6;
 	GPIO_Init(GPIOE,&GPIO_InitStructure);

	// 关闭flash
	gpio_pin_output(DEVICE_NAME_POWER_FLASH,0,0);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA,&GPIO_InitStructure);

	// 关闭电机
	gpio_pin_output(DEVICE_NAME_POWER_MOTOR,0,0);

	// 关闭rf433
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_Init(GPIOD,&GPIO_InitStructure);
	GPIO_SetBits(GPIOD, GPIO_Pin_1);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_Init(GPIOD,&GPIO_InitStructure);

	// 关闭指纹
	gpio_pin_output(DEVICE_NAME_POWER_FRONT,0,0);

	// 关闭gsm
	gpio_pin_output(DEVICE_NAME_POWER_GSM,0,0);

 	// 关闭A端口
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	// 设置RTC 唤醒时间
  RTC_Alarm_Wakeup_Time(RTC_ALARM_WAKEUP_TIME);

 	//stm32_gpio_all_close();
}


/** 
@brief  唤醒之后引脚处理
@param  none 
@retval none
*/


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
	//rt_dprintf(POWER_MANAGE_DEBUG,("STM32 Entry Sleep\n"));
  __WFI();
  //rt_dprintf(POWER_MANAGE_DEBUG,("STM32  Exit Sleep\n"));
}


/** 
@brief  stm32进入停机模式
@param  none 
@retval none
*/
static void stm32_stop_mode_entry(void)
{	
	// 调度上锁
	rt_enter_critical();
	
	// PWR的使能。
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

  // 备份gpio
  stm32_backup_restore(RT_TRUE);

  //PWR_WakeUpPinCmd(ENABLE);
  stm32_sleep_gpio_config();
	
	// 关闭stm32时钟 
  //stm32_sleep_rcc_manage(DISABLE);

  // 进入睡眠。
  PWR_EnterSTOPMode( PWR_Regulator_LowPower, PWR_STOPEntry_WFI);  

	// 打开stm32时钟
	//stm32_sleep_rcc_manage(ENABLE);

	// 还原gpio
	stm32_backup_restore(RT_FALSE);
	
	// 配置系统停止后时钟
  SYSCLKConfig_STOP();

  // 清除唤醒标志
	PWR_ClearFlag(PWR_FLAG_WU);

	// 清除待机模式
	PWR_ClearFlag(PWR_FLAG_SB);

	// 调度解锁
	rt_exit_critical();
}

/** 
@brief  空闲线程处理
@param  none 
@retval none
*/
void rt_thread_idle_process(void)
{
	if((SysSleep.IsSleep == 0)&&(SysSleep.WorkFlag == 0))
	{
		// 进入停机模式
		SysSleep.IsSleep = 1;
#ifdef USEING_SYS_STOP_MODE
		stm32_stop_mode_entry();
#endif
	}
	else
	{
    // CPU休眠
#ifdef USEING_SYS_SLEPP_MODE
    stm32_sleep_mode_entry();
#endif
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

	return 0;
}
INIT_APP_EXPORT(rt_thread_idle_set);


/** 
@brief 需要进行休眠与工作的线程名字
*/
static const char ThreadName[][RT_NAME_MAX] =
{
	"bat",
	"local",
	"gprs",
	"buzzer",
	"key",
	"BT_M",
	"NPDU",
	"sms",
	"gsm",
	"rf433",
	"init",
};
/** 
@brief  thread status manage
@param  thread object
@param  status 0:work or 1:sleep
@retval none 
*/
void rt_thread_status_manage(rt_thread_t thread,rt_uint8_t status)
{
	/* flag 的每一个位代表一个线程 */
	rt_uint32_t flag;
	rt_uint8_t  i;

	// 读取之前的线程工作状态
	flag = SysSleep.WorkFlag;

  // 将进入工作的线程位置1 进入休眠的线程位置0
	for(i=0;i<sizeof(ThreadName)/RT_NAME_MAX;i++)
	{
    if(rt_strcmp(thread->name,ThreadName[i]) == 0)
    {
      status ? (flag &= ~(0x01<<i)):(flag |= (0x01<<i));
    }
	}
	// 如果线程工作状态位有变化显示
	if(flag != SysSleep.WorkFlag)
	{
	  SysSleep.WorkFlag = flag;
	  SysSleep.IsSleep = 0;

	  // 调试信息
	  if(SysSleep.WorkFlag != 0)
	  {
      rt_dprintf(POWER_MANAGE_DEBUG,("Status:%x Thread In Work\n",SysSleep.WorkFlag));
      for(i=0;i<sizeof(ThreadName)/RT_NAME_MAX;i++)
      {
        if(flag & 0x01<<i)
        {
          rt_dprintf(POWER_MANAGE_DEBUG,("\"%s\" ",ThreadName[i]));
        }
      }
      rt_dprintf(POWER_MANAGE_DEBUG,("\n"));
	  }
	  else
	  {
			rt_dprintf(POWER_MANAGE_DEBUG,("STM32 System Entry Stop Mode\n"));
	  }
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

void sleepManage(rt_uint8_t cmd)
{
	switch(cmd)
	{
		case 0:
		{
			rt_kprintf("idle_set:\n");
			rt_kprintf("cmd:0 --help");
			rt_kprintf("cmd:1 --set sleep work mode\n");
			rt_kprintf("cmd:2 --cancel sleep work mode");
			break;
		}
		case 1:
		{
			rt_thread_idle_sethook(rt_thread_idle_process);
			break;
		}
		case 2:
		{
      rt_thread_idle_sethook(RT_NULL);
		}
	}
}

FINSH_FUNCTION_EXPORT(sleepManage,"(cmd) sleep mode manage cmd");

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

void gpio_ain(rt_uint8_t bit)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  GPIO_StructInit(&GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_InitStructure.GPIO_Pin = (0x01 << bit);
  GPIO_Init(GPIOB,&GPIO_InitStructure);	
}
FINSH_FUNCTION_EXPORT(gpio_ain,"(bit)set gpio ain mode");
#endif
