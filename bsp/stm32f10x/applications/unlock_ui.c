#include "unlock_ui.h"
#include "menu.h"
#include "local.h"
#include "gprsmailclass.h"

#define PAGE_MAX_SHOW_NUM					4

#define LCD_LINE_MAX_LEN					17							//留出一个结束符的位置
#define SHOW_PW_HIDE_CH						'*'
#define PASSWORD_DATA_POS					6								//密码数据的开始位置

static const rt_uint8_t UNLOCK_UI_TEXT[][LCD_LINE_MAX_LEN] =
{
	{"请输入密码"},
	{"密码 :"},
	{"密码错误"},
	{"开锁成功"},
	{"按返回键退出"}
};

static const rt_uint8_t SystemEnterMenuText[][LCD_LINE_MAX_LEN] = 
{
	{"开>>>>锁"},
	{"系统管理"},
};

//指纹解锁UI
static const rt_uint8_t FpUnlockUIText[][LCD_LINE_MAX_LEN] = 
{
	{"指纹解锁成功!"},
	{"指纹错误"},
};

static const rt_uint8_t PhUnlockUIText[][LCD_LINE_MAX_LEN] = 
{
 {"已通知主人"},
};

#define SYSTEM_ENTER_MENU_NUM						2	//菜单的最大个数
static void system_enter_menu_ui(rt_uint8_t InPOS)
{
	rt_uint8_t page;
	rt_uint8_t pos;
	rt_uint8_t i;

	page = InPOS /PAGE_MAX_SHOW_NUM;//计算显示在那一页
	pos = InPOS % PAGE_MAX_SHOW_NUM;//当前选中的位置
	gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
	for(i=0;i<PAGE_MAX_SHOW_NUM;i++)
	{
		if(page*PAGE_MAX_SHOW_NUM+i >= SYSTEM_ENTER_MENU_NUM)
		{
			break;
		}
		if(i == pos)
		{
			gui_display_string(SHOW_X_CENTERED((const char*)SystemEnterMenuText[page*PAGE_MAX_SHOW_NUM+i]),
												 SHOW_Y_LINE(i+1),
												 (rt_uint8_t *)SystemEnterMenuText[page*PAGE_MAX_SHOW_NUM+i],
												 GUI_BLACK);
		}
		else
		{
			gui_display_string(SHOW_X_CENTERED((const char*)SystemEnterMenuText[page*PAGE_MAX_SHOW_NUM+i]),
												 SHOW_Y_LINE(i+1),
												 (rt_uint8_t *)SystemEnterMenuText[page*PAGE_MAX_SHOW_NUM+i],
												 GUI_WIHIT);
		}
	}
	gui_display_update();
}

void system_menu1_show(void)
{
	system_enter_menu_ui(0);
}

void system_menu2_show(void)
{
	system_enter_menu_ui(1);
}

static rt_err_t unlock_password_verify(rt_uint8_t *password,rt_int32_t *ps_id)
{
	*ps_id = key_password_verify(password);

	if(*ps_id < 0)
	{
		return RT_ERROR;
	}
	
	return RT_EOK;
}

