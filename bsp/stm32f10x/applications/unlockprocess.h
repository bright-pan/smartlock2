#ifndef __UNLOCKPROCESS_H__
#define __UNLOCKPROCESS_H__
#include "rtthread.h"
#include "voice.h"
#include "gpio_pwm.h"
#include "local.h"

typedef struct 
{
	rt_uint16_t KeyMapPos;
}FPrintData;

void fprint_unlock_process(LOCAL_MAIL_TYPEDEF *mail);

void send_fprint_dat_mail(FPrintData *data);

void fprint_key_add(LOCAL_MAIL_TYPEDEF *mail);

rt_bool_t motor_status(void);

rt_bool_t motor_rotate(rt_bool_t direction);

rt_err_t fprint_module_init(void);

void motor_auto_lock(void);


#endif

