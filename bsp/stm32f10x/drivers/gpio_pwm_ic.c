/*********************************************************************
 * Filename:      gpio_pwm_ic.c
 *
 * Description:    
 *
 * Author:        Bright Pan <loststriker@gmail.com>
 * Created at:    2013-08-30
 *
 * Change Log:
 *
 * Copyright (C) 2013 Yuettak Co.,Ltd
 ********************************************************************/

#include "gpio_pwm_ic.h"
#include "gpio_exti.h"
#include "gpio_pin.h"
//#include "sms.h"
//#include "gprs.h"
#include "local.h"

/*
 *  GPIO ops function interfaces
 *
 */
static void __gpio_pin_configure(gpio_device *gpio)
{
  GPIO_InitTypeDef gpio_init_structure;
  struct gpio_pwm_ic_user_data *user = (struct gpio_pwm_ic_user_data*)gpio->parent.user_data;
  GPIO_StructInit(&gpio_init_structure);
  RCC_APB2PeriphClockCmd(user->gpio_clock,ENABLE);
  gpio_init_structure.GPIO_Mode = user->gpio_mode;
  gpio_init_structure.GPIO_Pin = user->gpio_pinx;
  gpio_init_structure.GPIO_Speed = user->gpio_speed;
  GPIO_Init(user->gpiox,&gpio_init_structure);
  if (user->gpio_pin_remap != RT_NULL)
  {
  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
    GPIO_PinRemapConfig(user->gpio_pin_remap, ENABLE);
  }
}
static void __gpio_nvic_configure(gpio_device *gpio,FunctionalState new_status)
{
  NVIC_InitTypeDef nvic_initstructure;
  struct gpio_pwm_ic_user_data* user = (struct gpio_pwm_ic_user_data *)gpio->parent.user_data;

  if (user->nvic_channel_1 != RT_NULL)
  {
    nvic_initstructure.NVIC_IRQChannel = user->nvic_channel_1;
    nvic_initstructure.NVIC_IRQChannelPreemptionPriority = user->nvic_preemption_priority;
    nvic_initstructure.NVIC_IRQChannelSubPriority = user->nvic_subpriority;
    nvic_initstructure.NVIC_IRQChannelCmd = new_status;
    NVIC_Init(&nvic_initstructure);
  }
    
  if (user->nvic_channel_2 != RT_NULL)
  {
    nvic_initstructure.NVIC_IRQChannel = user->nvic_channel_2;
    nvic_initstructure.NVIC_IRQChannelPreemptionPriority = user->nvic_preemption_priority;
    nvic_initstructure.NVIC_IRQChannelSubPriority = user->nvic_subpriority;
    nvic_initstructure.NVIC_IRQChannelCmd = new_status;
    NVIC_Init(&nvic_initstructure);
  }
}

static void __gpio_timer_configure(gpio_device *gpio)
{
  TIM_TimeBaseInitTypeDef time_base_structure;
  TIM_ICInitTypeDef time_ic_structure;
  struct gpio_pwm_ic_user_data *user = (struct gpio_pwm_ic_user_data*)gpio->parent.user_data;
  
  TIM_TimeBaseStructInit(&time_base_structure);
  TIM_ICStructInit(&time_ic_structure);
  
  if(user->timx == TIM1)
  {
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);
  }
  else
  {
    user->tim_rcc_cmd(user->tim_rcc, ENABLE);
  }
  TIM_DeInit(user->timx);
  /* timer base configuration */
  //rt_kprintf("user->tim_base_reload_value = %d\n",user->tim_base_reload_value);
  time_base_structure.TIM_Period = user->tim_base_reload_value;
  time_base_structure.TIM_Prescaler = (uint16_t) (SystemCoreClock / user->tim_base_clock) - 1;
  time_base_structure.TIM_ClockDivision = user->tim_base_clock_division;
  time_base_structure.TIM_CounterMode = user->tim_base_counter_mode;
  //time_base_structure.TIM_RepetitionCounter = 100;
  TIM_TimeBaseInit(user->timx, &time_base_structure);
   /* timer oc mode configuration */
  time_ic_structure.TIM_Channel = user->tim_ic_channel;
  time_ic_structure.TIM_ICPolarity = user->tim_ic_polarity;
  time_ic_structure.TIM_ICSelection = user->tim_ic_slection;
  time_ic_structure.TIM_ICPrescaler = user->tim_ic_prescaler;
  time_ic_structure.TIM_ICFilter = user->tim_ic_filter;

  TIM_ICInit(user->timx, &time_ic_structure);

  /*
  // Select the TIMx Input Trigger
  TIM_SelectInputTrigger(user->timx, user->tim_ic_input_trigger);
 
  // Select the slave Mode: Reset Mode
  TIM_SelectSlaveMode(user->timx, TIM_SlaveMode_);
 
  // Enable the Master/Slave Mode
  TIM_SelectMasterSlaveMode(user->timx, TIM_MasterSlaveMode_Enable);
  */
  if(RT_DEVICE_FLAG_INT_RX & gpio->parent.flag)
  {
    TIM_ITConfig(user->timx, user->tim_int_flag, ENABLE);
  }
}

