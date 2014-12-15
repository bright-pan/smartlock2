/*********************************************************************
 * Filename:      gpio_pin.c
 *
 * Description:
 *
 * Author:        wangzw <wangzw@yuettak.com>
 * Created at:    2013-04-22
 *
 * Modify:
 *
 * 2014-08-19 wangzw <wangzw@yuettak.com> 
 							管理员登陆界面
 							各级菜单界面
 							
 * Copyright (C) 2013 Yuettak Co.,Ltd
 ********************************************************************/

#include"menu_1.h"
#define PAGE_MAX_SHOW_NUM						4
#define SHOW_LAND_UI_STRING1				"管理员登陆"
#define SHOW_LAND_UI_STRING2				"密码:"
#define SHOW_LAND_UI_PS_ERR					"密码错误!"
#define SHOW_PW_HIDE_CH							'*'

//第一层菜单显示内容
#define MENU_FIRST_NUM							3			
static const rt_uint8_t MenuFirst[MENU_FIRST_NUM][8*2] = 
{
	{"1.用户管理"},
	{"2.系统管理"},
	{"3.>>>>退出"},
};

//菜单1下第二层菜单显示
#define MENU1_SECOND_NUM						2
static const rt_uint8_t Menu1Second[MENU1_SECOND_NUM][8*2] = 
{
	{"1.用户新建"},
	{"2.用户修改"},
	//{"3.管理员>>"},
};

//菜单2下第二层菜单显示
#define MENU2_SECOND_NUM						2
static const rt_uint8_t Menu2Second[MENU2_SECOND_NUM][8*2] = 
{
	{"1.本机信息"},
	{"2.参数设置"},
};

//管理员初始化界面信息
static const rt_uint8_t AdminInitUIText[][8*2] =
{
	"设置管理员手机",
	"手机号错误",
	"管理员账户",
	"正在创建..."
};
//管理员密码检测
rt_err_t admin_password_check(rt_uint8_t *password)
{	
	rt_err_t result;
	
	result = admin_password_verify(password);

	return result;
}

