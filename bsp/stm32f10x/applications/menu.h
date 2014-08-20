#ifndef __MENU_H__
#define __MENU_H__
#include "rtthread.h"
//#include "commfun.h"
#include "gui.h"
#define KEY_MAX_MENU_NUM					50

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

void key_input_processing(void);

rt_err_t menu_key_value_acquire(rt_uint8_t *KeyValue);

#endif
