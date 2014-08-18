#ifndef __MENU_H__
#define __MENU_H__
#include "rtthread.h"
#include "commfun_t.h"
#include "board.h"

#define KB_MAIL_TYPE_INPUT 1
#define KB_MAIL_TYPE_SETMODE 2
#define KB_MAIL_TYPE_TIMEOUT 3


////////////////////////////////////////////////////////////////////////////////////////////////////
typedef enum {
	KB_MODE_NORMAL_AUTH = 0,
	KB_MODE_SETTING_AUTH,
    KB_MODE_SETTING,
	KB_MODE_ADD_PASSWORD,
	KB_MODE_MODIFY_SUPERPWD,
	KB_MODE_ADD_FPRINT,
}KB_MODE_TYPEDEF;
////////////////////////////////////////////////////////////////////////////////////////////////////

rt_err_t send_key_value_mail(uint16_t type, KB_MODE_TYPEDEF mode, uint8_t c);


void key_input_processing(void);

#endif
