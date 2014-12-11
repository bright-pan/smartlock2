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
 * 2014-08-20 wangzw <wangzw@yuettak.com> 
 							GUI:人机接口，为输入输出提供接口
 							
 * Copyright (C) 2014 Yuettak Co.,Ltd
 ********************************************************************/

#include "gui.h"
#include "asiic168.h"
#include "gb1616.h"
#include "menu.h"
#include "gsm.h"

#ifdef USEING_BUZZER_FUN
#include "buzzer.h"
#endif

#define UI_SLEEP_TIME               5
////////////////////////////////////////////////////////////////////////////////////////////////////
//key api
static rt_mq_t key_mq;								//按键

/* 底层发送出来的按键接口 */
typedef struct {
	uint16_t type;
	KB_MODE_TYPEDEF mode;
	uint8_t c;
}KB_MAIL_TYPEDEF;

/* GUI 模块进入休眠的倒计时计数器 */
static volatile rt_uint8_t GUISleepTime = UI_SLEEP_TIME;

/** 
@brief  设置GUI模块进入休眠的等待时间
@param  value 秒没有按键按下进入休眠
@retval none
*/
void gui_sleep_time_set(rt_uint8_t value)
{
	GUISleepTime = value;
}


/** 
@brief  获取GUI模块进入休眠时间的等待时间
@param  none
@retval none
*/
rt_uint8_t gui_sleep_time_get(void)
{
	return GUISleepTime;
}


/** 
@brief  打开lcd的显示
@param  none
@retval none
*/
void gui_open_lcd_show(void)
{
	// 如果LCD关闭
	if(menu_event_process(1,MENU_EVT_LCD_CLOSE) == 0)
	{
		// 发送一个删除键唤醒lcd
    send_key_value_mail(KB_MAIL_TYPE_INPUT, KB_MODE_NORMAL_AUTH, MENU_DEL_VALUE);
	}
}


/** 
@brief  关闭lcd的显示
@param  none
@retval none
*/
void gui_close_lcd_show(void)
{
	// 关闭lcd模块
	lcd_on_off(RT_FALSE);

	// 关闭副板电源 
#ifdef NOT_CLOSE_FT_POWR
	gpio_pin_output(DEVICE_NAME_POWER_FRONT,0,0);
#endif
	rt_thread_delay(10);
	
  // UI进入睡眠状态
  rt_thread_entry_sleep(rt_thread_self());
}


/** 
@brief  发送一个按键值给菜单模块接口
@param  type 邮件类型
@param  mode 
@param  c    按键值 
@retval RT_EOK 有效按键 
				RT_ERROR 无效按键
*/
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


