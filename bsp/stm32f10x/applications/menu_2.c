/*********************************************************************
 * Filename:      menu_2.c
 *
 * Description:
 *
 * Author:        wangzw <wangzw@yuettak.com>
 * Created at:    2013-04-22
 *
 * Modify:
 *
 * 2014-08-19 wangzw <wangzw@yuettak.com> 
 							处理第三级目录下的菜单
 							
 * Copyright (C) 2013 Yuettak Co.,Ltd
 ********************************************************************/
#include "menu_2.h"
#define PAGE_MAX_SHOW_NUM					4
#define LCD_LINE_MAX_LEN					17							//留出一个结束符的位置
#define PHONE_STAND_LEN						11

#define STRING_BT_MAC							"MAC:"
#define STRING_DEVICE_ID					"ID :"

#define STRING_USER_NO						"用户编号:"
#define STRING_NEW_PASSWORD 			"输入新密码:"
#define STRING_NEW_REPASSWORD		 	"再输入一次:"
#define SHOW_PW_HIDE_CH						'*'
#define SHOW_EXIT_HINT						"请输入返回键退出"
#define SHOW_INPUT_ERR						"输入错误!!!"
#define SHOW_ADD_PS_ERR						"添加失败!!!"

static const rt_uint8_t MenuCommText[][LCD_LINE_MAX_LEN] = 
{
  {"请输入返回键退出"},
  {"操作成功!"},
};

//密码新增
static const rt_uint8_t PasswordAddText[][LCD_LINE_MAX_LEN]=
{
	{"用户编号:"},
	{"输入新密码:"},
	{"再输入一次:"},
	{"输入错误!!!"},
	{"添加失败!!!"},
};

//指纹新增文本
static const rt_uint8_t FrintAddText[][LCD_LINE_MAX_LEN] = 
{
	{"用户编号:"},
	{"指纹数量:"},
	{"按确定键开始"},
	{"正在采集..."},
};

//手机号码新增文本
static const rt_uint8_t PhoneAddText[][LCD_LINE_MAX_LEN] = 
{
	{"用户编号:"},
	{"请输入新电话:"},
	{"输入错误"},
};

//保存退出界面文本
static const rt_uint8_t SaveEixtText[][LCD_LINE_MAX_LEN] = 
{
	{"用户编号:"},
	{"保存成功!"},
	{"请按确定键退出"},
};

//退出界面文本
static const rt_uint8_t EixtText[][LCD_LINE_MAX_LEN] = 
{
  {"退出注册!"},
  {"请按确定键退出"},
};

static const rt_uint8_t UserSearchText[][LCD_LINE_MAX_LEN] =
{
	{"输入用户编号:"},
	{"找不到此编号!"},
	{"是否进入浏览界面"},
};


//用户添加列表
#define MENU_USER_ADD_NUM						6
static const rt_uint8_t MenuUserAdd_list[MENU_USER_ADD_NUM][8*2] = 
{
	{"新建密码"},
	{"新建指纹"},
	{"新建电话"},
	{"保存退出"},
	{"用户信息"},
	{">>>>退出"},
};


static void menu_user_add_ui(rt_uint8_t InPOS)
{
	rt_uint8_t page;
	rt_uint8_t pos;
	rt_uint8_t i;

	page = InPOS /PAGE_MAX_SHOW_NUM;//计算显示在那一页
	pos = InPOS % PAGE_MAX_SHOW_NUM;//当前选中的位置
	gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
	for(i=0;i<PAGE_MAX_SHOW_NUM;i++)
	{
		if(page*PAGE_MAX_SHOW_NUM+i >= MENU_USER_ADD_NUM)
		{
			break;
		}
		if(i == pos)
		{
			gui_display_string(SHOW_X_CENTERED((const char*)MenuUserAdd_list[page*PAGE_MAX_SHOW_NUM+i]),
												 SHOW_Y_LINE(i),
												 (rt_uint8_t *)MenuUserAdd_list[page*PAGE_MAX_SHOW_NUM+i],
												 GUI_BLACK);
		}
		else
		{
			gui_display_string(SHOW_X_CENTERED((const char*)MenuUserAdd_list[page*PAGE_MAX_SHOW_NUM+i]),
												 SHOW_Y_LINE(i),
												 (rt_uint8_t *)MenuUserAdd_list[page*PAGE_MAX_SHOW_NUM+i],
												 GUI_WIHIT);
		}
	}
	gui_display_update();
}

