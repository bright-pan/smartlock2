/*********************************************************************
 * Filename:      gpio_exti.c
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

#include "gpio_exti.h"
#include "gpio_pin.h"
#include "untils.h"
#include "keyboard.h"
#include "kb_dev.h"

extern rt_device_t rtc_device;

struct gpio_exti_user_data
{
    const char name[RT_NAME_MAX];
    GPIO_TypeDef* gpiox;//port
    rt_uint16_t gpio_pinx;//pin
    GPIOMode_TypeDef gpio_mode;//mode
    GPIOSpeed_TypeDef gpio_speed;//speed
    rt_uint32_t gpio_clock;//clock
    rt_uint8_t gpio_port_source;
    rt_uint8_t gpio_pin_source;
    rt_uint32_t exti_line;//exti_line
    EXTIMode_TypeDef exti_mode;//exti_mode
    EXTITrigger_TypeDef exti_trigger;//exti_trigger
    rt_uint8_t nvic_channel;
    rt_uint8_t nvic_preemption_priority;
    rt_uint8_t nvic_subpriority;
    rt_err_t (*gpio_exti_rx_indicate)(rt_device_t dev, rt_size_t size);//callback function for int
};

/*
 *  GPIO ops function interfaces
 *
 */
static void __gpio_nvic_configure(gpio_device *gpio,FunctionalState new_status)
{
    NVIC_InitTypeDef nvic_init_structure;
    struct gpio_exti_user_data* user = (struct gpio_exti_user_data *)gpio->parent.user_data;

    nvic_init_structure.NVIC_IRQChannel = user->nvic_channel;
    nvic_init_structure.NVIC_IRQChannelPreemptionPriority = user->nvic_preemption_priority;
    nvic_init_structure.NVIC_IRQChannelSubPriority = user->nvic_subpriority;
    nvic_init_structure.NVIC_IRQChannelCmd = new_status;
    NVIC_Init(&nvic_init_structure);
}

static void __gpio_exti_configure(gpio_device *gpio,FunctionalState new_status)
{
    EXTI_InitTypeDef exti_init_structure;
    struct gpio_exti_user_data* user = (struct gpio_exti_user_data *)gpio->parent.user_data;
    EXTI_StructInit(&exti_init_structure);
    exti_init_structure.EXTI_Line = user->exti_line;
    exti_init_structure.EXTI_Mode = user->exti_mode;
    exti_init_structure.EXTI_Trigger = user->exti_trigger;
    exti_init_structure.EXTI_LineCmd = new_status;
    EXTI_Init(&exti_init_structure);
}

static void __gpio_pin_configure(gpio_device *gpio)
{
    GPIO_InitTypeDef gpio_init_structure;
    struct gpio_exti_user_data *user = (struct gpio_exti_user_data*)gpio->parent.user_data;
    GPIO_StructInit(&gpio_init_structure);
    RCC_APB2PeriphClockCmd(user->gpio_clock,ENABLE);
    gpio_init_structure.GPIO_Mode = user->gpio_mode;
    gpio_init_structure.GPIO_Pin = user->gpio_pinx;
    gpio_init_structure.GPIO_Speed = user->gpio_speed;
    GPIO_Init(user->gpiox,&gpio_init_structure);
    GPIO_EXTILineConfig(user->gpio_port_source,user->gpio_pin_source);
}
/*
 * gpio ops configure
 */
rt_err_t gpio_exti_configure(gpio_device *gpio)
{
    __gpio_pin_configure(gpio);
    if(RT_DEVICE_FLAG_INT_RX & gpio->parent.flag)
    {
        __gpio_nvic_configure(gpio,ENABLE);
        __gpio_exti_configure(gpio,ENABLE);
    }

    return RT_EOK;
}
rt_err_t gpio_exti_control(gpio_device *gpio, rt_uint8_t cmd, void *arg)
{
	struct gpio_exti_user_data *user = (struct gpio_exti_user_data*)gpio->parent.user_data;


	switch (cmd)
	{
		case RT_DEVICE_CTRL_MASK_EXTI:
			{
				if (user->exti_mode == EXTI_Mode_Interrupt)
				{
					EXTI->IMR &= ~user->exti_line;
				}
				else
				{
					EXTI->EMR &= ~user->exti_line;
				}
				break;
			}
		case RT_DEVICE_CTRL_UNMASK_EXTI:
			{
				if (user->exti_mode == EXTI_Mode_Interrupt)
				{
					EXTI->IMR |= user->exti_line;
				}
				else
				{
					EXTI->EMR |= user->exti_line;
				}
				break;
			}
	}
	return RT_EOK;
}

void gpio_exti_out(gpio_device *gpio, rt_uint8_t data)
{
	struct gpio_exti_user_data *user = (struct gpio_exti_user_data*)gpio->parent.user_data;

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
		rt_kprintf("gpio exti device <%s> is can`t write! please check device flag RT_DEVICE_FLAG_WRONLY\n", user->name);
#endif
	}
}

rt_uint8_t gpio_exti_intput(gpio_device *gpio)
{
	struct gpio_exti_user_data* user = (struct gpio_exti_user_data *)gpio->parent.user_data;

	if (gpio->parent.flag & RT_DEVICE_FLAG_RDONLY)
	{
		return GPIO_ReadInputDataBit(user->gpiox,user->gpio_pinx);
	}
	else
	{
#ifdef RT_USING_FINSH
		rt_kprintf("gpio exti device <%s> is can`t read! please check device flag RT_DEVICE_FLAG_RDONLY\n", user->name);
#endif
		return 0;
	}
}

struct rt_gpio_ops gpio_exti_user_ops=
{
	gpio_exti_configure,
	gpio_exti_control,
	gpio_exti_out,
	gpio_exti_intput
};

/*******************************************************
 *
 *             gpio exti device register
 *
 *******************************************************/

/* switch1 device */
gpio_device switch1_device;
rt_timer_t switch1_exti_timer = RT_NULL;

void switch1_exti_timeout(void *parameters)
{
	rt_device_t device = RT_NULL;
	uint8_t data;

	device = rt_device_find(DEVICE_NAME_SWITCH1);
	if (device != RT_NULL)
	{
		rt_device_read(device,0,&data,0);
		if (data == SWITCH1_STATUS)
		{
            rt_kprintf("it is switch1 detect!\n");
			// produce mail
			send_alarm_mail(ALARM_TYPE_SWITCH1, ALARM_PROCESS_FLAG_LOCAL, SWITCH1_STATUS, 0);
		}
		rt_device_control(device, RT_DEVICE_CTRL_UNMASK_EXTI, (void *)0);
	}

	rt_timer_stop(switch1_exti_timer);
}


