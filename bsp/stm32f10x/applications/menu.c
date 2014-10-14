#include"menu.h"
#include"menu_1.h"
#include"menu_2.h"
#include"menu_3.h"
#include"unlock_ui.h"
#ifdef USEING_BUZZER_FUN 	
#include "buzzer.h"
#endif

#define SHOW_GLINT_CH							"_"

typedef void (*fun1)(void);

MenuManageDef MenuManage = 
{
	RT_NULL,
};

rt_uint8_t KeyFuncIndex = 0;
fun1 current_operation_index = RT_NULL;

fun1 cur_run_processing = RT_NULL;


//void(*current_operation_index)(void);


//菜单列表
KbdTabStruct	KeyTab[KEY_MAX_MENU_NUM] = 
{
	//顶层
	{0,0,0,1,0,menu_0_processing},//登陆界面

	//二级
	{1,2,3,4,3,menu_1_processing},//用户管理
	{2,3,1,6,3,menu_2_processing},//系统设置 
  {3,1,2,0,3,menu_3_processing},//用户管理

	//三级
	{4,5,5,8,1,menu_4_processing},//用户新增
	{5,4,4,22,1,menu_5_processing},//用户修改
	
	{6,7,7,20,2,menu_6_processing},//系统信息
	{7,6,6,7,2,menu_7_processing},//系统参数

	//四级
	{8,9,13,14,13,menu_8_processing},//新增密码 >>同时创建账号
	{9,10,8,15,13,menu_9_processing},//新增指纹
	{10,11,9,16,13,menu_10_processing},//新增手机
	{11,12,10,17,13,menu_11_processing},//保存退出
	{12,13,11,18,12,menu_12_processing},//查看信息
	{13,8,12,19,13,menu_13_processing},//退出

	//五级
	{14,14,14,14,8,menu_14_processing},//录入密码
	{15,15,15,15,9,menu_15_processing},//录入指纹
	{16,16,16,16,10,menu_16_processing},//录入电话
	{17,17,17,4,4,menu_17_processing},//保存退出
	{18,18,18,18,12,menu_18_processing},//查看信息
	{19,19,19,4,13,menu_19_processing},//退出

	//四级
	{20,20,20,20,6,menu_20_processing},//显示本机信息
	{21,21,21,21,7,menu_21_processing},//显示本机信息

	{22,22,22,23,5,menu_22_processing},//用户搜索界面

	//五级
	{23,24,27,28,26,menu_23_processing},//修改密码
	{24,25,23,29,26,menu_24_processing},//修改指纹
	{25,26,24,30,26,menu_25_processing},//修改电话
	{26,27,25,5,26,menu_26_processing},//保存退出
	{27,23,26,31,27,menu_27_processing},//退出
	
	{28,23,24,25,23,menu_28_processing},//修改密码处理
	{29,23,24,25,24,menu_29_processing},//修改指纹处理
	{30,23,24,25,25,menu_30_processing},//修改电话处理
	{31,31,31,5,26,menu_31_processing},//删除用户处理

	//三级菜单
	//{32,4,5,33,1,menu_32_processing},//管理员修改
	//{33,33,33,33,32,menu_33_processing},//管理员密码修改
};

//系统进入菜单
#define SYSTEM_ENTER_MENU_NUM				10
rt_uint8_t	SystemFuncIndex = 0;
fun1 System_menu_index = RT_NULL;

KbdTabStruct	SystemMenu[SYSTEM_ENTER_MENU_NUM] = 
{
  {0,1,1,2,0,system_menu1_show},//显示开锁
  {1,0,0,3,1,system_menu2_show},//系统管理
  {2,2,2,2,0,unlock_process_ui},//开锁
  {3,3,3,3,1,system_manage_processing},//进入系统管理
};