void menu_0_processing(void)
{
	rt_uint8_t buf[8];
	rt_uint8_t ShowBuf[8];
	rt_uint8_t GlintStatus = 0;

	gui_sleep_time_set(5);
	while(1)
	{
    gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
    gui_display_string(SHOW_X_CENTERED(SHOW_LAND_UI_STRING1),SHOW_Y_LINE(0),SHOW_LAND_UI_STRING1,GUI_WIHIT);
    
    gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(2),SHOW_LAND_UI_STRING2,GUI_WIHIT);
    
    //gui_box(SHOW_X_ROW8(5)-1,SHOW_Y_LINE(2)-1,SHOW_X_ROW8(15)+1,SHOW_Y_LINE(3)+1,1,0);
    
    gui_display_update();
    
    rt_memset(buf,0,8);
    rt_memset(ShowBuf,0,8);
    while(1)
    {
      rt_err_t result;
      rt_uint8_t KeyValue;
    
      result = gui_key_input(&KeyValue);
      if(result == RT_EOK)
      {
        if(KeyValue >= '0' && KeyValue <= '9')
        {
          result = string_add_char(buf,KeyValue,7);
          if(result == RT_EOK)
          {
            string_hide_string((const rt_uint8_t *)buf,ShowBuf,SHOW_PW_HIDE_CH,8);
            gui_clear(SHOW_X_ROW8(5),SHOW_Y_LINE(2),SHOW_X_ROW8(15),SHOW_Y_LINE(3));
            gui_display_string(SHOW_X_ROW8(5),SHOW_Y_LINE(2),ShowBuf,GUI_WIHIT);
          }
          else
          {
            //输入数量超过8个
          }
        }
        else if(KeyValue == MENU_SURE_VALUE)
        {
          rt_dprintf(MENU_DEBUG_THREAD,("Check Password\n"));
          result =  admin_password_check(buf);
          if(result == RT_EOK)
          {	
          	//管理员登陆成功
            KeyFuncIndex = KeyTab[ KeyFuncIndex].SureState;
            current_operation_index = KeyTab[KeyFuncIndex].CurrentOperate;
            current_operation_index();
            send_local_mail(ALARM_TYPE_SYSTEM_UNFREEZE,0,RT_NULL);
            
						//登陆后屏幕休眠时间变长
						gui_sleep_time_set(25);
						
						//清除钥匙错误计数
            key_error_alarm_manage(KEY_ERRNUM_MODE_CLAER,RT_NULL);
            return ;
          }
          else
          {
          	rt_uint8_t smsflag;
            //密码错误
            menu_operation_result_handle(1);
            gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(3),SHOW_LAND_UI_PS_ERR,GUI_WIHIT);
            gui_display_update();
            menu_input_sure_key(RT_TICK_PER_SECOND);
            //rt_thread_delay(RT_TICK_PER_SECOND);
            //密码错误3次报警
            if(key_error_alarm_manage(KEY_ERRNUM_MODE_ADDUP,&smsflag) ==  RT_TRUE)
	          {
	          	union alarm_data data;
	          	
	          	data.key.ID = KEY_ID_INVALID;
	          	data.key.Type = KEY_TYPE_KBOARD;
							
              send_local_mail(ALARM_TYPE_KEY_ERROR,0,&data);
              
							menu_operation_result_handle(3);
	          }
            break;
          }
        }
        else if(KeyValue == MENU_DEL_VALUE)
        {
          result = string_del_char(buf,8);
          if(result == RT_EOK)
          {
            string_hide_string((const rt_uint8_t *)buf,ShowBuf,SHOW_PW_HIDE_CH,8);
            gui_clear(SHOW_X_ROW8(5),SHOW_Y_LINE(2),SHOW_X_ROW8(15),SHOW_Y_LINE(3));
            gui_display_string(SHOW_X_ROW8(5),SHOW_Y_LINE(2),ShowBuf,GUI_WIHIT);
          }
          else
          {
						system_menu_choose(0);
						return ;
          }
        }
      }
      else
      {
      	//操作超时
	    	if(menu_event_process(2,MENU_EVT_OP_OUTTIME) == 0)
	    	{
					return ;
	    	}
	    	
      	GlintStatus++;
        menu_inputchar_glint(SHOW_X_ROW8(5+rt_strlen((const char*)ShowBuf)),SHOW_Y_LINE(2),GlintStatus%2);
      }
      gui_display_update();
    }

	}
}



////////////////////////////////////////////////////////////////////////////////////////////////////
//菜单UI代码
////////////////////////////////////////////////////////////////////////////////////////////////////

void menu_first_ui(rt_uint8_t InPOS)
{
	rt_uint8_t page;
	rt_uint8_t pos;
	rt_uint8_t i;

	page = InPOS /PAGE_MAX_SHOW_NUM;//计算显示在那一页
	pos = InPOS % PAGE_MAX_SHOW_NUM;//当前选中的位置
	gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
	for(i=0;i<PAGE_MAX_SHOW_NUM;i++)
	{
		if(page*PAGE_MAX_SHOW_NUM+i >= MENU_FIRST_NUM)
		{
			break;
		}
		if(i == pos)
		{
			gui_display_string(SHOW_X_CENTERED((const char*)MenuFirst[page*PAGE_MAX_SHOW_NUM+i]),
												 SHOW_Y_LINE(i),
												 (rt_uint8_t *)MenuFirst[page*PAGE_MAX_SHOW_NUM+i],
												 GUI_BLACK);
		}
		else
		{
			gui_display_string(SHOW_X_CENTERED((const char*)MenuFirst[page*PAGE_MAX_SHOW_NUM+i]),
												 SHOW_Y_LINE(i),
												 (rt_uint8_t *)MenuFirst[page*PAGE_MAX_SHOW_NUM+i],
												 GUI_WIHIT);
		}
	}
	gui_display_update();
}

