#include"menu.h"
#include"menu_1.h"
static rt_uint8_t KeyFuncIndex = 0;
void(*current_operation_index)(void);


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


//菜单列表
static KbdTabStruct	KeyTab[KEY_MAX_MENU_NUM] = 
{
	{0,0,0,0,0,menu_1_processing},
};

static rt_mq_t key_mq;

////////////////////////////////////////////////////////////////////////////////////////////////////
//key api
typedef struct {
	uint16_t type;
	KB_MODE_TYPEDEF mode;
	uint8_t c;
}KB_MAIL_TYPEDEF;

rt_err_t send_key_value_mail(uint16_t type, KB_MODE_TYPEDEF mode, uint8_t c)
{
	rt_err_t result = -RT_EFULL;
	KB_MAIL_TYPEDEF mail;

	if(key_mq == RT_NULL)
	{
		key_mq = rt_mq_create("kboard", sizeof(KB_MAIL_TYPEDEF),
						 5, RT_IPC_FLAG_FIFO);
	}
	if (key_mq != RT_NULL)
	{
		mail.type = type;
		mail.mode = mode;
		mail.c = c;
		result = rt_mq_send(key_mq, &mail, sizeof(mail));
		if (result == -RT_EFULL)
		{
#if (defined RT_USING_FINSH) && (defined KB_DEBUG)
			rt_kprintf("kb_mq is full!!!\n");
#endif
		}
	}
	else
	{
#if (defined RT_USING_FINSH) && (defined KB_DEBUG)
		rt_kprintf("kb_mq is RT_NULL!!!!\n");
#endif
	}
    return result;
}
////////////////////////////////////////////////////////////////////////////////////////////////////

void key_input_processing(void)
{
	rt_uint8_t KeyValue;
	rt_err_t result;
	KB_MAIL_TYPEDEF mail;

	rt_memset(&mail, 0, sizeof(mail));

	if(key_mq == RT_NULL)
	{
		key_mq = rt_mq_create("kboard", sizeof(KB_MAIL_TYPEDEF),
						 5, RT_IPC_FLAG_FIFO);
	}
	result = rt_mq_recv(key_mq, &mail, sizeof(mail), 100);
	if (result == RT_EOK) 
	{
		rt_kprintf("recv key value :%x\n",mail.c);
		switch(mail.c)
		{
			case 1:
			{
				
				KeyFuncIndex = KeyTab[ KeyFuncIndex].SureState;
				break;
			}
			case 2:
			{
				KeyFuncIndex = KeyTab[ KeyFuncIndex].BackState;
				break;
			}
			case 3:
			{
				KeyFuncIndex = KeyTab[ KeyFuncIndex].UpState;
				break;
			}
			case 4:
			{
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