#ifndef __GUI_H__
#define __GUI_H__
#include "sh1106.h"
#include "rtthread.h"
#include "board.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//按键接口
#define KB_MAIL_TYPE_INPUT 1
#define KB_MAIL_TYPE_SETMODE 2
#define KB_MAIL_TYPE_TIMEOUT 3


typedef enum {
	KB_MODE_NORMAL_AUTH = 0,
	KB_MODE_SETTING_AUTH,
  KB_MODE_SETTING,
	KB_MODE_ADD_PASSWORD,
	KB_MODE_MODIFY_SUPERPWD,
	KB_MODE_ADD_FPRINT,
}KB_MODE_TYPEDEF;
////////////////////////////////////////////////////////////////////////////////////////////////////


#define LCD_X_MAX			128
#define LCD_Y_MAX			64

#define SHOW_X_CENTERED(A)				(LCD_X_MAX-(rt_strlen(A)*8))/2				
#define SHOW_Y_LINE(A)						A*16	

rt_err_t send_key_value_mail(uint16_t type, KB_MODE_TYPEDEF mode, uint8_t c);

//键盘输入接口
rt_err_t gui_key_input(rt_uint8_t *KeyValue);

//显示字符串
void gui_display_string(rt_uint8_t x,rt_uint8_t y,rt_uint8_t *string);

//显示更新
void gui_display_update(void);

#endif

