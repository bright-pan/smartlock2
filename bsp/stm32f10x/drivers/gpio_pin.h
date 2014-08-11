/*********************************************************************
 * Filename:      gpio_pin.h
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

#ifndef _GPIO_PIN_H_
#define _GPIO_PIN_H_

#include <rthw.h>
#include <rtthread.h>
#include "stm32f10x.h"
#include "gpio.h"

#define GPIO_CMD_INIT_CONFIG						1

/*
#define DEVICE_NAME_FPRINT_RESET "fp_rst"
#define DEVICE_NAME_FPRINT_WAKEUP "fp_wkup"

#define DEVICE_NAME_GSM_POWER "g_power"
#define DEVICE_NAME_GSM_STATUS "g_stat"
#define DEVICE_NAME_GSM_LED "g_led"
#define DEVICE_NAME_GSM_DTR "g_dtr"

#define DEVICE_NAME_VOICE_RESET "vo_rst"
#define DEVICE_NAME_VOICE_SWITCH "vo_sw"
#define DEVICE_NAME_VOICE_AMP "vo_amp"


#define DEVICE_NAME_CAMERA_POWER "cm_power"
#define DEVICE_NAME_CAMERA_PHOTOSENSOR "cm_photo"
#define DEVICE_NAME_CAMERA_USART_TX	"cm_txpin"

#define DEVICE_NAME_RFID_POWER "rf_power"

#define DEVICE_NAME_MOTOR_STATUS1 "mt_stat1"
#define DEVICE_NAME_MOTOR_STATUS2 "mt_stat2"
#define DEVICE_NAME_MOTOR_STATUS3 "mt_stat3"
#define DEVICE_NAME_MOTOR_STATUS4 "mt_stat4"
*/

#define DEVICE_NAME_FLASH_POWER "pwr_flh"
#define DEVICE_NAME_MOTOR_POWER "pwr_mt"
#define DEVICE_NAME_BT_POWER "pwr_bt"
#define DEVICE_NAME_FRONT_POWER "pwr_frt"
#define DEVICE_NAME_GSM_POWER "pwr_gsm"

#define DEVICE_NAME_KB_IN1 "kb_in1"
#define DEVICE_NAME_KB_IN2 "kb_in2"
#define DEVICE_NAME_KB_IN3 "kb_in3"

#define DEVICE_NAME_KB_SC1 "kb_sc1"
#define DEVICE_NAME_KB_SC2 "kb_sc2"
#define DEVICE_NAME_KB_SC3 "kb_sc3"
#define DEVICE_NAME_KB_SC4 "kb_sc4"

__INLINE uint8_t 
gpio_pin_input(char *str, rt_uint8_t debug);

__INLINE void 
gpio_pin_output(char *str, const rt_uint8_t dat, rt_uint8_t debug);

#endif
