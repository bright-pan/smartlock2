/*********************************************************************
 * Filename:			rf433.h
 *
 * Description:
 *
 * Author:              Bright Pan
 * Email:				bright_pan@yuettak.com
 * Date:				2014-10-14
 *
 * Modify:
 *
 * Copyright (C) 2014 Yuettak Co.,Ltd
 ********************************************************************/
#ifndef RF433_H
#define RF433_H

#include "stm32f10x.h"

#define RF433_START 0x01
#define RF433_VERIFY 0x02
#define RF433_STOP 0x03

void
send_rf433_mail(s32 cmd, u8 *data);

#endif	/* RF433_H */