rt_err_t switch1_rx_ind(rt_device_t dev, rt_size_t size)
{
	//gpio_device *gpio = RT_NULL;
	//gpio = (gpio_device *)dev;
	rt_device_t device = RT_NULL;

	device = rt_device_find(DEVICE_NAME_SWITCH1);
	RT_ASSERT(device != RT_NULL);
	rt_device_control(device, RT_DEVICE_CTRL_MASK_EXTI, (void *)0);
	rt_timer_start(switch1_exti_timer);

	return RT_EOK;
}

struct gpio_exti_user_data switch1_user_data =
{
	DEVICE_NAME_SWITCH1,
	GPIOE,
	GPIO_Pin_7,
	GPIO_Mode_IPU,
	GPIO_Speed_50MHz,
	RCC_APB2Periph_GPIOE |RCC_APB2Periph_AFIO,
	GPIO_PortSourceGPIOE,
	GPIO_PinSource7,
	EXTI_Line7,
	EXTI_Mode_Interrupt,
	SWITCH1_EXTI_TRIGGER_MODE,
	EXTI9_5_IRQn,
	1,
	5,
	switch1_rx_ind,
};

int rt_hw_switch1_register(void)
{
    gpio_device *gpio_device = &switch1_device;
    struct gpio_exti_user_data *gpio_user_data = &switch1_user_data;

    gpio_device->ops = &gpio_exti_user_ops;

    rt_hw_gpio_register(gpio_device, gpio_user_data->name, (RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX), gpio_user_data);
    switch1_exti_timer = rt_timer_create("t_sw1",
										 switch1_exti_timeout,
										 RT_NULL,
										 SWITCH1_INT_INTERVAL,
										 RT_TIMER_FLAG_ONE_SHOT | RT_TIMER_FLAG_SOFT_TIMER);
    rt_device_set_rx_indicate((rt_device_t)gpio_device, gpio_user_data->gpio_exti_rx_indicate);

    return 0;
}

/* switch2 device */
gpio_device switch2_device;
rt_timer_t switch2_exti_timer = RT_NULL;

void switch2_exti_timeout(void *parameters)
{

	//time_t time;

	rt_device_t device = RT_NULL;
	uint8_t data;

	device = rt_device_find(DEVICE_NAME_SWITCH2);
	if (device != RT_NULL)
	{
		rt_device_read(device,0,&data,0);
		if (data == SWITCH2_STATUS) // rfid key is plugin
		{
            rt_kprintf("it is key2 detect!\n");
			// produce mail
			//rt_device_control(rtc_device, RT_DEVICE_CTRL_RTC_GET_TIME, &time);

			// send mail
			//send_alarm_mail(ALARM_TYPE_RFID_switch2, ALARM_PROCESS_FLAG_LOCAL, RFID_switch2_STATUS, time);
		}
		rt_device_control(device, RT_DEVICE_CTRL_UNMASK_EXTI, (void *)0);
	}

	rt_timer_stop(switch2_exti_timer);
}


rt_err_t switch2_rx_ind(rt_device_t dev, rt_size_t size)
{
	//gpio_device *gpio = RT_NULL;
	//gpio = (gpio_device *)dev;
	rt_device_t device = RT_NULL;

	device = rt_device_find(DEVICE_NAME_SWITCH2);
	RT_ASSERT(device != RT_NULL);
	rt_device_control(device, RT_DEVICE_CTRL_MASK_EXTI, (void *)0);
	rt_timer_start(switch2_exti_timer);

	return RT_EOK;
}

struct gpio_exti_user_data switch2_user_data =
{
    DEVICE_NAME_SWITCH2,
    GPIOE,
    GPIO_Pin_8,
    GPIO_Mode_IPU,
    GPIO_Speed_50MHz,
    RCC_APB2Periph_GPIOE |RCC_APB2Periph_AFIO,
    GPIO_PortSourceGPIOE,
    GPIO_PinSource8,
    EXTI_Line8,
    EXTI_Mode_Interrupt,
    SWITCH2_EXTI_TRIGGER_MODE,
    EXTI9_5_IRQn,
    1,
    5,
    switch2_rx_ind,
};

int rt_hw_switch2_register(void)
{
    gpio_device *gpio_device = &switch2_device;
    struct gpio_exti_user_data *gpio_user_data = &switch2_user_data;

    gpio_device->ops = &gpio_exti_user_ops;

    rt_hw_gpio_register(gpio_device, gpio_user_data->name, (RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX), gpio_user_data);
    switch2_exti_timer = rt_timer_create("t_sw2",
										 switch2_exti_timeout,
										 RT_NULL,
										 SWITCH2_INT_INTERVAL,
										 RT_TIMER_FLAG_ONE_SHOT | RT_TIMER_FLAG_SOFT_TIMER);
    rt_device_set_rx_indicate((rt_device_t)gpio_device, gpio_user_data->gpio_exti_rx_indicate);

    return 0;
}

/* switch3 device */
gpio_device switch3_device;
rt_timer_t switch3_exti_timer = RT_NULL;

void switch3_exti_timeout(void *parameters)
{

	//time_t time;

	rt_device_t device = RT_NULL;
	uint8_t data;

	device = rt_device_find(DEVICE_NAME_SWITCH3);
	if (device != RT_NULL)
	{
		rt_device_read(device,0,&data,0);
		if (data == SWITCH3_STATUS) // rfid key is plugin
		{
            rt_kprintf("it is key3 detect!\n");
			// produce mail
			//rt_device_control(rtc_device, RT_DEVICE_CTRL_RTC_GET_TIME, &time);

			// send mail
			//send_alarm_mail(ALARM_TYPE_RFID_switch2, ALARM_PROCESS_FLAG_LOCAL, RFID_switch2_STATUS, time);
		}
		rt_device_control(device, RT_DEVICE_CTRL_UNMASK_EXTI, (void *)0);
	}

	rt_timer_stop(switch3_exti_timer);
}


rt_err_t switch3_rx_ind(rt_device_t dev, rt_size_t size)
{
	//gpio_device *gpio = RT_NULL;
	//gpio = (gpio_device *)dev;
	rt_device_t device = RT_NULL;

	device = rt_device_find(DEVICE_NAME_SWITCH3);
	RT_ASSERT(device != RT_NULL);
	rt_device_control(device, RT_DEVICE_CTRL_MASK_EXTI, (void *)0);
	rt_timer_start(switch3_exti_timer);

	return RT_EOK;
}

