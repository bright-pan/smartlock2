#ifndef __MENU_H__
#define __MENU_H__
#include "rtthread.h"
//#include "commfun.h"
#include "gui.h"
#include "accountop.h"
#include "sms.h"
#include "gprsmailclass.h"
#include "local.h"
#include "FprintHandle.h"
#ifdef   USEING_RAM_DEBUG
#include "untils.h" //主要使用里面的 rt_dprintf
#endif

#ifndef USEING_RAM_DEBUG
#define rt_dprintf    RT_DEBUG_LOG
#endif


#define MENU_DEBUG_THREAD        23//菜单线程调试信息
#define MENU_DEBUG_KEY           24//按键调试信息打印

#define MENU_SURE_VALUE					 '#'
#define MENU_DEL_VALUE           '*'
#define MENU_UP_VALUE						 '2'
#define MENU_DOWN_VALUE					 '8'

#define USEING_SYSTEM_SHOW_STYLE1        

#define LCD_LINE_MAX_LEN					17							//留出一个结束符的位置

#define KEY_MAX_MENU_NUM					50
#define MENU_PASSWORD_MAX_LEN			8
#define MENU_PHONE_MAX_LEN				12

#define CONFIG_PASSWORD_LEN				6	//配置文件标准密码长度

#define MENU_EVT_OP_OUTTIME				(0X01<<0)//菜单操作超时事件
#define MENU_EVT_FREEZE						(0X01<<1)//
#define MENU_EVT_FP_UNLOCK        (0X01<<2)//指纹解锁成功
#define MENU_EVT_FP_ERROR         (0X01<<3)//指纹失败
#define MENU_EVT_PH_UNLOCK       (0X01<<4)//打电话开门提示

typedef struct 
{
	rt_uint8_t StateIndex;					//当前状态索引号
	rt_uint8_t DnState;							//按下“向下”键时转向的状态索引号
	rt_uint8_t UpState;							//按下“向上”键时转向的状态索引号
	rt_uint8_t SureState;						//按下“回车”键时转向的状态索引号
	rt_uint8_t BackState;						//按下“退回”键时转向的状态索引号
	void(*CurrentOperate)(void);		//当前状态应该执行的能操作
}KbdTabStruct;

typedef struct 
{
	rt_event_t event;
}MenuManageDef;

extern MenuManageDef MenuManage;


extern rt_uint8_t KeyFuncIndex;
extern const KbdTabStruct	KeyTab[KEY_MAX_MENU_NUM];
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
void menu_operation_result_handle(rt_uint8_t type);

rt_uint8_t menu_event_process(rt_uint8_t mode,rt_uint32_t type);

//输入确定键处理
rt_err_t menu_input_sure_key(rt_uint32_t OutTime);

//管理员相关处理初始化
rt_err_t admin_account_init(void);

#endif
