/*********************************************************************
 * Filename:      gpio_pin.c
 *
 * Description:
 *
 * Author:        wangzw <wangzw@yuettak.com>
 * Created at:    2013-04-22
 *
 * Modify:
 *
 * 2013-04-25 Bright Pan <loststriker@gmail.com>
 * 2013-04-27 Bright Pan <loststriker@gmail.com>
 *
 * Copyright (C) 2013 Yuettak Co.,Ltd
 ********************************************************************/

#include "gpio_pin.h"
#include "gpio_pwm.h"

struct gpio_pin_user_data
{
  const char name[RT_NAME_MAX];
  GPIO_TypeDef* gpiox;//port
  rt_uint16_t gpio_pinx;//pin
  GPIOMode_TypeDef gpio_mode;//mode
  GPIOSpeed_TypeDef gpio_speed;//speed
  rt_uint32_t gpio_clock;//clock
  rt_uint8_t gpio_default_output;
};
/*
 * gpio pin ops configure
 */
rt_err_t 
gpio_pin_configure(gpio_device *gpio)
{
  GPIO_InitTypeDef gpio_init_structure;
  struct gpio_pin_user_data *user = (struct gpio_pin_user_data*)gpio->parent.user_data;
  GPIO_StructInit(&gpio_init_structure);
  RCC_APB2PeriphClockCmd(user->gpio_clock,ENABLE);
  gpio_init_structure.GPIO_Mode = user->gpio_mode;
  gpio_init_structure.GPIO_Pin = user->gpio_pinx;
  gpio_init_structure.GPIO_Speed = user->gpio_speed;
  if (user->gpio_default_output)
  {
    GPIO_SetBits(user->gpiox,user->gpio_pinx);
  }
  else
  {
    GPIO_ResetBits(user->gpiox,user->gpio_pinx);
  }
  GPIO_Init(user->gpiox,&gpio_init_structure);

  return RT_EOK;
}

rt_err_t 
gpio_pin_control(gpio_device *gpio, rt_uint8_t cmd, void *arg)
{
	GPIO_InitTypeDef	gpio_struct;
	GPIOMode_TypeDef	gpio_mode = *(GPIOMode_TypeDef*)arg;
	struct gpio_pin_user_data *user = (struct gpio_pin_user_data*)gpio->parent.user_data;
	switch(cmd)
	{
		case GPIO_CMD_INIT_CONFIG://config gpio pin
		{
			gpio_struct.GPIO_Mode = gpio_mode;
			gpio_struct.GPIO_Pin = user->gpio_pinx;
			gpio_struct.GPIO_Speed = user->gpio_speed;
			GPIO_Init(user->gpiox,&gpio_struct);
			/* if gpio is output mode */
			if((gpio_mode != GPIO_Mode_IN_FLOATING)&&
				 (gpio_mode != GPIO_Mode_IPD)&&
				 (gpio_mode != GPIO_Mode_IPU))
			{
				gpio->parent.flag &= ~RT_DEVICE_FLAG_RDONLY;
				gpio->parent.flag |= RT_DEVICE_FLAG_WRONLY;
			}
			else
			{
				gpio->parent.flag &= ~RT_DEVICE_FLAG_WRONLY;
				gpio->parent.flag |= RT_DEVICE_FLAG_RDONLY;
			}
			break;
		}
		default:
		{
			break;
		}
	}
  return RT_EOK;
}

void 
gpio_pin_out(gpio_device *gpio, rt_uint8_t data)
{
  struct gpio_pin_user_data *user = (struct gpio_pin_user_data*)gpio->parent.user_data;
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
    rt_kprintf("gpio pin device <%s> is can`t write! please check device flag RT_DEVICE_FLAG_WRONLY\n", user->name);
#endif
  }
}

rt_uint8_t 
gpio_pin_intput(gpio_device *gpio)
{
  struct gpio_pin_user_data* user = (struct gpio_pin_user_data *)gpio->parent.user_data;

  if (gpio->parent.flag & RT_DEVICE_FLAG_RDONLY)
  {
    return GPIO_ReadInputDataBit(user->gpiox,user->gpio_pinx);
  }
  else
  {
#ifdef RT_USING_FINSH
    rt_kprintf("gpio pin device <%s> is can`t read! please check device flag RT_DEVICE_FLAG_RDONLY\n", user->name);
#endif
    return 0;
  }
}