struct gpio_exti_user_data switch3_user_data =
{
	DEVICE_NAME_SWITCH3,
	GPIOE,
	GPIO_Pin_9,
	GPIO_Mode_IPU,
	GPIO_Speed_50MHz,
	RCC_APB2Periph_GPIOE |RCC_APB2Periph_AFIO,
	GPIO_PortSourceGPIOE,
	GPIO_PinSource9,
	EXTI_Line9,
	EXTI_Mode_Interrupt,
	SWITCH3_EXTI_TRIGGER_MODE,
	EXTI9_5_IRQn,
	1,
	5,
	switch3_rx_ind,
};

int rt_hw_switch3_register(void)
{
    gpio_device *gpio_device = &switch3_device;
    struct gpio_exti_user_data *gpio_user_data = &switch3_user_data;

    gpio_device->ops = &gpio_exti_user_ops;

    rt_hw_gpio_register(gpio_device, gpio_user_data->name, (RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX), gpio_user_data);
    switch3_exti_timer = rt_timer_create("t_sw3",
										 switch3_exti_timeout,
										 RT_NULL,
										 SWITCH3_INT_INTERVAL,
										 RT_TIMER_FLAG_ONE_SHOT | RT_TIMER_FLAG_SOFT_TIMER);
    rt_device_set_rx_indicate((rt_device_t)gpio_device, gpio_user_data->gpio_exti_rx_indicate);

    return 0;
}

/* key device */
gpio_device key_device;
rt_timer_t key_exti_timer = RT_NULL;

__STATIC_INLINE uint8_t bit_to_index(uint16_t data)
{
    uint8_t result = 0;
    while (data)
    {
        result++;
        data >>= 1;
    }
    return result;
}

static const uint8_t char_remap[16] = {
    '?',
    '*', '0', '#',
    '7', '8', '9',
    '4', '5', '6',
    '1', '2', '3',
    '?', '?', '?',
};

void key_exti_timeout(void *parameters)
{
	rt_device_t device = RT_NULL;
	rt_device_t i2c_device = RT_NULL;
	uint16_t data = 0;
    static uint8_t error_detect = 0;
    rt_size_t size;
    uint8_t c;

	device = device_enable(DEVICE_NAME_KEY);
    i2c_device = device_enable(DEVICE_NAME_KEYBOARD);

	if (device != RT_NULL)
	{
		rt_device_read(device,0,&data,0);
		if (data == KEY_STATUS) // rfid key is plugin
		{
			data = 0;
			size = rt_device_read(i2c_device, 0, &data, 2);
			if (size == 2) {
				error_detect = 0;
				// filter keyboard input
				if (data != 0x0100) {
					data &= 0xfeff;
				}
				__REV16(data);
				c = char_remap[bit_to_index(data&0x0fff)];
#if (defined RT_USING_FINSH) && (defined KB_DEBUG)
				rt_kprintf("keyboard value is %04X, %c\n", data, c);
#endif
				// send mail
				send_kb_mail(KB_MAIL_TYPE_INPUT, KB_MODE_NORMAL_AUTH, c);
            } else {
#if (defined RT_USING_FINSH) && (defined KB_DEBUG)
                rt_kprintf("read key board failure!!!\n");
#endif
                if (error_detect++ > 2)
                {
                    rt_device_control(i2c_device, RT_DEVICE_CTRL_CONFIGURE, RT_NULL);
                    error_detect = 0;
                }
			}
			//send_alarm_mail(ALARM_TYPE_RFID_key, ALARM_PROCESS_FLAG_LOCAL, RFID_key_STATUS, time);
		}
		rt_device_control(device, RT_DEVICE_CTRL_UNMASK_EXTI, (void *)0);
	}

	rt_timer_stop(key_exti_timer);
}


rt_err_t key_rx_ind(rt_device_t dev, rt_size_t size)
{
	//gpio_device *gpio = RT_NULL;
	//gpio = (gpio_device *)dev;
	rt_device_t device = RT_NULL;

	device = rt_device_find(DEVICE_NAME_KEY);
	RT_ASSERT(device != RT_NULL);
	rt_device_control(device, RT_DEVICE_CTRL_MASK_EXTI, (void *)0);
	rt_timer_start(key_exti_timer);

	return RT_EOK;
}

struct gpio_exti_user_data key_user_data =
{
	DEVICE_NAME_KEY,
	GPIOE,
	GPIO_Pin_15,
	GPIO_Mode_IN_FLOATING,
	GPIO_Speed_50MHz,
	RCC_APB2Periph_GPIOE |RCC_APB2Periph_AFIO,
	GPIO_PortSourceGPIOE,
	GPIO_PinSource15,
	EXTI_Line15,
	EXTI_Mode_Interrupt,
	KEY_EXTI_TRIGGER_MODE,
	EXTI15_10_IRQn,
	1,
	5,
	key_rx_ind,
};

int rt_hw_key_register(void)
{
    gpio_device *gpio_device = &key_device;
    struct gpio_exti_user_data *gpio_user_data = &key_user_data;

    gpio_device->ops = &gpio_exti_user_ops;

    rt_hw_gpio_register(gpio_device, gpio_user_data->name, (RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX), gpio_user_data);
    key_exti_timer = rt_timer_create("t_key",
									 key_exti_timeout,
									 RT_NULL,
									 KEY_INT_INTERVAL,
									 RT_TIMER_FLAG_ONE_SHOT | RT_TIMER_FLAG_SOFT_TIMER);
    rt_device_set_rx_indicate((rt_device_t)gpio_device, gpio_user_data->gpio_exti_rx_indicate);

    return 0;
}
/* motor_status device pd8 */
/*rt_err_t motor_status_rx_ind(rt_device_t dev, rt_size_t size)
  {
  gpio_device *gpio = RT_NULL;
  time_t time;

  RT_ASSERT(dev != RT_NULL);
  gpio = (gpio_device *)dev; */
/* produce mail */
//rt_device_control(rtc_device, RT_DEVICE_CTRL_RTC_GET_TIME, &time);
/* send mail */
/*  send_alarm_mail(ALARM_TYPE_MOTOR_STATUS, ALARM_PROCESS_FLAG_LOCAL, gpio->pin_value, time);

	return RT_EOK;
	}*/
