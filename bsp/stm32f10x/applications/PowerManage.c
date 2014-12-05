#include "PowerManage.h"
#include "untils.h"
#include "gpio_pin.h"


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
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOA,&GPIO_InitStructure);
  //GPIO_Init(GPIOB,&GPIO_InitStructure);
  //GPIO_Init(GPIOC,&GPIO_InitStructure);
  //GPIO_Init(GPIOD,&GPIO_InitStructure);
  //GPIO_Init(GPIOE,&GPIO_InitStructure);
  GPIO_Init(GPIOF,&GPIO_InitStructure);
  
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
@brief  备份和还原gpio寄存器
@param  cmd  
				@arg RT_TRUE 备份
@retval none
*/
static void gpio_backup_restore(rt_bool_t cmd)
{
	static GPIO_TypeDef *gpio;

	if(cmd == RT_TRUE)
	{
    gpio = rt_calloc(6,sizeof(GPIO_TypeDef));
		RT_ASSERT(gpio != RT_NULL);
		
		gpio[0] = *GPIOA;
		gpio[1] = *GPIOB;
		gpio[2] = *GPIOC;
		gpio[3] = *GPIOD;
		gpio[4] = *GPIOE;
		gpio[5] = *GPIOF;
	}
	else
	{
		*GPIOA = gpio[0];
		*GPIOB = gpio[1];
		*GPIOC = gpio[2];
		*GPIOD = gpio[3];
		*GPIOE = gpio[4];
		*GPIOF = gpio[5];
		
		rt_free(gpio);
		gpio = RT_NULL;
	}
	

	
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
	//gpio_pin_output(DEVICE_NAME_RF_ENABLE, 0,0);

	// 关闭指纹
	gpio_pin_output(DEVICE_NAME_POWER_FRONT,0,0);

	// 关闭gsm
	gpio_pin_output(DEVICE_NAME_POWER_GSM,0,0);


 	//关闭rf433
 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_2;
 	GPIO_Init(GPIOD,&GPIO_InitStructure);

 	//关闭A端口
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOA,&GPIO_InitStructure);

	// 设置RTC 事件
	/* Wait till RTC Second event occurs */
	//RTC_ClearFlag(RTC_FLAG_SEC);
	//while(RTC_GetFlagStatus(RTC_FLAG_SEC) == RESET);
	
	/* Alarm in 3 second */
	//RTC_SetAlarm(RTC_GetCounter()+ 3);
	/* Wait until last write operation on RTC registers has finished */
	//RTC_WaitForLastTask();
 	//stm32_gpio_all_close();
}


/**
  * @brief  This function handles RTC Alarm interrupt request.
  * @param  None
  * @retval None
  */
void RTC_Alarm_Config(void)
{
  EXTI_InitTypeDef EXTI_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;

	PWR_BackupAccessCmd(ENABLE);
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

  /* Enable the RTC Alarm interrupt */
  RTC_ITConfig(RTC_IT_ALR, ENABLE);
  /* Wait until last write operation on RTC registers has finished */
  RTC_WaitForLastTask();
}
//INIT_DEVICE_EXPORT(RTC_Alarm_Config);


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
    battery_low_energy_check();
  }
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
  __WFI();
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
  gpio_backup_restore(RT_TRUE);

  //PWR_WakeUpPinCmd(ENABLE);
  stm32_sleep_gpio_config();
	
	// 关闭stm32时钟 
  //stm32_sleep_rcc_manage(DISABLE);

  // 进入睡眠。
  PWR_EnterSTOPMode( PWR_Regulator_LowPower, PWR_STOPEntry_WFI);  

	// 打开stm32时钟
	//stm32_sleep_rcc_manage(ENABLE);

	// 还原gpio
	gpio_backup_restore(RT_FALSE);
	
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
	else if(rt_strcmp(thread->name,"BT_M") == 0)
	{
	  status ? (flag &= ~(0x01<<5)):(flag |= (0x01<<5));
	}
	else if(rt_strcmp(thread->name,"NPDU") == 0)
	{
	  status ? (flag &= ~(0x01<<6)):(flag |= (0x01<<6));
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

#endif