struct rt_gpio_ops gpio_pin_user_ops=
{
  gpio_pin_configure,
  gpio_pin_control,
  gpio_pin_out,
  gpio_pin_intput
};

/**************************************************
 *           gpio pin device register
 *
 **************************************************/

/* RFID power device register
struct gpio_pin_user_data rfid_power_user_data =
{
  DEVICE_NAME_RFID_POWER,
  GPIOE,
  GPIO_Pin_11,
  GPIO_Mode_Out_PP,
  GPIO_Speed_50MHz,
  RCC_APB2Periph_GPIOE,
  0,//default value
};
gpio_device rfid_power_device;

void rt_hw_rfid_power_register(void)
{
  gpio_device *gpio_device = &rfid_power_device;
  struct gpio_pin_user_data *gpio_user_data = &rfid_power_user_data;

  gpio_device->ops = &gpio_pin_user_ops;
  rt_hw_gpio_register(gpio_device, gpio_user_data->name, (RT_DEVICE_FLAG_RDWR), gpio_user_data);
  }
*/


/* gsm led device
struct gpio_pin_user_data gsm_led_user_data =
{
  DEVICE_NAME_GSM_LED,
  GPIOE,
  GPIO_Pin_13,
  GPIO_Mode_Out_PP,
  GPIO_Speed_50MHz,
  RCC_APB2Periph_GPIOE,
  0,
};
gpio_device gsm_led_device;

void rt_hw_gsm_led_register(void)
{
  gpio_device *gpio_device = &gsm_led_device;
  struct gpio_pin_user_data *gpio_user_data = &gsm_led_user_data;

  gpio_device->ops = &gpio_pin_user_ops;
  rt_hw_gpio_register(gpio_device, gpio_user_data->name, (RT_DEVICE_FLAG_RDWR), gpio_user_data);
}
*/
/* gsm power device
struct gpio_pin_user_data gsm_power_user_data =
{
  DEVICE_NAME_GSM_POWER,
  GPIOD,
  GPIO_Pin_0,
  GPIO_Mode_Out_PP,
  GPIO_Speed_50MHz,
  RCC_APB2Periph_GPIOD,
  0,
};
gpio_device gsm_power_device;
void rt_hw_gsm_power_register(void)
{
  gpio_device *gpio_device = &gsm_power_device;
  struct gpio_pin_user_data *gpio_user_data = &gsm_power_user_data;

  gpio_device->ops = &gpio_pin_user_ops;
  rt_hw_gpio_register(gpio_device, gpio_user_data->name, (RT_DEVICE_FLAG_RDWR), gpio_user_data);
}
*/
/* gsm dtr device
struct gpio_pin_user_data gsm_dtr_user_data =
{
  DEVICE_NAME_GSM_DTR,
  GPIOC,
  GPIO_Pin_1,
  GPIO_Mode_Out_PP,
  GPIO_Speed_50MHz,
  RCC_APB2Periph_GPIOC,
  1,
};
gpio_device gsm_dtr_device;
void rt_hw_gsm_dtr_register(void)
{
  gpio_device *gpio_device = &gsm_dtr_device;
  struct gpio_pin_user_data *gpio_user_data = &gsm_dtr_user_data;

  gpio_device->ops = &gpio_pin_user_ops;
  rt_hw_gpio_register(gpio_device, gpio_user_data->name, (RT_DEVICE_FLAG_RDWR), gpio_user_data);
}
*/
/* gsm status device
struct gpio_pin_user_data gsm_status_user_data =
{
  DEVICE_NAME_GSM_STATUS,
  GPIOD,
  GPIO_Pin_7,
  GPIO_Mode_IN_FLOATING,
  GPIO_Speed_50MHz,
  RCC_APB2Periph_GPIOE,
  0,
};
gpio_device gsm_status_device;
void rt_hw_gsm_status_register(void)
{
  gpio_device *gpio_device = &gsm_status_device;
  struct gpio_pin_user_data *gpio_user_data = &gsm_status_user_data;

  gpio_device->ops = &gpio_pin_user_ops;
  rt_hw_gpio_register(gpio_device, gpio_user_data->name, (RT_DEVICE_FLAG_RDWR), gpio_user_data);
}
*/
/* voice reset device
struct gpio_pin_user_data voice_reset_user_data =
{
  DEVICE_NAME_VOICE_RESET,
  GPIOA,
  GPIO_Pin_12,
  GPIO_Mode_Out_PP,
  GPIO_Speed_50MHz,
  RCC_APB2Periph_GPIOA,
  0,
};
gpio_device voice_reset_device;

void rt_hw_voice_reset_register(void)
{
  gpio_device *gpio_device = &voice_reset_device;
  struct gpio_pin_user_data *gpio_user_data = &voice_reset_user_data;

  gpio_device->ops = &gpio_pin_user_ops;
  rt_hw_gpio_register(gpio_device, gpio_user_data->name, (RT_DEVICE_FLAG_RDWR), gpio_user_data);
}
*/
/* voice switch device
struct gpio_pin_user_data voice_switch_user_data =
{
  DEVICE_NAME_VOICE_SWITCH,
  GPIOC,
  GPIO_Pin_7,
  GPIO_Mode_Out_PP,
  GPIO_Speed_50MHz,
  RCC_APB2Periph_GPIOC,
  1,
};
gpio_device voice_switch_device;

void rt_hw_voice_switch_register(void)
{
  gpio_device *gpio_device = &voice_switch_device;
  struct gpio_pin_user_data *gpio_user_data = &voice_switch_user_data;

  gpio_device->ops = &gpio_pin_user_ops;
  rt_hw_gpio_register(gpio_device, gpio_user_data->name, (RT_DEVICE_FLAG_RDWR), gpio_user_data);
}
*/
/* voice amp device 
struct gpio_pin_user_data voice_amp_user_data =
{
  DEVICE_NAME_VOICE_AMP,
  GPIOA,
  GPIO_Pin_12,
  GPIO_Mode_Out_PP,
  GPIO_Speed_50MHz,
  RCC_APB2Periph_GPIOA,
  0,
};
gpio_device voice_amp_device;

int rt_hw_voice_amp_register(void)
{
  gpio_device *gpio_device = &voice_amp_device;
  struct gpio_pin_user_data *gpio_user_data = &voice_amp_user_data;

  gpio_device->ops = &gpio_pin_user_ops;
  rt_hw_gpio_register(gpio_device, gpio_user_data->name, (RT_DEVICE_FLAG_RDWR), gpio_user_data);

  return 0;
}
*/
/* gsm led device
struct gpio_pin_user_data test_user_data =
{
  "test",
  GPIOE,
  GPIO_Pin_13,
  GPIO_Mode_Out_PP,
  GPIO_Speed_50MHz,
  RCC_APB2Periph_GPIOE,
  0,
};
gpio_device test_device;

void rt_hw_test_register(void)
{
  gpio_device *gpio_device = &test_device;
  struct gpio_pin_user_data *gpio_user_data = &test_user_data;

  gpio_device->ops = &gpio_pin_user_ops;
  rt_hw_gpio_register(gpio_device, gpio_user_data->name, (RT_DEVICE_FLAG_RDWR), gpio_user_data);
}
*/

