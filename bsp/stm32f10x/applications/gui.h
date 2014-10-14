#ifndef __GUI_H__
#define __GUI_H__
#include "sh1106.h"
#include "rtthread.h"
#include "board.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//�����ӿ�
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

#define GUI_WIHIT			0
#define GUI_BLACK			1

#define SHOW_X_CENTERED(A)				(((LCD_X_MAX-(rt_strlen(A)*8))/2))			
#define SHOW_Y_LINE(A)						((A)*16)	
#define SHOW_X_ROW8(A)						((A)*8)
#define SHOW_X_ROW16(A)						((A)*16)



rt_err_t send_key_value_mail(uint16_t type, KB_MODE_TYPEDEF mode, uint8_t c);

//��������ӿ�
rt_err_t gui_key_input(rt_uint8_t *KeyValue);

//��ʾ�ַ���
void gui_display_string(rt_uint8_t x,rt_uint8_t y,const rt_uint8_t *string,rt_uint8_t color);

//����
void gui_clear(rt_uint8_t x1,rt_uint8_t y1,rt_uint8_t x2,rt_uint8_t y2);

//��ʾ����
void gui_display_update(void);

//����
void gui_line(rt_uint8_t x1,rt_uint8_t y1,rt_uint8_t x2,rt_uint8_t y2,rt_uint8_t color);  

//������
void gui_box(rt_uint8_t x0, rt_uint8_t y0, rt_uint8_t x1, rt_uint8_t y1,rt_uint8_t color,rt_uint8_t fill);

void gui_china16s(rt_uint8_t x, rt_uint8_t y, rt_uint8_t *s, rt_uint8_t fColor);

#endif

