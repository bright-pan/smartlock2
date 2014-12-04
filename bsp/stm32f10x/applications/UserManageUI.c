#include "UserManageUI.h"
#include"menu.h"

/**************************************************************************************************/
#define PASSWORD_MAX_LEN						7   //密码最大长度
#define PHONE_MAX_LEN               13  //手机最大长度
#define INPUT_PW_WAIT_TIME         30   //输入密码等待时间

/**************************************************************************************************/
/* 用户管理菜单列表 */
static const rt_uint8_t UserManageListText[][LCD_LINE_MAX_LEN] =
{
	{"1.密码管理"},
	{"2.指纹管理"},
	{"3.手机管理"},
};

/* 密码管理界面 */
static const rt_uint8_t PasswordManageText[][LCD_LINE_MAX_LEN] =
{
	{"1.添加"},
	{"2.删除"},
};

/* 添加密码界面 */
static const rt_uint8_t PasswordAddText[][LCD_LINE_MAX_LEN] =
{
	{"用户ID:"},
	{"存在钥匙数量"},
	{"请输入新密码"},
	{"请再输入一次"},
	{"密码已存在"},
	{"密码长度错误"},
	{"添加成功"},
	{"添加失败请重试"},
	{"两次输入不匹配"},
};

/* 删除密码界面 */
static const rt_uint8_t PasswordDelText[][LCD_LINE_MAX_LEN] =
{
	{"用户ID"},
	{"请输入"},
	{"您要删除的密码:"},
	{"密码删除成功"},
	{"密码长度错误"},
	{"该用户没有此密码"},
	{"密码删除失败"},
};

/* 指纹管理界面 */
static const rt_uint8_t FprintManageText[][LCD_LINE_MAX_LEN] =
{
	{"1.添加"},
	{"2.删除"},
};

/* 指纹添加界面 */
static const rt_uint8_t FprintAddText[][LCD_LINE_MAX_LEN] =
{
	{"用户ID"},
	{"采集新指纹"},
	{"请*键开始采集"},
	{"正在采集数据..."},
	{"指纹采集失败"},
	{"该指纹已存在"},
	{"添加成功"},
	{"添加失败"},
};

/* 指纹删除界面 */
static const rt_uint8_t FprintDelText[][LCD_LINE_MAX_LEN] =
{
	{"用户ID"},
	{"采集要删除的指纹"},
	{"请按*键开始采集"},
	{"采集失败"},
	{"该用户没有此指纹"},
	{"删除失败"},
};


/* 电话管理界面 */
static const rt_uint8_t PhoneManageText[][LCD_LINE_MAX_LEN] =
{
	{"1.添加"},
	{"2.删除"},
};

/* 电话添加界面 */
static const rt_uint8_t PhoneAddText[][LCD_LINE_MAX_LEN] =
{
	{"用户ID"},
	{"请输入新手机号码"},
	{"请再输入一次"},
	{"手机号长度错误"},
	{"该号码已经存在"},
	{"添加成功"},
	{"添加失败"},
	{"两次输入不匹配"},
};

/* 删除电话界面 */
static const rt_uint8_t PhoneDelText[][LCD_LINE_MAX_LEN] =
{
	{"用户ID"},
	{"请输入"},
	{"您要删除的手机号"},
	{"删除成功"},
	{"手机号长度错误"},
	{"该用户没有此号码"},
	{"删除失败"},
};


/**************************************************************************************************/

/** 
@brief  菜单列表显示
				添加
				删除
				修改
@param  none
@retval none
*/
static void menu_list_show_ui(const rt_uint8_t list[][LCD_LINE_MAX_LEN],	
																								rt_uint8_t InPOS,
																								rt_uint8_t ListSize)
{
	rt_uint8_t page;
	rt_uint8_t pos;
	rt_uint8_t i;
	rt_int32_t CurUserPos;

	if(InPOS >= ListSize)
	{
		rt_kprintf("%s InPos > %d is Error",__FUNCTION__,ListSize);
		return ;
	}
	CurUserPos = account_cur_pos_get();
	page = InPOS /PAGE_MAX_SHOW_NUM;//计算显示在那一页
	pos = InPOS % PAGE_MAX_SHOW_NUM;//当前选中的位置
	gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
	for(i=0;i<PAGE_MAX_SHOW_NUM;i++)
	{
		if(page*PAGE_MAX_SHOW_NUM+i >= ListSize)
		{
			break;
		}
		if(CurUserPos == 0)
		{
      if(i == pos)
      {
      	// 当前选中项
        gui_display_string(SHOW_X_CENTERED((const char*)list[page*PAGE_MAX_SHOW_NUM+i]),
                           SHOW_Y_LINE(i),
                           (rt_uint8_t *)list[page*PAGE_MAX_SHOW_NUM+i],
                           GUI_BLACK);
      }
      else
      {
        gui_display_string(SHOW_X_CENTERED((const char*)list[page*PAGE_MAX_SHOW_NUM+i]),
                           SHOW_Y_LINE(i),
                           (rt_uint8_t *)list[page*PAGE_MAX_SHOW_NUM+i],
                           GUI_WIHIT);
      }
		}
		else
		{
			if(i == pos)
			{
			  // 当前选中项
				gui_display_string(SHOW_X_CENTERED((const char*)list[page*PAGE_MAX_SHOW_NUM+i]),
				                   SHOW_Y_LINE(i),
				                   (rt_uint8_t *)list[page*PAGE_MAX_SHOW_NUM+i],
				                   GUI_BLACK);
			}
			else
			{
			gui_display_string(SHOW_X_CENTERED((const char*)list[page*PAGE_MAX_SHOW_NUM+i]),
			                   SHOW_Y_LINE(i),
			                   (rt_uint8_t *)list[page*PAGE_MAX_SHOW_NUM+i],
			                   GUI_WIHIT);
			}
		}
	}
	gui_display_update();
}