/* motor status1 device
gpio_device motor_status1_device;

struct gpio_pin_user_data motor_status1_user_data =
{
  DEVICE_NAME_MOTOR_STATUS1,
  GPIOD,
  GPIO_Pin_3,
  GPIO_Mode_IN_FLOATING,
  GPIO_Speed_50MHz,
  RCC_APB2Periph_GPIOD |RCC_APB2Periph_AFIO,
  1
};

int rt_hw_motor_status1_register(void)
{
	gpio_device *gpio_device = &motor_status1_device;
	struct gpio_pin_user_data *gpio_user_data = &motor_status1_user_data;

	gpio_device->ops = &gpio_pin_user_ops;
	rt_hw_gpio_register(gpio_device, gpio_user_data->name, (RT_DEVICE_FLAG_RDWR), gpio_user_data);
    return 0;
}
*/
/* motor status2 device
gpio_device motor_status2_device;

struct gpio_pin_user_data motor_status2_user_data =
{
  DEVICE_NAME_MOTOR_STATUS2,
  GPIOD,
  GPIO_Pin_2,
  GPIO_Mode_IN_FLOATING,
  GPIO_Speed_50MHz,
  RCC_APB2Periph_GPIOD |RCC_APB2Periph_AFIO,
  1
};

int rt_hw_motor_status2_register(void)
{
	gpio_device *gpio_device = &motor_status2_device;
	struct gpio_pin_user_data *gpio_user_data = &motor_status2_user_data;

	gpio_device->ops = &gpio_pin_user_ops;
	rt_hw_gpio_register(gpio_device, gpio_user_data->name, (RT_DEVICE_FLAG_RDWR), gpio_user_data);
    return 0;
}
*/
/* motor status3 device
gpio_device motor_status3_device;

struct gpio_pin_user_data motor_status3_user_data =
{
  DEVICE_NAME_MOTOR_STATUS3,
  GPIOD,
  GPIO_Pin_1,
  GPIO_Mode_IN_FLOATING,
  GPIO_Speed_50MHz,
  RCC_APB2Periph_GPIOD |RCC_APB2Periph_AFIO,
  1
};

int rt_hw_motor_status3_register(void)
{
	gpio_device *gpio_device = &motor_status3_device;
	struct gpio_pin_user_data *gpio_user_data = &motor_status3_user_data;

	gpio_device->ops = &gpio_pin_user_ops;
	rt_hw_gpio_register(gpio_device, gpio_user_data->name, (RT_DEVICE_FLAG_RDWR), gpio_user_data);
    return 0;
}
*/
/* motor status4 device
gpio_device motor_status4_device;

struct gpio_pin_user_data motor_status4_user_data =
{
  DEVICE_NAME_MOTOR_STATUS4,
  GPIOD,
  GPIO_Pin_0,
  GPIO_Mode_IN_FLOATING,
  GPIO_Speed_50MHz,
  RCC_APB2Periph_GPIOD |RCC_APB2Periph_AFIO,
  1
};

int rt_hw_motor_status4_register(void)
{
	gpio_device *gpio_device = &motor_status4_device;
	struct gpio_pin_user_data *gpio_user_data = &motor_status4_user_data;

	gpio_device->ops = &gpio_pin_user_ops;
	rt_hw_gpio_register(gpio_device, gpio_user_data->name, (RT_DEVICE_FLAG_RDWR), gpio_user_data);
    return 0;
}
*/