//选中密码
void menu_8_processing(void)
{
  menu_user_add_ui(0);
}

//选中指纹
void menu_9_processing(void)
{
  menu_user_add_ui(1);
}


//选中添加手机号
void menu_10_processing(void)
{	
  menu_user_add_ui(2);
}


//选中保存退出
void menu_11_processing(void)
{
  menu_user_add_ui(3);
}


//选中退出
void menu_12_processing(void)
{
  menu_user_add_ui(4);
}

//选中查看信息
void menu_13_processing(void)
{
  menu_user_add_ui(5);
}

//检查新增密码是否合法
rt_err_t add_new_password_check(rt_uint8_t password[])
{
	if(password[0] == 0)
	{
		//新增密码为空
		return RT_ERROR;
	}
	
	return RT_EOK;
}

//输入密码处理
void menu_14_processing(void)
{
	rt_uint8_t buf[MENU_PASSWORD_MAX_LEN];
	rt_uint8_t buf1[MENU_PASSWORD_MAX_LEN];
	rt_uint8_t ShowBuf[MENU_PASSWORD_MAX_LEN];
	rt_uint8_t GlintStatus;

	while(1)
	{
    gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
    gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(0),PasswordAddText[0],GUI_WIHIT);
    gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(1),PasswordAddText[1],GUI_WIHIT);
    gui_display_update();
    //第一次输入密码
    rt_memset(buf,0,MENU_PASSWORD_MAX_LEN);
    rt_memset(ShowBuf,0,MENU_PASSWORD_MAX_LEN);
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
          result = string_add_char(buf,KeyValue,8);
          if(result == RT_EOK)
          {
            string_hide_string((const rt_uint8_t *)buf,ShowBuf,SHOW_PW_HIDE_CH,8);
            gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(2),SHOW_X_ROW8(15),SHOW_Y_LINE(3));
            gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(2),ShowBuf,GUI_WIHIT);
          }
          else
          {
            //输入数量超过8个
          }
        }
        else if(KeyValue == '*')
        {
          //检测输入的密码是否合法
          result = add_new_password_check(buf);
          if(result != RT_EOK)
          {
          	//密码不合法
    				gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PasswordAddText[3],GUI_WIHIT);
    				gui_display_update();
    				rt_thread_delay(RT_TICK_PER_SECOND);
    				gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(3),SHOW_X_ROW8(15),SHOW_Y_LINE(4));
          }
          else
          {
            //新密码是合法的
            rt_memcpy(buf1,buf,MENU_PASSWORD_MAX_LEN);
            break;
          }
          //新密码输入完成 进入验证。
        }
        else if(KeyValue == '#')
        {
          rt_kprintf("删除\nn");
          result = string_del_char(buf,8);
          if(result == RT_EOK)
          {
            string_hide_string((const rt_uint8_t *)buf,ShowBuf,SHOW_PW_HIDE_CH,8);
            gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(2),SHOW_X_ROW8(15),SHOW_Y_LINE(3));
            gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(2),ShowBuf,GUI_WIHIT);
          }
         	else
         	{
         		gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),MenuCommText[0],GUI_WIHIT);
         		gui_display_update();
						return ;
         	}
        }
      }
      else
      {
        //闪烁提示
        GlintStatus++;
        menu_inputchar_glint(SHOW_X_ROW8(rt_strlen((const char *)ShowBuf)),SHOW_Y_LINE(2),GlintStatus%2);
      }
      //更新显示
      
      gui_display_update();
    }
    
    //第二次输入密码
    rt_memset(buf,0,MENU_PASSWORD_MAX_LEN);
    rt_memset(ShowBuf,0,MENU_PASSWORD_MAX_LEN);
    gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
    gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(0),PasswordAddText[0],GUI_WIHIT);
    gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(1),PasswordAddText[2],GUI_WIHIT);
    gui_display_update();
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
          result = string_add_char(buf,KeyValue,8);
          if(result == RT_EOK)
          {
            string_hide_string((const rt_uint8_t *)buf,ShowBuf,SHOW_PW_HIDE_CH,8);
            gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(2),SHOW_X_ROW8(15),SHOW_Y_LINE(3));
            gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(2),ShowBuf,GUI_WIHIT);
          }
          else
          {
            //输入数量超过8个
          }
        }
        else if(KeyValue == '*')
        {
          //检测输入的密码是否合法
          rt_uint32_t temp;
          
          temp = rt_memcmp(buf1,buf,rt_strlen((const char *)buf));
          if(temp != 0)
          {
            //密码匹配错误
            gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PasswordAddText[3],GUI_WIHIT);
    				gui_display_update();
    				rt_thread_delay(RT_TICK_PER_SECOND);
    				gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(3),SHOW_X_ROW8(15),SHOW_Y_LINE(4));
          }
          else
          {
            //添加成功保存对象
          }
          //新密码输入完成 进入验证。
          break;
        }
        else if(KeyValue == '#')
        {
          rt_kprintf("删除\nn");
          result = string_del_char(buf,8);
          if(result == RT_EOK)
          {
            string_hide_string((const rt_uint8_t *)buf,ShowBuf,SHOW_PW_HIDE_CH,8);
            gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(2),SHOW_X_ROW8(15),SHOW_Y_LINE(3));
            gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(2),ShowBuf,GUI_WIHIT);
          }
          else
          {
          	rt_kprintf("退出钥匙添加\n");
						//return ;
          }
        }
      }
      else
      {
        //闪烁提示
        GlintStatus++;
        menu_inputchar_glint(SHOW_X_ROW8(rt_strlen((const char *)ShowBuf)),SHOW_Y_LINE(2),GlintStatus%2);
      }
      //更新显示
      
      gui_display_update();
    }

	}
}