//系统进入界面处理
void system_entry_ui_processing(void)
{
  rt_err_t result;
	rt_uint8_t KeyValue;

	result = gui_key_input(&KeyValue);
	if (result == RT_EOK) 
	{ 
	  switch(KeyValue)
	  {
	    case MENU_SURE_VALUE:
	    {
	      //确定
	      if(System_menu_index == RT_NULL)
	      {
	        break;
	      }
	      SystemFuncIndex = SystemMenu[ SystemFuncIndex].SureState;
	      break;
	    }
	    case MENU_DEL_VALUE:
	    {
	      //取消
	      SystemFuncIndex = SystemMenu[ SystemFuncIndex].BackState;
	      break;
	    }
	    case MENU_UP_VALUE:
	    {
	      //上
	      SystemFuncIndex = SystemMenu[ SystemFuncIndex].UpState;
	      break;
	    }
	    case MENU_DOWN_VALUE:
	    {
	      //下
	      SystemFuncIndex = SystemMenu[ SystemFuncIndex].DnState;
	      break;
	    }
	    default:
	    {
	    	SystemFuncIndex = SystemMenu[ SystemFuncIndex].SureState;
	      break;
	    }
	  }
	  System_menu_index = SystemMenu[SystemFuncIndex].CurrentOperate;
	  System_menu_index();
	}
	else
	{
		//操作超时
		if(menu_event_process(2,MENU_EVT_OP_OUTTIME) == 0)
		{
      system_menu_choose(0);
		}
	}
}

//系统管理菜单处理
void system_manage_ui_processing(void)
{
	rt_err_t result;
	rt_uint8_t KeyValue;
	
	result = gui_key_input(&KeyValue);
	if (result == RT_EOK) 
	{	
		rt_kprintf("recv key value :%c\n",KeyValue);
		/*if(KeyValue > '0' && KeyValue < '8')
		{
			rt_uint8_t MenuPos;
			rt_uint8_t i;
			
			MenuPos -= '0';
			MenuPos -= 1;
			
			rt_kprintf("快捷键%d\n",MenuPos);
			for(i = 0;i<MenuPos%4;i++)
			{
        SystemFuncIndex = SystemMenu[ SystemFuncIndex].DnState;
			}
			SystemFuncIndex = SystemMenu[ SystemFuncIndex].SureState;
			System_menu_index = SystemMenu[SystemFuncIndex].CurrentOperate;
	 		System_menu_index();
		}*/
		switch(KeyValue)
		{
			case MENU_SURE_VALUE:
			{
				//确定
				if(current_operation_index == RT_NULL)
				{
					break;
				}
				KeyFuncIndex = KeyTab[ KeyFuncIndex].SureState;
				break;
			}
			case MENU_DEL_VALUE:
			{
				//取消
				KeyFuncIndex = KeyTab[ KeyFuncIndex].BackState;
				break;
			}
			case MENU_UP_VALUE:
			{
				//上
				KeyFuncIndex = KeyTab[ KeyFuncIndex].UpState;
				break;
			}
			case MENU_DOWN_VALUE:
			{
				//下
				KeyFuncIndex = KeyTab[ KeyFuncIndex].DnState;
				break;
			}
			default:
			{
				break;
			}
		}
		current_operation_index = KeyTab[KeyFuncIndex].CurrentOperate;
		current_operation_index();
	}
	else
	{
    //操作超时
	  if(menu_event_process(2,MENU_EVT_OP_OUTTIME) == 0)
	  {
	    system_menu_choose(0);
	  }
	}
}

void system_menu_choose(rt_uint8_t menu)
{
	switch(menu)
	{
		case 0:
		{
			SystemFuncIndex = 0;
			KeyFuncIndex = 0;
			System_menu_index = RT_NULL;
			cur_run_processing = system_entry_ui_processing;
			break;
		}
		case 1:
		{
			KeyFuncIndex = 0;
			SystemFuncIndex = 0;
			current_operation_index = RT_NULL;
			cur_run_processing = system_manage_ui_processing;

			//KeyFuncIndex = KeyTab[ KeyFuncIndex].SureState;
			current_operation_index = KeyTab[KeyFuncIndex].CurrentOperate;
			current_operation_index();
			break;
		}
		default:
		{
			cur_run_processing = system_entry_ui_processing;
			break;
		}
	}
}

void key_input_processing_init(void)
{
	if(MenuManage.event == RT_NULL)
	{
		MenuManage.event = rt_event_create("menu",RT_IPC_FLAG_FIFO);
		RT_ASSERT(MenuManage.event != RT_NULL);
	}
  system_menu_choose(0);
}

void key_input_processing(void)
{
	cur_run_processing();
}

//菜单运行确定按钮处理函数
void menu_run_sure_process(void)
{
	KeyFuncIndex = KeyTab[ KeyFuncIndex].SureState;
	current_operation_index = KeyTab[KeyFuncIndex].CurrentOperate;
	current_operation_index();
}