/* led 1 device 
struct gpio_pin_user_data led1_user_data =
{
    "led1",
    GPIOD,
    GPIO_Pin_11,
    GPIO_Mode_Out_PP,
    GPIO_Speed_50MHz,
    RCC_APB2Periph_GPIOE,
    0,
};
gpio_device led1_device;

int rt_hw_led1_register(void)
{
    gpio_device *gpio_device = &led1_device;
    struct gpio_pin_user_data *gpio_user_data = &led1_user_data;

    gpio_device->ops = &gpio_pin_user_ops;
    rt_hw_gpio_register(gpio_device, gpio_user_data->name, (RT_DEVICE_FLAG_RDWR), gpio_user_data);
    return 0;
}
*/
/* led 2 device 
struct gpio_pin_user_data led2_user_data =
{
    "led2",
    GPIOD,
    GPIO_Pin_10,
    GPIO_Mode_Out_PP,
    GPIO_Speed_50MHz,
    RCC_APB2Periph_GPIOE,
    0,
};
gpio_device led2_device;

int rt_hw_led2_register(void)
{
    gpio_device *gpio_device = &led2_device;
    struct gpio_pin_user_data *gpio_user_data = &led2_user_data;

    gpio_device->ops = &gpio_pin_user_ops;
    rt_hw_gpio_register(gpio_device, gpio_user_data->name, (RT_DEVICE_FLAG_RDWR), gpio_user_data);
    return 0;
}
*/
/* led logo device 
struct gpio_pin_user_data led_logo_user_data =
{
    DEVICE_NAME_LOGO_LED,
    GPIOD,
    GPIO_Pin_14,
    GPIO_Mode_Out_PP,
    GPIO_Speed_50MHz,
    RCC_APB2Periph_GPIOE,
    0,
};
gpio_device led_logo_device;

int rt_hw_led_logo_register(void)
{
    gpio_device *gpio_device = &led_logo_device;
    struct gpio_pin_user_data *gpio_user_data = &led_logo_user_data;

    gpio_device->ops = &gpio_pin_user_ops;
    rt_hw_gpio_register(gpio_device, gpio_user_data->name, (RT_DEVICE_FLAG_RDWR), gpio_user_data);
    return 0;
}
*/
/* fprint reset device
struct gpio_pin_user_data fprint_reset_user_data =
{
    DEVICE_NAME_FPRINT_RESET,
    GPIOA,
    GPIO_Pin_0,
    GPIO_Mode_Out_PP,
    GPIO_Speed_50MHz,
    RCC_APB2Periph_GPIOA,
    1,
};
gpio_device fprint_reset_device;

int rt_hw_fprint_reset_register(void)
{
    gpio_device *gpio_device = &fprint_reset_device;
    struct gpio_pin_user_data *gpio_user_data = &fprint_reset_user_data;

    gpio_device->ops = &gpio_pin_user_ops;
    rt_hw_gpio_register(gpio_device, gpio_user_data->name, (RT_DEVICE_FLAG_RDWR), gpio_user_data);
    return 0;
}
*/
/* fprint wakeup device
struct gpio_pin_user_data fprint_wakeup_user_data =
{
    DEVICE_NAME_FPRINT_WAKEUP,
    GPIOA,
    GPIO_Pin_1,
    GPIO_Mode_Out_PP,
    GPIO_Speed_50MHz,
    RCC_APB2Periph_GPIOA,
    0,
};
gpio_device fprint_wakeup_device;

int rt_hw_fprint_wakeup_register(void)
{
    gpio_device *gpio_device = &fprint_wakeup_device;
    struct gpio_pin_user_data *gpio_user_data = &fprint_wakeup_user_data;

    gpio_device->ops = &gpio_pin_user_ops;
    rt_hw_gpio_register(gpio_device, gpio_user_data->name, (RT_DEVICE_FLAG_RDWR), gpio_user_data);
    return 0;
}
*/
/* camera power device 
struct gpio_pin_user_data camera_power_user_data =
{
    DEVICE_NAME_CAMERA_POWER,
    GPIOC,
    GPIO_Pin_12,
    GPIO_Mode_Out_PP,
    GPIO_Speed_50MHz,
    RCC_APB2Periph_GPIOC,
    1,
};
gpio_device camera_power_device;

int rt_hw_camera_power_register(void)
{
    gpio_device *gpio_device = &camera_power_device;
    struct gpio_pin_user_data *gpio_user_data = &camera_power_user_data;

    gpio_device->ops = &gpio_pin_user_ops;
    rt_hw_gpio_register(gpio_device, gpio_user_data->name, (RT_DEVICE_FLAG_RDWR), gpio_user_data);
    return 0;
}
*/