/** 
@brief  密码管理界面
				密码管理
				指纹管理
				手机管理
@param  none
@retval none
*/
void password_manage_ui(void)
{
	menu_list_show_ui(UserManageListText,0,sizeof(UserManageListText)/LCD_LINE_MAX_LEN);
}


/** 
@brief  指纹管理界面
				密码管理
				指纹管理
				手机管理

@param  none
@retval none
*/
void fprint_manage_ui(void)
{
  menu_list_show_ui(UserManageListText,1,sizeof(UserManageListText)/LCD_LINE_MAX_LEN);
}


/** 
@brief  手机号管理界面
				密码管理
				指纹管理
				手机管理

@param  none
@retval none
*/
void phone_manage_ui(void)
{
	menu_list_show_ui(UserManageListText,2,sizeof(UserManageListText)/LCD_LINE_MAX_LEN);
}


/** 
@brief  密码管理添加界面
				添加
				删除
@param  none
@retval none
*/
void password_add_ui(void)
{
	menu_list_show_ui(PasswordManageText,0,sizeof(PasswordManageText)/LCD_LINE_MAX_LEN);
}


/** 
@brief  密码管理删除界面
				添加
				删除
@param  none
@retval none
*/
void password_del_ui(void)
{
	menu_list_show_ui(PasswordManageText,1,sizeof(PasswordManageText)/LCD_LINE_MAX_LEN);
}


/** 
@brief  密码添加不同页面显示界面
				添加
				删除
@param  buf 显示缓冲区
@param  page 不同的显示界面
				@arg 0 输入新密码界面
				@arg 1 再输入一次的界面
				@arg 2 显示密码已存在
				@arg 3 显示密码长度不够
				@arg 4 录入新密码成功
				@arg 5 录入新密码失败
				@arg 6 两次输入不匹配
@retval none
*/
static void password_add_ui_page(rt_uint8_t *buf,rt_uint8_t page)
{
	rt_uint8_t CurUserPos;
	
	rt_uint8_t UserMaxPsNum;
  CurUserPos = account_cur_pos_get();
  
  UserMaxPsNum = user_valid_password_num();

	//初始化界面需要清屏
  if((page == 1)||(page == 0))
  {
    gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
  }

	menu_clear_line(3);
  switch(page)
  {
		case 0:
		{
			//输入新密码
			gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(0),PasswordAddText[0],GUI_WIHIT);
			rt_sprintf((char *)buf,"%03d",CurUserPos);
			gui_display_string(SHOW_X_ROW8(12),SHOW_Y_LINE(0),buf,GUI_WIHIT);

			//gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(1),PasswordAddText[1],GUI_WIHIT);
			//rt_sprintf((char *)buf,"%02d",UserMaxPsNum);
			//gui_display_string(SHOW_X_ROW8(13),SHOW_Y_LINE(1),buf,GUI_WIHIT);

			gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(1),PasswordAddText[2],GUI_WIHIT);
			break;
		}
		case 1:
		{
			//再输入一次
	    gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(0),PasswordAddText[0],GUI_WIHIT);
			rt_sprintf((char *)buf,"%03d",CurUserPos);
	    gui_display_string(SHOW_X_ROW8(12),SHOW_Y_LINE(0),buf,GUI_WIHIT);
	    
	    //gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(1),PasswordAddText[1],GUI_WIHIT);
	    //rt_sprintf((char *)buf,"%02d",UserMaxPsNum);
	    //gui_display_string(SHOW_X_ROW8(13),SHOW_Y_LINE(1),buf,GUI_WIHIT);

	    gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(1),PasswordAddText[3],GUI_WIHIT);
			break;
		}
		case 2:
		{
			gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PasswordAddText[4],GUI_WIHIT);
			gui_display_update();
			menu_input_sure_key(RT_TICK_PER_SECOND*2);
			//gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(3),SHOW_X_ROW8(15),SHOW_Y_LINE(4));

			break;
		}
		case 3:
		{
			gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PasswordAddText[5],GUI_WIHIT);
			gui_display_update();
			menu_input_sure_key(RT_TICK_PER_SECOND*2);
			//gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(3),SHOW_X_ROW8(15),SHOW_Y_LINE(4));

			break;
		}
		case 4:
		{
			gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PasswordAddText[6],GUI_WIHIT);
			gui_display_update();
			menu_input_sure_key(RT_TICK_PER_SECOND*2);
			//gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(3),SHOW_X_ROW8(15),SHOW_Y_LINE(4));

			break;
		}
		case 5:
		{
			gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PasswordAddText[7],GUI_WIHIT);
			gui_display_update();
			menu_input_sure_key(RT_TICK_PER_SECOND*2);
			//gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(3),SHOW_X_ROW8(15),SHOW_Y_LINE(4));

			break;
		}
		case 6:
		{
			gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PasswordAddText[8],GUI_WIHIT);
			gui_display_update();
			menu_input_sure_key(RT_TICK_PER_SECOND*2);
			//gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(3),SHOW_X_ROW8(15),SHOW_Y_LINE(4));

			break;
		}
		default:
		{
			break;
		}
  }
  gui_display_update();
}


