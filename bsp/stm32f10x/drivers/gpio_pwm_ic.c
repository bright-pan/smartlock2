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
#include "sms.h"
#include "gprs.h"
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
  user->tim_rcc_cmd(user->tim_rcc, ENABLE);
  /* timer base configuration */
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
  TIM_SelectSlaveMode(user->timx, TIM_SlaveMode_Reset);
 
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
  GPIOB,
  GPIO_Pin_6,
  GPIO_Mode_IN_FLOATING,
  GPIO_Speed_50MHz,
  RCC_APB2Periph_GPIOB,
  RT_NULL,
  /* timer base */
  TIM4,
  RCC_APB1Periph_TIM4,
  72000000,
  65535,
  0,
  TIM_CounterMode_Up,
  /* timer ic */
  TIM_ICSelection_DirectTI,
  TIM_ICPSC_DIV1,
  0,// filter value
  TIM_ICPolarity_Falling,
  TIM_Channel_1,
  TIM_TS_TI1FP1,
  TIM_IT_CC1|TIM_IT_Update,
  0,// pulse counts
  RT_NULL,
  TIM4_IRQn,
  1,
  0,
  RT_NULL,
};

gpio_device infra_pulse_pwm_ic_device;

void rt_hw_infra_pulse_pwm_ic_register(void)
{
  gpio_device *pwm_device = &infra_pulse_pwm_ic_device;
  struct gpio_pwm_ic_user_data *pwm_user_data = &infra_pulse_pwm_ic_user_data;

  pwm_device->ops = &gpio_pwm_ic_user_ops;
  pwm_user_data->tim_rcc_cmd = RCC_APB1PeriphClockCmd;
  rt_hw_gpio_register(pwm_device,pwm_user_data->name, (RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_PWM_RX | RT_DEVICE_FLAG_INT_RX), pwm_user_data);
}

#ifdef RT_USING_FINSH
#include <finsh.h>
void pwm_ic_start(char *str)
{
  rt_device_t device = RT_NULL;
  device = rt_device_find(str);
  if(device != RT_NULL)
  {
    rt_device_control(device, RT_DEVICE_CTRL_PWM_IC_START, (void *)0);
  }
  
}
FINSH_FUNCTION_EXPORT(pwm_ic_start, pwm_set_pulse_counts[device_name x])

void pwm_ic_stop(char *str)
{
  rt_device_t device = RT_NULL;
  device = rt_device_find(str);
  if(device != RT_NULL)
  {
    rt_device_control(device, RT_DEVICE_CTRL_PWM_IC_STOP, (void *)0);
  }
}
FINSH_FUNCTION_EXPORT(pwm_ic_stop, pwm_set_pulse_value[device_name x])

uint32_t pwm_ic_get(char *str)
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
  extern __IO uint16_t IC2Value;
  extern __IO uint32_t ic_update;
  uint32_t	result;
  char str[20];

	result = (65535.0*ic_update + IC2Value)/72.0;
	rt_sprintf(str,"%d",result);
  rt_kprintf("%d, %ld, %s\n", IC2Value, ic_update,str );

}
uint32_t	pwm_ic_time(void)
{
	extern __IO uint16_t IC2Value;
	extern __IO uint32_t ic_update;
	uint32_t result;

	result = (65535.0*ic_update + IC2Value)/72.0;
	IC2Value = ic_update = 0;
	
	return result;
}
FINSH_FUNCTION_EXPORT(pwm_ic_print, pwm_send_pulse[device_name])
FINSH_FUNCTION_EXPORT(pwm_ic_get, pwm_send_pulse[device_name])
#endif
