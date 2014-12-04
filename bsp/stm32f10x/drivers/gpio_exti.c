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
#include "fprint.h"
#include "gui.h"
#include "rf433.h"
#include "gsm.h"

extern rt_device_t rtc_device;

#define KB_DEBUG 0

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
    rt_timer_t timer;
};

/*
 *  GPIO ops function interfaces
 *
 */
static void 
__gpio_nvic_configure(gpio_device *gpio,FunctionalState new_status)
{
    NVIC_InitTypeDef nvic_init_structure;
    struct gpio_exti_user_data* user = (struct gpio_exti_user_data *)gpio->parent.user_data;

    nvic_init_structure.NVIC_IRQChannel = user->nvic_channel;
    nvic_init_structure.NVIC_IRQChannelPreemptionPriority = user->nvic_preemption_priority;
    nvic_init_structure.NVIC_IRQChannelSubPriority = user->nvic_subpriority;
    nvic_init_structure.NVIC_IRQChannelCmd = new_status;
    NVIC_Init(&nvic_init_structure);
}

static void 
__gpio_exti_configure(gpio_device *gpio,FunctionalState new_status)
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

static void 
__gpio_pin_configure(gpio_device *gpio)
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
rt_err_t 
gpio_exti_configure(gpio_device *gpio)
{
    __gpio_pin_configure(gpio);
    if(RT_DEVICE_FLAG_INT_RX & gpio->parent.flag)
    {
        __gpio_nvic_configure(gpio,ENABLE);
        __gpio_exti_configure(gpio,ENABLE);
    }

    return RT_EOK;
}
rt_err_t 
gpio_exti_control(gpio_device *gpio, rt_uint8_t cmd, void *arg)
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

void 
gpio_exti_out(gpio_device *gpio, rt_uint8_t data)
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

rt_uint8_t 
gpio_exti_intput(gpio_device *gpio)
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
/*
* Function gpio_config ()
*
* 将IO端口都设置成模拟输入，以降低功耗以及增强电磁兼容
*
*/
//IO端口配置结构体变量
static void gpio_config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |
    RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD |
    RCC_APB2Periph_GPIOE, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    //GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |
    //RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE, DISABLE);
    RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE, DISABLE);

    }
/* switch1 device */
gpio_device switch1_device;
static GPIO_TypeDef gpio_bk[7];
static RCC_TypeDef rcc_bk;
static void 
gpio_backup(void) {
    gpio_bk[0] = *GPIOA;
    gpio_bk[1] = *GPIOB;
    gpio_bk[2] = *GPIOC;
    gpio_bk[3] = *GPIOD;
    gpio_bk[4] = *GPIOE;
    gpio_bk[5] = *GPIOF;
    gpio_bk[6] = *GPIOG;
    rcc_bk = *RCC;
}
static void 
gpio_restore(void) {
    *GPIOA = gpio_bk[0];
    *GPIOB = gpio_bk[1];
    *GPIOC = gpio_bk[2];
    *GPIOD = gpio_bk[3];
    *GPIOE = gpio_bk[4];
    *GPIOF = gpio_bk[5];
    *GPIOG = gpio_bk[6];
    *RCC = rcc_bk;
}
void switch1_exti_timeout(void *parameters)
{
	gpio_device *gpio = (gpio_device *)parameters;
    struct gpio_exti_user_data *gpio_user_data = gpio->parent.user_data;

	gpio->ops->control(gpio, RT_DEVICE_CTRL_MASK_EXTI, (void *)0); 
    if (gpio->ops->intput(gpio) == SWITCH1_STATUS)
    {
        rt_kprintf("it is switch1 detect!\n");
        send_local_mail(ALARM_TYPE_SYSTEM_RESET,0,RT_NULL);
        //gpio_backup();
        /*
        gpio_pin_output(DEVICE_NAME_POWER_FLASH,0,0);
        gpio_pin_output(DEVICE_NAME_POWER_MOTOR,0,0);
        gpio_pin_output(DEVICE_NAME_POWER_BT,0,0);
        gpio_pin_output(DEVICE_NAME_POWER_FRONT,0,0);
        gpio_pin_output(DEVICE_NAME_POWER_GSM,0,0);
        */
        //gpio_config();
        //gpio_restore();
        //PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
    }
	gpio->ops->control(gpio, RT_DEVICE_CTRL_UNMASK_EXTI, (void *)0); 
	rt_timer_stop(gpio_user_data->timer);
}


rt_err_t switch1_rx_ind(rt_device_t dev, rt_size_t size)
{
    struct gpio_exti_user_data *gpio_user_data = ((gpio_device *)dev)->parent.user_data;
	rt_timer_start(gpio_user_data->timer);

	return RT_EOK;
}