/*
  gpio_device motor_status_device;

  struct gpio_exti_user_data motor_status_user_data =
  {
  DEVICE_NAME_MOTOR_STATUS,
  GPIOD,
  GPIO_Pin_8,
  GPIO_Mode_IN_FLOATING,
  GPIO_Speed_50MHz,
  RCC_APB2Periph_GPIOD |RCC_APB2Periph_AFIO,
  GPIO_PortSourceGPIOD,
  GPIO_PinSource8,
  EXTI_Line8,
  EXTI_Mode_Interrupt,
  EXTI_Trigger_Rising,
  EXTI9_5_IRQn,
  1,
  5,
  motor_status_rx_ind,
  };

  void rt_hw_motor_status_register(void)
  {
  gpio_device *gpio_device = &motor_status_device;
  struct gpio_exti_user_data *gpio_user_data = &motor_status_user_data;

  gpio_device->ops = &gpio_exti_user_ops;

  rt_hw_gpio_register(gpio_device, gpio_user_data->name, (RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX), gpio_user_data);
  rt_device_set_rx_indicate((rt_device_t)gpio_device, gpio_user_data->gpio_exti_rx_indicate);
  }*/
/* camera_photosensor device pb0 */
/*rt_err_t camera_photosensor_rx_ind(rt_device_t dev, rt_size_t size)
  {
  gpio_device *gpio = RT_NULL;
  time_t time;

  RT_ASSERT(dev != RT_NULL);
  gpio = (gpio_device *)dev; */
/* produce mail */
//rt_device_control(rtc_device, RT_DEVICE_CTRL_RTC_GET_TIME, &time);
/* send mail */
/*send_alarm_mail(ALARM_TYPE_CAMERA_PHOTOSENSOR, ALARM_PROCESS_FLAG_LOCAL, gpio->pin_value, time);

  return RT_EOK;
  }*/

/*gpio_device camera_photosensor_device;

  struct gpio_exti_user_data camera_photosensor_user_data =
  {
  DEVICE_NAME_CAMERA_PHOTOSENSOR,
  GPIOB,
  GPIO_Pin_0,
  GPIO_Mode_IN_FLOATING,
  GPIO_Speed_50MHz,
  RCC_APB2Periph_GPIOB |RCC_APB2Periph_AFIO,
  GPIO_PortSourceGPIOB,
  GPIO_PinSource0,
  EXTI_Line0,
  EXTI_Mode_Interrupt,
  EXTI_Trigger_Rising,
  EXTI0_IRQn,
  1,
  5,
  camera_photosensor_rx_ind,
  };

  void rt_hw_camera_photosensor_register(void)
  {
  gpio_device *gpio_device = &camera_photosensor_device;
  struct gpio_exti_user_data *gpio_user_data = &camera_photosensor_user_data;

  gpio_device->ops = &gpio_exti_user_ops;

  rt_hw_gpio_register(gpio_device, gpio_user_data->name, (RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX), gpio_user_data);
  rt_device_set_rx_indicate((rt_device_t)gpio_device, gpio_user_data->gpio_exti_rx_indicate);
  }*/
//#define CM_IR_SAMPLE_TIME               0
///* camera_irdasensor device pb0 */
//rt_err_t camera_irdasensor_rx_ind(rt_device_t dev, rt_size_t size)
//{
//  //gpio_device *gpio = RT_NULL;
//  time_t time;
//  static time_t time_old = 0;
//  extern rt_sem_t cm_ir_sem;

//  RT_ASSERT(dev != RT_NULL);
//  //gpio = (gpio_device *)dev;
//  /* produce mail */
//  rt_device_control(rtc_device, RT_DEVICE_CTRL_RTC_GET_TIME, &time);
//
//  if ((time - time_old) > CM_IR_SAMPLE_TIME || (0 == time_old))//0 == time_old open electrify power
//  {
//     rt_kprintf("&&time_old = %d time = %d\n",time_old,time);
//     time_old = time;
//  }
//  else
//  {
//    rt_kprintf("time_old = %d time = %d\n",time_old,time);
//  	//time_old = time;
//    return RT_EOK;
//  }
//  /* send mail ir deal thread*/
//	rt_sem_release(cm_ir_sem);
//
//	//send_alarm_mail(ALARM_TYPE_CAMERA_IRDASENSOR, ALARM_PROCESS_FLAG_SMS | ALARM_PROCESS_FLAG_GPRS | ALARM_PROCESS_FLAG_LOCAL, gpio->pin_value, time);

//  return RT_EOK;
//}

//gpio_device camera_irdasensor_device;

//struct gpio_exti_user_data camera_irdasensor_user_data =
//{
//  DEVICE_NAME_CAMERA_IRDASENSOR,
//  GPIOB,
//  GPIO_Pin_1,
//  GPIO_Mode_IPD,
//  GPIO_Speed_50MHz,
//  RCC_APB2Periph_GPIOB |RCC_APB2Periph_AFIO,
//  GPIO_PortSourceGPIOB,
//  GPIO_PinSource1,
//  EXTI_Line1,
//  EXTI_Mode_Interrupt,
//  EXTI_Trigger_Rising,
//  EXTI1_IRQn,
//  1,
//  5,
//  camera_irdasensor_rx_ind,
//};

//void rt_hw_camera_irdasensor_register(void)
//{
//  gpio_device *gpio_device = &camera_irdasensor_device;
//  struct gpio_exti_user_data *gpio_user_data = &camera_irdasensor_user_data;

//  gpio_device->ops = &gpio_exti_user_ops;
//
//  rt_hw_gpio_register(gpio_device, gpio_user_data->name, (RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX), gpio_user_data);
//  rt_device_set_rx_indicate((rt_device_t)gpio_device, gpio_user_data->gpio_exti_rx_indicate);
//}

