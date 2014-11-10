#ifndef __PVDPROCESS_H__
#define __PVDPROCESS_H__
#include "stm32f10x.h"
#include "rtthread.h"

typedef void (*PVDCallback)(void);

void PVD_IRQCallBackSet(PVDCallback fun);

#endif