void menu_1_processing(void)
{
	menu_first_ui(0);
}


void menu_2_processing(void)
{
	menu_first_ui(1);
}

void menu_3_processing(void)
{
	menu_first_ui(2);
}

void menu1_second_ui(rt_uint8_t InPOS)
{
	rt_uint8_t page;
	rt_uint8_t pos;
	rt_uint8_t i;

	page = InPOS /PAGE_MAX_SHOW_NUM;//计算显示在那一页
	pos = InPOS % PAGE_MAX_SHOW_NUM;//当前选中的位置
	gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
  for(i=0;i<PAGE_MAX_SHOW_NUM;i++)
  {
    if(page*PAGE_MAX_SHOW_NUM+i >= MENU1_SECOND_NUM)
    {
      break;
    }
    if(i == pos)
    {
      gui_display_string(SHOW_X_CENTERED((const char*)Menu1Second[page*PAGE_MAX_SHOW_NUM+i]),
                         SHOW_Y_LINE(i),
                         (rt_uint8_t *)Menu1Second[page*PAGE_MAX_SHOW_NUM+i],
                         GUI_BLACK);
    }
    else
    {
      gui_display_string(SHOW_X_CENTERED((const char*)Menu1Second[page*PAGE_MAX_SHOW_NUM+i]),
                         SHOW_Y_LINE(i),
                         (rt_uint8_t *)Menu1Second[page*PAGE_MAX_SHOW_NUM+i],
                         GUI_WIHIT);
    }
  }

	gui_display_update();
}

void menu_4_processing(void)
{
  menu1_second_ui(0);
}

void menu_5_processing(void)
{
	menu1_second_ui(1);
}

/*void menu_32_processing(void)
{
  menu1_second_ui(2);
}*/

void menu2_second_ui(rt_uint8_t InPOS)
{
	rt_uint8_t page;
	rt_uint8_t pos;
	rt_uint8_t i;

	page = InPOS /PAGE_MAX_SHOW_NUM;//计算显示在那一页
	pos = InPOS % PAGE_MAX_SHOW_NUM;//当前选中的位置
	gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
  for(i=0;i<PAGE_MAX_SHOW_NUM;i++)
  {
    if(page*PAGE_MAX_SHOW_NUM+i >= MENU2_SECOND_NUM)
    {
      break;
    }
    if(i == pos)
    {
      gui_display_string(SHOW_X_CENTERED((const char*)Menu2Second[page*PAGE_MAX_SHOW_NUM+i]),
                        SHOW_Y_LINE(i),
                        (rt_uint8_t *)Menu2Second[page*PAGE_MAX_SHOW_NUM+i],
                        GUI_BLACK);
    }
    else
    {
      gui_display_string(SHOW_X_CENTERED((const char*)Menu2Second[page*PAGE_MAX_SHOW_NUM+i]),
                         SHOW_Y_LINE(i),
                         (rt_uint8_t *)Menu2Second[page*PAGE_MAX_SHOW_NUM+i],
                         GUI_WIHIT);
    }
  }

	gui_display_update();
}


void menu_6_processing(void)
{
	menu2_second_ui(0);
}

void menu_7_processing(void)
{
	menu2_second_ui(1);
}

static rt_err_t admin_one_phone_check(rt_uint8_t *phone)
{
	if(rt_strlen((const char *)phone) < 11)
	{
		return RT_ERROR;
	}

	return RT_EOK;
}

