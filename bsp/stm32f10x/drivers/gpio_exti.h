/*********************************************************************
 * Filename:      gpio_exti.h
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

#ifndef _GPIO_EXTI_H_
#define _GPIO_EXTI_H_

#include <rthw.h>
#include <rtthread.h>
#include <stm32f10x.h>
#include "gpio.h"
//#include "alarm.h"

//#define DEVICE_NAME_LOCK_SHELL "lk_shell"// length <= 8
//#define DEVICE_NAME_LOCK_TEMPERATURE "lk_temp"
//#define DEVICE_NAME_GATE_TEMPERATURE "gt_temp"
//#define DEVICE_NAME_LOCK_GATE "lk_gate"

//#define DEVICE_NAME_CAMERA_COVER "cm_cover"
//#define DEVICE_NAME_CAMERA_IRDASENSOR "cm_irda"

//#define DEVICE_NAME_GSM_RING "g_ring"

//#define DEVICE_NAME_MOTOR_STATUS "mt_stat"

#define DEVICE_NAME_KEY_DETECT "k_det"

//#define DEVICE_NAME_BATTERY_SWITCH "bat_sw" /* BATTERY DEVICE NAME */

//#define DEVICE_NAME_BUTTON_ADJUST_IR "adjustIR"

#define RT_DEVICE_CTRL_MASK_EXTI 0x15    /* mask exti */
#define RT_DEVICE_CTRL_UNMASK_EXTI  0x16    /* unmask exti */

#define KEY_EXTI_TRIGGER_MODE EXTI_Trigger_Rising
#define KEY_DETECT_STATUS 1
#define KEY_INT_INTERVAL 100

//#define GSM_RING_EXTI_TRIGGER_MODE EXTI_Trigger_Rising
//#define GSM_RING_DETECT_STATUS 1
//#define GSM_RING_INT_INTERVAL 1

//#define LOCK_GATE_EXTI_TRIGGER_MODE EXTI_Trigger_Rising
//#define LOCK_GATE_DETECT_STATUS 1
//#define LOCK_GATE_INT_INTERVAL 100

//#define LOCK_SHELL_EXTI_TRIGGER_MODE EXTI_Trigger_Rising
//#define LOCK_SHELL_DETECT_STATUS 1
//#define LOCK_SHELL_INT_INTERVAL 100

//#define GATE_TEMPERATURE_EXTI_TRIGGER_MODE EXTI_Trigger_Rising
//#define GATE_TEMPERATURE_DETECT_STATUS 1
//#define GATE_TEMPERATURE_INT_INTERVAL 100

//#define LOCK_TEMPERATURE_EXTI_TRIGGER_MODE EXTI_Trigger_Rising
//#define LOCK_TEMPERATURE_DETECT_STATUS 1
//#define LOCK_TEMPERATURE_INT_INTERVAL 100
//  
//#define BATTERY_SWITCH_EXTI_TRIGGER_MODE EXTI_Trigger_Rising_Falling
//#define BATTERY_SWITCH_DETECT_STATUS 1
//#define BATTERY_SWITCH_INT_INTERVAL 10

//#ifdef USE_BUTTON_ADJUST_IR
//#define BUTTON_ADJUST_IR_EXTI_TRIGGER_MODE EXTI_Trigger_Rising_Falling
//#define BUTTON_ADJUST_IR_DETECT_STATUS 1
//#define BUTTON_ADJUST_IR_INT_INTERVAL 10
//#endif

//void rt_hw_lock_shell_register(void);
//void rt_hw_lock_temperature_register(void);
//void rt_hw_lock_gate_register(void);

//void rt_hw_gate_temperature_register(void);

//void rt_hw_camera_photosensor_register(void);
//void rt_hw_camera_irdasensor_register(void);

//void rt_hw_rfid_key_detect_register(void);

//void rt_hw_gsm_ring_register(void);

//void rt_hw_battery_switch_register(void);
//    
//void rt_hw_key2_register(void);

//void rt_hw_button_adjust_ir_register(void);


#endif
