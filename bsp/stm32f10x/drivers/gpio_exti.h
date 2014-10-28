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
#include "alarm.h"

//#define DEVICE_NAME_LOCK_SHELL "lk_shell"// length <= 8
//#define DEVICE_NAME_LOCK_TEMPERATURE "lk_temp"
//#define DEVICE_NAME_GATE_TEMPERATURE "gt_temp"
//#define DEVICE_NAME_LOCK_GATE "lk_gate"

//#define DEVICE_NAME_CAMERA_COVER "cm_cover"
//#define DEVICE_NAME_CAMERA_IRDASENSOR "cm_irda"

//#define DEVICE_NAME_GSM_RING "g_ring"

//#define DEVICE_NAME_MOTOR_STATUS "mt_stat"

#define DEVICE_NAME_SWITCH1 "sw1"
#define DEVICE_NAME_SWITCH2 "sw2"
//#define DEVICE_NAME_SWITCH3 "sw3"
#define DEVICE_NAME_BREAK "brk"
#define DEVICE_NAME_MAG "mag"
#define DEVICE_NAME_KB_INTR "kb_intr"

#define DEVICE_NAME_FP_TOUCH "fp_t"
#define DEVICE_NAME_GSM_RING "g_ring"
#define DEVICE_NAME_HALL "hall"
#define DEVICE_NAME_BT_LED "BT_LED"
//#define DEVICE_NAME_BATTERY_SWITCH "bat_sw" /* BATTERY DEVICE NAME */

//#define DEVICE_NAME_BUTTON_ADJUST_IR "adjustIR"

#define RT_DEVICE_CTRL_MASK_EXTI 0x15    /* mask exti */
#define RT_DEVICE_CTRL_UNMASK_EXTI  0x16    /* unmask exti */

#define SWITCH1_EXTI_TRIGGER_MODE EXTI_Trigger_Falling
#define SWITCH1_STATUS 0
#define SWITCH1_INT_INTERVAL 100

#define SWITCH2_EXTI_TRIGGER_MODE EXTI_Trigger_Falling
#define SWITCH2_STATUS 0
#define SWITCH2_INT_INTERVAL 100

/*
#define SWITCH3_EXTI_TRIGGER_MODE EXTI_Trigger_Rising
#define SWITCH3_STATUS 1
#define SWITCH3_INT_INTERVAL 100
*/
#define KB_INTR_EXTI_TRIGGER_MODE EXTI_Trigger_Falling
#define KB_INTR_STATUS 0
#define KB_INTR_INT_INTERVAL 10

#define FP_TOUCH_EXTI_TRIGGER_MODE EXTI_Trigger_Rising
#define FP_TOUCH_STATUS 1
#define FP_TOUCH_INT_INTERVAL 10

#define BREAK_EXTI_TRIGGER_MODE EXTI_Trigger_Rising
#define BREAK_STATUS 1
#define BREAK_INT_INTERVAL 100

#define MAG_EXTI_TRIGGER_MODE EXTI_Trigger_Rising
#define MAG_STATUS 1
#define MAG_INT_INTERVAL 100

#define GSM_RING_EXTI_TRIGGER_MODE EXTI_Trigger_Falling
#define GSM_RING_STATUS 0
#define GSM_RING_INT_INTERVAL 100

#define HALL_EXTI_TRIGGER_MODE EXTI_Trigger_Rising
#define HALL_STATUS 1
#define HALL_INT_INTERVAL 100

#define BT_LED_EXTI_TRIGGER_MODE EXTI_Trigger_Falling
#define BT_LED_STATUS 0
#define BT_LED_INT_INTERVAL 100

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

#endif
