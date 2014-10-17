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

	rt_kprintf("进入一级1菜单\n");

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
          result = string_add_char(buf,KeyValue,8);
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
          rt_kprintf("核对密码\n");
          result =  admin_password_check(buf);
          if(result == RT_EOK)
          {
            KeyFuncIndex = KeyTab[ KeyFuncIndex].SureState;
            current_operation_index = KeyTab[KeyFuncIndex].CurrentOperate;
            current_operation_index();
            return ;
          }
          else
          {
            //密码错误
            menu_error_handle(1);
            gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(3),SHOW_LAND_UI_PS_ERR,GUI_WIHIT);
            gui_display_update();
            rt_thread_delay(RT_TICK_PER_SECOND);
            //密码错误3次报警
            if(key_error_alarm_manage(0) ==  RT_TRUE)
	          {
	          	union alarm_data data;
	          	
	          	data.key.ID = KEY_ID_INVALID;
	          	data.key.Type = KEY_TYPE_KBOARD;
	          	data.key.sms = 1;
              send_local_mail(ALARM_TYPE_KEY_ERROR,0,&data);
              
							menu_error_handle(3);
	          }
            break;
          }
        }
        else if(KeyValue == MENU_DEL_VALUE)
        {
          rt_kprintf("删除\nn");
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
	rt_kprintf("进入二级1菜单\n");
	
	menu_first_ui(0);
}


void menu_2_processing(void)
{
	rt_kprintf("进入二级2菜单\n");
	menu_first_ui(1);
}

void menu_3_processing(void)
{
	rt_kprintf("进入三级1菜单\n");
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
	rt_kprintf("进入三级3菜单\n");
  menu1_second_ui(0);
}

void menu_5_processing(void)
{
	rt_kprintf("进入三级3菜单\n");
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
	rt_kprintf("进入三级3菜单\n");
	menu2_second_ui(0);
}

void menu_7_processing(void)
{
	rt_kprintf("进入三级3菜单\n");
	menu2_second_ui(1);
}