struct gpio_exti_user_data switch1_user_data =
{
	DEVICE_NAME_SWITCH1,
	GPIOC,
	GPIO_Pin_7,
	GPIO_Mode_IPU,
	GPIO_Speed_50MHz,
	RCC_APB2Periph_GPIOC |RCC_APB2Periph_AFIO,
	GPIO_PortSourceGPIOC,
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
    gpio_user_data->timer = rt_timer_create("t_sw1",
										 switch1_exti_timeout,
										 gpio_device,
										 SWITCH1_INT_INTERVAL,
										 RT_TIMER_FLAG_ONE_SHOT | RT_TIMER_FLAG_SOFT_TIMER);
    rt_device_set_rx_indicate((rt_device_t)gpio_device, gpio_user_data->gpio_exti_rx_indicate);

    return 0;
}

void btled_exti_timeout(void *parameters)
{
	gpio_device *gpio = (gpio_device *)parameters;
    struct gpio_exti_user_data *gpio_user_data = gpio->parent.user_data;

	gpio->ops->control(gpio, RT_DEVICE_CTRL_MASK_EXTI, (void *)0); 
    if (gpio->ops->intput(gpio) == BT_LED_STATUS)
    {
        rt_kprintf("it is BT LED detect!\n");

        //PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
    }
	gpio->ops->control(gpio, RT_DEVICE_CTRL_UNMASK_EXTI, (void *)0); 
	rt_timer_stop(gpio_user_data->timer);
}


rt_err_t btled_rx_ind(rt_device_t dev, rt_size_t size)
{
	struct gpio_exti_user_data *gpio_user_data = ((gpio_device *)dev)->parent.user_data;
	rt_timer_start(gpio_user_data->timer);

	return RT_EOK;
}

struct gpio_exti_user_data btled_user_data =
{
	DEVICE_NAME_BT_LED,
	GPIOC,
	GPIO_Pin_9,
	GPIO_Mode_IN_FLOATING,
	GPIO_Speed_50MHz,
	RCC_APB2Periph_GPIOC |RCC_APB2Periph_AFIO,
	GPIO_PortSourceGPIOC,
	GPIO_PinSource9,
	EXTI_Line9,
	EXTI_Mode_Interrupt,
	BT_LED_EXTI_TRIGGER_MODE,
	EXTI9_5_IRQn,
	1,
	5,
	btled_rx_ind,
};

gpio_device btled_device;

int rt_hw_bt_led_register(void)
{
    gpio_device *gpio_device = &btled_device;
    struct gpio_exti_user_data *gpio_user_data = &btled_user_data;

    gpio_device->ops = &gpio_exti_user_ops;

    rt_hw_gpio_register(gpio_device, gpio_user_data->name, (RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX), gpio_user_data);
    gpio_user_data->timer = rt_timer_create("bt_led",
										 btled_exti_timeout,
										 gpio_device,
										 BT_LED_INT_INTERVAL,
										 RT_TIMER_FLAG_ONE_SHOT | RT_TIMER_FLAG_SOFT_TIMER);
    rt_device_set_rx_indicate((rt_device_t)gpio_device, gpio_user_data->gpio_exti_rx_indicate);

    return 0;
}


/* switch2 device */
gpio_device switch2_device;

void switch2_exti_timeout(void *parameters)
{
	gpio_device *gpio = (gpio_device *)parameters;
    struct gpio_exti_user_data *gpio_user_data = gpio->parent.user_data;

	gpio->ops->control(gpio, RT_DEVICE_CTRL_MASK_EXTI, (void *)0); 
    if (gpio->ops->intput(gpio) == SWITCH2_STATUS)
    {
        rt_kprintf("it is key2 detect!\n");

        rt_enter_critical();
        gpio_backup();
        gpio_pin_output(DEVICE_NAME_POWER_FLASH,0,0);
        gpio_pin_output(DEVICE_NAME_POWER_MOTOR,0,0);
        gpio_pin_output(DEVICE_NAME_POWER_BT,0,0);
        gpio_pin_output(DEVICE_NAME_POWER_FRONT,0,0);
        gpio_pin_output(DEVICE_NAME_POWER_GSM,0,0);
        gpio_config();
        PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
        rt_exit_critical();
    }
	gpio->ops->control(gpio, RT_DEVICE_CTRL_UNMASK_EXTI, (void *)0); 
	rt_timer_stop(gpio_user_data->timer);
}


rt_err_t switch2_rx_ind(rt_device_t dev, rt_size_t size)
{
    struct gpio_exti_user_data *gpio_user_data = ((gpio_device *)dev)->parent.user_data;
	rt_timer_start(gpio_user_data->timer);

	return RT_EOK;
}

struct gpio_exti_user_data switch2_user_data =
{
    DEVICE_NAME_SWITCH2,
    GPIOC,
    GPIO_Pin_6,
    GPIO_Mode_IPU,
    GPIO_Speed_50MHz,
    RCC_APB2Periph_GPIOC |RCC_APB2Periph_AFIO,
    GPIO_PortSourceGPIOC,
    GPIO_PinSource6,
    EXTI_Line6,
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
    gpio_user_data->timer = rt_timer_create("t_sw2",
										 switch2_exti_timeout,
										 gpio_device,
										 SWITCH2_INT_INTERVAL,
										 RT_TIMER_FLAG_ONE_SHOT | RT_TIMER_FLAG_SOFT_TIMER);
    rt_device_set_rx_indicate((rt_device_t)gpio_device, gpio_user_data->gpio_exti_rx_indicate);