//新增指纹
void menu_15_processing(void)
{
	rt_uint8_t KeyValue;
	rt_uint8_t FrintNum;
	rt_err_t	 result;
	rt_uint8_t buf[LCD_LINE_MAX_LEN];
	
  //gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
  //获取当前指纹数量
  FrintNum = 0;
  while(1)
  {
  	gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
    gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(0),FrintAddText[0],GUI_WIHIT);
    gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(1),FrintAddText[1],GUI_WIHIT);
    rt_sprintf((char *)buf,"%d",FrintNum);
    gui_display_string(SHOW_X_ROW8(rt_strlen((const char *)FrintAddText[1])),SHOW_Y_LINE(1),buf,GUI_WIHIT);
    gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(2),FrintAddText[2],GUI_WIHIT);
    gui_display_update();
		while(1)
		{
			result = gui_key_input(&KeyValue);
			if(result == RT_EOK)
			{
				if(KeyValue == '*')
				{
					//采集指纹
					gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(2),SHOW_X_ROW8(15),SHOW_Y_LINE(3));
					gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(2),FrintAddText[3],GUI_WIHIT);
					gui_display_update();
					result = RT_EOK;
					if(result == RT_EOK)
					{
						//指纹采集成功
						rt_thread_delay(100);
						gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(2),SHOW_X_ROW8(15),SHOW_Y_LINE(3));
						gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(2),MenuCommText[1],GUI_WIHIT);
						gui_display_update();
						rt_thread_delay(100);
						FrintNum++;
						break;
					}
					else
					{
						//指纹采集失败
					}
				}
				else if(KeyValue == '#')
				{
					gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),MenuCommText[0],GUI_WIHIT);
         	gui_display_update();
					return ;
				}
			}
			else
			{

			}
		}
		rt_thread_delay(1);
  }
  //gui_display_update();
}