/*
 * gpio ops configure
 */
rt_err_t gpio_pwm_ic_configure(gpio_device *gpio)
{	
  __gpio_pin_configure(gpio);
  if(RT_DEVICE_FLAG_INT_RX & gpio->parent.flag)
  {
    __gpio_nvic_configure(gpio, ENABLE);
  }
      
  if(RT_DEVICE_FLAG_PWM_RX & gpio->parent.flag)
  {	
    __gpio_timer_configure(gpio);
  }
  return RT_EOK;
}
rt_err_t gpio_pwm_ic_control(gpio_device *gpio, rt_uint8_t cmd, void *arg)
{
  struct gpio_pwm_ic_user_data *user = (struct gpio_pwm_ic_user_data*)gpio->parent.user_data;
  
  switch (cmd)
  {
    case RT_DEVICE_CTRL_PWM_IC_START:
      {
        TIM_Cmd(user->timx, ENABLE);
        break;
      }
    case RT_DEVICE_CTRL_PWM_IC_STOP:
      {
        TIM_Cmd(user->timx, DISABLE);
        TIM_SetCounter(user->timx,0);
        break;
      }
    case RT_DEVICE_CTRL_PWM_IC_WIDTH:
      {
        *(rt_uint16_t *)arg = user->tim_ic_width_value;
        break;
      }
  }
  return RT_EOK;
}

void gpio_pwm_ic_out(gpio_device *gpio, rt_uint8_t data)
{
  struct gpio_pwm_ic_user_data *user = (struct gpio_pwm_ic_user_data*)gpio->parent.user_data;

  if (gpio->parent.flag & RT_DEVICE_FLAG_WRONLY)
  {
    if (data)
    {
      GPIO_SetBits(user->gpiox,user->gpio_pinx);
    }
    else
    {
      GPIO_ResetBits(user->gpiox,user->gpio_pinx);
    }
  }
  else
  {
#ifdef RT_USING_FINSH
    rt_kprintf("gpio pwm device <%s> is can`t write! please check device flag RT_DEVICE_FLAG_WRONLY\n", user->name);
#endif
  }
}

rt_uint8_t gpio_pwm_ic_intput(gpio_device *gpio)
{
  struct gpio_pwm_ic_user_data* user = (struct gpio_pwm_ic_user_data *)gpio->parent.user_data;

  if (gpio->parent.flag & RT_DEVICE_FLAG_RDONLY)
  {
    return GPIO_ReadInputDataBit(user->gpiox,user->gpio_pinx);
  }
  else
  {
#ifdef RT_USING_FINSH
    rt_kprintf("gpio pwm device <%s> is can`t read! please check device flag RT_DEVICE_FLAG_RDONLY\n", user->name);
#endif
    return 0;
  }
}

struct rt_gpio_ops gpio_pwm_ic_user_ops= 
{
  gpio_pwm_ic_configure,
  gpio_pwm_ic_control,
  gpio_pwm_ic_out,
  gpio_pwm_ic_intput
};

/* infra pwm ic */
struct gpio_pwm_ic_user_data infra_pulse_pwm_ic_user_data = 
{
  DEVICE_NAME_INFRA_PULSE_PWM_IC,
  GPIOE,
  GPIO_Pin_11,
  GPIO_Mode_IN_FLOATING,
  GPIO_Speed_50MHz,
  RCC_APB2Periph_GPIOE,
  GPIO_FullRemap_TIM1,
  /* timer base */
  TIM1,
  RCC_APB2Periph_TIM1,
  72000000,
  65535,
  0,
  TIM_CounterMode_Up,
  /* timer ic */
  TIM_ICSelection_DirectTI,
  TIM_ICPSC_DIV1,
  0,// filter value
  TIM_ICPolarity_Falling,
  TIM_Channel_2,
  TIM_TS_TI2FP2,
  TIM_IT_CC2|TIM_IT_Update,
  0,// pulse counts
  TIM1_UP_IRQn,
  TIM1_CC_IRQn,
  2,
  1,
  RT_NULL,
};

gpio_device infra_pulse_pwm_ic_device;

int rt_hw_infra_pulse_pwm_ic_register(void)
{
  gpio_device *pwm_device = &infra_pulse_pwm_ic_device;
  struct gpio_pwm_ic_user_data *pwm_user_data = &infra_pulse_pwm_ic_user_data;

  pwm_device->ops = &gpio_pwm_ic_user_ops;
  pwm_user_data->tim_rcc_cmd = RCC_APB1PeriphClockCmd;
  rt_hw_gpio_register(pwm_device,pwm_user_data->name, (RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_PWM_RX | RT_DEVICE_FLAG_INT_RX), pwm_user_data);
  
  return 0;
}


INIT_DEVICE_EXPORT(rt_hw_infra_pulse_pwm_ic_register);



__IO uint16_t IC2Value = 0;
__IO uint8_t ic_complete_flag = 0;
__IO uint32_t ic_update = 0;
static uint32_t ic_palarity_flag = TIM_ICPolarity_Falling;