/** 
@brief  按键输入接口
@param  KeyValue 按键值 
@retval RT_EOK 有效按键 
				RT_ERROR 无效按键
*/
rt_err_t gui_key_input(rt_uint8_t *KeyValue)
{
	rt_err_t            result;
	KB_MAIL_TYPEDEF     mail;
	static rt_uint8_t   SleepCnt = UI_SLEEP_TIME;//休眠计数
	
	rt_memset(&mail, 0, sizeof(mail));

	if(key_mq == RT_NULL)
	{
		key_mq = rt_mq_create("kboard", sizeof(KB_MAIL_TYPEDEF),
						 5, RT_IPC_FLAG_FIFO);
	}
	// 清除休眠倒计时
	if(menu_event_process(2,MENU_EVT_CLR_LCD_SLEEP) == 0)
	{
		rt_kprintf("clear lcd sleep\n");
		// 清除屏幕关闭计时器重新设置屏幕关闭时间
		SleepCnt = 0;
		gui_sleep_time_set(UI_SLEEP_TIME);
	}
	result = rt_mq_recv(key_mq, &mail, sizeof(mail),RT_TICK_PER_SECOND);
	if(result == RT_EOK)
	{
    rt_dprintf(MENU_DEBUG_KEY,("key recv value %c\n",mail.c));
		// 按键声音
		#ifdef USEING_BUZZER_FUN
    buzzer_send_mail(BZ_TYPE_KEY);
    #endif
    
    if(SleepCnt >= GUISleepTime)
    {
    	// ui线程进入工作状态
			rt_thread_entry_work(rt_thread_self());
			
    	// 休眠后点亮屏幕
    	gpio_pin_output(DEVICE_NAME_POWER_FRONT,1,0);
			lcd_on_off(RT_TRUE);
			menu_event_process(2,MENU_EVT_LCD_CLOSE);

			// 重置屏幕休眠时间
			gui_sleep_time_set(UI_SLEEP_TIME);
    }
    SleepCnt = 0;
    switch(mail.c)
    {
			case KEY_ENTRY_SYS_MANAGE:
			{
				// 触发进入系统管理界面
				
				break;
			}
			case KEY_START_RING_VALUE:
			{
				if(local_event_process(1,LOCAL_EVT_SYSTEM_FREEZE) == 0)
				{
					//系统冻结
					rt_kprintf("system is freeze\n");
					result = RT_ERROR;
					
					break;
				}
				// 触发接电话功能
				send_local_mail(ALARM_TYPE_GSM_RING_REQUEST,0,RT_NULL);

				// 发送电话开门UI事件
				menu_event_process(0,MENU_EVT_PH_UNLOCK);
				result = RT_ERROR;
				break;
			}
			default:
			{
				break;
			}
    }
	}
	else
	{
		SleepCnt++;
		if(SleepCnt >= GUISleepTime) //一定要和开启的时候判断范围一样否则会有1s中屏幕不亮也可以操作的bug
		{
			SleepCnt = GUISleepTime;
			// 屏幕休眠
			gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);	

			// 更新显存
			gui_display_update();
			
			// 操作超时
	    menu_event_process(0,MENU_EVT_OP_OUTTIME);

	    // LCD处于关闭状态
      menu_event_process(0,MENU_EVT_LCD_CLOSE);

			// 关闭液晶模块
	    gui_close_lcd_show();
		}
	}
	
	*KeyValue = mail.c;
	return result;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/** 
@brief  显示字符串包括中文数字和字母
@param  x 左上角坐标x
@param  y 左上角坐标y
@param  string 显示内容
@param	color 显示颜色
@retval none 
*/
void gui_display_string(rt_uint8_t x,rt_uint8_t y,const rt_uint8_t *string,rt_uint8_t color)
{	
  //lcd_display_chinese(x,y,string,rt_strlen(string),color);
  gui_china16s(x,y,(rt_uint8_t *)string,color);
}


/** 
@brief  清除一个矩形区域的显示内容为背景颜色
@param  x1 左上角坐标x
@param  y1 左上角坐标y
@param  x2 右上角坐标x
@param  y2 右上角坐标y
@retval none 
*/
void gui_clear(rt_uint8_t x1,rt_uint8_t y1,rt_uint8_t x2,rt_uint8_t y2)
{
	lcd_clear(x1,y1,x2-x1,y2-y1);
}


/** 
@brief  更新显示缓冲区到屏幕上
@param  none
@retval none 
*/
void gui_display_update(void)
{
	lcd_display(0,0,LCD_X_MAX,LCD_Y_MAX);
}


/** 
@brief  显示一条线
@param  x1 左上角坐标x
@param  y1 左上角坐标y
@param  x2 右上角坐标x
@param  y2 右上角坐标y
@param	color 颜色
@retval none 
*/
void gui_line(rt_uint8_t x1,rt_uint8_t y1,rt_uint8_t x2,rt_uint8_t y2,rt_uint8_t color)  
{  
	rt_uint8_t t;  
	rt_uint8_t xerr=0,yerr=0,delta_x,delta_y,distance;  
	rt_int8_t incx,incy;  
	rt_uint8_t row,col;  
	delta_x = x2-x1;//计算坐标增量  
	delta_y = y2-y1;  
	col=x1;  
	row=y1; 

	if(delta_x>0)
	{
			incx=1;//设置单步方向 
	} 
	else   
	{  
	    if(delta_x==0) 
	    {
					incx=0;//垂直线 
	    } 
	    else 
	    {
	    	incx=-1;
	    	delta_x=-delta_x;
	    }  
	}  
	if(delta_y>0)
	{
			incy=1;
	}  
	else  
	{  
	    if(delta_y==0)
	    {
					incy=0;//水平线 
	    } 
	    else 
	    {
	    	incy=-1;
	    	delta_y=-delta_y;
	    }  
	}  
	if(delta_x>delta_y) 
	{
			distance=delta_x;//选取基本增量坐标轴  
	}
	else
	{
	  distance=delta_y; 
	} 

	for(t=0;t<=distance+1;t++)  
	{                                     //画线输出  
		lcd_pixer(col, row, color);
		xerr+=delta_x;  
		yerr+=delta_y;  
		if(xerr>distance)  
		{  
		    xerr-=distance;  
		    col+=incx;  
		}  
		if(yerr>distance)  
		{  
		    yerr-=distance;  
		    row+=incy;  
		}  
  }  
}


