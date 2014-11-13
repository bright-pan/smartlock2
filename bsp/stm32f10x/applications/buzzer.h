#ifndef __BUZZEER_H__
#define __BUZZEER_H__
#include "gpio_pwm.h"

typedef enum
{
	BZ_TYPE_KEY = 0x01,
	BZ_TYPE2,
	BZ_TYPE_LOCK,
	BZ_TYPE_UNLOCK,
	BZ_TYPE_INIT,
	BZ_TYPE_INPUT_ERROR,
	BZ_TYPE_KEY_ERROR,
	BZ_TYPE_OPOK,
	BZ_TYPE_RF433_STRART,
}BuzzerType;

rt_err_t buzzer_send_mail(BuzzerType type);
#endif