/* flash power device */
struct gpio_pin_user_data flash_power_user_data =
{
    DEVICE_NAME_FLASH_POWER,
    GPIOB,
    GPIO_Pin_1,
    GPIO_Mode_Out_PP,
    GPIO_Speed_50MHz,
    RCC_APB2Periph_GPIOB,
    0,
};
gpio_device flash_power_device;

int 
rt_hw_flash_power_register(void)
{
    gpio_device *gpio_device = &flash_power_device;
    struct gpio_pin_user_data *gpio_user_data = &flash_power_user_data;

    gpio_device->ops = &gpio_pin_user_ops;
    rt_hw_gpio_register(gpio_device, gpio_user_data->name, (RT_DEVICE_FLAG_RDWR), gpio_user_data);
    return 0;
}

/* motor power device */
struct gpio_pin_user_data motor_power_user_data =
{
    DEVICE_NAME_MOTOR_POWER,
    GPIOB,
    GPIO_Pin_2,
    GPIO_Mode_Out_PP,
    GPIO_Speed_50MHz,
    RCC_APB2Periph_GPIOB,
    0,
};
gpio_device motor_power_device;

int 
rt_hw_motor_power_register(void)
{
    gpio_device *gpio_device = &motor_power_device;
    struct gpio_pin_user_data *gpio_user_data = &motor_power_user_data;

    gpio_device->ops = &gpio_pin_user_ops;
    rt_hw_gpio_register(gpio_device, gpio_user_data->name, (RT_DEVICE_FLAG_RDWR), gpio_user_data);
    return 0;
}