/* gsm_ring device
   gpio_device gsm_ring_device;
   rt_timer_t gsm_ring_exti_timer = RT_NULL;

   void gsm_ring_exti_timeout(void *parameters)
   {
   time_t time;

   rt_device_t device = RT_NULL;
   rt_device_t g_stat_dev = RT_NULL;
   uint8_t data,gstat;

   device = rt_device_find(DEVICE_NAME_GSM_RING);
   g_stat_dev = rt_device_find(DEVICE_NAME_GSM_STATUS);
   if (device != RT_NULL)
   {
   rt_device_read(device,0,&data,0);
   rt_device_read(g_stat_dev,0,&gstat,1);
   if (data == GSM_RING_DETECT_STATUS && gstat != 0) // gsm ring
   {
   // produce mail
   RT_ASSERT(rtc_device != RT_NULL);
   rt_device_control(rtc_device, RT_DEVICE_CTRL_RTC_GET_TIME, &time);

   // send mail
   send_alarm_mail(ALARM_TYPE_GSM_RING, ALARM_PROCESS_FLAG_LOCAL, GSM_RING_DETECT_STATUS, time);

   // device is none activate
   if(!device_parameters.device_statu)
   {
   device_activate_deal(RT_TRUE,0,0);
   }
   }
   else
   {
   RT_DEBUG_LOG(PRINTF_RING_HW_CASE,("ring error Ring_pin = %d GSM_stat = %d\n",data,gstat));
   }
   rt_device_control(device, RT_DEVICE_CTRL_UNMASK_EXTI, (void *)0);
   }

   rt_timer_stop(gsm_ring_exti_timer);
   }

   rt_err_t gsm_ring_rx_ind(rt_device_t dev, rt_size_t size)
   {
   rt_device_t device = RT_NULL;

   device = rt_device_find(DEVICE_NAME_GSM_RING);
   RT_ASSERT(device != RT_NULL);
   rt_device_control(device, RT_DEVICE_CTRL_MASK_EXTI, (void *)0);
   rt_timer_start(gsm_ring_exti_timer);
   return RT_EOK;
   }

   struct gpio_exti_user_data gsm_ring_user_data =
   {
   DEVICE_NAME_GSM_RING,
   GPIOD,
   GPIO_Pin_13,
   GPIO_Mode_IN_FLOATING,
   GPIO_Speed_50MHz,
   RCC_APB2Periph_GPIOD |RCC_APB2Periph_AFIO,
   GPIO_PortSourceGPIOD,
   GPIO_PinSource13,
   EXTI_Line13,
   EXTI_Mode_Interrupt,
   GSM_RING_EXTI_TRIGGER_MODE,
   EXTI15_10_IRQn,
   1,
   5,
   gsm_ring_rx_ind,
   };

   void rt_hw_gsm_ring_register(void)
   {
   gpio_device *gpio_device = &gsm_ring_device;
   struct gpio_exti_user_data *gpio_user_data = &gsm_ring_user_data;

   gpio_device->ops = &gpio_exti_user_ops;
   rt_hw_gpio_register(gpio_device, gpio_user_data->name, (RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX), gpio_user_data);
   gsm_ring_exti_timer = rt_timer_create("g_ring",
   gsm_ring_exti_timeout,
   RT_NULL,
   GSM_RING_INT_INTERVAL,
   RT_TIMER_FLAG_ONE_SHOT);
   rt_device_set_rx_indicate((rt_device_t)gpio_device, gpio_user_data->gpio_exti_rx_indicate);
   }
*/
/* lock_gate device
   gpio_device lock_gate_device;

   rt_timer_t lock_gate_exti_timer = RT_NULL;

   void lock_gate_exti_timeout(void *parameters)
   {
   time_t time;

   rt_device_t device = RT_NULL;
   uint8_t data;

   device = rt_device_find(DEVICE_NAME_LOCK_GATE);
   if (device != RT_NULL)
   {
   rt_device_read(device,0,&data,0);
   if (data == LOCK_GATE_DETECT_STATUS) // lock gate
   {
   // produce mail
   RT_ASSERT(rtc_device != RT_NULL);
   rt_device_control(rtc_device, RT_DEVICE_CTRL_RTC_GET_TIME, &time);

   // send mail
   send_alarm_mail(ALARM_TYPE_LOCK_GATE, ALARM_PROCESS_FLAG_LOCAL, LOCK_GATE_DETECT_STATUS, time);
   }
   rt_device_control(device, RT_DEVICE_CTRL_UNMASK_EXTI, (void *)0);
   }

   rt_timer_stop(lock_gate_exti_timer);
   }

   rt_err_t lock_gate_rx_ind(rt_device_t dev, rt_size_t size)
   {
   rt_device_t device = RT_NULL;

   device = rt_device_find(DEVICE_NAME_LOCK_GATE);
   RT_ASSERT(device != RT_NULL);
   rt_device_control(device, RT_DEVICE_CTRL_MASK_EXTI, (void *)0);
   rt_timer_start(lock_gate_exti_timer);
   return RT_EOK;
   }

   struct gpio_exti_user_data lock_gate_user_data =
   {
   DEVICE_NAME_LOCK_GATE,
   GPIOD,
   GPIO_Pin_12,
   GPIO_Mode_IN_FLOATING,
   GPIO_Speed_50MHz,
   RCC_APB2Periph_GPIOD |RCC_APB2Periph_AFIO,
   GPIO_PortSourceGPIOD,
   GPIO_PinSource12,
   EXTI_Line12,
   EXTI_Mode_Interrupt,
   LOCK_GATE_EXTI_TRIGGER_MODE,
   EXTI15_10_IRQn,
   1,
   5,
   lock_gate_rx_ind,
   };

   void rt_hw_lock_gate_register(void)
   {
   gpio_device *gpio_device = &lock_gate_device;
   struct gpio_exti_user_data *gpio_user_data = &lock_gate_user_data;

   gpio_device->ops = &gpio_exti_user_ops;

   rt_hw_gpio_register(gpio_device, gpio_user_data->name, (RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX), gpio_user_data);
   lock_gate_exti_timer = rt_timer_create("lk_gate",
   lock_gate_exti_timeout,
   RT_NULL,
   LOCK_GATE_INT_INTERVAL,
   RT_TIMER_FLAG_ONE_SHOT);
   rt_device_set_rx_indicate((rt_device_t)gpio_device, gpio_user_data->gpio_exti_rx_indicate);
   }
*/