    return 0;
}
/* switch3 device 
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
	GPIOD,
	GPIO_Pin_15,
	GPIO_Mode_IPU,
	GPIO_Speed_50MHz,
	RCC_APB2Periph_GPIOD |RCC_APB2Periph_AFIO,
	GPIO_PortSourceGPIOD,
	GPIO_PinSource15,
	EXTI_Line15,
	EXTI_Mode_Interrupt,
	SWITCH3_EXTI_TRIGGER_MODE,
	EXTI15_10_IRQn,
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
*/

/* gpio break */
gpio_device break_device;

void break_exti_timeout(void *parameters)
{
	gpio_device *gpio = (gpio_device *)parameters;
    struct gpio_exti_user_data *gpio_user_data = gpio->parent.user_data;

	gpio->ops->control(gpio, RT_DEVICE_CTRL_MASK_EXTI, (void *)0); 
    if (gpio->ops->intput(gpio) == BREAK_STATUS)
    {
        rt_kprintf("it is BREAK detect!\n");
    }
	gpio->ops->control(gpio, RT_DEVICE_CTRL_UNMASK_EXTI, (void *)0); 
	rt_timer_stop(gpio_user_data->timer);

}


rt_err_t break_rx_ind(rt_device_t dev, rt_size_t size)
{
    struct gpio_exti_user_data *gpio_user_data = ((gpio_device *)dev)->parent.user_data;
	rt_timer_start(gpio_user_data->timer);

	return RT_EOK;
}

struct gpio_exti_user_data break_user_data =
{
    DEVICE_NAME_BREAK,
    GPIOB,
    GPIO_Pin_15,
    GPIO_Mode_IPU,
    GPIO_Speed_50MHz,
    RCC_APB2Periph_GPIOB |RCC_APB2Periph_AFIO,
    GPIO_PortSourceGPIOB,
    GPIO_PinSource15,
    EXTI_Line15,
    EXTI_Mode_Interrupt,
    BREAK_EXTI_TRIGGER_MODE,
    EXTI15_10_IRQn,
    1,
    5,
    break_rx_ind,
};

int rt_hw_break_register(void)
{
    gpio_device *gpio_device = &break_device;
    struct gpio_exti_user_data *gpio_user_data = &break_user_data;

    gpio_device->ops = &gpio_exti_user_ops;

    rt_hw_gpio_register(gpio_device, gpio_user_data->name, (RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX), gpio_user_data);
    gpio_user_data->timer = rt_timer_create("t_brk",
										 break_exti_timeout,
										 gpio_device,
										 BREAK_INT_INTERVAL,
										 RT_TIMER_FLAG_ONE_SHOT | RT_TIMER_FLAG_SOFT_TIMER);
    rt_device_set_rx_indicate((rt_device_t)gpio_device, gpio_user_data->gpio_exti_rx_indicate);

    return 0;
}

/* gpio mag */
gpio_device mag_device;

void mag_exti_timeout(void *parameters)
{
	gpio_device *gpio = (gpio_device *)parameters;
    struct gpio_exti_user_data *gpio_user_data = gpio->parent.user_data;

	gpio->ops->control(gpio, RT_DEVICE_CTRL_MASK_EXTI, (void *)0); 
    if (gpio->ops->intput(gpio) == MAG_STATUS)
    {
        rt_kprintf("it is MAG detect!\n");
        //点亮屏幕
        gui_open_lcd_show();
    }
	gpio->ops->control(gpio, RT_DEVICE_CTRL_UNMASK_EXTI, (void *)0); 
	rt_timer_stop(gpio_user_data->timer);
}


rt_err_t mag_rx_ind(rt_device_t dev, rt_size_t size)
{
    struct gpio_exti_user_data *gpio_user_data = ((gpio_device *)dev)->parent.user_data;
	rt_timer_start(gpio_user_data->timer);

	return RT_EOK;
}

struct gpio_exti_user_data mag_user_data =
{
    DEVICE_NAME_MAG,
    GPIOB,
    GPIO_Pin_12,
    GPIO_Mode_IPU,
    GPIO_Speed_50MHz,
    RCC_APB2Periph_GPIOB |RCC_APB2Periph_AFIO,
    GPIO_PortSourceGPIOB,
    GPIO_PinSource12,
    EXTI_Line12,
    EXTI_Mode_Interrupt,
    MAG_EXTI_TRIGGER_MODE,
    EXTI15_10_IRQn,
    1,
    5,
    mag_rx_ind,
};