/** 
@brief  输入一行字符串按确定键退出
@param  ShowBuf   显存
@param  DataBuf   数据缓冲区
@param  InputLen  需要输入长度
@param  StartLine 显示起始行
@param  ShowMode  显示模式
				@arg 0    不隐藏显示
				@arg 1		隐藏显示
@retval none
*/
static rt_err_t gui_input_string(rt_uint8_t *ShowBuf,
																		 	rt_uint8_t *DataBuf,
																		 	rt_uint8_t InputLen,
																		 	rt_uint8_t StartLine,
																		 	rt_uint8_t ShowMode)
{
	rt_uint8_t KeyValue; //按键接收值
	rt_err_t   result;   //接收结果用
	rt_err_t   FunResult; //函数运行结果
  rt_uint8_t GlintStatus;//闪烁状态

	while(1)
	{
		result = gui_key_input(&KeyValue);
		if(result == RT_EOK)
		{
			// 有效按键
      if(KeyValue >= '0' && KeyValue <= '9')
      {
      	// 输入0~9
        result = string_add_char(DataBuf,KeyValue,InputLen);
        if(result == RT_EOK)
        {
        	if(ShowMode == 1)
        	{
        		//隐藏字符串
            string_hide_string((const rt_uint8_t *)DataBuf,ShowBuf,SHOW_PW_HIDE_CH,PASSWORD_MAX_LEN);
        	}
          
          gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(StartLine),SHOW_X_ROW8(15),SHOW_Y_LINE(StartLine+1));
          gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(StartLine),ShowBuf,GUI_WIHIT);
        }
        else
        {
          //输入数量超过8个
        }
      }
      else if(KeyValue == MENU_SURE_VALUE)
      {
        FunResult = RT_EOK;
        break;
      }
      else if(KeyValue == MENU_DEL_VALUE)
      {
        result = string_del_char(DataBuf,PASSWORD_MAX_LEN);
        if(result == RT_EOK)
        {
        	if(ShowMode == 1)
        	{
        		//隐藏字符串
            string_hide_string((const rt_uint8_t *)DataBuf,ShowBuf,SHOW_PW_HIDE_CH,PASSWORD_MAX_LEN);
        	}
          gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(StartLine),SHOW_X_ROW8(15),SHOW_Y_LINE(StartLine+1));
          gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(StartLine),ShowBuf,GUI_WIHIT);
        }
        else
        {
        	//没有输入字符
          FunResult = RT_EEMPTY;
          break;
        }
      }

		}
		else
		{
			//操作超时
			if(menu_event_process(2.,MENU_EVT_OP_OUTTIME) == 0)
			{
			    return RT_ETIMEOUT;
			}
      GlintStatus++;
      // 闪烁提示
      menu_inputchar_glint(SHOW_X_ROW8(rt_strlen((const char *)ShowBuf)),SHOW_Y_LINE(StartLine),GlintStatus%2);
		}
		//更新显示
		gui_display_update();
	}

	return FunResult;
}


/** 
@brief  密码添加检查
@param  password 密码
@retval RT_EEMPTY 密码长度不够
				RT_ERROR  密码已存在
				RT_EOK    密码合法
*/
static rt_err_t Password_add_check(rt_uint8_t *password)
{
	rt_err_t result = RT_EOK;
	rt_int32_t Kresult;

	if(rt_strlen((const char*)password) < CONFIG_PASSWORD_LEN)
	{
		//新增密码为空
		rt_kprintf("rt_strlen((const char*)password) = %d\n",rt_strlen((const char*)password));
		return RT_EEMPTY;
	}

	Kresult = device_config_key_verify(KEY_TYPE_KBOARD,password,rt_strlen((const char*)password));
	if(Kresult >= 0)
	{
		result = RT_ERROR;
	}
	
	return result;
}


