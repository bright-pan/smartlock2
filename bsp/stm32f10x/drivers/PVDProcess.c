#include "PVDProcess.h"



static PVDCallback PVD_IRQCallback = RT_NULL;

/**
  * @brief  set PVD interrupt process callback
  * @param  None
  * @retval None
  */
void PVD_IRQCallBackSet(PVDCallback fun)
{
	if(fun != RT_NULL)
	{
		if(PVD_IRQCallback == RT_NULL)
		{
      PVD_IRQCallback = fun;
		}
		else
		{
			rt_kprintf("%s PVD_IRQCallback is Use\n",__FUNCTION__);
		}
	}
	else
	{
		rt_kprintf("%s Use Error\n",__FUNCTION__);
	}
}

/**
  * @brief  Configures EXTI Lines.
  * @param  None
  * @retval None
  */
static void EXTI_Configuration(void)
{
  EXTI_InitTypeDef EXTI_InitStructure;

  /* Configure EXTI Line16(PVD Output) to generate an interrupt on rising and
     falling edges */
  EXTI_ClearITPendingBit(EXTI_Line16);
  EXTI_InitStructure.EXTI_Line = EXTI_Line16;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
}

/**
  * @brief  Configures NVIC and Vector Table base location.
  * @param  None
  * @retval None
  */
static void NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  
  /* Configure one bit for preemption priority */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
  
  /* Enable the PVD Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = PVD_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

int PVD_hw_init(void)
{
  /* Enable PWR and BKP clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

  /* Configure EXTI Line to generate an interrupt on falling edge */
  EXTI_Configuration();

  /* NVIC configuration */
  NVIC_Configuration();
 
  /* Configure the PVD Level to 2.9V */
  PWR_PVDLevelConfig(PWR_PVDLevel_2V9);

  /* Enable the PVD Output */
  PWR_PVDCmd(ENABLE);

  return 0;
}
INIT_COMPONENT_EXPORT(PVD_hw_init);

/******************************************************************************/
/*            STM32F10x Peripherals Interrupt Handlers                        */
/******************************************************************************/

/**
  * @brief  This function handles the PVD Output interrupt request.
  * @param  None
  * @retval None
  */
void PVD_IRQHandler(void)
{
  if(EXTI_GetITStatus(EXTI_Line16) != RESET)
  {
    /* Toggle LED1 */
    rt_kprintf("system PVD entry\n");

		if(PVD_IRQCallback != RT_NULL)
		{
			PVD_IRQCallback();
		}
		
    /* Clear the Key Button EXTI line pending bit */
    EXTI_ClearITPendingBit(EXTI_Line16);
  }
}