int rt_hw_mag_register(void)
{
    gpio_device *gpio_device = &mag_device;
    struct gpio_exti_user_data *gpio_user_data = &mag_user_data;

    gpio_device->ops = &gpio_exti_user_ops;

    rt_hw_gpio_register(gpio_device, gpio_user_data->name, (RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX), gpio_user_data);
    gpio_user_data->timer = rt_timer_create("t_mag",
										 mag_exti_timeout,
										 gpio_device,
										 MAG_INT_INTERVAL,
										 RT_TIMER_FLAG_ONE_SHOT | RT_TIMER_FLAG_SOFT_TIMER);
    rt_device_set_rx_indicate((rt_device_t)gpio_device, gpio_user_data->gpio_exti_rx_indicate);

    return 0;
}

/* fp_touch device */
gpio_device fp_touch_device;

void fp_touch_exti_timeout(void *parameters)
{
	gpio_device *gpio = (gpio_device *)parameters;
    struct gpio_exti_user_data *gpio_user_data = gpio->parent.user_data;

	gpio->ops->control(gpio, RT_DEVICE_CTRL_MASK_EXTI, (void *)0); 
    if (gpio->ops->intput(gpio) == FP_TOUCH_STATUS)
    {
        rt_kprintf("it is fprint touch!\n");
        //fp_inform();
        send_alarm_mail(ALARM_TYPE_FPRINT_INFORM, 0, 0, 0);
    }
	gpio->ops->control(gpio, RT_DEVICE_CTRL_UNMASK_EXTI, (void *)0); 
	rt_timer_stop(gpio_user_data->timer);
}

rt_err_t fp_touch_rx_ind(rt_device_t dev, rt_size_t size)
{
    struct gpio_exti_user_data *gpio_user_data = ((gpio_device *)dev)->parent.user_data;
	rt_timer_start(gpio_user_data->timer);

	return RT_EOK;
}

struct gpio_exti_user_data fp_touch_user_data =
{
	DEVICE_NAME_FP_TOUCH,
	GPIOA,
	GPIO_Pin_1,
	GPIO_Mode_IPD,
	GPIO_Speed_50MHz,
	RCC_APB2Periph_GPIOA |RCC_APB2Periph_AFIO,
	GPIO_PortSourceGPIOA,
	GPIO_PinSource1,
	EXTI_Line1,
	EXTI_Mode_Interrupt,
	FP_TOUCH_EXTI_TRIGGER_MODE,
	EXTI1_IRQn,
	1,
	5,
	fp_touch_rx_ind,
};

int rt_hw_fp_touch_register(void)
{
    gpio_device *gpio_device = &fp_touch_device;
    struct gpio_exti_user_data *gpio_user_data = &fp_touch_user_data;

    gpio_device->ops = &gpio_exti_user_ops;

    rt_hw_gpio_register(gpio_device, gpio_user_data->name, (RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX), gpio_user_data);
    gpio_user_data->timer = rt_timer_create("t_fpt",
										 fp_touch_exti_timeout,
										 gpio_device,
										 FP_TOUCH_INT_INTERVAL,
										 RT_TIMER_FLAG_ONE_SHOT | RT_TIMER_FLAG_SOFT_TIMER);
    rt_device_set_rx_indicate((rt_device_t)gpio_device, gpio_user_data->gpio_exti_rx_indicate);

    return 0;
}

/* kb_intr device */
gpio_device kb_intr_device;

__STATIC_INLINE uint8_t 
bit_to_index(uint16_t data)
{
    uint8_t result = 0;
    while (data)
    {
        result++;
        data >>= 1;
    }
    return result;
}

/*static const uint8_t char_remap[16] = {
    '?',
    '3', '6', '9',
    '#', '2', '5',
    '8', '0', '1',
    '4', '7', '*',
    '?', '?', '?',
};*/

static const uint8_t char_remap[16] = {
    '?',
    '*', '7', '4',
    '1', '0', '8',
    '5', '2', '#',
    '9', '6', '3',
    '?', '?', '?',
};
static const uint8_t char_long_remap[16] = {
    '?',
    'G', '7', '4',
    '1', '0', '8',
    '5', '2', 'S',
    '9', '6', '3',
    '?', '?', '?',
};

