/*********************************************************************
 * Filename:      gpio_pwm.c
 * 
 * Description:    
 *
 * Author:        Bright Pan <loststriker@gmail.com>
 * Created at:    2013-04-27
 *                
 * Change Log:
 *
 * Copyright (C) 2013 Yuettak Co.,Ltd
 ********************************************************************/

#include "gpio_pwm.h"
#include "gpio_exti.h"
#include "gpio_pin.h"
//#include "sms.h"
//#include "gprs.h"
//#include "local.h"
//#include "voiceapp.h"
struct gpio_pwm_user_data
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
  rt_uint32_t tim_oc_mode;//output compare mode
  rt_uint16_t tim_oc_output_state;//output value state
  rt_uint16_t tim_oc_pulse_value;//pulse value
  rt_uint16_t tim_oc_polarity;
  rt_uint16_t tim_oc_channel;
  rt_uint16_t tim_int_flag;
  __IO rt_uint32_t tim_pulse_counts;
  rt_uint8_t nvic_channel_1;
  rt_uint8_t nvic_channel_2;  
  rt_uint8_t nvic_preemption_priority;
  rt_uint8_t nvic_subpriority;
  void (* tim_oc_init)(TIM_TypeDef *TIMx, TIM_OCInitTypeDef *TIM_OCInitStruct);
  void (* tim_oc_set_compare)(TIM_TypeDef* TIMx, uint16_t Compare1);
  void (* tim_rcc_cmd)(uint32_t RCC_APB2Periph, FunctionalState NewState);
};

/*
 *  GPIO ops function interfaces
 *
 */
