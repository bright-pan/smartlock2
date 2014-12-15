#include "KeyModifyUI.h"
#include "fprint.h"
#include "local.h"
#define ADMIN_DATA_POS						0
#include "menu_2.h"



static const rt_uint8_t FprintModifyText[][8*2] = 
{
	{"1.新增指纹"},
	{"2.修改指纹"},
	{"3.删除指纹"},
	{"当前用户编号:"},
	{"请按*采集新指纹"},
	{"正在采集. . ."},
	{"注册成功^_^"},
	{"注册失败@@@"},
	{"请输入原指纹"},
	{"按确定键开始"},
	{"请输入新指纹"},
	{"修改成功"},
	{"修改失败"},
};

//指纹修改界面
// 0  新建
// 1  修改
// 2  删除
// 0xff 返回
static rt_uint8_t fprint_modify_fun_choose_ui(void)
{
	rt_uint8_t i;

	gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
	for(i = 0;i < 3;i++)
	{
		gui_display_string(SHOW_X_CENTERED(FprintModifyText[i]),SHOW_Y_LINE(i+1),FprintModifyText[i],GUI_WIHIT);
	}
	gui_display_update();
	while(1)
	{
		rt_err_t    InResult;
		rt_uint8_t  KeyValue;

		InResult = gui_key_input(&KeyValue);
		if(InResult == RT_EOK)
		{
			if(KeyValue == '1')
			{
				return 0;
			}
			else if(KeyValue == '2')
			{
				return 1;
			}
			else if(KeyValue == '3')
			{
				return 2;
			}
			else if(KeyValue == '#')
			{
				return 0xff;
			}
		}
		else
		{
			//操作超时
    	if(menu_event_process(2,MENU_EVT_OP_OUTTIME) == 0)
    	{
				return 0xff;
    	}
		}
	}
}

static rt_mq_t 				FpRight_mq = RT_NULL;

void modfiyui_fp_right_mail_send(FPINTF_USER *user)
{
	rt_kprintf("modfiy ui send fp right mail\n");
	if(FpRight_mq == RT_NULL)
	{
		FpRight_mq = rt_mq_create("fpright",sizeof(FPINTF_USER),1,RT_IPC_FLAG_FIFO);
		RT_ASSERT(FpRight_mq != RT_NULL);
	}
	
  rt_mq_send(FpRight_mq,user,sizeof(FPINTF_USER));
}

//新建指纹
static void fprint_new_create_ui(void)
{
	rt_int32_t CurUserPos;
	rt_uint8_t *ShowBuf;
	
	ShowBuf = rt_calloc(1,MENU_PASSWORD_MAX_LEN);
	RT_ASSERT(ShowBuf != RT_NULL);

	while(1)
	{
    gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
    gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(0),FprintModifyText[3],GUI_WIHIT);
    
    CurUserPos = account_cur_pos_get();
    rt_sprintf((char *)ShowBuf,"%03d",CurUserPos);
    gui_display_string(SHOW_X_ROW8(12),SHOW_Y_LINE(0),ShowBuf,GUI_WIHIT);
    
    gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(1),FprintModifyText[4],GUI_WIHIT);
    gui_display_update();
    while(1)
    {
      rt_err_t   result;
      rt_uint8_t KeyValue;
      rt_uint8_t CrateResult = 0;
      
      result = gui_key_input(&KeyValue);
      if(result == RT_EOK)
      {
        if(KeyValue == '*')
        {
          //采集指纹
          gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(2),SHOW_X_ROW8(15),SHOW_Y_LINE(3));
          gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(2),FprintModifyText[5],GUI_WIHIT);
          gui_display_update();
          if(CurUserPos == ADMIN_DATA_POS)
          {
            result = admin_modify_fprint(30*RT_TICK_PER_SECOND);
          }
          else
          {
            result = user_add_fprint(30*RT_TICK_PER_SECOND);
          }
          if(result == RT_EOK)
          {
            //指纹采集成功
            rt_kprintf("采集成功\n");
            CrateResult = 6;
          }
          else
          {
            //指纹采集失败
            rt_kprintf("采集失败\n");
            CrateResult = 7;  
          }
          gui_clear(SHOW_X_ROW8(0),SHOW_Y_LINE(2),SHOW_X_ROW8(15),SHOW_Y_LINE(3));
          gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(2),FprintModifyText[CrateResult],GUI_WIHIT);
          gui_display_update();
          rt_thread_delay(RT_TICK_PER_SECOND*2);
          break;
        }
        else if(KeyValue == '#')
        {
          //gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(3),MenuCommText[0],GUI_WIHIT);
          gui_display_update();
          
          rt_free(ShowBuf);
          return ;
        }
      }
      else
      {
    		//操作超时
	    	if(menu_event_process(2,MENU_EVT_OP_OUTTIME) == 0)
	    	{
					return ;
	    	}
      }
    }

	}
	
	
}