/* lock_shell device
   gpio_device lock_shell_device;

   rt_timer_t lock_shell_exti_timer = RT_NULL;

   void lock_shell_exti_timeout(void *parameters)
   {
   time_t time;

   rt_device_t device = RT_NULL;
   uint8_t data;

   device = rt_device_find(DEVICE_NAME_LOCK_SHELL);
   if (device != RT_NULL)
   {
   rt_device_read(device,0,&data,0);
   if (data == LOCK_SHELL_DETECT_STATUS) // lock gate
   {
   // produce mail
   RT_ASSERT(rtc_device != RT_NULL);
   rt_device_control(rtc_device, RT_DEVICE_CTRL_RTC_GET_TIME, &time);

   // send mail
   send_alarm_mail(ALARM_TYPE_LOCK_SHELL, ALARM_PROCESS_FLAG_SMS | ALARM_PROCESS_FLAG_GPRS | ALARM_PROCESS_FLAG_LOCAL, LOCK_SHELL_DETECT_STATUS, time);
   }
   rt_device_control(device, RT_DEVICE_CTRL_UNMASK_EXTI, (void *)0);
   }

   rt_timer_stop(lock_shell_exti_timer);
   }

   rt_err_t lock_shell_rx_ind(rt_device_t dev, rt_size_t size)
   {
   rt_device_t device = RT_NULL;

   device = rt_device_find(DEVICE_NAME_LOCK_SHELL);
   RT_ASSERT(device != RT_NULL);
   rt_device_control(device, RT_DEVICE_CTRL_MASK_EXTI, (void *)0);
   rt_timer_start(lock_shell_exti_timer);
   return RT_EOK;
   }

   struct gpio_exti_user_data lock_shell_user_data =
   {
   DEVICE_NAME_LOCK_SHELL,
   GPIOD,
   GPIO_Pin_11,
   GPIO_Mode_IN_FLOATING,
   GPIO_Speed_50MHz,
   RCC_APB2Periph_GPIOD |RCC_APB2Periph_AFIO,
   GPIO_PortSourceGPIOD,
   GPIO_PinSource11,
   EXTI_Line11,
   EXTI_Mode_Interrupt,
   LOCK_SHELL_EXTI_TRIGGER_MODE,
   EXTI15_10_IRQn,
   1,
   5,
   lock_shell_rx_ind,
   };

   void rt_hw_lock_shell_register(void)
   {
   gpio_device *gpio_device = &lock_shell_device;
   struct gpio_exti_user_data *gpio_user_data = &lock_shell_user_data;

   gpio_device->ops = &gpio_exti_user_ops;

   rt_hw_gpio_register(gpio_device, gpio_user_data->name, (RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX), gpio_user_data);
   lock_shell_exti_timer = rt_timer_create("lk_shell",
   lock_shell_exti_timeout,
   RT_NULL,
   LOCK_SHELL_INT_INTERVAL,
   RT_TIMER_FLAG_ONE_SHOT);
   rt_device_set_rx_indicate((rt_device_t)gpio_device, gpio_user_data->gpio_exti_rx_indicate);
   }
*/
/* gate temperature device register
   gpio_device gate_temperature_device;

   rt_timer_t gate_temperature_exti_timer = RT_NULL;

   void gate_temperature_exti_timeout(void *parameters)
   {
   time_t time;

   rt_device_t device = RT_NULL;
   uint8_t data;

   device = rt_device_find(DEVICE_NAME_GATE_TEMPERATURE);
   if (device != RT_NULL)
   {
   rt_device_read(device,0,&data,0);
   if (data == GATE_TEMPERATURE_DETECT_STATUS) // lock gate
   {
   // produce mail
   RT_ASSERT(rtc_device != RT_NULL);
   rt_device_control(rtc_device, RT_DEVICE_CTRL_RTC_GET_TIME, &time);

   // send mail
   send_alarm_mail(ALARM_TYPE_GATE_TEMPERATURE, ALARM_PROCESS_FLAG_SMS | ALARM_PROCESS_FLAG_LOCAL, GATE_TEMPERATURE_DETECT_STATUS, time);
   }
   rt_device_control(device, RT_DEVICE_CTRL_UNMASK_EXTI, (void *)0);
   }

   rt_timer_stop(gate_temperature_exti_timer);
   }

   rt_err_t gate_temperature_rx_ind(rt_device_t dev, rt_size_t size)
   {
   rt_device_t device = RT_NULL;

   device = rt_device_find(DEVICE_NAME_GATE_TEMPERATURE);
   RT_ASSERT(device != RT_NULL);
   rt_device_control(device, RT_DEVICE_CTRL_MASK_EXTI, (void *)0);
   rt_timer_start(gate_temperature_exti_timer);
   return RT_EOK;
   }

   struct gpio_exti_user_data gate_temperature_user_data =
   {
   DEVICE_NAME_GATE_TEMPERATURE,
   GPIOD,
   GPIO_Pin_14,
   GPIO_Mode_IN_FLOATING,
   GPIO_Speed_50MHz,
   RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO,
   GPIO_PortSourceGPIOD,
   GPIO_PinSource14,
   EXTI_Line14,
   EXTI_Mode_Interrupt,
   GATE_TEMPERATURE_EXTI_TRIGGER_MODE,
   EXTI15_10_IRQn,
   1,
   5,
   gate_temperature_rx_ind,
   };

   void rt_hw_gate_temperature_register(void)
   {
   gpio_device *gpio_device = &gate_temperature_device;
   struct gpio_exti_user_data *gpio_user_data = &gate_temperature_user_data;

   gpio_device->ops = &gpio_exti_user_ops;

   rt_hw_gpio_register(gpio_device, gpio_user_data->name, (RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX), gpio_user_data);
   gate_temperature_exti_timer = rt_timer_create("gt_temp",
   gate_temperature_exti_timeout,
   RT_NULL,
   GATE_TEMPERATURE_INT_INTERVAL,
   RT_TIMER_FLAG_ONE_SHOT);
   rt_device_set_rx_indicate((rt_device_t)gpio_device, gpio_user_data->gpio_exti_rx_indicate);
   }
*/
/* lock temperature device register
   gpio_device lock_temperature_device;

   rt_timer_t lock_temperature_exti_timer = RT_NULL;

   void lock_temperature_exti_timeout(void *parameters)
   {
   time_t time;

   rt_device_t device = RT_NULL;
   uint8_t data;

   device = rt_device_find(DEVICE_NAME_LOCK_TEMPERATURE);
   if (device != RT_NULL)
   {
   rt_device_read(device,0,&data,0);
   if (data == LOCK_TEMPERATURE_DETECT_STATUS) // lock gate
   {
   // produce mail
   RT_ASSERT(rtc_device != RT_NULL);
   rt_device_control(rtc_device, RT_DEVICE_CTRL_RTC_GET_TIME, &time);

   // send mail
   send_alarm_mail(ALARM_TYPE_LOCK_TEMPERATURE, ALARM_PROCESS_FLAG_SMS | ALARM_PROCESS_FLAG_GPRS | ALARM_PROCESS_FLAG_LOCAL, LOCK_TEMPERATURE_DETECT_STATUS, time);
   }
   rt_device_control(device, RT_DEVICE_CTRL_UNMASK_EXTI, (void *)0);
   }

   rt_timer_stop(lock_temperature_exti_timer);
   }

   rt_err_t lock_temperature_rx_ind(rt_device_t dev, rt_size_t size)
   {
   rt_device_t device = RT_NULL;

   device = rt_device_find(DEVICE_NAME_LOCK_TEMPERATURE);
   RT_ASSERT(device != RT_NULL);
   rt_device_control(device, RT_DEVICE_CTRL_MASK_EXTI, (void *)0);
   rt_timer_start(lock_temperature_exti_timer);
   return RT_EOK;
   }

   struct gpio_exti_user_data lock_temperature_user_data =
   {
   DEVICE_NAME_LOCK_TEMPERATURE,
   GPIOD,
   GPIO_Pin_15,
   GPIO_Mode_IN_FLOATING,
   GPIO_Speed_50MHz,
   RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO,
   GPIO_PortSourceGPIOD,
   GPIO_PinSource15,
   EXTI_Line15,
   EXTI_Mode_Interrupt,
   LOCK_TEMPERATURE_EXTI_TRIGGER_MODE,
   EXTI15_10_IRQn,
   1,
   5,
   lock_temperature_rx_ind,
   };

   void rt_hw_lock_temperature_register(void)
   {
   gpio_device *gpio_device = &lock_temperature_device;
   struct gpio_exti_user_data *gpio_user_data = &lock_temperature_user_data;

   gpio_device->ops = &gpio_exti_user_ops;

   rt_hw_gpio_register(gpio_device, gpio_user_data->name, (RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX), gpio_user_data);
   lock_temperature_exti_timer = rt_timer_create("lk_temp",
   lock_temperature_exti_timeout,
   RT_NULL,
   LOCK_TEMPERATURE_INT_INTERVAL,
   RT_TIMER_FLAG_ONE_SHOT);
   rt_device_set_rx_indicate((rt_device_t)gpio_device, gpio_user_data->gpio_exti_rx_indicate);
   }
*/
/* battery switch device register
   gpio_device battery_switch_device;

   rt_timer_t battery_switch_exti_timer = RT_NULL;

   void battery_switch_exti_timeout(void *parameters)
   {
   time_t time;
   rt_device_t device = RT_NULL;

   device = rt_device_find(DEVICE_NAME_BATTERY_SWITCH);
   if (device != RT_NULL)
   {
   // produce mail
   RT_ASSERT(rtc_device != RT_NULL);
   rt_device_control(rtc_device, RT_DEVICE_CTRL_RTC_GET_TIME, &time);

   // send mail
   send_alarm_mail(ALARM_TYPE_BATTERY_SWITCH, ALARM_PROCESS_FLAG_LOCAL, BATTERY_SWITCH_DETECT_STATUS, time);
   sys_power_supply(RT_TRUE,0,0);
   }
   rt_device_control(device, RT_DEVICE_CTRL_UNMASK_EXTI, (void *)0);
   rt_timer_stop(battery_switch_exti_timer);
   }

   rt_err_t battery_switch_rx_ind(rt_device_t dev, rt_size_t size)
   {
   rt_device_t device = RT_NULL;

   device = rt_device_find(DEVICE_NAME_BATTERY_SWITCH);
   RT_ASSERT(device != RT_NULL);
   rt_device_control(device, RT_DEVICE_CTRL_MASK_EXTI, (void *)0);
   rt_timer_start(battery_switch_exti_timer);
   return RT_EOK;
   }

   struct gpio_exti_user_data battery_switch_user_data =
   {
   DEVICE_NAME_BATTERY_SWITCH,
   GPIOD,
   GPIO_Pin_9,
   GPIO_Mode_IN_FLOATING,
   GPIO_Speed_50MHz,
   RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO,
   GPIO_PortSourceGPIOD,
   GPIO_PinSource9,
   EXTI_Line9,
   EXTI_Mode_Interrupt,
   BATTERY_SWITCH_EXTI_TRIGGER_MODE,
   EXTI9_5_IRQn,
   1,
   5,
   battery_switch_rx_ind,
   };

   void rt_hw_battery_switch_register(void)
   {
   gpio_device *gpio_device = &battery_switch_device;
   struct gpio_exti_user_data *gpio_user_data = &battery_switch_user_data;

   gpio_device->ops = &gpio_exti_user_ops;

   rt_hw_gpio_register(gpio_device, gpio_user_data->name, (RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX), gpio_user_data);
   battery_switch_exti_timer = rt_timer_create("bat_sw",
   battery_switch_exti_timeout,
   RT_NULL,
   BATTERY_SWITCH_INT_INTERVAL,
   RT_TIMER_FLAG_ONE_SHOT);
   rt_device_set_rx_indicate((rt_device_t)gpio_device, gpio_user_data->gpio_exti_rx_indicate);
   }
*/
/* key2 device */
//rt_err_t key2_rx_ind(rt_device_t dev, rt_size_t size)
//{
//  gpio_device *gpio = RT_NULL;
//  RT_ASSERT(dev != RT_NULL);
//  gpio = (gpio_device *)dev;