/** 
@brief  添加新密码处理
				输入新密码
				再输入一次
				显示结果
@param  none
@retval none
*/
void password_add_process(void)
{
	rt_uint8_t *ShowBuf; 		//显示缓冲区
	rt_uint8_t *DataBuf; 		//保存数据buf
	rt_uint8_t *Password; 	//密码
	rt_err_t   result;   		//接收结果用
	
	// 获取内存
	ShowBuf = rt_calloc(1,LCD_LINE_MAX_LEN);
	DataBuf = rt_calloc(1,LCD_LINE_MAX_LEN);
	Password = rt_calloc(1,PASSWORD_MAX_LEN);
	RT_ASSERT(ShowBuf != RT_NULL);
	RT_ASSERT(DataBuf != RT_NULL);
	RT_ASSERT(Password!= RT_NULL);

  while(1)
  {
  	// 显示初始化界面
 		password_add_ui_page(ShowBuf,0);
 		
		rt_memset(ShowBuf,0,LCD_LINE_MAX_LEN);
		rt_memset(DataBuf,0,LCD_LINE_MAX_LEN);

		result = gui_input_string(ShowBuf,DataBuf,PASSWORD_MAX_LEN,2,1);
		if(result != RT_EOK)
		{
			break;
		}
		else
		{
			// 检测密码是否合法
			result = Password_add_check(DataBuf);
      if(result != RT_EOK)
      {
      	// 密码不合法
      	menu_operation_result_handle(1);

				if(result == RT_EEMPTY)
				{
					//显示密码长度不够
					password_add_ui_page(ShowBuf,3);
				}
				else
				{
					// 显示密码不合法
					password_add_ui_page(ShowBuf,2);
				}
			
				continue;
      }
      else
      {
        // 新密码是合法的
        rt_memcpy(Password,DataBuf,PASSWORD_MAX_LEN);
      }
		}

		// 显示再次输入密码
 		password_add_ui_page(ShowBuf,1);
 		
		rt_memset(ShowBuf,0,LCD_LINE_MAX_LEN);
		rt_memset(DataBuf,0,LCD_LINE_MAX_LEN);

		result = gui_input_string(ShowBuf,DataBuf,PASSWORD_MAX_LEN,2,1);
		if(result == RT_EEMPTY)
		{
			// 取消第二次输入密码
			continue;
		}
		else if(result == RT_ETIMEOUT)
		{
			// 超时
			break;
		}
		else if(result == RT_EOK)
		{
			// 输入密码正确
			if(rt_memcmp(Password,DataBuf,rt_strlen((const char*)Password)) == 0)
			{
		    result = key_add_password(Password);
		    if(result == RT_EOK)
		    {
					result = account_cur_add_password(Password);
			    if(result == RT_EOK)
			    {
						// 录入成功
						password_add_ui_page(ShowBuf,4);
			    }
			    else
			    {
            password_add_ui_page(ShowBuf,5);
			    }
		    }
		    else
		    {
					password_add_ui_page(ShowBuf,5);
		    }
			}
			else
			{
				// 两次密码输入不一致
				password_add_ui_page(ShowBuf,6);
			}
		}
  }

	
	
	// 释放内存
	rt_free(ShowBuf);
	rt_free(DataBuf);	
	rt_free(Password);
}


/** 
@brief  密码删除不同页面显示界面
@param  buf 显存
@param  page 不同页面
				@arg 0 提示输入要删除密码的界面
				@arg 1 提示密码删除成功
				@arg 2 提示密码长度错误
				@arg 3 提示密码不存在
				@arg 4 提示删除失败
@retval none
*/
static void password_del_ui_page(rt_uint8_t *buf,rt_uint8_t page)
{
	rt_uint8_t CurUserPos;
	
	rt_uint8_t UserMaxPsNum;
  CurUserPos = account_cur_pos_get();
  
  UserMaxPsNum = user_valid_password_num();

	// 初始化界面需要清屏
  if((page == 0))
  {
    gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
  }

	menu_clear_line(3);
  switch(page)
  {
		case 0:
		{
			// 提示输入要删除的密码界面
			gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(0),PasswordDelText[0],GUI_WIHIT);
			rt_sprintf((char *)buf,"%03d",CurUserPos);
			gui_display_string(SHOW_X_ROW8(12),SHOW_Y_LINE(0),buf,GUI_WIHIT);

			gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(1),PasswordDelText[1],GUI_WIHIT);
			gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(2),PasswordDelText[2],GUI_WIHIT);
			break;
		}
		case 1:
		{
			// 提示密码删除成功
			gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PasswordDelText[3],GUI_WIHIT);
			gui_display_update();
			menu_input_sure_key(RT_TICK_PER_SECOND*2);

			break;
		}
		case 2:
		{
			// 提示密码长度错误
			gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PasswordDelText[4],GUI_WIHIT);
			gui_display_update();
			menu_input_sure_key(RT_TICK_PER_SECOND*2);
			break;
		}
		case 3:
		{ 
			// 提示密码不存在
			gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PasswordDelText[5],GUI_WIHIT);
			gui_display_update();
			menu_input_sure_key(RT_TICK_PER_SECOND*2);
			break;
		}
		case 4:
		{ 
			// 提示密码不存在
			gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PasswordDelText[6],GUI_WIHIT);
			gui_display_update();
			menu_input_sure_key(RT_TICK_PER_SECOND*2);
			break;
		}
		default:
		{
			break;
		}
  }
  gui_display_update();

}