rt_err_t add_new_phone_check(rt_uint8_t phone[])
{
	rt_uint8_t len;

	len = rt_strlen((const char *)phone);
	if(len < PHONE_STAND_LEN)
	{
		return RT_ERROR;
	}	
	
	return RT_EOK;
}
//添加手机号码
void menu_16_processing(void)
{
	rt_uint8_t PhoneNum = 0;
	rt_uint8_t buf[LCD_LINE_MAX_LEN];
	rt_uint8_t PhoneBuf[MENU_PHONE_MAX_LEN+1];
	rt_uint8_t KeyValue;
	rt_uint8_t result;
	rt_uint8_t GlintStatus;
	
  //gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);

  while(1)
  {
  	gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
    gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(0),PhoneAddText[0],GUI_WIHIT);
    gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(1),PhoneAddText[1],GUI_WIHIT);
    rt_sprintf((char *)buf,"%d",PhoneNum);
    gui_display_string(SHOW_X_ROW8(rt_strlen((const char *)PhoneAddText[1])),SHOW_Y_LINE(1),buf,GUI_WIHIT);
    gui_display_update();

    rt_memset(PhoneBuf,0,MENU_PHONE_MAX_LEN);
		while(1)
		{
			result = gui_key_input(&KeyValue);
      if(RT_EOK == result)
      {
        //有按键
        if(KeyValue >= '0' && KeyValue <= '9')
        {
          result = string_add_char(PhoneBuf,KeyValue,MENU_PHONE_MAX_LEN);
          if(result == RT_EOK)
          {
            gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(2),SHOW_X_ROW8(15),SHOW_Y_LINE(3));
            gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(2),PhoneBuf,GUI_WIHIT);
          }
          else
          {
            //输入数量超过8个
          }
        }
        else if(KeyValue == '*')
        {
          //检测输入的密码是否合法
          result = add_new_phone_check(PhoneBuf);
          if(result != RT_EOK)
          {
          	//密码不合法
    				gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PhoneAddText[2],GUI_WIHIT);
    				gui_display_update();
    				rt_thread_delay(RT_TICK_PER_SECOND);
    				gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(3),SHOW_X_ROW8(15),SHOW_Y_LINE(4));
          }
          else
          {
            //新密码是合法的
            //rt_memcpy(buf1,buf,MENU_PASSWORD_MAX_LEN);
            break;
          }
          //新密码输入完成 进入验证。
        }
        else if(KeyValue == '#')
        {
          rt_kprintf("删除\nn");
          result = string_del_char(PhoneBuf,MENU_PHONE_MAX_LEN);
          if(result == RT_EOK)
          {
            gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(2),SHOW_X_ROW8(15),SHOW_Y_LINE(3));
            gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(2),PhoneBuf,GUI_WIHIT);
          }
         	else
         	{
         		gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),MenuCommText[0],GUI_WIHIT);
         		gui_display_update();
						return ;
         	}
        }
      }
      else
      {
        //闪烁提示
        GlintStatus++;
        menu_inputchar_glint(SHOW_X_ROW8(rt_strlen((const char *)PhoneBuf)),SHOW_Y_LINE(2),GlintStatus%2);
      }
      //更新显示
      
      gui_display_update();
			rt_thread_delay(1);
		}
		rt_thread_delay(1);
  }
  //gui_display_update();
}

//保存并退出
void menu_17_processing(void)
{
  gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
	gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(0),SaveEixtText[0],GUI_WIHIT);
	gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(1),SaveEixtText[1],GUI_WIHIT);
	gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(2),SaveEixtText[2],GUI_WIHIT);
  gui_display_update();
}

void menu_18_processing(void)
{
  gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
  
  gui_display_update();
}

void menu_19_processing(void)
{
  gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
  gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(0),EixtText[0],GUI_WIHIT);
	gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(1),EixtText[1],GUI_WIHIT);
  gui_display_update();
}

rt_err_t search_user_id_check(rt_uint8_t *buf,rt_uint8_t *user)
{
	sscanf((const char*)buf,"%s",user);
	if(rt_strlen((const char *)buf) == 0)
	{
		return RT_ERROR;
	}
	
	return RT_EOK;
}