//  rt_kprintf("key2 ok, value is 0x%x\n", gpio->pin_value);
//  return RT_EOK;
//}

//struct gpio_exti_user_data key2_user_data =
//{
//  "key2",
//  GPIOE,
//  GPIO_Pin_6,
//  GPIO_Mode_IPD,
//  GPIO_Speed_50MHz,
//  RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO,
//  GPIO_PortSourceGPIOE,
//  GPIO_PinSource6,
//  EXTI_Line6,
//  EXTI_Mode_Interrupt,
//  EXTI_Trigger_Rising,
//  EXTI9_5_IRQn,
//  1,
//  4,
//  key2_rx_ind,
//};

//gpio_device key2_device;
//void rt_hw_key2_register(void)
//{
//  gpio_device *key_device = &key2_device;
//  struct gpio_exti_user_data *key_user_data = &key2_user_data;

//  key_device->ops = &gpio_exti_user_ops;
//
//  rt_hw_gpio_register(key_device,key_user_data->name, (RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX), key_user_data);
//  rt_device_set_rx_indicate((rt_device_t)key_device,key_user_data->gpio_exti_rx_indicate);
//}

//#ifdef USE_BUTTON_ADJUST_IR
///* button adjust adjust pb5 */
//gpio_device button_adjust_ir_device;
//rt_timer_t  button_adjust_ir_exti_timer = RT_NULL;