static void fprint_modify_ui(void)
{
	rt_int32_t CurUserPos;
	rt_uint8_t *ShowBuf;
	rt_err_t   result;
	
	ShowBuf = rt_calloc(1,MENU_PASSWORD_MAX_LEN);
	RT_ASSERT(ShowBuf != RT_NULL);
	
	gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
	gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(0),FprintModifyText[3],GUI_WIHIT);

	CurUserPos = account_cur_pos_get();
	rt_sprintf((char *)ShowBuf,"%03d",CurUserPos);
	gui_display_string(SHOW_X_ROW8(12),SHOW_Y_LINE(0),ShowBuf,GUI_WIHIT);

	gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(1),FprintModifyText[8],GUI_WIHIT);
	gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(2),FprintModifyText[9],GUI_WIHIT);
	gui_display_update();

  if(FpRight_mq == RT_NULL)
  {
    FpRight_mq = rt_mq_create("fpright",sizeof(FPINTF_USER),1,RT_IPC_FLAG_FIFO);
    RT_ASSERT(FpRight_mq != RT_NULL);
  }
	while(1)
	{
		FPINTF_USER FpKeyDat;
		
    //读空指纹
    result = rt_mq_recv(FpRight_mq,&FpKeyDat,sizeof(FPINTF_USER),RT_WAITING_NO);
    
		result = menu_input_sure_key(0);
		if(result != RT_EOK)
		{	
			// 操作超时
      rt_free(ShowBuf);
      if(result == RT_ETIMEOUT)
      {
        menu_event_process(2,MENU_EVT_OP_OUTTIME);
      }
			return ;
		}
		gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(2),FprintModifyText[5],GUI_WIHIT);
		gui_display_update();
		//输入原始指纹
		result = rt_mq_recv(FpRight_mq,&FpKeyDat,sizeof(FPINTF_USER),RT_TICK_PER_SECOND*30);
		if(result == RT_ERROR)
		{
			rt_kprintf("原始指纹输入错误\n");
			return ;
		}

		gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(1),FprintModifyText[10],GUI_WIHIT);
		gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(2),FprintModifyText[9],GUI_WIHIT);
		gui_display_update();
		
		result = menu_input_sure_key(0);
		if(result == RT_ERROR)
		{	
			// 操作超时
			rt_free(ShowBuf);
      if(result == RT_ETIMEOUT)
	    {
	      menu_event_process(2,MENU_EVT_OP_OUTTIME);
	    }
			return ;
		}
		gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(2),FprintModifyText[5],GUI_WIHIT);
		gui_display_update();
		//输入指纹
		rt_kprintf("modify pos is %d\n",FpKeyDat.KeyPos);
		result = user_modify_fprint(FpKeyDat.KeyPos,RT_TICK_PER_SECOND*30);
		if(result == RT_EOK)
		{
			rt_kprintf("修改成功\n");

			gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(2),FprintModifyText[11],GUI_WIHIT);
			gui_display_update();
			rt_free(ShowBuf);
			return;
		}
		rt_kprintf("修改失败\n");
		
		gui_display_string(SHOW_X_ROW16(0),SHOW_Y_LINE(2),FprintModifyText[12],GUI_WIHIT);
		gui_display_update();
    rt_free(ShowBuf);
		return ;
	}
}

static void fprint_delete_ui(void)
{
	
}

void menu_fprint_modify_ui(void)
{
	rt_uint8_t *ShowBuf;
	rt_uint8_t FunChoose;
	
	ShowBuf = rt_calloc(1,MENU_PASSWORD_MAX_LEN);
	RT_ASSERT(ShowBuf != RT_NULL);

	FunChoose = fprint_modify_fun_choose_ui();
	if(FunChoose == 0xff)
	{
		return ;
	}
	else if(FunChoose == 0)
	{
    fprint_new_create_ui();
	}
	else if(FunChoose == 1)
	{
    fprint_modify_ui();
	}
	else if(FunChoose == 2)
	{
		fprint_delete_ui();
	}
  rt_free(ShowBuf);
}