__STATIC_INLINE uint16_t
kb_read(void)
{
    uint16_t data = 0;
    if (!gpio_pin_input(DEVICE_NAME_KB_IN1, KB_DEBUG))
    {
        gpio_pin_output(DEVICE_NAME_KB_SC1,1, KB_DEBUG);
        gpio_pin_output(DEVICE_NAME_KB_SC2,1, KB_DEBUG);
        gpio_pin_output(DEVICE_NAME_KB_SC3,1, KB_DEBUG);
        if (!gpio_pin_input(DEVICE_NAME_KB_IN1, KB_DEBUG)) {
            data |= 0x01;
            goto ERROR;
        }
        gpio_pin_output(DEVICE_NAME_KB_SC1,1, KB_DEBUG);
        gpio_pin_output(DEVICE_NAME_KB_SC2,1, KB_DEBUG);
        gpio_pin_output(DEVICE_NAME_KB_SC3,0, KB_DEBUG);
        if (!gpio_pin_input(DEVICE_NAME_KB_IN1, KB_DEBUG))
            data |= 0x02;
        gpio_pin_output(DEVICE_NAME_KB_SC1,1, KB_DEBUG);
        gpio_pin_output(DEVICE_NAME_KB_SC2,0, KB_DEBUG);
        gpio_pin_output(DEVICE_NAME_KB_SC3,1, KB_DEBUG);
        if (!gpio_pin_input(DEVICE_NAME_KB_IN1, KB_DEBUG))
            data |= 0x04;
        gpio_pin_output(DEVICE_NAME_KB_SC1,0, KB_DEBUG);
        gpio_pin_output(DEVICE_NAME_KB_SC2,1, KB_DEBUG);
        gpio_pin_output(DEVICE_NAME_KB_SC3,1, KB_DEBUG);
        if (!gpio_pin_input(DEVICE_NAME_KB_IN1, KB_DEBUG))
            data |= 0x08;
        gpio_pin_output(DEVICE_NAME_KB_SC1,0, KB_DEBUG);
        gpio_pin_output(DEVICE_NAME_KB_SC2,0, KB_DEBUG);
        gpio_pin_output(DEVICE_NAME_KB_SC3,0, KB_DEBUG);

    }
    if (!gpio_pin_input(DEVICE_NAME_KB_IN2, KB_DEBUG))
    {
        gpio_pin_output(DEVICE_NAME_KB_SC1,1, KB_DEBUG);
        gpio_pin_output(DEVICE_NAME_KB_SC2,1, KB_DEBUG);
        gpio_pin_output(DEVICE_NAME_KB_SC3,1, KB_DEBUG);
        if (!gpio_pin_input(DEVICE_NAME_KB_IN2, KB_DEBUG)) {
            data |= 0x10;
            goto ERROR;
        }
        gpio_pin_output(DEVICE_NAME_KB_SC1,1, KB_DEBUG);
        gpio_pin_output(DEVICE_NAME_KB_SC2,1, KB_DEBUG);
        gpio_pin_output(DEVICE_NAME_KB_SC3,0, KB_DEBUG);
        if (!gpio_pin_input(DEVICE_NAME_KB_IN2, KB_DEBUG))
            data |= 0x20;
        gpio_pin_output(DEVICE_NAME_KB_SC1,1, KB_DEBUG);
        gpio_pin_output(DEVICE_NAME_KB_SC2,0, KB_DEBUG);
        gpio_pin_output(DEVICE_NAME_KB_SC3,1, KB_DEBUG);
        if (!gpio_pin_input(DEVICE_NAME_KB_IN2, KB_DEBUG))
            data |= 0x40;
        gpio_pin_output(DEVICE_NAME_KB_SC1,0, KB_DEBUG);
        gpio_pin_output(DEVICE_NAME_KB_SC2,1, KB_DEBUG);
        gpio_pin_output(DEVICE_NAME_KB_SC3,1, KB_DEBUG);
        if (!gpio_pin_input(DEVICE_NAME_KB_IN2, KB_DEBUG))
            data |= 0x80;
        gpio_pin_output(DEVICE_NAME_KB_SC1,0, KB_DEBUG);
        gpio_pin_output(DEVICE_NAME_KB_SC2,0, KB_DEBUG);
        gpio_pin_output(DEVICE_NAME_KB_SC3,0, KB_DEBUG);
        
    }
    if (!gpio_pin_input(DEVICE_NAME_KB_IN3, KB_DEBUG))
    {
        gpio_pin_output(DEVICE_NAME_KB_SC1,1, KB_DEBUG);
        gpio_pin_output(DEVICE_NAME_KB_SC2,1, KB_DEBUG);
        gpio_pin_output(DEVICE_NAME_KB_SC3,1, KB_DEBUG);
        if (!gpio_pin_input(DEVICE_NAME_KB_IN3, KB_DEBUG)) {
            data |= 0x0100;
            goto ERROR;
        }
        gpio_pin_output(DEVICE_NAME_KB_SC1,1, KB_DEBUG);
        gpio_pin_output(DEVICE_NAME_KB_SC2,1, KB_DEBUG);
        gpio_pin_output(DEVICE_NAME_KB_SC3,0, KB_DEBUG);
        if (!gpio_pin_input(DEVICE_NAME_KB_IN3, KB_DEBUG))
            data |= 0x0200;
        gpio_pin_output(DEVICE_NAME_KB_SC1,1, KB_DEBUG);
        gpio_pin_output(DEVICE_NAME_KB_SC2,0, KB_DEBUG);
        gpio_pin_output(DEVICE_NAME_KB_SC3,1, KB_DEBUG);
        if (!gpio_pin_input(DEVICE_NAME_KB_IN3, KB_DEBUG))
            data |= 0x0400;
        gpio_pin_output(DEVICE_NAME_KB_SC1,0, KB_DEBUG);
        gpio_pin_output(DEVICE_NAME_KB_SC2,1, KB_DEBUG);
        gpio_pin_output(DEVICE_NAME_KB_SC3,1, KB_DEBUG);
        if (!gpio_pin_input(DEVICE_NAME_KB_IN3, KB_DEBUG))
            data |= 0x0800;
        gpio_pin_output(DEVICE_NAME_KB_SC1,0, KB_DEBUG);
        gpio_pin_output(DEVICE_NAME_KB_SC2,0, KB_DEBUG);
        gpio_pin_output(DEVICE_NAME_KB_SC3,0, KB_DEBUG);
        
    }
    //rt_device_read(dev_in2,0,&temp,1);

ERROR:
    gpio_pin_output(DEVICE_NAME_KB_SC1,0, KB_DEBUG);
    gpio_pin_output(DEVICE_NAME_KB_SC2,0, KB_DEBUG);
    gpio_pin_output(DEVICE_NAME_KB_SC3,0, KB_DEBUG);
    return data;

}

