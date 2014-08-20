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

////////////////////////////////////////////////////////////////////////////////////////////////////
//key api
static rt_mq_t key_mq;
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

rt_err_t gui_key_input(rt_uint8_t *KeyValue)
{
	rt_err_t result;
	KB_MAIL_TYPEDEF mail;

	rt_memset(&mail, 0, sizeof(mail));

	if(key_mq == RT_NULL)
	{
		key_mq = rt_mq_create("kboard", sizeof(KB_MAIL_TYPEDEF),
						 5, RT_IPC_FLAG_FIFO);
	}
	result = rt_mq_recv(key_mq, &mail, sizeof(mail),RT_WAITING_NO);

	*KeyValue = mail.c;
	return result;
}
////////////////////////////////////////////////////////////////////////////////////////////////////

void gui_display_string(rt_uint8_t x,rt_uint8_t y,rt_uint8_t *string,rt_uint8_t color)
{	
  lcd_display_string(x,y,string,rt_strlen(string),color);
}
void gui_clear(rt_uint8_t x1,rt_uint8_t y1,rt_uint8_t x2,rt_uint8_t y2)
{
	lcd_clear(x1,y1,x2-x1,y2-y1);
}
void gui_display_update(void)
{
	lcd_display(0,0,LCD_X_MAX,LCD_Y_MAX);
}

void gui_line(rt_uint8_t x1,rt_uint8_t y1,rt_uint8_t x2,rt_uint8_t y2,rt_uint8_t color)  
{  
	rt_uint8_t t;  
	rt_uint8_t xerr=0,yerr=0,delta_x,delta_y,distance;  
	rt_uint8_t incx,incy;  
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


