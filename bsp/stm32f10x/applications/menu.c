#include"menu.h"
#include"menu_1.h"
#include"menu_2.h"
#define SHOW_GLINT_CH							"_"


rt_uint8_t KeyFuncIndex = 0;
typedef void (*fun1)(void);

fun1 current_operation_index = RT_NULL;

//void(*current_operation_index)(void);


//菜单列表
KbdTabStruct	KeyTab[KEY_MAX_MENU_NUM] = 
{
	//顶层
	{0,0,0,1,0,menu_0_processing},//登陆界面

	//二级
	{1,2,3,4,1,menu_1_processing},//用户管理
	{2,3,1,6,1,menu_2_processing},//系统设置 
  {3,1,2,0,1,menu_3_processing},//用户管理

	//三级
	{4,5,5,8,1,menu_4_processing},//用户新增
	{5,4,4,5,1,menu_5_processing},//用户修改
	
	{6,7,7,6,2,menu_6_processing},//系统信息
	{7,6,6,7,2,menu_7_processing},//系统参数

	//四级
	{8,9,13,14,8,menu_8_processing},//新增密码 >>同时创建账号
	{9,10,8,15,8,menu_9_processing},//新增指纹
	{10,11,9,16,8,menu_10_processing},//新增手机
	{11,12,10,17,8,menu_11_processing},//保存退出
	{12,13,11,18,12,menu_12_processing},//查看信息
	{13,8,12,4,8,menu_13_processing},//退出

	//五级
	{14,14,14,14,8,menu_14_processing},//录入密码
	{15,14,14,14,8,menu_15_processing},//新增密码处理
	{16,14,14,14,8,menu_16_processing},//新增密码处理
	{17,14,14,8,11,menu_17_processing},//新增密码处理
	{18,14,14,14,8,menu_18_processing},//新增密码处理
	{19,14,14,14,8,menu_19_processing},//新增密码处理
};


void key_input_processing(void)
{
	rt_err_t result;
	rt_uint8_t KeyValue;
	
	result = gui_key_input(&KeyValue);
	if (result == RT_EOK) 
	{	
		rt_kprintf("recv key value :%c\n",KeyValue);
		switch(KeyValue)
		{
			case '*':
			{
				//确定
				if(current_operation_index == RT_NULL)
				{
					break;
				}
				KeyFuncIndex = KeyTab[ KeyFuncIndex].SureState;
				break;
			}
			case '#':
			{
				//取消
				KeyFuncIndex = KeyTab[ KeyFuncIndex].BackState;
				break;
			}
			case '8':
			{
				//上
				KeyFuncIndex = KeyTab[ KeyFuncIndex].UpState;
				break;
			}
			case '0':
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
	rt_kprintf("hide  :%s\n",str);   
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