/* bt power device */
struct gpio_pin_user_data bt_power_user_data =
{
    DEVICE_NAME_BT_POWER,
    GPIOE,
    GPIO_Pin_7,
    GPIO_Mode_Out_PP,
    GPIO_Speed_50MHz,
    RCC_APB2Periph_GPIOE,
    0,
};
gpio_device bt_power_device;

int 
rt_hw_bt_power_register(void)
{
    gpio_device *gpio_device = &bt_power_device;
    struct gpio_pin_user_data *gpio_user_data = &bt_power_user_data;

    gpio_device->ops = &gpio_pin_user_ops;
    rt_hw_gpio_register(gpio_device, gpio_user_data->name, (RT_DEVICE_FLAG_RDWR), gpio_user_data);
    return 0;
}

/* front power device */
struct gpio_pin_user_data front_power_user_data =
{
    DEVICE_NAME_FRONT_POWER,
    GPIOE,
    GPIO_Pin_8,
    GPIO_Mode_Out_PP,
    GPIO_Speed_50MHz,
    RCC_APB2Periph_GPIOE,
    1,
};
gpio_device front_power_device;

int 
rt_hw_front_power_register(void)
{
    gpio_device *gpio_device = &front_power_device;
    struct gpio_pin_user_data *gpio_user_data = &front_power_user_data;

    gpio_device->ops = &gpio_pin_user_ops;
    rt_hw_gpio_register(gpio_device, gpio_user_data->name, (RT_DEVICE_FLAG_RDWR), gpio_user_data);
    return 0;
}

/* gsm power device */
struct gpio_pin_user_data gsm_power_user_data =
{
    DEVICE_NAME_GSM_POWER,
    GPIOE,
    GPIO_Pin_9,
    GPIO_Mode_Out_PP,
    GPIO_Speed_50MHz,
    RCC_APB2Periph_GPIOE,
    0,
};
gpio_device gsm_power_device;

int 
rt_hw_gsm_power_register(void)
{
    gpio_device *gpio_device = &gsm_power_device;
    struct gpio_pin_user_data *gpio_user_data = &gsm_power_user_data;

    gpio_device->ops = &gpio_pin_user_ops;
    rt_hw_gpio_register(gpio_device, gpio_user_data->name, (RT_DEVICE_FLAG_RDWR), gpio_user_data);
    return 0;
}

/* kb_in1 device */
gpio_device kb_in1_device;

struct gpio_pin_user_data kb_in1_user_data =
{
    DEVICE_NAME_KB_IN1,
    GPIOD,
    GPIO_Pin_14,
    GPIO_Mode_IPU,
    GPIO_Speed_50MHz,
    RCC_APB2Periph_GPIOD,
	0
};

int 
rt_hw_kb_in1_register(void)
{
    gpio_device *gpio_device = &kb_in1_device;
    struct gpio_pin_user_data *gpio_user_data = &kb_in1_user_data;

    gpio_device->ops = &gpio_pin_user_ops;

    rt_hw_gpio_register(gpio_device, gpio_user_data->name, (RT_DEVICE_FLAG_RDWR), gpio_user_data);
    return 0;
}

/* kb_in2 device */
gpio_device kb_in2_device;

struct gpio_pin_user_data kb_in2_user_data =
{
    DEVICE_NAME_KB_IN2,
    GPIOD,
    GPIO_Pin_13,
    GPIO_Mode_IPU,
    GPIO_Speed_50MHz,
    RCC_APB2Periph_GPIOD,
	0
};

int 
rt_hw_kb_in2_register(void)
{
    gpio_device *gpio_device = &kb_in2_device;
    struct gpio_pin_user_data *gpio_user_data = &kb_in2_user_data;

    gpio_device->ops = &gpio_pin_user_ops;

    rt_hw_gpio_register(gpio_device, gpio_user_data->name, (RT_DEVICE_FLAG_RDWR), gpio_user_data);
    return 0;
}

/* kb_in3 device */
gpio_device kb_in3_device;

struct gpio_pin_user_data kb_in3_user_data =
{
    DEVICE_NAME_KB_IN3,
    GPIOD,
    GPIO_Pin_12,
    GPIO_Mode_IPU,
    GPIO_Speed_50MHz,
    RCC_APB2Periph_GPIOD,
	1
};