/** 
@brief  删除密码检查
@param  none
@retval none
*/
static rt_err_t password_del_check(rt_uint8_t *password)
{
	rt_err_t result = RT_ERROR;
	rt_int32_t Kresult;
  rt_uint8_t CurUserPos;
  struct account_head *ah;

  CurUserPos = account_cur_pos_get();

	if(rt_strlen((const char*)password) < CONFIG_PASSWORD_LEN)
	{
		// 输入的密码不能为空
		rt_kprintf("rt_strlen((const char*)password) = %d\n",rt_strlen((const char*)password));
		return RT_EEMPTY;
	}

	Kresult = device_config_key_verify(KEY_TYPE_KBOARD,password,rt_strlen((const char*)password));
	if(Kresult >= 0)
	{
		rt_uint8_t i;
		
		ah = rt_calloc(1,sizeof(*ah));
		RT_ASSERT(ah != RT_NULL);

		device_config_account_operate(CurUserPos,ah,0);

		for(i=0;i<ACCOUNT_KEY_NUMBERS;i++)
		{
			if(ah->key[i] == Kresult)
			{
				result = RT_EOK;
				break;
			}
		}
		rt_free(ah);
	}
	
	return result;
}


/** 
@brief  删除新密码处理
				输入要删除密码
				显示结果
@param  none
@retval none
*/
void password_del_process(void)
{
	rt_uint8_t *ShowBuf; 		//显示缓冲区
	rt_uint8_t *DataBuf; 		//保存数据buf
  rt_err_t   result;      //接收结果用

	// 获取内存
	ShowBuf = rt_calloc(1,LCD_LINE_MAX_LEN);
	DataBuf = rt_calloc(1,LCD_LINE_MAX_LEN);
	RT_ASSERT(ShowBuf != RT_NULL);
	RT_ASSERT(DataBuf != RT_NULL);

	while(1)
	{
    password_del_ui_page(ShowBuf,0);

    rt_memset(ShowBuf,0,LCD_LINE_MAX_LEN);
		rt_memset(DataBuf,0,LCD_LINE_MAX_LEN);

		result = gui_input_string(ShowBuf,DataBuf,PASSWORD_MAX_LEN,3,1);
		if(result != RT_EOK)
		{
			// 超时或者返回
			break;
		}
		else
		{
			result = password_del_check(DataBuf);
			if(result == RT_EEMPTY)
			{
				//密码长度错误
				password_del_ui_page(ShowBuf,2);
			}
			else if(result == RT_ERROR)
			{
				//没有这个密码
				password_del_ui_page(ShowBuf,3);
			}
			else if(result == RT_EOK)
			{
				//可以删除
				result = key_password_str_delete(DataBuf);
				if(result != RT_EOK)
				{
					//删除失败
					password_del_ui_page(ShowBuf,4);
					
				}
				password_del_ui_page(ShowBuf,1);
			}
			
		}
	}
	

	// 释放内存
	rt_free(ShowBuf);
	rt_free(DataBuf);	
}


