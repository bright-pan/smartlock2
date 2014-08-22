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
	result = rt_mq_recv(key_mq, &mail, sizeof(mail),RT_TICK_PER_SECOND);

	*KeyValue = mail.c;
	return result;
}
////////////////////////////////////////////////////////////////////////////////////////////////////

void gui_display_string(rt_uint8_t x,rt_uint8_t y,rt_uint8_t *string,rt_uint8_t color)
{	
  //lcd_display_chinese(x,y,string,rt_strlen(string),color);
  gui_china16s(x,y,string,color);
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
			gui_china16(x+l*8,y,(rt_uint8_t*)s,fColor);
			s+=2;l+=2;
		}
	}	
}