void TIM1_UP_IRQHandler(void)
{
	  /* enter interrupt */
  rt_interrupt_enter();
	if(TIM_GetITStatus(infra_pulse_pwm_ic_user_data.timx,TIM_IT_Update) == SET)
	{
	  if(ic_palarity_flag != infra_pulse_pwm_ic_user_data.tim_ic_polarity)
	  {
	    ic_update++;
	  }
	}
	TIM_ClearITPendingBit(infra_pulse_pwm_ic_user_data.timx, infra_pulse_pwm_ic_user_data.tim_int_flag);
	 /* leave interrupt */
  rt_interrupt_leave();
}


void TIM1_CC_IRQHandler(void)
{
  /* enter interrupt */
  rt_interrupt_enter();

  /* tim1 isr */
  if(TIM_GetITStatus(infra_pulse_pwm_ic_user_data.timx, TIM_IT_CC2) == SET)
  {
    //TIM_ClearITPendingBit(infra_pulse_pwm_ic_user_data.timx, TIM_IT_CC2);
    if (ic_palarity_flag == infra_pulse_pwm_ic_user_data.tim_ic_polarity)
		{
			// Get the Input Capture value
			TIM_SetCounter(infra_pulse_pwm_ic_user_data.timx, 0);
			ic_palarity_flag = (infra_pulse_pwm_ic_user_data.tim_ic_polarity == TIM_ICPolarity_Rising) ? TIM_ICPolarity_Falling : TIM_ICPolarity_Rising;
			TIM_OC2PolarityConfig(infra_pulse_pwm_ic_user_data.timx, ic_palarity_flag); //CC1P=1 ÉèÖÃÎªÏÂ½µÑØ²¶»ñ
			ic_update = 0;
		  //rt_kprintf("TIM_ICPolarity_Falling\n");
		}
		else
		{
			IC2Value = TIM_GetCapture1(infra_pulse_pwm_ic_user_data.timx);
			//rt_kprintf("IC2Value = %d,ic_update = %d\n",IC2Value,ic_update);
			ic_palarity_flag = infra_pulse_pwm_ic_user_data.tim_ic_polarity;
			TIM_OC2PolarityConfig(infra_pulse_pwm_ic_user_data.timx, ic_palarity_flag); //CC1P=1 ÉèÖÃÎªÏÂ½µÑØ²¶»ñ      
			ic_complete_flag = 1;
			//TIM_ITConfig(infra_pulse_pwm_ic_user_data.timx,infra_pulse_pwm_ic_user_data.tim_int_flag,DISABLE);
			//rt_kprintf("TIM_ICPolarity_Rising\n");
		}
  }
	TIM_ClearITPendingBit(infra_pulse_pwm_ic_user_data.timx, infra_pulse_pwm_ic_user_data.tim_int_flag);
  /* leave interrupt */
  rt_interrupt_leave();
}



#ifdef RT_USING_FINSH
#include <finsh.h>
void pwm_ic_start(char *str)
{
  rt_device_t device = RT_NULL;
  device = rt_device_find(str);
  if(device != RT_NULL)
  {
  	if(!(device->open_flag & RT_DEVICE_OFLAG_OPEN))
  	{
			rt_device_open(device,RT_DEVICE_OFLAG_RDWR);
  	}
    rt_device_control(device, RT_DEVICE_CTRL_PWM_IC_START, (void *)0);
  }
  
}
FINSH_FUNCTION_EXPORT(pwm_ic_start, pwm_set_pulse_counts[device_name x]);

void pwm_ic_stop(char *str)
{
  rt_device_t device = RT_NULL;
  device = rt_device_find(str);
  if(device != RT_NULL)
  {
    rt_device_control(device, RT_DEVICE_CTRL_PWM_IC_STOP, (void *)0);
  }
}
FINSH_FUNCTION_EXPORT(pwm_ic_stop, pwm_set_pulse_value[device_name x]);

rt_uint32_t pwm_ic_get(char *str)
{
  uint32_t width;
  rt_device_t device = RT_NULL;
  device = rt_device_find(str);
  if(device != RT_NULL)
  {
    rt_device_control(device, RT_DEVICE_CTRL_PWM_IC_WIDTH, (void *)&width);
  }
  
  return width;
}

void pwm_ic_print()
{
  uint32_t	result;
  char str[20];

	result = (65535.0*ic_update + IC2Value)/72.0;
	rt_sprintf(str,"%d",result);
  rt_kprintf("%d, %ld, %s\n", IC2Value, ic_update,str );

}
rt_uint32_t	pwm_ic_time(void)
{
	uint32_t result;

	result = (65535.0*ic_update + IC2Value)/72.0;
	IC2Value = ic_update = 0;
	
	return result;
}
FINSH_FUNCTION_EXPORT(pwm_ic_print, pwm_send_pulse[device_name])
FINSH_FUNCTION_EXPORT(pwm_ic_get, pwm_send_pulse[device_name])

#endif
