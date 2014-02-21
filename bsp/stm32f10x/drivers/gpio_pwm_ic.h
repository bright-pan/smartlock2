/*********************************************************************
 * Filename:      gpio_pwm_ic.h
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

#ifndef _GPIO_PWM_IC_H_
#define _GPIO_PWM_IC_H_

#include <rthw.h>
#include <rtthread.h>
#include <stm32f10x.h>
#include "gpio.h"

#define RT_DEVICE_CTRL_PWM_IC_START       0x14    /* enable receive irq */
#define RT_DEVICE_CTRL_PWM_IC_STOP 0x15    /* disable receive irq */
#define RT_DEVICE_CTRL_PWM_IC_WIDTH  0x16    /* disable receive irq */
#define RT_DEVICE_FLAG_PWM_RX           0x2000 /* flag mask for gpio pwm mode */
//#define RT_DEVICE_FLAG_ONE_PULSE        0x2000
#define DEVICE_NAME_INFRA_PULSE_PWM_IC "infra_ic"

struct gpio_pwm_ic_user_data
{
  const char name[RT_NAME_MAX];
  GPIO_TypeDef* gpiox;//port
  rt_uint16_t gpio_pinx;//pin
  GPIOMode_TypeDef gpio_mode;//mode
  GPIOSpeed_TypeDef gpio_speed;//speed
  rt_uint32_t gpio_clock;//clock
  rt_uint32_t gpio_pin_remap;//gpio pin remap
  TIM_TypeDef *timx;//tim[1-8]
  rt_uint32_t tim_rcc;
  rt_uint32_t tim_base_clock;// tim clock and < system core freq
  rt_uint16_t tim_base_reload_value;
  rt_uint16_t tim_base_clock_division;
  rt_uint16_t tim_base_counter_mode;//counter up or down
  rt_uint32_t tim_ic_slection;//ic input source
  rt_uint16_t tim_ic_prescaler;//ic psc
  rt_uint16_t tim_ic_filter;//ic filter value
  rt_uint16_t tim_ic_polarity;
  rt_uint16_t tim_ic_channel;
  rt_uint16_t tim_ic_input_trigger;
  rt_uint16_t tim_int_flag;
  __IO rt_uint16_t tim_ic_width_value;
  rt_uint8_t nvic_channel_1;
  rt_uint8_t nvic_channel_2;  
  rt_uint8_t nvic_preemption_priority;
  rt_uint8_t nvic_subpriority;
  void (* tim_rcc_cmd)(uint32_t RCC_APB2Periph, FunctionalState NewState);
};

void rt_hw_infra_pulse_pwm_ic_register(void);

#endif