int 
rt_hw_kb_in3_register(void)
{
    gpio_device *gpio_device = &kb_in3_device;
    struct gpio_pin_user_data *gpio_user_data = &kb_in3_user_data;

    gpio_device->ops = &gpio_pin_user_ops;

    rt_hw_gpio_register(gpio_device, gpio_user_data->name, (RT_DEVICE_FLAG_RDWR), gpio_user_data);
    return 0;
}

/* kb_scan1 device */
gpio_device kb_scan1_device;

struct gpio_pin_user_data kb_scan1_user_data =
{
    DEVICE_NAME_KB_SC1,
    GPIOD,
    GPIO_Pin_11,
    GPIO_Mode_Out_PP,
    GPIO_Speed_50MHz,
    RCC_APB2Periph_GPIOD,
	0
};

int 
rt_hw_kb_scan1_register(void)
{
    gpio_device *gpio_device = &kb_scan1_device;
    struct gpio_pin_user_data *gpio_user_data = &kb_scan1_user_data;

    gpio_device->ops = &gpio_pin_user_ops;

    rt_hw_gpio_register(gpio_device, gpio_user_data->name, (RT_DEVICE_FLAG_RDWR), gpio_user_data);
    return 0;
}

/* kb_scan2 device */
gpio_device kb_scan2_device;

struct gpio_pin_user_data kb_scan2_user_data =
{
    DEVICE_NAME_KB_SC2,
    GPIOD,
    GPIO_Pin_10,
    GPIO_Mode_Out_PP,
    GPIO_Speed_50MHz,
    RCC_APB2Periph_GPIOD,
	0
};

int 
rt_hw_kb_scan2_register(void)
{
    gpio_device *gpio_device = &kb_scan2_device;
    struct gpio_pin_user_data *gpio_user_data = &kb_scan2_user_data;

    gpio_device->ops = &gpio_pin_user_ops;

    rt_hw_gpio_register(gpio_device, gpio_user_data->name, (RT_DEVICE_FLAG_RDWR), gpio_user_data);
    return 0;
}

/* kb_scan3 device */
gpio_device kb_scan3_device;

struct gpio_pin_user_data kb_scan3_user_data =
{
    DEVICE_NAME_KB_SC3,
    GPIOD,
    GPIO_Pin_9,
    GPIO_Mode_Out_PP,
    GPIO_Speed_50MHz,
    RCC_APB2Periph_GPIOD,
	0
};

int 
rt_hw_kb_scan3_register(void)
{
    gpio_device *gpio_device = &kb_scan3_device;
    struct gpio_pin_user_data *gpio_user_data = &kb_scan3_user_data;

    gpio_device->ops = &gpio_pin_user_ops;

    rt_hw_gpio_register(gpio_device, gpio_user_data->name, (RT_DEVICE_FLAG_RDWR), gpio_user_data);
    return 0;
}

/* camera usart tx pin input */
/*gpio_device camera_TX_device;

struct gpio_pin_user_data camera_TX_user_data =
{
    DEVICE_NAME_CAMERA_USART_TX,
    GPIOC,
    GPIO_Pin_12,
    GPIO_Mode_IN_FLOATING,
    GPIO_Speed_50MHz,
    RCC_APB2Periph_GPIOC|RCC_APB2Periph_AFIO,
	0
};

int rt_hw_camera_usart_tx(void)
{
    gpio_device *gpio_device = &camera_TX_device;
    struct gpio_pin_user_data *gpio_user_data = &camera_TX_user_data;

    gpio_device->ops = &gpio_pin_user_ops;

    rt_hw_gpio_register(gpio_device, gpio_user_data->name, (RT_DEVICE_FLAG_RDWR), gpio_user_data);
    return 0;
}

// camera photosensor device
gpio_device camera_photosensor_device;

struct gpio_pin_user_data camera_photosensor_user_data =
{
  DEVICE_NAME_CAMERA_PHOTOSENSOR,
  GPIOB,
  GPIO_Pin_0,
  GPIO_Mode_IN_FLOATING,
  GPIO_Speed_50MHz,
  RCC_APB2Periph_GPIOB |RCC_APB2Periph_AFIO,
	1
};

void rt_hw_camera_photosensor_register(void)
{
  gpio_device *gpio_device = &camera_photosensor_device;
  struct gpio_pin_user_data *gpio_user_data = &camera_photosensor_user_data;

  gpio_device->ops = &gpio_pin_user_ops;

  rt_hw_gpio_register(gpio_device, gpio_user_data->name, (RT_DEVICE_FLAG_RDWR), gpio_user_data);
}

*/