//从字符串中添加字符
rt_err_t string_add_char(rt_uint8_t str[],rt_uint8_t ch,rt_uint8_t str_size)
{
	rt_uint8_t i;

	RT_ASSERT(str_size > 1);
	
	for(i=0;i<str_size-1;i++)
	{
		if(str[i] == 0)
		{
			str[i] = ch;
			str[i+1] = 0;
			    
			return RT_EOK;
		}
	}

	return RT_ERROR;
}

//从字符串最后删除一个字符
rt_err_t string_del_char(rt_uint8_t str[],rt_uint8_t str_size)
{
	rt_uint8_t i;

	RT_ASSERT(str_size > 1);
	
	for(i=0;i<str_size;i++)
	{
		if(i == 0)
		{
			if(str[i] == 0)
			{
				return RT_ERROR;
			}
		}
		else if(str[i] == 0)
		{
			str[i-1] = 0;

			return RT_EOK;
		}
	}

	return RT_ERROR;
}

//字符串转换为隐藏字符穿
void string_hide_string(const rt_uint8_t src[],rt_uint8_t str[],rt_uint8_t ch,rt_uint8_t size)
{
  rt_uint8_t i;
  
	RT_ASSERT(size > 1);

  rt_memset(str,0,size);

  rt_kprintf("input :%s\n",src);  

	for(i=0;i<rt_strlen((const char*)src);i++)
	{
		if(src[i] != 0)
		{
			str[i] = ch;
		}
	}
	//rt_kprintf("hide  :%s\n",str);   
}

void menu_inputchar_glint(rt_uint8_t x,rt_uint8_t y,rt_uint8_t status)
{
	if(status == 1)
	{
		gui_display_string(x,y,SHOW_GLINT_CH,GUI_WIHIT);
	}
	else
	{
    gui_clear(x,y,x+8,y+16);
	}
}

void menu_error_handle(rt_uint8_t type)
{
	switch(type)
	{
		case 1:
		{
			#ifdef USEING_BUZZER_FUN
			buzzer_send_mail(BZ_TYPE_ERROR1);
			#endif
			break;
		}
		case 2:
		{
			break;
		}
		case 3:
		{
			//密码错误
			#ifdef USEING_BUZZER_FUN
			buzzer_send_mail(BZ_TYPE_ERROR3);
			gprs_key_error_mail(KEY_TYPE_KBOARD);
			#endif
			break;
		}
		default:
		{
			break;
		}
	}
}

/*
功能:操作菜单中事件
参数:mode 模式  type 事件类型
返回: -------------------------
		 |模式 |成功|失败|功能    |
		 |0    |0   |1   |发送事件|
		 |1    |0   |1   |收到事件|
		 |2    |0   |1   |清除事件|
		 --------------------------
*/
rt_uint8_t menu_event_process(rt_uint8_t mode,rt_uint32_t type)
{
	rt_uint32_t value;
	rt_err_t    result;
	rt_uint8_t  return_data = 1;
	
	//net_evt_mutex_op(RT_TRUE);

	if(MenuManage.event == RT_NULL)
	{
    MenuManage.event  = rt_event_create("menu",RT_IPC_FLAG_FIFO);
    RT_ASSERT(MenuManage.event  != RT_NULL);
	}
	switch(mode)
	{
		case 0:	//set event 
		{
			result = rt_event_send(MenuManage.event ,type);
			if(result == RT_EOK)
			{
				return_data = 0;
			}
			break;
		}
		case 1:	//get event 
		{
			result = rt_event_recv(MenuManage.event ,
			                       type,
			                       RT_EVENT_FLAG_OR,
			                       RT_WAITING_NO,&value);
			if(result == RT_EOK)
			{
				return_data = 0;
			}
			else if(result == -RT_ETIMEOUT)
			{
				return_data = 1;
			}
			break;
		}
		case 2://clean event
		{
			result = rt_event_recv(MenuManage.event,
			                       type,
			                       RT_EVENT_FLAG_OR | 
			                       RT_EVENT_FLAG_CLEAR,
			                       RT_WAITING_NO,&value);
			if(result == RT_EOK)
			{
				return_data = 0;
			}
			break;
		}
    case 3://clean all event 
    {
      result = rt_event_recv(MenuManage.event,
                             0xffffffff,
                             RT_EVENT_FLAG_OR | 
                             RT_EVENT_FLAG_CLEAR,
                             RT_WAITING_NO,&value);
      if(result == RT_EOK)
      {
        return_data = 0;
      }
      break;
    }
    default:
    {
			break;
    }
	}

	//net_evt_mutex_op(RT_FALSE);
	return return_data;
}