rt_err_t unlock_process_ui(void)
{
	rt_uint8_t buf[8];
	rt_uint8_t ShowBuf[8];
	rt_uint8_t GlintStatus = 0;

	while(1)
	{
		gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);

	  gui_display_string(SHOW_X_CENTERED(UNLOCK_UI_TEXT[0]),SHOW_Y_LINE(0),UNLOCK_UI_TEXT[0],GUI_WIHIT);

	  gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(2),UNLOCK_UI_TEXT[1],GUI_WIHIT);

	  gui_display_update(); 
	  rt_memset(buf,0,8);
	  rt_memset(ShowBuf,0,8);
	  while(1)
	  {
	    rt_err_t result;
	    rt_uint8_t KeyValue;
	    
	    result = gui_key_input(&KeyValue);
	    if(RT_EOK == result)
	    {
	      //有按键
	      if(KeyValue >= '0' && KeyValue <= '9')
	      {
	        result = string_add_char(buf,KeyValue,7);
	        if(result == RT_EOK)
	        {
	          string_hide_string((const rt_uint8_t *)buf,ShowBuf,SHOW_PW_HIDE_CH,8);
	          gui_clear(SHOW_X_ROW8(PASSWORD_DATA_POS),SHOW_Y_LINE(2),SHOW_X_ROW8(15),SHOW_Y_LINE(3));
	          gui_display_string(SHOW_X_ROW8(PASSWORD_DATA_POS),SHOW_Y_LINE(2),ShowBuf,GUI_WIHIT);
	        }
	        else
	        {
	          //输入数量超过8个
	        }
	      }
	      else if(KeyValue == MENU_SURE_VALUE)
	      {
	      	rt_int32_t ps_id;
	      	
	        //检测输入的密码是否合法
	        result = unlock_password_verify(buf,&ps_id);
	        if(result == RT_EOK)
	        {
						result = key_permission_check(ps_id);
	        }
	        if(result != RT_EOK)
	        {
	          //密码不合法
	          union alarm_data data;
						data.key.ID = KEY_TYPE_INVALID;
						data.key.Type = KEY_TYPE_KBOARD;
						
	          menu_operation_result_handle(1);
	          gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),UNLOCK_UI_TEXT[2],GUI_WIHIT);
	          gui_display_update();
	          menu_input_sure_key(RT_TICK_PER_SECOND);
	          gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(3),SHOW_X_ROW8(15),SHOW_Y_LINE(4));

	          if(key_error_alarm_manage(0,&data.key.sms) ==  RT_TRUE)
	          {
							menu_operation_result_handle(3);
	          }
	          send_local_mail(ALARM_TYPE_KEY_ERROR,0,&data);
	          break;
	        }
	        else
	        {
	        	union alarm_data data;
	        	
          	data.key.ID = ps_id;
						data.key.Type = KEY_TYPE_KBOARD;
	          send_local_mail(ALARM_TYPE_KEY_RIGHT,0,&data);
			     
	          //新密码是合法的
	          gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
	          gui_display_string(SHOW_X_CENTERED(UNLOCK_UI_TEXT[3]),SHOW_Y_LINE(2),UNLOCK_UI_TEXT[3],GUI_WIHIT);
	          gui_display_update();
						menu_input_sure_key(RT_TICK_PER_SECOND);
						//密码匹配的结果
	          rt_kprintf("This %d key Open the door success\n",ps_id);
	          #ifndef USEING_SYSTEM_SHOW_STYLE1
	          system_menu_choose(1);
	          #endif
	          key_error_alarm_manage(KEY_ERRNUM_MODE_CLAER,RT_NULL);
	          return RT_EOK;
	        }
	      }
	      else if(KeyValue == MENU_DEL_VALUE)
	      {
	        result = string_del_char(buf,8);
	        if(result == RT_EOK)
	        {
	          string_hide_string((const rt_uint8_t *)buf,ShowBuf,SHOW_PW_HIDE_CH,8);
	          gui_clear(SHOW_X_ROW8(PASSWORD_DATA_POS),SHOW_Y_LINE(2),SHOW_X_ROW8(15),SHOW_Y_LINE(3));
	          gui_display_string(SHOW_X_ROW8(PASSWORD_DATA_POS),SHOW_Y_LINE(2),ShowBuf,GUI_WIHIT);
	        }
	        else
	        {
	          gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),UNLOCK_UI_TEXT[4],GUI_WIHIT);
	          gui_display_update();
	         	return RT_EOK;
	        }
	      }
	      #ifdef USEING_SYSTEM_SHOW_STYLE1
	      else if(KeyValue == KEY_ENTRY_SYS_MANAGE)
	      {
					system_menu_choose(1);
					return RT_ERROR;
	      }
	      #endif
	      gui_display_update(); 
	    }
	    else
	    {
	    	rt_bool_t EvtProcessResult;
	    	
	    	//操作超时
	    	if(menu_event_process(2,MENU_EVT_OP_OUTTIME) == 0)
	    	{
					return RT_ETIMEOUT;
	    	}
	      //闪烁提示
	      GlintStatus++;
	      menu_inputchar_glint(SHOW_X_ROW8(PASSWORD_DATA_POS+rt_strlen((const char *)ShowBuf)),SHOW_Y_LINE(2),GlintStatus%2);
	      gui_display_update();
	      //指纹处理结果显示
	      EvtProcessResult = fprint_unlock_result_show();
				if(EvtProcessResult == RT_TRUE)
	      {
					return RT_EOK;
	      }
	      EvtProcessResult = phone_unlock_result_show();
	      {
					return RT_EOK;
	      }
	    }
	  }
	}
}

void unlock_process_ui1(void)
{
	unlock_process_ui();
}


void system_manage_processing(void)
{
	system_menu_choose(1);
}

//指纹开锁结果显示
//返回 RT_TRUE:表示有指纹输入
//返回 RT_TRUE:表示没有指纹输入
rt_bool_t fprint_unlock_result_show(void)
{
	if(menu_event_process(2,MENU_EVT_FP_UNLOCK) == 0)
	{
		//显示指纹解锁成功
		gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
		gui_display_string(SHOW_X_CENTERED(FpUnlockUIText[0]),SHOW_Y_LINE(2),FpUnlockUIText[0],GUI_WIHIT);
		gui_display_update();
		menu_input_sure_key(RT_TICK_PER_SECOND);
		gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);

		return RT_TRUE;
	}
	else if(menu_event_process(2,MENU_EVT_FP_ERROR) == 0)
	{
		//显示指纹解锁错误
		gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
		gui_display_string(SHOW_X_CENTERED(FpUnlockUIText[1]),SHOW_Y_LINE(2),FpUnlockUIText[1],GUI_WIHIT);
		gui_display_update();
		menu_input_sure_key(RT_TICK_PER_SECOND);	
		gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);

		return RT_TRUE;
	}

	return RT_FALSE;
}


//打电话开门ui
//返回 RT_TRUE:表示有打电话开门事件
//返回 RT_TRUE:表示没有打电话开门事件
rt_bool_t phone_unlock_result_show(void)
{
	if(menu_event_process(2,MENU_EVT_PH_UNLOCK) == 0)
	{
		//显示电话开门提示
		gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
		gui_display_string(SHOW_X_CENTERED(PhUnlockUIText[0]),SHOW_Y_LINE(2),PhUnlockUIText[0],GUI_WIHIT);
		gui_display_update();
		menu_input_sure_key(RT_TICK_PER_SECOND);
		gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);

		return RT_TRUE;
	}

	return RT_FALSE;
}