/** 
@brief  管理员手机输入UI
@param  none
@retval None
*/
rt_err_t admin_phone_input_UI(rt_uint8_t *Phone)
{
	rt_uint8_t *buf = RT_NULL;
	rt_uint8_t GlintStatus;

ADMIN_CRATE_UI:
	if(menu_event_process(1,MENU_EVT_LCD_CLOSE) == 0)
	{
		rt_uint8_t KeyValue;
		rt_err_t   result;
		do
		{
			//等待按键唤醒
			result = gui_key_input(&KeyValue);
		}
		while(result != RT_EOK);

		goto ADMIN_CRATE_UI;
	}
  gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);

  gui_display_string(SHOW_X_CENTERED(AdminInitUIText[0]),SHOW_Y_LINE(1),AdminInitUIText[0],GUI_WIHIT);
  gui_display_update();
  buf = rt_calloc(1,MENU_PHONE_MAX_LEN);
	while(1)
	{
		rt_err_t   result;
		rt_uint8_t KeyValue;
    result = gui_key_input(&KeyValue);
	  if(RT_EOK == result)
	  {
	    //有按键
	    if(KeyValue >= '0' && KeyValue <= '9')
	    {
	      result = string_add_char(buf,KeyValue,MENU_PHONE_MAX_LEN);
	      if(result == RT_EOK)
	      {
	        gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(2),SHOW_X_ROW8(15),SHOW_Y_LINE(3));
	        gui_display_string(SHOW_X_CENTERED(buf),SHOW_Y_LINE(2),buf,GUI_WIHIT);
	      }
	      else
	      {
	        //输入数量超过8个
	      }
	    }
	    else if(KeyValue == MENU_SURE_VALUE)
	    {
	      //检测输入的手机是否合法
	      result = admin_one_phone_check(buf);
	      if(result != RT_EOK)
	      {
	        //不合法
	        menu_operation_result_handle(1);
	        gui_display_string(SHOW_X_CENTERED(AdminInitUIText[1]),SHOW_Y_LINE(3),AdminInitUIText[1],GUI_WIHIT);
	        gui_display_update();
	        rt_thread_delay(RT_TICK_PER_SECOND);
	        gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(3),SHOW_X_ROW8(15),SHOW_Y_LINE(4));
	      }
	      else
	      {
	        //合法
	        rt_memcpy(Phone,buf,MENU_PHONE_MAX_LEN);
	        break;
	      }
	      //新密码输入完成 进入验证。
	    }
	    else if(KeyValue == MENU_DEL_VALUE)
	    {
	      result = string_del_char(buf,MENU_PHONE_MAX_LEN);
	      if(result == RT_EOK)
	      {
	        gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(2),SHOW_X_ROW8(15),SHOW_Y_LINE(3));
	        gui_display_string(SHOW_X_CENTERED(buf),SHOW_Y_LINE(2),buf,GUI_WIHIT);
	      }
	      /*else
	      {
	        gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),AdminInitUIText[2],GUI_WIHIT);
	        gui_display_update();
	        rt_free(buf);
	        return ;
	      }*/
	    }
	  }
	  else
	  {
	    //操作超时
	    if(menu_event_process(2,MENU_EVT_OP_OUTTIME) == 0)
	    {
	    	rt_free(buf);
	    	goto ADMIN_CRATE_UI;
	      //return RT_ETIMEOUT;
	    }
	    //闪烁提示
	    GlintStatus++;
	    menu_inputchar_glint(SHOW_X_ROW8(rt_strlen((const char*)buf))+SHOW_X_CENTERED(buf),SHOW_Y_LINE(2),GlintStatus%2);
	  }
	  //更新显示
	  
	  gui_display_update();
	  rt_thread_delay(1);

	}
	gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
  gui_display_string(SHOW_X_CENTERED(AdminInitUIText[2]),SHOW_Y_LINE(1),AdminInitUIText[2],GUI_WIHIT);  
  gui_display_string(SHOW_X_CENTERED(AdminInitUIText[3]),SHOW_Y_LINE(2),AdminInitUIText[3],GUI_WIHIT);
  gui_display_update();
	if(menu_input_sure_key(200) == RT_ETIMEOUT)
	{
    menu_event_process(2,MENU_EVT_OP_OUTTIME);
	}
	gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
	gui_display_update();

	return RT_EOK;
}

/** 
@brief  创建新的管理账户
@param  none
@retval None
*/
rt_err_t admin_account_init(void)
{
	admin_create(admin_phone_input_UI);

	return RT_EOK;
}