void 
kb_intr_exti_timeout(void *parameters)
{
	gpio_device *gpio = (gpio_device *)parameters;
    //struct gpio_exti_user_data *gpio_user_data = gpio->parent.user_data;
    uint16_t data,data2;
    uint8_t c;
    s32 i, cnts = 0;

	gpio->ops->control(gpio, RT_DEVICE_CTRL_MASK_EXTI, (void *)0);
    if (gpio->ops->intput(gpio) == KB_INTR_STATUS) {
        data = kb_read();
        for (i = 0; i < 5000; i++) {
            if (gpio->ops->intput(gpio) == KB_INTR_STATUS) {
                data2 = kb_read();
                if (data2) {
                    if (data == data2)
                        cnts++;
                    else {
                        if (cnts > 500)
                            data = data2;
                        cnts = 0;
                    }
                }
            } else if (cnts > 500){
                break;
            }
        }
        //rt_kprintf("key value : %x\n", data);
            if (cnts < 3000)
                c = char_remap[bit_to_index(data&0x0fff)];
            else
                c = char_long_remap[bit_to_index(data&0x0fff)];
        RT_DEBUG_LOG(0,("key value : 0x%x, index :%d, char :%c\n", data, bit_to_index(data&0x0fff), c));
        send_key_value_mail(KB_MAIL_TYPE_INPUT, KB_MODE_NORMAL_AUTH, c);
        //send_kb_mail(KB_MAIL_TYPE_INPUT, KB_MODE_NORMAL_AUTH, c);
    }
    gpio->ops->control(gpio, RT_DEVICE_CTRL_UNMASK_EXTI, (void *)0);
	//rt_timer_stop(gpio_user_data->timer);
}


rt_err_t 
kb_intr_rx_ind(rt_device_t dev, rt_size_t size)
{
    struct gpio_exti_user_data *gpio_user_data = ((gpio_device *)dev)->parent.user_data;
	rt_timer_start(gpio_user_data->timer);
	return RT_EOK;
}

struct gpio_exti_user_data kb_intr_user_data =
{
	DEVICE_NAME_KB_INTR,
	GPIOD,
	GPIO_Pin_8,
	GPIO_Mode_IPU,
	GPIO_Speed_50MHz,
	RCC_APB2Periph_GPIOD |RCC_APB2Periph_AFIO,
	GPIO_PortSourceGPIOD,
	GPIO_PinSource8,
	EXTI_Line8,
	EXTI_Mode_Interrupt,
	KB_INTR_EXTI_TRIGGER_MODE,
	EXTI9_5_IRQn,
	1,
	5,
	kb_intr_rx_ind,
};

int 
rt_hw_kb_intr_register(void)
{
    gpio_device *gpio_device = &kb_intr_device;
    struct gpio_exti_user_data *gpio_user_data = &kb_intr_user_data;

    gpio_device->ops = &gpio_exti_user_ops;

    rt_hw_gpio_register(gpio_device, gpio_user_data->name, (RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX), gpio_user_data);
    gpio_user_data->timer = rt_timer_create("t_kintr",
									 kb_intr_exti_timeout,
									 gpio_device,
									 KB_INTR_INT_INTERVAL,
									 RT_TIMER_FLAG_ONE_SHOT | RT_TIMER_FLAG_SOFT_TIMER);
    rt_device_set_rx_indicate((rt_device_t)gpio_device, gpio_user_data->gpio_exti_rx_indicate);

    return 0;
}

gpio_device gsm_ring_device;

