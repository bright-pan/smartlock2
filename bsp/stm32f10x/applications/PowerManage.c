#include "PowerManage.h"
#include "untils.h"
#include "gpio_pin.h"


/***************************************************************************************************/
#define POWER_MANAGE_DEBUG  31   //��Դ������Կ���

/***************************************************************************************************/
/* test thread low power useing */

typedef struct 
{
	rt_uint32_t WorkFlag;			// ������־ÿһ��λ����һ���߳�
	rt_uint8_t  IsSleep;			// �Ƿ���Ҫ���������ж�
}SystemSleepDef,*SystemSleepDef_p;
static SystemSleepDef SysSleep = 
{
 0,
 1, //û��һ���߳̿�ʼ����ʱ���ж�����
};

/*
����:��Դ�¼�����
����:mode ģʽ  type �¼�����
����: -------------------------
		 |ģʽ |�ɹ�|ʧ��|����    |
		 |0    |0   |1   |�����¼�|
		 |1    |0   |1   |�յ��¼�|
		 |2    |0   |1   |����¼�|
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
@brief  �ر�stm32����GPIO
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
@brief  ����͹���ʱʱ�ӿ���
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
@brief  ���ݺͻ�ԭSTM32�Ĵ���
@param  cmd  
				@arg RT_TRUE ����
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
		
		// ����GPIO�Ĵ���
		gpio[0] = *GPIOA;
		gpio[1] = *GPIOB;
		gpio[2] = *GPIOC;
		gpio[3] = *GPIOD;
		gpio[4] = *GPIOE;
		gpio[5] = *GPIOF;

		// �����жϼĴ���
		//*exti = *EXTI;

		// ����AFIO
		//*afio =	*AFIO;
	}
	else
	{
		// ��ԭGPIO�Ĵ���
		*GPIOA = gpio[0];
		*GPIOB = gpio[1];
		*GPIOC = gpio[2];
		*GPIOD = gpio[3];
		*GPIOE = gpio[4];
		*GPIOF = gpio[5];
		
		// ��ԭ�жϼĴ���
		//*EXTI = *exti;
		
		// ��ԭAFIO
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
@brief  ����RTC�����¼���ʱ��
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
@brief  ����֮ǰ���Ŵ���
@param  none 
@retval none
*/
static void stm32_sleep_gpio_config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  GPIO_StructInit(&GPIO_InitStructure);
  // ���ÿ��йܽ�
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_5|GPIO_Pin_12|GPIO_Pin_13;
 	GPIO_Init(GPIOC,&GPIO_InitStructure);

 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5;
 	GPIO_Init(GPIOD,&GPIO_InitStructure);
 	
 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6;
 	GPIO_Init(GPIOE,&GPIO_InitStructure);

	// �ر�flash
	gpio_pin_output(DEVICE_NAME_POWER_FLASH,0,0);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA,&GPIO_InitStructure);

	// �رյ��
	gpio_pin_output(DEVICE_NAME_POWER_MOTOR,0,0);

	// �ر�rf433
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_Init(GPIOD,&GPIO_InitStructure);
	GPIO_SetBits(GPIOD, GPIO_Pin_1);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_Init(GPIOD,&GPIO_InitStructure);

	// �ر�ָ��
	gpio_pin_output(DEVICE_NAME_POWER_FRONT,0,0);

	// �ر�gsm
	gpio_pin_output(DEVICE_NAME_POWER_GSM,0,0);

 	// �ر�A�˿�
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	// ����RTC ����ʱ��
  RTC_Alarm_Wakeup_Time(RTC_ALARM_WAKEUP_TIME);

 	//stm32_gpio_all_close();
}


/** 
@brief  ����֮�����Ŵ���
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
@brief  stm32����˯��ģʽ
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
@brief  stm32����ͣ��ģʽ
@param  none 
@retval none
*/
static void stm32_stop_mode_entry(void)
{	
	// ��������
	rt_enter_critical();
	
	// PWR��ʹ�ܡ�
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

  // ����gpio
  stm32_backup_restore(RT_TRUE);

  //PWR_WakeUpPinCmd(ENABLE);
  stm32_sleep_gpio_config();
	
	// �ر�stm32ʱ�� 
  //stm32_sleep_rcc_manage(DISABLE);

  // ����˯�ߡ�
  PWR_EnterSTOPMode( PWR_Regulator_LowPower, PWR_STOPEntry_WFI);  

	// ��stm32ʱ��
	//stm32_sleep_rcc_manage(ENABLE);

	// ��ԭgpio
	stm32_backup_restore(RT_FALSE);
	
	// ����ϵͳֹͣ��ʱ��
  SYSCLKConfig_STOP();

  // ������ѱ�־
	PWR_ClearFlag(PWR_FLAG_WU);

	// �������ģʽ
	PWR_ClearFlag(PWR_FLAG_SB);

	// ���Ƚ���
	rt_exit_critical();
}

/** 
@brief  �����̴߳���
@param  none 
@retval none
*/
void rt_thread_idle_process(void)
{
	if((SysSleep.IsSleep == 0)&&(SysSleep.WorkFlag == 0))
	{
		// ����ͣ��ģʽ
		SysSleep.IsSleep = 1;
#ifdef USEING_SYS_STOP_MODE
		stm32_stop_mode_entry();
#endif
	}
	else
	{
    // CPU����
#ifdef USEING_SYS_SLEPP_MODE
    stm32_sleep_mode_entry();
#endif
	}
}



/** 
@brief  ���ÿ����̹߳��Ӻ���
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
@brief ��Ҫ���������빤�����߳�����
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
	/* flag ��ÿһ��λ����һ���߳� */
	rt_uint32_t flag;
	rt_uint8_t  i;

	// ��ȡ֮ǰ���̹߳���״̬
	flag = SysSleep.WorkFlag;

  // �����빤�����߳�λ��1 �������ߵ��߳�λ��0
	for(i=0;i<sizeof(ThreadName)/RT_NAME_MAX;i++)
	{
    if(rt_strcmp(thread->name,ThreadName[i]) == 0)
    {
      status ? (flag &= ~(0x01<<i)):(flag |= (0x01<<i));
    }
	}
	// ����̹߳���״̬λ�б仯��ʾ
	if(flag != SysSleep.WorkFlag)
	{
	  SysSleep.WorkFlag = flag;
	  SysSleep.IsSleep = 0;

	  // ������Ϣ
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