/** 
@brief  指纹添加不同页面显示界面
				添加
				删除
@param  buf 显示缓冲区
@param  page 不同的显示界面
				@arg 0 指纹采集开始界面
				@arg 1 指纹真正采集界面
				@arg 2 采集数据失败
				@arg 3 该指纹已存在
				@arg 4 添加成功
				@arg 5 添加失败
@retval none
*/
static void fprint_add_ui_page(rt_uint8_t *buf,rt_uint8_t page)
{
	rt_uint8_t CurUserPos;
	
	rt_uint8_t UserMaxPsNum;
  CurUserPos = account_cur_pos_get();
  
  UserMaxPsNum = user_valid_password_num();

	//初始化界面需要清屏
  if((page == 0))
  {
    gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
  }

	menu_clear_line(3);
  switch(page)
  {
		case 0:
		{
			// 新指纹采集界面
			gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(0),FprintAddText[0],GUI_WIHIT);
			rt_sprintf((char *)buf,"%03d",CurUserPos);
			gui_display_string(SHOW_X_ROW8(12),SHOW_Y_LINE(0),buf,GUI_WIHIT);
			
			gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(1),FprintAddText[1],GUI_WIHIT);
			
			gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(3),FprintAddText[2],GUI_WIHIT);
			break;
		}
		case 1:
		{
			// 指纹正采集界面
	    gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),FprintAddText[3],GUI_WIHIT);
			gui_display_update();
			menu_input_sure_key(RT_TICK_PER_SECOND*2);
			break;
		}
		case 2:
		{ 
			// 采集数据失败
			gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),FprintAddText[4],GUI_WIHIT);
			gui_display_update();
			menu_input_sure_key(RT_TICK_PER_SECOND*2);
			break;
		}
		case 3:
		{ 
			// 该指纹已存在
			gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),FprintAddText[5],GUI_WIHIT);
			gui_display_update();
			menu_input_sure_key(RT_TICK_PER_SECOND*2);
			break;
		}
		case 4:
		{ 
			// 提示添加成功
			gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),FprintAddText[6],GUI_WIHIT);
			gui_display_update();
			menu_input_sure_key(RT_TICK_PER_SECOND*2);
			break;
		}
		case 5:
		{ 
			// 提示添加失败
			gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),FprintAddText[7],GUI_WIHIT);
			gui_display_update();
			menu_input_sure_key(RT_TICK_PER_SECOND*2);
			break;
		}
		default:
		{
			break;
		}
  }
  gui_display_update();
}


/** 
@brief  指纹添加界面
@param  none
@retval none
*/
void fprint_add_ui(void)
{
	menu_list_show_ui(FprintManageText,0,sizeof(FprintManageText)/LCD_LINE_MAX_LEN);
}


/** 
@brief  指纹添加界面
@param  none
@retval none
*/
void fprint_del_ui(void)
{
  menu_list_show_ui(FprintManageText,1,sizeof(FprintManageText)/LCD_LINE_MAX_LEN);
}

/** 
@brief  指纹添加界面
@param  none
@retval none
*/
void fprint_add_process(void)
{
	rt_uint8_t *ShowBuf; 		//显示缓冲区
	rt_uint8_t KeyValue;		//按键值
	rt_err_t   result;			//结果
	// 获取内存
	ShowBuf = rt_calloc(1,LCD_LINE_MAX_LEN);
	
	while(1)
	{
		
		// 指纹采集初始化界面
		fprint_add_ui_page(ShowBuf,0);

		result = gui_key_input(&KeyValue);
		if(result == RT_EOK)
		{
			
		}
	}

	// 释放内存
	rt_free(ShowBuf);
}


/** 
@brief  指纹添加界面
@param  none
@retval none
*/
void fprint_del_process(void)
{

}


/** 
@brief  手机号添加界面
@param  none
@retval none
*/
void phone_add_ui(void)
{
	menu_list_show_ui(PhoneManageText,0,sizeof(PhoneManageText)/LCD_LINE_MAX_LEN);
}


/** 
@brief  手机删除界面
@param  none
@retval none
*/
void phone_del_ui(void)
{
	menu_list_show_ui(PhoneManageText,1,sizeof(PhoneManageText)/LCD_LINE_MAX_LEN);
}


/** 
@brief  密码添加不同页面显示界面
				添加
				删除
@param  buf 显示缓冲区
@param  page 不同的显示界面
				@arg 0 输入新手机号码界面
				@arg 1 再输入一次手机号码的界面
				@arg 2 输入的手机号码长度不对
				@arg 3 输入的手机号码已经存在
				@arg 4 手机号添加成功
				@arg 5 手机号添加失败
				@arg 6 两次输入不匹配
@retval none
*/
static void phone_add_ui_page(rt_uint8_t *buf,rt_uint8_t page)
{
	rt_uint8_t CurUserPos;
	
	rt_uint8_t UserMaxPsNum;
  CurUserPos = account_cur_pos_get();
  
  UserMaxPsNum = user_valid_password_num();

	// 初始化界面需要清屏
  if((page == 1)||(page == 0))
  {
    gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
  }

	// 清除第四行
	menu_clear_line(3);
  switch(page)
  {
		case 0:
		{
			//输入新密码
			gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(0),PhoneAddText[0],GUI_WIHIT);
			rt_sprintf((char *)buf,"%03d",CurUserPos);
			gui_display_string(SHOW_X_ROW8(12),SHOW_Y_LINE(0),buf,GUI_WIHIT);
			
			gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(1),PhoneAddText[1],GUI_WIHIT);
			break;
		}
		case 1:
		{
			//再输入一次
	    gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(0),PhoneAddText[0],GUI_WIHIT);
			rt_sprintf((char *)buf,"%03d",CurUserPos);
	    gui_display_string(SHOW_X_ROW8(12),SHOW_Y_LINE(0),buf,GUI_WIHIT);

	    gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(1),PhoneAddText[2],GUI_WIHIT);
			break;
		}
		case 2:
		{ 
			// 长度错我
			gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PhoneAddText[3],GUI_WIHIT);
			gui_display_update();
			menu_input_sure_key(RT_TICK_PER_SECOND*2);
			break;
		}
		case 3:
		{ 
			// 提示手机号已经存在
			gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PhoneAddText[4],GUI_WIHIT);
			gui_display_update();
			menu_input_sure_key(RT_TICK_PER_SECOND*2);
			break;
		}
		case 4:
		{ 
			// 提示添加成功
			gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PhoneAddText[5],GUI_WIHIT);
			gui_display_update();
			menu_input_sure_key(RT_TICK_PER_SECOND*2);
			break;
		}
		case 5:
		{ 
			// 提示添加失败
			gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PhoneAddText[6],GUI_WIHIT);
			gui_display_update();
			menu_input_sure_key(RT_TICK_PER_SECOND*2);
			break;
		}
		case 6:
		{ 
			// 提示两次输入不匹配
			gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PhoneAddText[7],GUI_WIHIT);
			gui_display_update();
			menu_input_sure_key(RT_TICK_PER_SECOND*2);
			break;
		}
		default:
		{
			break;
		}
  }
  gui_display_update();
}