void gsm_ring_exti_timeout(void *parameters)
{
	gpio_device *gpio = (gpio_device *)parameters;
    struct gpio_exti_user_data *gpio_user_data = gpio->parent.user_data;

	gpio->ops->control(gpio, RT_DEVICE_CTRL_MASK_EXTI, (void *)0); 
    if (gpio->ops->intput(gpio) == GSM_RING_STATUS)
    {
        rt_kprintf("it is gsm ring!\n");
        // produce mail
        //send_alarm_mail(ALARM_TYPE_SWITCH1, ALARM_PROCESS_FLAG_LOCAL, SWITCH1_STATUS, 0);
        gsm_ring_process(1);
    }
	gpio->ops->control(gpio, RT_DEVICE_CTRL_UNMASK_EXTI, (void *)0); 
	rt_timer_stop(gpio_user_data->timer);
}


rt_err_t gsm_ring_rx_ind(rt_device_t dev, rt_size_t size)
{
    struct gpio_exti_user_data *gpio_user_data = ((gpio_device *)dev)->parent.user_data;
	rt_timer_start(gpio_user_data->timer);
	return RT_EOK;
}

struct gpio_exti_user_data gsm_ring_user_data =
{
   DEVICE_NAME_GSM_RING,
   GPIOE,
   GPIO_Pin_14,
   GPIO_Mode_IPU,
   GPIO_Speed_50MHz,
   RCC_APB2Periph_GPIOE |RCC_APB2Periph_AFIO,
   GPIO_PortSourceGPIOE,
   GPIO_PinSource14,
   EXTI_Line14,
   EXTI_Mode_Interrupt,
   GSM_RING_EXTI_TRIGGER_MODE,
   EXTI15_10_IRQn,
   1,
   5,
   gsm_ring_rx_ind,
};

int rt_hw_gsm_ring_register(void)
{
    gpio_device *gpio_device = &gsm_ring_device;
    struct gpio_exti_user_data *gpio_user_data = &gsm_ring_user_data;

    gpio_device->ops = &gpio_exti_user_ops;

    rt_hw_gpio_register(gpio_device, gpio_user_data->name, (RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX), gpio_user_data);
    gpio_user_data->timer = rt_timer_create("t_ring",
										 gsm_ring_exti_timeout,
										 gpio_device,
										 GSM_RING_INT_INTERVAL,
										 RT_TIMER_FLAG_ONE_SHOT | RT_TIMER_FLAG_SOFT_TIMER);
    rt_device_set_rx_indicate((rt_device_t)gpio_device, gpio_user_data->gpio_exti_rx_indicate);

    return 0;
}

/* hall device */
gpio_device hall_device;

void hall_exti_timeout(void *parameters)
{    
	gpio_device *gpio = (gpio_device *)parameters;
    struct gpio_exti_user_data *gpio_user_data = gpio->parent.user_data;

	gpio->ops->control(gpio, RT_DEVICE_CTRL_MASK_EXTI, (void *)0); 
    if (gpio->ops->intput(gpio) == HALL_STATUS)
    {
            rt_kprintf("it is HALL!\n");
			// produce mail
			//send_alarm_mail(ALARM_TYPE_SWITCH1, ALARM_PROCESS_FLAG_LOCAL, SWITCH1_STATUS, 0);

			send_rf433_mail(RF433_START, RT_NULL);
    }
	gpio->ops->control(gpio, RT_DEVICE_CTRL_UNMASK_EXTI, (void *)0); 
	rt_timer_stop(gpio_user_data->timer);
}


rt_err_t hall_rx_ind(rt_device_t dev, rt_size_t size)
{
    struct gpio_exti_user_data *gpio_user_data = ((gpio_device *)dev)->parent.user_data;
	rt_timer_start(gpio_user_data->timer);

	return RT_EOK;
}

struct gpio_exti_user_data hall_user_data =
{
   DEVICE_NAME_HALL,
   GPIOC,
   GPIO_Pin_3,
   GPIO_Mode_IPU,
   GPIO_Speed_50MHz,
   RCC_APB2Periph_GPIOC |RCC_APB2Periph_AFIO,
   GPIO_PortSourceGPIOC,
   GPIO_PinSource3,
   EXTI_Line3,
   EXTI_Mode_Interrupt,
   HALL_EXTI_TRIGGER_MODE,
   EXTI3_IRQn,
   1,
   5,
   hall_rx_ind,
};

int rt_hw_hall_register(void)
{
    gpio_device *gpio_device = &hall_device;
    struct gpio_exti_user_data *gpio_user_data = &hall_user_data;

    gpio_device->ops = &gpio_exti_user_ops;

    rt_hw_gpio_register(gpio_device, gpio_user_data->name, (RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX), gpio_user_data);
    gpio_user_data->timer = rt_timer_create("t_hall",
										 hall_exti_timeout,
										 gpio_device,
										 HALL_INT_INTERVAL,
										 RT_TIMER_FLAG_ONE_SHOT | RT_TIMER_FLAG_SOFT_TIMER);
    rt_device_set_rx_indicate((rt_device_t)gpio_device, gpio_user_data->gpio_exti_rx_indicate);

    return 0;
}

