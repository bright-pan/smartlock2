#include "stm32f10x.h"
#include "rtthread.h"

/*
 * ��������NVIC_Configuration
 * ����  ������Ƕ�������жϿ�����NVIC
 * ����  ����
 * ���  ����
 * ����  ���ڲ�����
 */
static void NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  
  /* Configure one bit for preemption priority */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
  
  /* ����P[A|B|C|D|E]0Ϊ�ж�Դ */
  NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

/*
 * ��������EXTI_PB0_Config
 * ����  ������ PB0 Ϊ���жϿڣ��������ж����ȼ�
 * ����  ����
 * ���  ����
 * ����  ���ⲿ����
 */
void EXTI_PD2_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure; 
	EXTI_InitTypeDef EXTI_InitStructure;

	/* config the extiline(PB0) clock and AFIO clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO,ENABLE);

	/* config the NVIC(PB0) */
	NVIC_Configuration();

	/* EXTI line gpio config(PB0) */	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;       
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	 // ��������
  GPIO_Init(GPIOD, &GPIO_InitStructure);

	/* EXTI line(PB0) mode config */
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource2); 
  EXTI_InitStructure.EXTI_Line = EXTI_Line2;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling; //�½����ж�
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure); 
}

/*
 * ��������TIM2_NVIC_Configuration
 * ����  ��TIM2�ж����ȼ�����
 * ����  ����
 * ���  ����	
 */
void TIM6_NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure; 
    
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);  													
    NVIC_InitStructure.NVIC_IRQChannel = TIM6_IRQn;	  
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;	
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}


/*TIM_Period--1000   TIM_Prescaler--71 -->�ж�����Ϊ1ms*/
void TIM6_Configuration(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
    TIM_DeInit(TIM6);
    TIM_TimeBaseStructure.TIM_Period=10;		 								/* �Զ���װ�ؼĴ������ڵ�ֵ(����ֵ) */
    /* �ۼ� TIM_Period��Ƶ�ʺ����һ�����»����ж� */
    TIM_TimeBaseStructure.TIM_Prescaler= (72 - 1);				    /* ʱ��Ԥ��Ƶ�� 72M/72 */
    TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1; 		/* ������Ƶ */
    TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; /* ���ϼ���ģʽ */
    TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);
    TIM_ClearFlag(TIM6, TIM_FLAG_Update);							    		/* �������жϱ�־ */
    TIM_ITConfig(TIM6,TIM_IT_Update,ENABLE);
    TIM_Cmd(TIM6, ENABLE);																		/* ����ʱ�� */    
}

int rt433_hw_init(void)
{
	NVIC_Configuration();
	EXTI_PD2_Config();
	TIM6_NVIC_Configuration();
	TIM6_Configuration();

	return 0;
}
//INIT_APP_EXPORT(rt433_hw_init);

typedef struct
{
	rt_uint32_t low_time;
	rt_uint8_t  LowEnd;
	rt_uint32_t Time[10];
}RF433CodeDef;

static RF433CodeDef rf433manage = 
{
	0,
	0,
};

void 
EXTI2_IRQHandler(void)
{
	/* enter interrupt */
	rt_interrupt_enter();
	
	if(EXTI_GetITStatus(EXTI_Line2) == SET)
	{	
		u8 pin;
		pin = GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_2);
		if(pin == 0)
		{
			rf433manage.low_time = 0;
		}
		else
		{
			//rf433manage.LowEnd = 1;
			//if(rf433manage.LowEnd == 1)
			{
				if(rf433manage.low_time*100 >= 250*100)
				{
					rf433manage.Time[0] = rf433manage.low_time;
					rt_kprintf("����ͷ���ź�%d\n",rf433manage.Time[0]);
				}
				rf433manage.low_time = 0;
			}
		}
		EXTI_ClearITPendingBit(EXTI_Line2);
	}

	/* leave interrupt */
	rt_interrupt_leave();
}

/**
  * @brief  This function handles TIM2 interrupt request.
  * @param  None
  * @retval : None
  */
void TIM6_IRQHandler(void)
{
		/* enter interrupt */
	rt_interrupt_enter();
	if ( TIM_GetITStatus(TIM6 , TIM_IT_Update) != RESET ) 
	{	
		rf433manage.low_time++;

		TIM_ClearITPendingBit(TIM6 , TIM_FLAG_Update);    
	}		
		/* leave interrupt */
	rt_interrupt_leave();
}