static  rt_uint8_t  TextUser[100][LCD_LINE_MAX_LEN] = 
{
	{"user1"},
	{"user2"},
	{"user3"},
	{"user5"},
	{"user6"},
	{"user7"},
	{"user8"},
	{"user9"},
};
static void user_list_processing(void)
{
	rt_uint8_t KeyValue;
	rt_err_t	 result;
	rt_uint8_t buf[LCD_LINE_MAX_LEN];
	rt_uint8_t ShowStart = 0;
	rt_uint8_t i;
	
	gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);

	//初始化测试数据
	for(i= 0;i<100;i++)
	{
		rt_memset(buf,0,LCD_LINE_MAX_LEN);
		rt_sprintf(buf,"User_%d",i);
		rt_strncpy(TextUser[i],buf,LCD_LINE_MAX_LEN);
	}
	while(1)
	{
    result = gui_key_input(&KeyValue);
    if(result == RT_EOK)
    {
      if(KeyValue == '8')
      {
        //上
      }
      else if(KeyValue == '0')
      {
        //下
      }
      else if(KeyValue == '7')
      {
        //左
        if(ShowStart > 4)
        {
					ShowStart-=4;
        }
      }
      else if(KeyValue == '9')
      {
        //右
        if(ShowStart < 100)
        {
          ShowStart+=4;
        }
      }
      else if(KeyValue == '*')
      {
        //确定
      }
      else if(KeyValue == '#')
      {
        //取消
      }
    }
    for(i = 0;i<4;i++)
			{

				rt_memset(buf,0,LCD_LINE_MAX_LEN);
				rt_sprintf(buf,"%03d",ShowStart+i);
				gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(i),buf,GUI_WIHIT);
				
				rt_memset(buf,0,LCD_LINE_MAX_LEN);
				rt_strncpy(buf,TextUser[ShowStart+i],LCD_LINE_MAX_LEN);
				gui_display_string(SHOW_X_ROW16(4),SHOW_Y_LINE(i),buf,GUI_WIHIT);
			}
    gui_display_update();
	}
}

//搜素界面
void menu_22_processing(void)
{
	rt_uint8_t KeyValue;
	rt_err_t   result;
	rt_uint8_t GlintStatus;
	rt_uint8_t UserPos;
	rt_uint8_t buf[LCD_LINE_MAX_LEN];
	
  gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);

  gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(0),UserSearchText[0],GUI_WIHIT);
  gui_display_update();
  rt_memset(buf,0,LCD_LINE_MAX_LEN);
 	while(1)
	{
		result = gui_key_input(&KeyValue);
  	if(RT_EOK == result)
	  {
	    //有按键
	    if(KeyValue >= '0' && KeyValue <= '9')
	    {
	      result = string_add_char(buf,KeyValue,MENU_PHONE_MAX_LEN);
	      if(result == RT_EOK)
	      {
	        gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(1),SHOW_X_ROW8(15),SHOW_Y_LINE(2));
	        gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(1),buf,GUI_WIHIT);
	      }
	      else
	      {
	        //输入数量超过8个
	      }
	    }
	    else if(KeyValue == '*')
	    {
	      //检测是否能找到这个人
	      result = search_user_id_check(buf,&UserPos);
	      if(result != RT_EOK)
	      {
	      	//如果找不到这个人则进入浏览界面
					gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(2),UserSearchText[1],GUI_WIHIT);
					gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),UserSearchText[2],GUI_WIHIT);
					gui_display_update();
					//rt_thread_delay(RT_TICK_PER_SECOND);
					

					//确定键进入浏览 返回键重新输入
					while(1)
					{
            result = gui_key_input(&KeyValue);
						if(result == RT_EOK)
						{
							if(KeyValue == '*')
							{
								//进入浏览界面
								user_list_processing();
							}
							else if(KeyValue == '#')
							{
								//返回键
								gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(2),SHOW_X_ROW8(16),SHOW_Y_LINE(3));
								gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(3),SHOW_X_ROW8(16),SHOW_Y_LINE(4));
								break;
							}
						}
					}
	      }
	      else
	      {
	        //如果能找到这个人进入修改界面
	       	rt_kprintf("User Pos=%d\n",UserPos);
	        break;
	      }
	      //新密码输入完成 进入验证。
	    }
	    else if(KeyValue == '#')
	    {
	      rt_kprintf("删除\nn");
	      result = string_del_char(buf,MENU_PHONE_MAX_LEN);
	      if(result == RT_EOK)
	      {
	        gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(1),SHOW_X_ROW8(15),SHOW_Y_LINE(2));
	        gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(1),buf,GUI_WIHIT);
	      }
	     	else
	     	{
	     		gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),MenuCommText[0],GUI_WIHIT);
	     		gui_display_update();
					return ;
	     	}
	    }
	  }
	  else
	  {
	    //闪烁提示
	    GlintStatus++;
	    menu_inputchar_glint(SHOW_X_ROW8(rt_strlen((const char *)buf)),SHOW_Y_LINE(1),GlintStatus%2);
	  }
	  //更新显示
  
  gui_display_update();
	rt_thread_delay(1);
	}
  gui_display_update();
}

//浏览界面
void menu_23_processing(void)
{
  gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
  
  gui_display_update();
}

//修改界面