__INLINE void 
gpio_pin_output(char *str, const rt_uint8_t dat, rt_uint8_t debug)
{
    rt_device_t device = RT_NULL;
    device = rt_device_find(str);
    if (device != RT_NULL)
    {
        if (device->open_flag == RT_DEVICE_OFLAG_CLOSE)
        {
            rt_device_open(device, RT_DEVICE_OFLAG_RDWR);
        }
        rt_device_write(device,0,&dat,0);
    }
    else
    {
#ifdef RT_USING_FINSH
        RT_DEBUG_LOG(debug, ("the gpio device %s is not found!\n", str));
#endif
    }
}

__INLINE uint8_t 
gpio_pin_input(char *str, rt_uint8_t debug)
{
    rt_device_t device = RT_NULL;
    rt_uint8_t dat;
    device = rt_device_find(str);
    if (device != RT_NULL)
    {
        if (device->open_flag == RT_DEVICE_OFLAG_CLOSE)
        {
            rt_device_open(device, RT_DEVICE_OFLAG_RDWR);
        }
        rt_device_read(device,0,&dat,0);
#ifdef RT_USING_FINSH
        RT_DEBUG_LOG(debug, ("the gpio pin device <%s> value is %d\n", str, dat));
#endif
    }
    else
    {
#ifdef RT_USING_FINSH
        RT_DEBUG_LOG(debug, ("the gpio device %s is not found!\n", str));
#endif
    }
    return dat;
}
/*
INIT_DEVICE_EXPORT(rt_hw_led1_register);
INIT_DEVICE_EXPORT(rt_hw_led2_register);
INIT_DEVICE_EXPORT(rt_hw_led_logo_register);
INIT_DEVICE_EXPORT(rt_hw_voice_amp_register);
INIT_DEVICE_EXPORT(rt_hw_camera_power_register);
INIT_DEVICE_EXPORT(rt_hw_motor_status1_register);
INIT_DEVICE_EXPORT(rt_hw_motor_status2_register);
INIT_DEVICE_EXPORT(rt_hw_motor_status3_register);
INIT_DEVICE_EXPORT(rt_hw_motor_status4_register);

INIT_DEVICE_EXPORT(rt_hw_fprint_reset_register);
INIT_DEVICE_EXPORT(rt_hw_fprint_wakeup_register);
*/

INIT_DEVICE_EXPORT(rt_hw_flash_power_register);
INIT_DEVICE_EXPORT(rt_hw_motor_power_register);
INIT_DEVICE_EXPORT(rt_hw_bt_power_register);
INIT_DEVICE_EXPORT(rt_hw_front_power_register);
INIT_DEVICE_EXPORT(rt_hw_gsm_power_register);

INIT_DEVICE_EXPORT(rt_hw_kb_in1_register);
INIT_DEVICE_EXPORT(rt_hw_kb_in2_register);
INIT_DEVICE_EXPORT(rt_hw_kb_in3_register);
INIT_DEVICE_EXPORT(rt_hw_kb_scan1_register);
INIT_DEVICE_EXPORT(rt_hw_kb_scan2_register);
INIT_DEVICE_EXPORT(rt_hw_kb_scan3_register);

/*
  INIT_DEVICE_EXPORT(rt_hw_rtc_init);
*/

#ifdef RT_USING_FINSH
#include <finsh.h>
/*
void led(const char *str, const rt_uint8_t dat)
{
  rt_device_t led = RT_NULL;
  led = rt_device_find(str);
  if (led != RT_NULL)
  {
    rt_device_write(led,0,&dat,0);
  }
  else
  {
#ifdef RT_USING_FINSH
    rt_kprintf("the led device <%s>is not found!\n", str);
#endif
  }
}
//FINSH_FUNCTION_EXPORT(led, led[device_name 0/1])
*/
FINSH_FUNCTION_EXPORT(gpio_pin_output, [device_name <0 1>])
FINSH_FUNCTION_EXPORT(gpio_pin_input, [device_name result])
#endif