static void __gpio_pin_configure(gpio_device *gpio)
{
    GPIO_InitTypeDef gpio_init_structure;
    struct gpio_pwm_user_data *user = (struct gpio_pwm_user_data*)gpio->parent.user_data;
    GPIO_StructInit(&gpio_init_structure);
    RCC_APB2PeriphClockCmd(user->gpio_clock,ENABLE);
    gpio_init_structure.GPIO_Mode = user->gpio_mode;
    gpio_init_structure.GPIO_Pin = user->gpio_pinx;
    gpio_init_structure.GPIO_Speed = user->gpio_speed;
    GPIO_Init(user->gpiox,&gpio_init_structure);

    if (IS_GPIO_REMAP(user->gpio_pin_remap))
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
        GPIO_PinRemapConfig(user->gpio_pin_remap, ENABLE);
    }
}
static void __gpio_nvic_configure(gpio_device *gpio,FunctionalState new_status)
{
  NVIC_InitTypeDef nvic_initstructure;
  struct gpio_pwm_user_data* user = (struct gpio_pwm_user_data *)gpio->parent.user_data;

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

static void __gpio_timer_base_configure(gpio_device *gpio)
{
    TIM_TimeBaseInitTypeDef time_base_structure;
    struct gpio_pwm_user_data *user = (struct gpio_pwm_user_data*)gpio->parent.user_data;
    TIM_TimeBaseStructInit(&time_base_structure);
    /* timer base configuration */
    time_base_structure.TIM_Period = user->tim_base_reload_value;
    time_base_structure.TIM_Prescaler = (uint16_t) (SystemCoreClock / user->tim_base_clock) - 1;
    time_base_structure.TIM_ClockDivision = user->tim_base_clock_division;
    time_base_structure.TIM_CounterMode = user->tim_base_counter_mode;
    //time_base_structure.TIM_RepetitionCounter = 100;
    TIM_TimeBaseInit(user->timx, &time_base_structure);
}

static void __gpio_timer_oc_configure(gpio_device *gpio)
{
    TIM_OCInitTypeDef time_oc_structure;
    struct gpio_pwm_user_data *user = (struct gpio_pwm_user_data*)gpio->parent.user_data;
    TIM_OCStructInit(&time_oc_structure);
    /* timer oc mode configuration */
    time_oc_structure.TIM_OCMode = user->tim_oc_mode;
    time_oc_structure.TIM_OutputState = user->tim_oc_output_state;
    time_oc_structure.TIM_Pulse = user->tim_oc_pulse_value;
    time_oc_structure.TIM_OCPolarity = user->tim_oc_polarity;
    //time_oc_structure.TIM_OCIdleState = TIM_OCIdleState_Reset;
    user->tim_oc_init(user->timx, &time_oc_structure);
    //TIM_OC1PreloadConfig(user->timx, TIM_OCPreload_Disable);
}

static void __gpio_timer_configure(gpio_device *gpio)
{
    struct gpio_pwm_user_data *user = (struct gpio_pwm_user_data*)gpio->parent.user_data;
    user->tim_rcc_cmd(user->tim_rcc, ENABLE);
    __gpio_timer_base_configure(gpio);
    __gpio_timer_oc_configure(gpio);
    /*
    if(RT_DEVICE_FLAG_ONE_PULSE & gpio->parent.flag)
    {
        TIM_SelectOnePulseMode(user->timx, TIM_OPMode_Single);
    }
    */
    if(RT_DEVICE_FLAG_INT_TX & gpio->parent.flag)
    {
        TIM_ITConfig(user->timx, user->tim_int_flag, ENABLE);
    }
    /* TIM1 Main Output Enable */
    if (IS_TIM_LIST2_PERIPH(user->timx))
    {
        TIM_CtrlPWMOutputs(user->timx, ENABLE);
    }
    //TIM_Cmd(user->timx, ENABLE);
}

/*
 * gpio ops configure
 */
rt_err_t gpio_pwm_configure(gpio_device *gpio)
{	
  __gpio_pin_configure(gpio);
  if(RT_DEVICE_FLAG_INT_TX & gpio->parent.flag)
  {
    __gpio_nvic_configure(gpio, ENABLE);
  }
      
  if(RT_DEVICE_FLAG_PWM_TX & gpio->parent.flag)
  {	
    __gpio_timer_configure(gpio);
  }
  return RT_EOK;
}
rt_err_t gpio_pwm_control(gpio_device *gpio, rt_uint8_t cmd, void *arg)
{
  struct gpio_pwm_user_data *user = (struct gpio_pwm_user_data*)gpio->parent.user_data;
  
  switch (cmd)
  {
    case RT_DEVICE_CTRL_SEND_PULSE:
      {
        if (user->tim_pulse_counts > 0)
        {
          TIM_CCxCmd(user->timx, user->tim_oc_channel, TIM_CCx_Enable);
          TIM_Cmd(user->timx, ENABLE);
        }
        break;
      }
    case RT_DEVICE_CTRL_SET_PULSE_VALUE:
      {
        user->tim_oc_pulse_value = *(rt_uint16_t *)arg;
        user->tim_oc_set_compare(user->timx, user->tim_oc_pulse_value);        
        break;
      }
    case RT_DEVICE_CTRL_SET_PULSE_COUNTS:
      {
        user->tim_pulse_counts = *(rt_uint32_t *)arg;
        break;
      }
    case RT_DEVICE_CTRL_CONFIG_DEVICE:
    {	
    	gpio->ops->configure((gpio_device *)arg);
			break;
    }
  }
  return RT_EOK;
}

void gpio_pwm_out(gpio_device *gpio, rt_uint8_t data)
{
  struct gpio_pwm_user_data *user = (struct gpio_pwm_user_data*)gpio->parent.user_data;

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

rt_uint8_t gpio_pwm_intput(gpio_device *gpio)
{
  struct gpio_pwm_user_data* user = (struct gpio_pwm_user_data *)gpio->parent.user_data;

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

struct rt_gpio_ops gpio_pwm_user_ops= 
{
  gpio_pwm_configure,
  gpio_pwm_control,
  gpio_pwm_out,
  gpio_pwm_intput
};

/* voice data device register 
struct gpio_pwm_user_data voice_data_user_data = 
{
  DEVICE_NAME_VOICE_DATA,
  GPIOA,
  GPIO_Pin_2,
  GPIO_Mode_AF_PP,
  GPIO_Speed_50MHz,
  RCC_APB2Periph_GPIOA,
  RT_NULL,
  // timer base 
  TIM5,
  RCC_APB1Periph_TIM5,
  24000000,
  4800,//period=200us
  0,
  TIM_CounterMode_Up,
  // timer oc 
  TIM_OCMode_PWM2,
  TIM_OutputState_Enable,
  2400,// pulse value 100us
  TIM_OCPolarity_High,
  TIM_Channel_3,
  TIM_IT_CC3 | TIM_IT_Update,
  0,// pulse counts
  TIM5_IRQn,
  RT_NULL,
  1,
  0,
  RT_NULL,
  RT_NULL,
  RT_NULL,
};

gpio_device voice_data_device;

int rt_hw_voice_data_register(void)
{
    gpio_device *pwm_device = &voice_data_device;
    struct gpio_pwm_user_data *pwm_user_data = &voice_data_user_data;

    pwm_device->ops = &gpio_pwm_user_ops;
    pwm_user_data->tim_oc_init = TIM_OC3Init;
    pwm_user_data->tim_oc_set_compare = TIM_SetCompare3;
    pwm_user_data->tim_rcc_cmd = RCC_APB1PeriphClockCmd;
    rt_hw_gpio_register(pwm_device,pwm_user_data->name, (RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_PWM_TX | RT_DEVICE_FLAG_INT_TX), pwm_user_data);
    return 0;
}
*/

/* voice reset device register
struct gpio_pwm_user_data voice_reset_user_data = 
{
    DEVICE_NAME_VOICE_RESET,
    GPIOA,
    GPIO_Pin_3,
    GPIO_Mode_AF_PP,
    GPIO_Speed_50MHz,
    RCC_APB2Periph_GPIOA,
    RT_NULL,
    // timer base
    TIM5,
    RCC_APB1Periph_TIM5,
    24000000,
    4800,//period=200us
    0,
    TIM_CounterMode_Up,
    // timer oc
    TIM_OCMode_PWM2,
    TIM_OutputState_Enable,
    2400,// pulse value 100us
    TIM_OCPolarity_High,
    TIM_Channel_4,
    TIM_IT_CC4 | TIM_IT_Update,
    0,// pulse counts
    TIM5_IRQn,
    RT_NULL,
    1,
    0,
    RT_NULL,
    RT_NULL,
    RT_NULL,
};

gpio_device voice_reset_device;

int rt_hw_voice_reset_register(void)
{
    gpio_device *pwm_device = &voice_reset_device;
    struct gpio_pwm_user_data *pwm_user_data = &voice_reset_user_data;

    pwm_device->ops = &gpio_pwm_user_ops;
    pwm_user_data->tim_oc_init = TIM_OC4Init;
    pwm_user_data->tim_oc_set_compare = TIM_SetCompare4;
    pwm_user_data->tim_rcc_cmd = RCC_APB1PeriphClockCmd;
    rt_hw_gpio_register(pwm_device,pwm_user_data->name, (RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_PWM_TX | RT_DEVICE_FLAG_INT_TX), pwm_user_data);
    return 0;
}
*/
/* Motor 1 device */
struct gpio_pwm_user_data motor1_user_data = 
{
    DEVICE_NAME_MOTOR1,
    GPIOC,
    GPIO_Pin_6,
    GPIO_Mode_AF_PP,
    GPIO_Speed_50MHz,
    RCC_APB2Periph_GPIOC,
    GPIO_FullRemap_TIM3,
    /* timer base */
    TIM3,
    RCC_APB1Periph_TIM3,
    24000000,
    65535,
    0,
    TIM_CounterMode_Up,
    /* timer oc */
    TIM_OCMode_PWM2,
    TIM_OutputState_Enable,
    3000,// pulse value
    TIM_OCPolarity_High,
    TIM_Channel_1,
    TIM_IT_CC1 | TIM_IT_Update,
    0,// pulse counts
    TIM3_IRQn,
    RT_NULL,
    1,
    0,
    RT_NULL,
    RT_NULL,
    RT_NULL,
};

gpio_device motor1_device;

int rt_hw_motor1_register(void)
{
    gpio_device *pwm_device = &motor1_device;
    struct gpio_pwm_user_data *pwm_user_data = &motor1_user_data;

    pwm_device->ops = &gpio_pwm_user_ops;
    pwm_user_data->tim_oc_init = TIM_OC1Init;
    pwm_user_data->tim_oc_set_compare = TIM_SetCompare1;
    pwm_user_data->tim_rcc_cmd = RCC_APB1PeriphClockCmd;
    rt_hw_gpio_register(pwm_device,pwm_user_data->name, (RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_PWM_TX | RT_DEVICE_FLAG_INT_TX), pwm_user_data);
    return 0;
}

/* Motor 2 device */
struct gpio_pwm_user_data motor2_user_data = 
{
    DEVICE_NAME_MOTOR2,
    GPIOC,
    GPIO_Pin_7,
    GPIO_Mode_AF_PP,
    GPIO_Speed_50MHz,
    RCC_APB2Periph_GPIOC,
    GPIO_FullRemap_TIM3,
    /* timer base */
    TIM3,
    RCC_APB1Periph_TIM3,
    24000000,
    65535,
    0,
    TIM_CounterMode_Up,
    /* timer oc */
    TIM_OCMode_PWM2,
    TIM_OutputState_Enable,
    3000,// pulse value
    TIM_OCPolarity_High,
    TIM_Channel_2,
    TIM_IT_CC2 | TIM_IT_Update,
    0,// pulse counts
    TIM3_IRQn,
    RT_NULL,
    1,
    0,
    RT_NULL,
    RT_NULL,
    RT_NULL,
};

gpio_device motor2_device;

int rt_hw_motor2_register(void)
{
    gpio_device *pwm_device = &motor2_device;
    struct gpio_pwm_user_data *pwm_user_data = &motor2_user_data;

    pwm_device->ops = &gpio_pwm_user_ops;
    pwm_user_data->tim_oc_init = TIM_OC2Init;
    pwm_user_data->tim_oc_set_compare = TIM_SetCompare2;
    pwm_user_data->tim_rcc_cmd = RCC_APB1PeriphClockCmd;
    rt_hw_gpio_register(pwm_device,pwm_user_data->name, (RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_PWM_TX | RT_DEVICE_FLAG_INT_TX), pwm_user_data);
    return 0;
}

/* Motor 3 device */
struct gpio_pwm_user_data motor3_user_data = 
{
    DEVICE_NAME_MOTOR3,
    GPIOC,
    GPIO_Pin_8,
    GPIO_Mode_AF_PP,
    GPIO_Speed_50MHz,
    RCC_APB2Periph_GPIOC,
    GPIO_FullRemap_TIM3,
    /* timer base */
    TIM3,
    RCC_APB1Periph_TIM3,
    24000000,
    65535,
    0,
    TIM_CounterMode_Up,
    /* timer oc */
    TIM_OCMode_PWM2,
    TIM_OutputState_Enable,
    3000,// pulse value
    TIM_OCPolarity_High,
    TIM_Channel_3,
    TIM_IT_CC3 | TIM_IT_Update,
    0,// pulse counts
    TIM3_IRQn,
    RT_NULL,
    1,
    0,
    RT_NULL,
    RT_NULL,
    RT_NULL,
};

gpio_device motor3_device;

int rt_hw_motor3_register(void)
{
    gpio_device *pwm_device = &motor3_device;
    struct gpio_pwm_user_data *pwm_user_data = &motor3_user_data;

    pwm_device->ops = &gpio_pwm_user_ops;
    pwm_user_data->tim_oc_init = TIM_OC3Init;
    pwm_user_data->tim_oc_set_compare = TIM_SetCompare3;
    pwm_user_data->tim_rcc_cmd = RCC_APB1PeriphClockCmd;
    rt_hw_gpio_register(pwm_device,pwm_user_data->name, (RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_PWM_TX | RT_DEVICE_FLAG_INT_TX), pwm_user_data);
    return 0;
}

/* Motor 4 device */
struct gpio_pwm_user_data motor4_user_data = 
{
    DEVICE_NAME_MOTOR4,
    GPIOC,
    GPIO_Pin_9,
    GPIO_Mode_AF_PP,
    GPIO_Speed_50MHz,
    RCC_APB2Periph_GPIOC,
    GPIO_FullRemap_TIM3,
    /* timer base */
    TIM3,
    RCC_APB1Periph_TIM3,
    24000000,
    65535,
    0,
    TIM_CounterMode_Up,
    /* timer oc */
    TIM_OCMode_PWM2,
    TIM_OutputState_Enable,
    3000,// pulse value
    TIM_OCPolarity_High,
    TIM_Channel_4,
    TIM_IT_CC4 | TIM_IT_Update,
    0,// pulse counts
    TIM3_IRQn,
    RT_NULL,
    1,
    0,
    RT_NULL,
    RT_NULL,
    RT_NULL,
};

gpio_device motor4_device;

int rt_hw_motor4_register(void)
{
    gpio_device *pwm_device = &motor4_device;
    struct gpio_pwm_user_data *pwm_user_data = &motor4_user_data;

    pwm_device->ops = &gpio_pwm_user_ops;
    pwm_user_data->tim_oc_init = TIM_OC4Init;
    pwm_user_data->tim_oc_set_compare = TIM_SetCompare4;
    pwm_user_data->tim_rcc_cmd = RCC_APB1PeriphClockCmd;
    rt_hw_gpio_register(pwm_device,pwm_user_data->name, (RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_PWM_TX | RT_DEVICE_FLAG_INT_TX), pwm_user_data);
    return 0;
}

/* lcd led device */
struct gpio_pwm_user_data lcd_led_user_data = 
{
    DEVICE_NAME_LCD_LED,
    GPIOD,
    GPIO_Pin_12,
    GPIO_Mode_AF_PP,
    GPIO_Speed_50MHz,
    RCC_APB2Periph_GPIOD,
    GPIO_Remap_TIM4,
    /* timer base */
    TIM4,
    RCC_APB1Periph_TIM4,
    72000000,
    1895,//38KHZ 1/38000/(1/72000000)
    0,
    TIM_CounterMode_Up,
    /* timer oc */
    TIM_OCMode_PWM2,
    TIM_OutputState_Enable,
    900,// pulse value 50%
    TIM_OCPolarity_High,
    TIM_Channel_1,
    TIM_IT_CC1 | TIM_IT_Update,
    0,
    TIM4_IRQn,
    RT_NULL,
    1,
    0,
    RT_NULL,
    RT_NULL,
    RT_NULL,
};

gpio_device lcd_led_device;

int rt_hw_lcd_led_register(void)
{
    gpio_device *pwm_device = &lcd_led_device;
    struct gpio_pwm_user_data *pwm_user_data = &lcd_led_user_data;

    pwm_device->ops = &gpio_pwm_user_ops;
    pwm_user_data->tim_oc_init = TIM_OC1Init;
    pwm_user_data->tim_oc_set_compare = TIM_SetCompare1;
    pwm_user_data->tim_rcc_cmd = RCC_APB1PeriphClockCmd;
    rt_hw_gpio_register(pwm_device,pwm_user_data->name, (RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_PWM_TX | RT_DEVICE_FLAG_INT_TX), pwm_user_data);
    return 0;
}

/* camera led device */
struct gpio_pwm_user_data camera_led_user_data = 
{
    DEVICE_NAME_CAMERA_LED,
    GPIOD,
    GPIO_Pin_13,
    GPIO_Mode_AF_PP,
    GPIO_Speed_50MHz,
    RCC_APB2Periph_GPIOD,
    GPIO_Remap_TIM4,
    /* timer base */
    TIM4,
    RCC_APB1Periph_TIM4,
    72000000,
    1895,//38KHZ 1/38000/(1/72000000)
    0,
    TIM_CounterMode_Up,
    /* timer oc */
    TIM_OCMode_PWM2,
    TIM_OutputState_Enable,
    900,// pulse value 50%
    TIM_OCPolarity_High,
    TIM_Channel_2,
    TIM_IT_CC2 | TIM_IT_Update,
    0,
    TIM4_IRQn,
    RT_NULL,
    1,
    0,
    RT_NULL,
    RT_NULL,
    RT_NULL,
};

gpio_device camera_led_device;

int rt_hw_camera_led_register(void)
{
    gpio_device *pwm_device = &camera_led_device;
    struct gpio_pwm_user_data *pwm_user_data = &camera_led_user_data;

    pwm_device->ops = &gpio_pwm_user_ops;
    pwm_user_data->tim_oc_init = TIM_OC2Init;
    pwm_user_data->tim_oc_set_compare = TIM_SetCompare2;
    pwm_user_data->tim_rcc_cmd = RCC_APB1PeriphClockCmd;
    rt_hw_gpio_register(pwm_device,pwm_user_data->name, (RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_PWM_TX | RT_DEVICE_FLAG_INT_TX), pwm_user_data);
    return 0;
}

/* logo led device */
struct gpio_pwm_user_data logo_led_user_data = 
{
    DEVICE_NAME_LOGO_LED,
    GPIOD,
    GPIO_Pin_14,
    GPIO_Mode_AF_PP,
    GPIO_Speed_50MHz,
    RCC_APB2Periph_GPIOD,
    GPIO_Remap_TIM4,
    /* timer base */
    TIM4,
    RCC_APB1Periph_TIM4,
    72000000,
    1895,//38KHZ 1/38000/(1/72000000)
    0,
    TIM_CounterMode_Up,
    /* timer oc */
    TIM_OCMode_PWM2,
    TIM_OutputState_Enable,
    900,// pulse value 50%
    TIM_OCPolarity_High,
    TIM_Channel_3,
    TIM_IT_CC3 | TIM_IT_Update,
    0,
    TIM4_IRQn,
    RT_NULL,
    1,
    0,
    RT_NULL,
    RT_NULL,
    RT_NULL,
};

gpio_device logo_led_device;

int rt_hw_logo_led_register(void)
{
    gpio_device *pwm_device = &logo_led_device;
    struct gpio_pwm_user_data *pwm_user_data = &logo_led_user_data;

    pwm_device->ops = &gpio_pwm_user_ops;
    pwm_user_data->tim_oc_init = TIM_OC3Init;
    pwm_user_data->tim_oc_set_compare = TIM_SetCompare3;
    pwm_user_data->tim_rcc_cmd = RCC_APB1PeriphClockCmd;
    rt_hw_gpio_register(pwm_device,pwm_user_data->name, (RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_PWM_TX | RT_DEVICE_FLAG_INT_TX), pwm_user_data);
    return 0;
}
/* infra pulse device */
struct gpio_pwm_user_data infra_pulse_user_data = 
{
    DEVICE_NAME_INFRA_PULSE,
    GPIOD,
    GPIO_Pin_15,
    GPIO_Mode_AF_PP,
    GPIO_Speed_50MHz,
    RCC_APB2Periph_GPIOD,
    GPIO_Remap_TIM4,
    /* timer base */
    TIM4,
    RCC_APB1Periph_TIM4,
    72000000,
    1895,//38KHZ 1/38000/(1/72000000)
    0,
    TIM_CounterMode_Up,
    /* timer oc */
    TIM_OCMode_PWM2,
    TIM_OutputState_Enable,
    700,// pulse value 50%
    TIM_OCPolarity_High,
    TIM_Channel_4,
    TIM_IT_CC4 | TIM_IT_Update,
    0,
    TIM4_IRQn,
    RT_NULL,
    1,
    0,
    RT_NULL,
    RT_NULL,
    RT_NULL,
};

gpio_device infra_pulse_device;

int rt_hw_infra_pulse_register(void)
{
    gpio_device *pwm_device = &infra_pulse_device;
    struct gpio_pwm_user_data *pwm_user_data = &infra_pulse_user_data;

    pwm_device->ops = &gpio_pwm_user_ops;
    pwm_user_data->tim_oc_init = TIM_OC4Init;
    pwm_user_data->tim_oc_set_compare = TIM_SetCompare4;
    pwm_user_data->tim_rcc_cmd = RCC_APB1PeriphClockCmd;
    rt_hw_gpio_register(pwm_device,pwm_user_data->name, (RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_PWM_TX | RT_DEVICE_FLAG_INT_TX), pwm_user_data);
    return 0;
}

void delay(rt_uint32_t counts)
{
  rt_uint32_t i = 0;
  for (; i < counts; i++)
  {
    
  }
}

/*
#ifdef USE_BUTTON_ADJUST_IR
#include <dfs_posix.h>
#include <rtthread.h>

void set_infra_pulse_value(rt_uint16_t value)
{
  rt_device_t dev;

	infra_pulse_user_data.tim_oc_pulse_value = value;
	infra_pulse_device.parent.user_data = (void *)&infra_pulse_user_data;
	dev = rt_device_find(DEVICE_NAME_INFRA_PULSE);
	if(dev != RT_NULL)
	{
		rt_device_control(dev,RT_DEVICE_CTRL_CONFIG_DEVICE,(void *)&infra_pulse_device);
	}
}

void get_infa_pulse_value(void)
{
	int file_id;

	file_id = open(IR_DUTY_RATIO_FILE_NAME,O_RDONLY,0x777);
	if(file_id > 0)
	{
		read(file_id,&infra_pulse_user_data.tim_oc_pulse_value,sizeof(infra_pulse_user_data.tim_oc_pulse_value));
		set_infra_pulse_value(infra_pulse_user_data.tim_oc_pulse_value);
	}
	close(file_id);
}
void save_infra_pulse_file(void)
{
	int file_id;

	file_id = open(IR_DUTY_RATIO_FILE_NAME,O_RDWR | O_CREAT,0x777);
	if(file_id >= 0)
	{
		write(file_id,&infra_pulse_user_data.tim_oc_pulse_value,sizeof(infra_pulse_user_data.tim_oc_pulse_value));
	}
	close(file_id);
}
void button_adjustIR_deal(void)
{
	static int  adjust = 0;

	adjust += 10;
	
	if(adjust < 1800)
	{
		infra_pulse_user_data.tim_oc_pulse_value = 0;
		infra_pulse_user_data.tim_oc_pulse_value = adjust;
	}
	else if(adjust >= 1800)
	{
		infra_pulse_user_data.tim_oc_pulse_value = 1800;
		infra_pulse_user_data.tim_oc_pulse_value -= adjust-1800;
		if(adjust >= 3600)
		{
			adjust = 0;
		}
	}
	rt_kprintf("infra_pulse = %d adjust = %d\n",infra_pulse_user_data.tim_oc_pulse_value,adjust);
	set_infra_pulse_value(infra_pulse_user_data.tim_oc_pulse_value);
	save_infra_pulse_file();
}
#endif

#ifdef USE_OLD_VOICE_IC
void voice_output(rt_uint16_t counts)
{

	extern rt_sem_t voice_start_sem;
	
	rt_sem_release(voice_start_sem);
	counts++;//avoid warning
}
#else
void voice_output(rt_uint16_t counts ,rt_uint16_t	delay)
{
	VOICE_DATA	voicedata;
	rt_err_t		result;	

	voicedata.cnt = counts;
	voicedata.delay = delay;
  result = rt_mq_send(voice_start_mq,(void *)&voicedata,sizeof(voicedata));
  if(result == -RT_EFULL)
  {
		rt_kprintf("voice mq is full\n");
  }
}
#endif

rt_mutex_t motor_action_mutex = RT_NULL;

void motor_mutex_operate(rt_uint8_t		cmd)
{
	if(motor_action_mutex == RT_NULL)
	{
		rt_kprintf("motor_mutex_operate is NULL!!\n");
		
		return ;
	}
	if(cmd)
	{
		rt_mutex_take(motor_action_mutex,RT_WAITING_FOREVER);
	}
	else
	{
		rt_mutex_release(motor_action_mutex);
	}
}
*/
/*
 * direction:
 * lock -> mt_stat(1) -> mt_a（clockwise）, unlock -> mt_stat(0) -> mt_b(counterclockwise)
 
int8_t lock_output(uint8_t direction)
{
  rt_device_t device_motor_a = RT_NULL;
  rt_device_t device_motor_b = RT_NULL;
  rt_device_t device_motor_status = RT_NULL;
  uint16_t counts = 200;
  uint16_t period = 50;
  uint8_t dat = 0;

  motor_mutex_operate(RT_TRUE);
  device_motor_a = rt_device_find(DEVICE_NAME_MOTOR_A_PULSE);
  device_motor_b = rt_device_find(DEVICE_NAME_MOTOR_B_PULSE);
  device_motor_status = rt_device_find(DEVICE_NAME_MOTOR_STATUS);
  RT_ASSERT(device_motor_a != RT_NULL);
  RT_ASSERT(device_motor_b != RT_NULL);
  RT_ASSERT(device_motor_status != RT_NULL);

  rt_device_read(device_motor_status,0,&dat,0);
  if (dat != direction)
  {
    if (direction == GATE_UNLOCK) //unlock
    {
      #ifdef USE_MOTOR_ADJUST_B
      period = 1;
      #else
      period = 50;
      #endif
      while (counts-- > 0)
      {
      	rt_kprintf("unlock cnt = %d\n",counts);
        rt_device_control(device_motor_b, RT_DEVICE_CTRL_SET_PULSE_COUNTS, &period);
        rt_device_control(device_motor_b, RT_DEVICE_CTRL_SEND_PULSE, (void *)0);
				#ifdef USE_MOTOR_ADJUST_B
        rt_thread_delay(2);
        #else
    	  rt_thread_delay(50);
        #endif
        rt_device_read(device_motor_status,0,&dat,0);
        if (dat == GATE_UNLOCK)
        {
          // Voice prompt unlock success 
          #ifndef	USE_OLD_VOICE_IC
          voice_output(VOICE_UNLOCK_OK,VOICE_UNLOCK_OK_T);
          #endif          
          break;
        }
        #ifdef USE_MOTOR_ADJUST_B
        else if(counts == 0) //adjust value is 0
        #else
        else//none adjust
        #endif
        {
					//motor error
					send_sms_mail(ALARM_TYPE_MOTOR_FAULT, 0);
					send_gprs_mail(ALARM_TYPE_MOTOR_FAULT, 0, 0,RT_NULL);
					motor_mutex_operate(RT_FALSE);
					return -1;
        }
      }
 
    }
    else //lock
    {
      while (counts-- > 0)
      {
      	#ifdef USE_MOTOR_ADJUST_A
        period = 1;
        #else
    	  period = 50;
        #endif
        
        rt_device_control(device_motor_a, RT_DEVICE_CTRL_SET_PULSE_COUNTS, &period);
        rt_device_control(device_motor_a, RT_DEVICE_CTRL_SEND_PULSE, (void *)0);
        #ifdef USE_MOTOR_ADJUST_A
        rt_thread_delay(2);
        #else
    	  rt_thread_delay(50);
        #endif
        rt_device_read(device_motor_status,0,&dat,0);
        if (dat == GATE_LOCK)
        {
          break;
        }
        #ifdef USE_MOTOR_ADJUST_A
        if(counts == 0)
        #else
        else
        #endif
        {
					send_sms_mail(ALARM_TYPE_MOTOR_FAULT, 0);
					send_gprs_mail(ALARM_TYPE_MOTOR_FAULT, 0, 0,RT_NULL);
					motor_mutex_operate(RT_FALSE);
					return -1;
        }
      }
    }
  }
  else
  {
  	rt_device_read(device_motor_status,0,&dat,0);
  	if(dat == GATE_LOCK)
  	{
      rt_kprintf("$$$$Motor STATUS IS LOCK\n");
  	}
  	else if(dat == GATE_UNLOCK)
  	{
      rt_kprintf("$$$$Motor STATUS IS UNLOCK");
  	}
  }
  motor_mutex_operate(RT_FALSE);
  return 0;
}
*/

void TIM3_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();
    /* cc1 isr */
    if (TIM_GetITStatus(motor1_user_data.timx, TIM_IT_CC1) == SET)
    {
        TIM_ClearITPendingBit(motor1_user_data.timx, TIM_IT_CC1);
        if (motor1_user_data.tim_pulse_counts > 0)
        {
            motor1_user_data.tim_pulse_counts--;
        }
    }
    /* cc2 isr */
    if (TIM_GetITStatus(motor2_user_data.timx, TIM_IT_CC2) == SET)
    {
        TIM_ClearITPendingBit(motor2_user_data.timx, TIM_IT_CC2);
        if (motor2_user_data.tim_pulse_counts > 0)
        {
            motor2_user_data.tim_pulse_counts--;
        }
    }
    /* cc3 isr */
    if (TIM_GetITStatus(motor3_user_data.timx, TIM_IT_CC3) == SET)
    {
        TIM_ClearITPendingBit(motor3_user_data.timx, TIM_IT_CC3);
        if (motor3_user_data.tim_pulse_counts > 0)
        {
            motor3_user_data.tim_pulse_counts--;
        }
    }
    /* cc4 isr */
    if (TIM_GetITStatus(motor4_user_data.timx, TIM_IT_CC4) == SET)
    {
        TIM_ClearITPendingBit(motor4_user_data.timx, TIM_IT_CC4);
        if (motor4_user_data.tim_pulse_counts > 0)
        {
            motor4_user_data.tim_pulse_counts--;
        }
    }
    /* tim5 update isr */
    if(TIM_GetITStatus(motor1_user_data.timx, TIM_IT_Update) == SET)
    {
        TIM_ClearITPendingBit(motor1_user_data.timx, TIM_IT_Update);
        if (motor1_user_data.tim_pulse_counts == 0)
        {
            TIM_CCxCmd(motor1_user_data.timx, motor1_user_data.tim_oc_channel, TIM_CCx_Disable);
            if (motor2_user_data.tim_pulse_counts == 0 &&
                motor3_user_data.tim_pulse_counts == 0 &&
                motor4_user_data.tim_pulse_counts == 0)
            {
                TIM_Cmd(motor1_user_data.timx, DISABLE);
            }
        }
        if (motor2_user_data.tim_pulse_counts == 0)
        {
            TIM_CCxCmd(motor2_user_data.timx, motor2_user_data.tim_oc_channel, TIM_CCx_Disable);
            if (motor1_user_data.tim_pulse_counts == 0 &&
                motor3_user_data.tim_pulse_counts == 0 &&
                motor4_user_data.tim_pulse_counts == 0)
            {
                TIM_Cmd(motor2_user_data.timx, DISABLE);
            }
        }
        if (motor3_user_data.tim_pulse_counts == 0)
        {
            TIM_CCxCmd(motor3_user_data.timx, motor3_user_data.tim_oc_channel, TIM_CCx_Disable);
            if (motor1_user_data.tim_pulse_counts == 0 &&
                motor2_user_data.tim_pulse_counts == 0 &&
                motor4_user_data.tim_pulse_counts == 0)
            {
                TIM_Cmd(motor3_user_data.timx, DISABLE);
            }
        }
        if (motor4_user_data.tim_pulse_counts == 0)
        {
            TIM_CCxCmd(motor4_user_data.timx, motor4_user_data.tim_oc_channel, TIM_CCx_Disable);
            if (motor1_user_data.tim_pulse_counts == 0 &&
                motor2_user_data.tim_pulse_counts == 0 &&
                motor3_user_data.tim_pulse_counts == 0)
            {
                TIM_Cmd(motor4_user_data.timx, DISABLE);
            }
        }
    }
    /* leave interrupt */
    rt_interrupt_leave();
}

void TIM4_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();
    /* tim4 cc1 isr */
    if (TIM_GetITStatus(lcd_led_user_data.timx, TIM_IT_CC1) == SET)
    {
        TIM_ClearITPendingBit(lcd_led_user_data.timx, TIM_IT_CC1);
        if (lcd_led_user_data.tim_pulse_counts > 0)
        {
            lcd_led_user_data.tim_pulse_counts--;
        }
    }
    /* tim4 cc2 isr */
    if (TIM_GetITStatus(camera_led_user_data.timx, TIM_IT_CC2) == SET)
    {
        TIM_ClearITPendingBit(camera_led_user_data.timx, TIM_IT_CC2);
        if (camera_led_user_data.tim_pulse_counts > 0)
        {
            camera_led_user_data.tim_pulse_counts--;
        }
    }
    /* tim4 cc3 isr */
    if (TIM_GetITStatus(camera_led_user_data.timx, TIM_IT_CC3) == SET)
    {
        TIM_ClearITPendingBit(logo_led_user_data.timx, TIM_IT_CC3);
        if (logo_led_user_data.tim_pulse_counts > 0)
        {
            logo_led_user_data.tim_pulse_counts--;
        }
    }
    /* tim4 cc4 isr */
    if (TIM_GetITStatus(infra_pulse_user_data.timx, TIM_IT_CC4) == SET)
    {
        TIM_ClearITPendingBit(infra_pulse_user_data.timx, TIM_IT_CC4);
        if (infra_pulse_user_data.tim_pulse_counts > 0)
        {
            infra_pulse_user_data.tim_pulse_counts--;
        }
    }
    /* tim5 update isr */
    if(TIM_GetITStatus(lcd_led_user_data.timx, TIM_IT_Update) == SET)
    {
        TIM_ClearITPendingBit(lcd_led_user_data.timx, TIM_IT_Update);
        if (lcd_led_user_data.tim_pulse_counts == 0)
        {
            TIM_CCxCmd(lcd_led_user_data.timx, lcd_led_user_data.tim_oc_channel, TIM_CCx_Disable);
            if (camera_led_user_data.tim_pulse_counts == 0 &&
                logo_led_user_data.tim_pulse_counts == 0 &&
                infra_pulse_user_data.tim_pulse_counts == 0)
            {
                TIM_Cmd(lcd_led_user_data.timx, DISABLE);
            }
        }
        if (camera_led_user_data.tim_pulse_counts == 0)
        {
            TIM_CCxCmd(camera_led_user_data.timx, camera_led_user_data.tim_oc_channel, TIM_CCx_Disable);
            if (lcd_led_user_data.tim_pulse_counts == 0 &&
                logo_led_user_data.tim_pulse_counts == 0 &&
                infra_pulse_user_data.tim_pulse_counts == 0)
            {
                TIM_Cmd(camera_led_user_data.timx, DISABLE);
            }
        }
        if (logo_led_user_data.tim_pulse_counts == 0)
        {
            TIM_CCxCmd(logo_led_user_data.timx, logo_led_user_data.tim_oc_channel, TIM_CCx_Disable);
            if (camera_led_user_data.tim_pulse_counts == 0 &&
                lcd_led_user_data.tim_pulse_counts == 0 &&
                infra_pulse_user_data.tim_pulse_counts == 0)
            {
                TIM_Cmd(logo_led_user_data.timx, DISABLE);
            }
        }
        if (infra_pulse_user_data.tim_pulse_counts == 0)
        {
            TIM_CCxCmd(infra_pulse_user_data.timx, infra_pulse_user_data.tim_oc_channel, TIM_CCx_Disable);
            if (camera_led_user_data.tim_pulse_counts == 0 &&
              logo_led_user_data.tim_pulse_counts == 0 &&
              lcd_led_user_data.tim_pulse_counts == 0)
            {
                TIM_Cmd(infra_pulse_user_data.timx, DISABLE);
            }
        }
    }
    /* leave interrupt */
    rt_interrupt_leave();

}
INIT_DEVICE_EXPORT(rt_hw_motor1_register);
INIT_DEVICE_EXPORT(rt_hw_motor2_register);
INIT_DEVICE_EXPORT(rt_hw_motor3_register);
INIT_DEVICE_EXPORT(rt_hw_motor4_register);

INIT_DEVICE_EXPORT(rt_hw_lcd_led_register);
INIT_DEVICE_EXPORT(rt_hw_camera_led_register);
INIT_DEVICE_EXPORT(rt_hw_logo_led_register);
INIT_DEVICE_EXPORT(rt_hw_infra_pulse_register);

#ifdef RT_USING_FINSH
#include <finsh.h>
void pwm_set_counts(char *str, rt_uint32_t counts)
{
  rt_device_t device = RT_NULL;
  device = rt_device_find(str);
  if(device != RT_NULL)
  {
    rt_device_control(device, RT_DEVICE_CTRL_SET_PULSE_COUNTS, (void *)&counts);
  }
}
FINSH_FUNCTION_EXPORT(pwm_set_counts, pwm_set_pulse_counts[device_name x])

void pwm_set_value(char *str, rt_uint16_t time)
{
  rt_device_t device = RT_NULL;
  device = rt_device_find(str);
  if(device != RT_NULL)
  {
    rt_device_control(device, RT_DEVICE_CTRL_SET_PULSE_VALUE, (void *)&time);
  }
}
FINSH_FUNCTION_EXPORT(pwm_set_value, pwm_set_pulse_value[device_name x]);

void pwm_send_pulse(char *str)
{
  rt_device_t device = RT_NULL;
  device = rt_device_find(str);
  if(!(device->open_flag & RT_DEVICE_OFLAG_OPEN))
  {
		rt_device_open(device,RT_DEVICE_OFLAG_RDWR);
  }
  if(device != RT_NULL)
  {
    rt_device_control(device, RT_DEVICE_CTRL_SEND_PULSE, (void *)0);
  }
}
FINSH_FUNCTION_EXPORT(pwm_send_pulse, pwm_send_pulse[device_name]);

//FINSH_FUNCTION_EXPORT(voice_output, voice_output[counts]);
/*
void motor_rotate(rt_int16_t data)
{
  if(data < 0)
  {
    data = 0 - data;
    pwm_set_counts(DEVICE_NAME_MOTOR_A_PULSE,data);
    pwm_send_pulse(DEVICE_NAME_MOTOR_A_PULSE);
  }
  else
  {
    pwm_set_counts(DEVICE_NAME_MOTOR_B_PULSE,data);
    pwm_send_pulse(DEVICE_NAME_MOTOR_B_PULSE);
  }
}
FINSH_FUNCTION_EXPORT(motor_rotate, motor_rotate[angle]);
*/
#endif
