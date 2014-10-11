#ifndef __MENU_H__
#define __MENU_H__
#include "rtthread.h"
//#include "commfun.h"
#include "gui.h"
#include"accountop.h"

#define LCD_LINE_MAX_LEN					17							//留出一个结束符的位置

#define KEY_MAX_MENU_NUM					50
#define MENU_PASSWORD_MAX_LEN			8
#define MENU_PHONE_MAX_LEN				12

#define CONFIG_PASSWORD_LEN				6	//配置文件标准密码长度

typedef struct 
{
	rt_uint8_t StateIndex;					//当前状态索引号
	rt_uint8_t DnState;							//按下“向下”键时转向的状态索引号
	rt_uint8_t UpState;							//按下“向上”键时转向的状态索引号
	rt_uint8_t SureState;						//按下“回车”键时转向的状态索引号
	rt_uint8_t BackState;						//按下“退回”键时转向的状态索引号
	void(*CurrentOperate)(void);		//当前状态应该执行的能操作
}KbdTabStruct;

extern rt_uint8_t KeyFuncIndex;
extern KbdTabStruct	KeyTab[KEY_MAX_MENU_NUM];
extern void(*current_operation_index)(void);

void key_input_processing(void);

rt_err_t menu_key_value_acquire(rt_uint8_t *KeyValue);

void menu_run_sure_process(void);

rt_err_t string_add_char(rt_uint8_t str[],rt_uint8_t ch,rt_uint8_t str_size);

rt_err_t string_del_char(rt_uint8_t str[],rt_uint8_t str_size);

void string_hide_string(const rt_uint8_t src[],rt_uint8_t str[],rt_uint8_t ch,rt_uint8_t size);

void menu_inputchar_glint(rt_uint8_t x,rt_uint8_t y,rt_uint8_t status);

void key_input_processing_init(void);

void system_menu_choose(rt_uint8_t menu);

//错误处理
void menu_error_handle(rt_uint8_t type);
#endif
