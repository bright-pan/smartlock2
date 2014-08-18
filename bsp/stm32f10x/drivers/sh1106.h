/*********************************************************************
 * Filename:      sh1106.h
 *
 * Description:
 *
 * Author:        Bright Pan <loststriker@gmail.com>
 * Created at:    2014-08-14
 *
 * Modify:
 *
 *
 * Copyright (C) 2014 Yuettak Co.,Ltd
 ********************************************************************/

#ifndef _SH1106_H_
#define _SH1106_H_
#include "stm32f10x.h"

void
lcd_display_string(u8 , u8 , u8 *, u8);

void
lcd_display_chinese(u8, u8, u8 *, u8);

void
lcd_display(u8, u8, u8, u8);

void
lcd_clear(u8, u8, u8, u8);

void
lcd_inverse(u8, u8, u8, u8);

void
lcd_display_logo(void);

void
lcd_display_bmp(u8, u8, u8, u8 *, u8);

#endif