/** 
@brief  画一个矩形
@param  x0 左上角坐标x
@param  y0 左上角坐标y
@param  x1 右上角坐标x
@param  y1 右上角坐标y
@param	color 颜色
@param  fill 填充
@retval none 
*/
void gui_box(rt_uint8_t x0, rt_uint8_t y0, rt_uint8_t x1, rt_uint8_t y1,rt_uint8_t color,rt_uint8_t fill)
{
	rt_uint8_t i,j = 0;
	if(fill)	   //是否填充颜色
	{
		for(i=x0;i<x1;i++)
		{
			for(j=y0;j<y1;j++)
			{
        lcd_pixer(i,j, color);
			}
		}
		return;
	}
	gui_line(x0,y0,x0,y1,color);	
	gui_line(x0,y1,x1,y1,color);
	gui_line(x1,y0,x1,y1,color);
	gui_line(x0,y0,x1,y0,color);
}


void gui_put_char(rt_uint8_t x, rt_uint8_t y,rt_uint8_t c, rt_uint8_t color)
{  
	rt_uint8_t  s_x ,s_y, temp ;	  
	c -= 32;

	for(s_y=0 ; s_y < 16 ; s_y++)
	{
		if(s_y+y<LCD_Y_MAX)
		{
		   temp = font16x8[c*16+s_y] ;
		   for( s_x=0 ; s_x<8 ; s_x++ )
		   {
				if(x+s_x<LCD_X_MAX)
				{
					if(temp&0x80)
					{
						lcd_pixer(x+s_x,y+s_y,!color); 
					}
					else
					{
					 	lcd_pixer(x+s_x,y+s_y,color); 
					}
					temp<<=1;
				}
		   }
		}
	}
}

void gui_put_chars(rt_uint8_t x, rt_uint8_t y,rt_uint8_t *s, rt_uint8_t color)
{  
	for(;*s!='\0';s++)
	{
		gui_put_char(x, y,*s, color);
		x=x+8;
	}
}


void gui_china16(rt_uint8_t x, rt_uint8_t  y, rt_uint8_t c[], rt_uint8_t fColor)
{
	rt_uint8_t i,j,k;					//i是用于32个数据j是用于一个字节扫描k表示汉字个数

	//rt_kprintf("\n");
	
	for (k=0;k<200;k++) { //200标示自建汉字库中的个数，循环查询内码
	  if ((codeGB_16[k].Index[0]==c[0])&&(codeGB_16[k].Index[1]==c[1])){  //判断是佛为要显示的汉字
    	for(i=0;i<32;i++) 
    	{
			  unsigned short m=codeGB_16[k].Msk[i];//吧要显示的数据值赋给m
			  //if(i%2 == 0)
				//{
				//	rt_kprintf("\n");
				//}
			  for(j=0;j<8;j++) 
			  {	//如果最高位是一则显示字体颜色
					if((m&0x80)==0x80) 
					{
						lcd_pixer(x+((i%2)*8)+j,y+i/2,!fColor); 
						//rt_kprintf("1");
					}
					else 
					{				//否则显示背景颜色
						lcd_pixer(x+((i%2)*8)+j,y+i/2,fColor); 
						//rt_kprintf(" ");
					}
					m<<=1;				//向左移动一位
				} 
		  }
		  return ;
		}  
	  }
	  //rt_kprintf("\n");
}

void gui_china16s(rt_uint8_t x, rt_uint8_t y, rt_uint8_t *s, rt_uint8_t fColor)
{
	rt_uint8_t l=0;
	while(*s) 
	{
		if( *s < 0x80) 
		{
			gui_put_char(x+l*8,y,*s,fColor);
			s++;l++;
		}
		else
		{
			//rt_kprintf("x:%d y:%d,%s\n",x+l*8,y,s);
			gui_china16(x+l*8,y,(rt_uint8_t*)s,fColor);
			s+=2;l+=2;
		}
	}	
}












#ifdef RT_USING_FINSH
#include <finsh.h>

void GUI_KeyInput(rt_uint8_t value)
{
	send_key_value_mail(KB_MAIL_TYPE_INPUT, KB_MODE_NORMAL_AUTH, value);
}
FINSH_FUNCTION_EXPORT(GUI_KeyInput,"Input analog buttons");
#endif