void
EXTI1_IRQHandler(void)
{
	extern void rt_hw_gpio_isr(gpio_device *gpio);

	/* enter interrupt */
	rt_interrupt_enter();
	if(EXTI_GetITStatus(EXTI_Line1) == SET)
	{
		rt_hw_gpio_isr(&fp_touch_device);
		EXTI_ClearITPendingBit(EXTI_Line1);
	}
	/* leave interrupt */
	rt_interrupt_leave();
}

void
EXTI3_IRQHandler(void)
{
	extern void rt_hw_gpio_isr(gpio_device *gpio);

	/* enter interrupt */
	rt_interrupt_enter();
	if(EXTI_GetITStatus(EXTI_Line3) == SET)
	{
		rt_hw_gpio_isr(&hall_device);
		EXTI_ClearITPendingBit(EXTI_Line3);
	}
	/* leave interrupt */
	rt_interrupt_leave();
}

void 
EXTI9_5_IRQHandler(void)
{
	extern void rt_hw_gpio_isr(gpio_device *gpio);

	/* enter interrupt */
	rt_interrupt_enter();
	/* lock_shell exti isr */
	if(EXTI_GetITStatus(EXTI_Line7) == SET)
	{
				rt_hw_gpio_isr(&switch1_device);
        EXTI_ClearITPendingBit(EXTI_Line7);
        //SystemInit();
        //SystemCoreClockUpdate();
        //gpio_restore();
    }

	if(EXTI_GetITStatus(EXTI_Line6) == SET)
	{
		rt_hw_gpio_isr(&switch2_device);
		EXTI_ClearITPendingBit(EXTI_Line6);
	}
    
	if(EXTI_GetITStatus(EXTI_Line9) == SET)
	{
		rt_hw_gpio_isr(&btled_device);
		EXTI_ClearITPendingBit(EXTI_Line9);
	}
    
    // keyboard interrupt
	if(EXTI_GetITStatus(EXTI_Line8) == SET)
	{
		rt_hw_gpio_isr(&kb_intr_device);
		EXTI_ClearITPendingBit(EXTI_Line8);
	}
	/* leave interrupt */
	rt_interrupt_leave();
}

void 
EXTI15_10_IRQHandler(void)
{
	extern void rt_hw_gpio_isr(gpio_device *gpio);

	/* enter interrupt */
	rt_interrupt_enter();
	if(EXTI_GetITStatus(EXTI_Line15) == SET)
	{
		rt_hw_gpio_isr(&break_device);
		EXTI_ClearITPendingBit(EXTI_Line15);
	}
	if(EXTI_GetITStatus(EXTI_Line14) == SET)
	{
		rt_hw_gpio_isr(&gsm_ring_device);
		EXTI_ClearITPendingBit(EXTI_Line14);
	}
	if(EXTI_GetITStatus(EXTI_Line12) == SET)
	{
		rt_hw_gpio_isr(&mag_device);
		EXTI_ClearITPendingBit(EXTI_Line12);
	}
	/* leave interrupt */
	rt_interrupt_leave();
}

static int 
rt_hw_gpio_exti_enable(void)
{
    device_enable(DEVICE_NAME_SWITCH1);
    //device_enable(DEVICE_NAME_SWITCH2);
    //device_enable(DEVICE_NAME_SWITCH3);
    device_enable(DEVICE_NAME_KB_INTR);
    device_enable(DEVICE_NAME_FP_TOUCH);
    device_enable(DEVICE_NAME_GSM_RING);
    device_enable(DEVICE_NAME_BREAK);
    device_enable(DEVICE_NAME_MAG);
    device_enable(DEVICE_NAME_HALL);
    return 0;
}
INIT_DEVICE_EXPORT(rt_hw_switch1_register);
//INIT_DEVICE_EXPORT(rt_hw_switch2_register);
INIT_DEVICE_EXPORT(rt_hw_bt_led_register);

//INIT_DEVICE_EXPORT(rt_hw_switch3_register);
INIT_DEVICE_EXPORT(rt_hw_kb_intr_register);
INIT_DEVICE_EXPORT(rt_hw_fp_touch_register);
INIT_DEVICE_EXPORT(rt_hw_gsm_ring_register);
INIT_DEVICE_EXPORT(rt_hw_break_register);
INIT_DEVICE_EXPORT(rt_hw_mag_register);
INIT_DEVICE_EXPORT(rt_hw_hall_register);
INIT_APP_EXPORT(rt_hw_gpio_exti_enable);

#ifdef RT_USING_FINSH
#include <finsh.h>
void kb_intr_test(int cnts, int delay)
{
    int i;
    for (i = 0; i < cnts; ++i) {
        if (rt_timer_start(kb_intr_user_data.timer) != -RT_EOK)
            rt_kprintf("timer start error \n");
    
        delay_us(delay);
    }
}
FINSH_FUNCTION_EXPORT(kb_intr_test, kb_intr_test[cnts]);

#endif
