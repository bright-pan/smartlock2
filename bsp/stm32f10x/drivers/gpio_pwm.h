/*********************************************************************
 * Filename:      gpio_pwm.h
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

#ifndef _GPIO_PWM_H_
#define _GPIO_PWM_H_

#include <rthw.h>
#include <rtthread.h>
#include <stm32f10x.h>
#include "gpio.h"

#define RT_DEVICE_CTRL_SEND_PULSE       0x14    /* enable receive irq */
#define RT_DEVICE_CTRL_SET_PULSE_COUNTS 0x15    /* disable receive irq */
#define RT_DEVICE_CTRL_SET_PULSE_VALUE  0x16    /* disable receive irq */
#define RT_DEVICE_CTRL_CONFIG_DEVICE    0x17    /* Reconfigure the device */


#define RT_DEVICE_FLAG_PWM_TX           0x1000 /* flag mask for gpio pwm mode */
//#define RT_DEVICE_FLAG_ONE_PULSE        0x2000
#define DEVICE_NAME_MOTOR1 "mt1"
#define DEVICE_NAME_MOTOR2 "mt2"
#define DEVICE_NAME_LOGO_LED "logo"

#define DEVICE_NAME_SPEAK "speak"

extern rt_mutex_t motor_action_mutex;

#ifdef USE_OLD_VOICE_IC
void voice_output(rt_uint16_t counts);
#else
void voice_output(rt_uint16_t counts ,rt_uint16_t	delay);
#endif
#define GATE_LOCK 0
#define GATE_UNLOCK 1
int8_t lock_output(uint8_t direction);

#endif