/** 
@brief  手机号码添加处理
@param  none
@retval none
*/
void phone_add_process(void)
{
	rt_uint8_t *ShowBuf; 		//显示缓冲区
	rt_uint8_t *DataBuf; 		//保存数据buf
	rt_uint8_t *phone; 			//手机号
	rt_err_t   result;   		//接收结果用
	
	// 获取内存
	ShowBuf = rt_calloc(1,LCD_LINE_MAX_LEN);
	DataBuf = rt_calloc(1,LCD_LINE_MAX_LEN);
	phone = rt_calloc(1,PHONE_MAX_LEN);
	RT_ASSERT(ShowBuf != RT_NULL);
	RT_ASSERT(DataBuf != RT_NULL);
	RT_ASSERT(phone != RT_NULL);

  while(1)
  {
  	// 显示初始化界面
 		phone_add_ui_page(ShowBuf,0);
 		
		rt_memset(ShowBuf,0,LCD_LINE_MAX_LEN);
		rt_memset(DataBuf,0,LCD_LINE_MAX_LEN);

		result = gui_input_string(ShowBuf,DataBuf,PHONE_MAX_LEN,2,0);
		if(result != RT_EOK)
		{
			break;
		}
		else
		{
			// 检测手机是否合法
			result = user_phone_add_check(DataBuf);
      if(result != RT_EOK)
      {
      	// 输入的手机号不合法
      	menu_operation_result_handle(1);

				if(result == RT_EEMPTY)
				{
					// 显示手机号长度不够
					phone_add_ui_page(ShowBuf,2);
				}
				else
				{
					// 显示手机号已经存在
					phone_add_ui_page(ShowBuf,3);
				}
			
				continue;
      }
      else
      {
        // 手机号合法
        rt_memcpy(phone,DataBuf,PHONE_MAX_LEN);
      }
		}

		// 显示再次输入手机号
 		phone_add_ui_page(ShowBuf,1);
 		
		rt_memset(ShowBuf,0,LCD_LINE_MAX_LEN);
		rt_memset(DataBuf,0,LCD_LINE_MAX_LEN);

		result = gui_input_string(ShowBuf,DataBuf,PHONE_MAX_LEN,2,0);
		if(result == RT_EEMPTY)
		{
			// 取消第二次输入手机号
			continue;
		}
		else if(result == RT_ETIMEOUT)
		{
			// 超时
			break;
		}
		else if(result == RT_EOK)
		{
			// 输入手机号完成
			if(rt_memcmp(phone,DataBuf,rt_strlen(phone)) == 0)
			{
		    result = user_cur_add_phone(phone);
		    if(result == RT_EOK)
		    {
					// 添加成功
					phone_add_ui_page(ShowBuf,4);
		    }
		    else
		    {
		    	// 添加失败
					phone_add_ui_page(ShowBuf,5);
		    }
			}
			else
			{
				// 两次输入不一致
				phone_add_ui_page(ShowBuf,6);
			}
		}
  }

	// 释放内存
	rt_free(ShowBuf);
	rt_free(DataBuf);	
	rt_free(phone);

}