//void button_adjust_ir_exti_timeout(void *parameters)
//{
//  time_t time;

//  rt_device_t device = RT_NULL;
//  uint8_t data;

//  device = rt_device_find(DEVICE_NAME_BUTTON_ADJUST_IR);
//  if (device != RT_NULL)
//  {
//    rt_device_read(device,0,&data,0);
//    if (data == BUTTON_ADJUST_IR_DETECT_STATUS) // rfid key is plugin
//    {
//      // produce mail
//      RT_ASSERT(rtc_device != RT_NULL);
//      rt_device_control(rtc_device, RT_DEVICE_CTRL_RTC_GET_TIME, &time);

//      // send mail
//      send_alarm_mail(ALARM_TYPE_BUTTON_ADJUST_IR, ALARM_PROCESS_FLAG_LOCAL, BUTTON_ADJUST_IR_DETECT_STATUS, time);
//    }
//    else
//    {
//      rt_timer_stop(button_adjust_ir_exti_timer);
//    }
//    rt_device_control(device, RT_DEVICE_CTRL_UNMASK_EXTI, (void *)0);
//  }

//}


//rt_err_t button_adjust_ir_rx_ind(rt_device_t dev, rt_size_t size)
//{
//  //gpio_device *gpio = RT_NULL;
//  //gpio = (gpio_device *)dev;
//  rt_device_t device = RT_NULL;

//  device = rt_device_find(DEVICE_NAME_BUTTON_ADJUST_IR);
//  RT_ASSERT(device != RT_NULL);
//  rt_device_control(device, RT_DEVICE_CTRL_MASK_EXTI, (void *)0);
//  rt_timer_start(button_adjust_ir_exti_timer);
//
//  return RT_EOK;
//}

//struct gpio_exti_user_data button_adjust_ir_user_data =
//{
//  DEVICE_NAME_BUTTON_ADJUST_IR,
//  GPIOB,
//  GPIO_Pin_5,
//  GPIO_Mode_IN_FLOATING,
//  GPIO_Speed_50MHz,
//  RCC_APB2Periph_GPIOB |RCC_APB2Periph_AFIO,
//  GPIO_PortSourceGPIOB,
//  GPIO_PinSource5,
//  EXTI_Line5,
//  EXTI_Mode_Interrupt,
//  RFID_KEY_EXTI_TRIGGER_MODE,
//  EXTI9_5_IRQn,
//  1,
//  5,
//  button_adjust_ir_rx_ind,
//};

//void rt_hw_button_adjust_ir_register(void)
//{
//  gpio_device *gpio_device = &button_adjust_ir_device;
//  struct gpio_exti_user_data *gpio_user_data = &button_adjust_ir_user_data;

//  gpio_device->ops = &gpio_exti_user_ops;

//  rt_hw_gpio_register(gpio_device, gpio_user_data->name, (RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX), gpio_user_data);
//  button_adjust_ir_exti_timer = rt_timer_create("adjustIR",
//                                               button_adjust_ir_exti_timeout,
//                                               RT_NULL,
//                                               BUTTON_ADJUST_IR_INT_INTERVAL,
//                                               RT_TIMER_FLAG_PERIODIC);
//  rt_device_set_rx_indicate((rt_device_t)gpio_device, gpio_user_data->gpio_exti_rx_indicate);


//}
//#endif

//#ifdef RT_USING_FINSH
//#include <finsh.h>
//void key(rt_int8_t num)
//{
//  rt_device_t key = RT_NULL;
//  rt_int8_t dat = 0;

//  if(num == 1)
//  {
//    key = rt_device_find("key1");
//    if (key != RT_NULL)
//    {
//      rt_device_read(key,0,&dat,0);
//#ifdef RT_USING_FINSH
//      rt_kprintf("key1 = 0x%x\n",dat);
//#endif
//    }
//    else
//    {
//#ifdef RT_USING_FINSH
//      rt_kprintf("the device is not found!\n");
//#endif
//    }
//  }
//  if(num == 2)
//  {
//    key = rt_device_find("key2");
//    if (key != RT_NULL)
//    {
//      rt_device_read(key,0,&dat,0);
//#ifdef RT_USING_FINSH
//      rt_kprintf("key2 = 0x%x\n",dat);
//#endif
//    }
//    else
//    {
//#ifdef RT_USING_FINSH
//      rt_kprintf("the device is not found!\n");
//#endif
//    }
//  }
//}
//FINSH_FUNCTION_EXPORT(key, key[1 2] = x)
//#endif


void EXTI9_5_IRQHandler(void)
{
	extern void rt_hw_gpio_isr(gpio_device *gpio);

	/* enter interrupt */
	rt_interrupt_enter();
	/* lock_shell exti isr */
	if(EXTI_GetITStatus(EXTI_Line7) == SET)
	{
		rt_hw_gpio_isr(&switch1_device);
		EXTI_ClearITPendingBit(EXTI_Line7);
	}
	if(EXTI_GetITStatus(EXTI_Line8) == SET)
	{
		rt_hw_gpio_isr(&switch2_device);
		EXTI_ClearITPendingBit(EXTI_Line8);
	}
	if(EXTI_GetITStatus(EXTI_Line9) == SET)
	{
		rt_hw_gpio_isr(&switch3_device);
		EXTI_ClearITPendingBit(EXTI_Line9);
	}
	/* leave interrupt */
	rt_interrupt_leave();
}

void EXTI15_10_IRQHandler(void)
{
	extern void rt_hw_gpio_isr(gpio_device *gpio);

	/* enter interrupt */
	rt_interrupt_enter();
    // key board int
	if(EXTI_GetITStatus(EXTI_Line15) == SET)
	{
		rt_hw_gpio_isr(&key_device);
		EXTI_ClearITPendingBit(EXTI_Line15);
	}
	/* leave interrupt */
	rt_interrupt_leave();
}

static int rt_hw_gpio_exti_enable(void)
{
    device_enable(DEVICE_NAME_SWITCH1);
    device_enable(DEVICE_NAME_SWITCH2);
    device_enable(DEVICE_NAME_SWITCH3);
    device_enable(DEVICE_NAME_KEY);
    return 0;
}

INIT_DEVICE_EXPORT(rt_hw_switch1_register);
INIT_DEVICE_EXPORT(rt_hw_switch2_register);
INIT_DEVICE_EXPORT(rt_hw_switch3_register);

INIT_DEVICE_EXPORT(rt_hw_key_register);

INIT_APP_EXPORT(rt_hw_gpio_exti_enable);