/** 
@brief  电话删除不同页面显示界面
@param  buf 显存
@param  page 不同页面
				@arg 0 提示输入要删除电话码的界面
				@arg 1 提示手机号删除成功
				@arg 2 提示输入手机号长度错误
				@arg 3 提示手机号不存在
				@arg 4 提示删除失败
@retval none
*/
static void phone_del_ui_page(rt_uint8_t *buf,rt_uint8_t page)
{
	rt_uint8_t CurUserPos;
	
	rt_uint8_t UserMaxPsNum;
  CurUserPos = account_cur_pos_get();
  
  UserMaxPsNum = user_valid_password_num();

	// 初始化界面需要清屏
  if((page == 0))
  {
    gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
  }

	gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(3),SHOW_X_ROW8(15),SHOW_Y_LINE(4));
  switch(page)
  {
		case 0:
		{
			// 提示输入要删除的密码界面
			gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(0),PhoneDelText[0],GUI_WIHIT);
			rt_sprintf((char *)buf,"%03d",CurUserPos);
			gui_display_string(SHOW_X_ROW8(12),SHOW_Y_LINE(0),buf,GUI_WIHIT);

			gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(1),PhoneDelText[1],GUI_WIHIT);
			gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(2),PhoneDelText[2],GUI_WIHIT);
			break;
		}
		case 1:
		{
			// 提示密码删除成功
			gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PhoneDelText[3],GUI_WIHIT);
			gui_display_update();
			menu_input_sure_key(RT_TICK_PER_SECOND*2);

			break;
		}
		case 2:
		{
			// 提示密码长度错误
			gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PhoneDelText[4],GUI_WIHIT);
			gui_display_update();
			menu_input_sure_key(RT_TICK_PER_SECOND*2);
			break;
		}
		case 3:
		{ 
			// 提示密码不存在
			gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PhoneDelText[5],GUI_WIHIT);
			gui_display_update();
			menu_input_sure_key(RT_TICK_PER_SECOND*2);
			break;
		}
		case 4:
		{ 
			// 提示删除失败
			gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),PhoneDelText[6],GUI_WIHIT);
			gui_display_update();
			menu_input_sure_key(RT_TICK_PER_SECOND*2);
			break;
		}
		default:
		{
			break;
		}
  }
  gui_display_update();

}


/** 
@brief  删除手机检查
@param  none
@retval none
*/
static rt_err_t phone_del_check(rt_uint8_t *phone)
{
	rt_err_t result = RT_ERROR;
	rt_int32_t PHresult;
  rt_uint8_t CurUserPos;
  struct account_head *ah;

  CurUserPos = account_cur_pos_get();

	if(rt_strlen((const char*)phone) < CONFIG_PHONE_LET)
	{
		// 输入的手机不能为空
		rt_kprintf("rt_strlen((const char*)phone) = %d\n",rt_strlen((const char*)phone));
		return RT_EEMPTY;
	}

	PHresult = device_config_phone_verify(phone,rt_strlen((const char*)phone));
	if(PHresult >= 0)
	{
		rt_uint8_t i;
		
		ah = rt_calloc(1,sizeof(*ah));
		RT_ASSERT(ah != RT_NULL);

		device_config_account_operate(CurUserPos,ah,0);

		for(i=0;i<ACCOUNT_KEY_NUMBERS;i++)
		{
			if(ah->phone[i] == PHresult)
			{
				result = RT_EOK;
				break;
			}
		}
		rt_free(ah);
	}
	
	return result;
}


/** 
@brief  手机号码删除处理
@param  none
@retval none
*/
void phone_del_process(void)
{
	rt_uint8_t *ShowBuf; 		//显示缓冲区
	rt_uint8_t *DataBuf; 		//保存数据buf
  rt_err_t   result;      //接收结果用

	// 获取内存
	ShowBuf = rt_calloc(1,LCD_LINE_MAX_LEN);
	DataBuf = rt_calloc(1,LCD_LINE_MAX_LEN);
	RT_ASSERT(ShowBuf != RT_NULL);
	RT_ASSERT(DataBuf != RT_NULL);

	while(1)
	{
    phone_del_ui_page(ShowBuf,0);

    rt_memset(ShowBuf,0,LCD_LINE_MAX_LEN);
		rt_memset(DataBuf,0,LCD_LINE_MAX_LEN);

		result = gui_input_string(ShowBuf,DataBuf,PHONE_MAX_LEN,3,1);
		if(result != RT_EOK)
		{
			// 超时或者返回
			break;
		}
		else
		{
			result = phone_del_check(DataBuf);
			if(result == RT_EEMPTY)
			{
				//密码长度错误
				phone_del_ui_page(ShowBuf,2);
			}
			else if(result == RT_ERROR)
			{
				//没有这个密码
				phone_del_ui_page(ShowBuf,3);
			}
			else if(result == RT_EOK)
			{
				//可以删除
				result = user_phone_string_delete(DataBuf);
				if(result != RT_EOK)
				{
					//删除失败
					phone_del_ui_page(ShowBuf,4);
					
				}
				phone_del_ui_page(ShowBuf,1);
			}
			
		}
	}
	

	// 释放内存
	rt_free(ShowBuf);
	rt_free(DataBuf);	
}




