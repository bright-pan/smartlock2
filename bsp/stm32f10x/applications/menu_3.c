#include "menu_3.h"

#define LCD_LINE_MAX_LEN					17							//留出一个结束符的位置

static const rt_uint8_t LocalDevInofText[][LCD_LINE_MAX_LEN] =
{
	{"ID:"},
	{"MAC:"},
};

static const rt_uint8_t SystmeArgText[][LCD_LINE_MAX_LEN] = 
{
	{"报警时间间隔"},
};


static void hex_to_string(rt_uint8_t data[],rt_uint8_t hex[],rt_uint8_t num)
{	
	const rt_uint8_t asiic[] = "0123456789ABCDEF";
	rt_uint8_t i;

	for(i = 0;i < num*2;i++)
	{
		if(i % 2 == 0)
		{
      data[i] = asiic[hex[i/2]/16];
		}
		else
		{
      data[i] =	asiic[hex[i/2]%16];
		}
	}
}
//显示本机信息
void menu_20_processing(void)
{
	rt_device_t dev;
	rt_uint8_t	*buf;
	rt_uint8_t  *id;
	
  buf = rt_calloc(1,19);

  gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
	//显示ID
  gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(0),LocalDevInofText[0],GUI_WIHIT);
	
	id = rt_calloc(1,9);
	device_config_device_id_operate(id,0);
	hex_to_string(buf,id,8);
	gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(1),buf,GUI_WIHIT);
	rt_free(id);
  //显示蓝牙MAC
	gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(2),LocalDevInofText[1],GUI_WIHIT);

			
	dev = rt_device_find("Blooth");
	if(!(dev->open_flag & RT_DEVICE_OFLAG_OPEN))
	{
		rt_kprintf("open blooth module\n");
		rt_device_open(dev,RT_DEVICE_OFLAG_OPEN);
	}
	rt_device_control(dev,4,buf);
	gui_display_string(SHOW_X_ROW8(rt_strlen((const char *)LocalDevInofText[1])),SHOW_Y_LINE(2),buf,GUI_WIHIT);
  gui_display_update();
 	rt_free(buf);
}
void menu_21_processing(void)
{
  gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
  gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(0),SystmeArgText[0],GUI_WIHIT);
  gui_display_update();
}



#ifdef RT_USING_FINSH
#include <finsh.h>


void hex_str_test(void)
{
	rt_uint8_t buf[19];
	rt_uint8_t buf1[8] = {0x12,0x23,0x34,0x45,0x67,0x78,0x89};
  //struct device_configure config;

	//device_config_file_operate(&config,0);
	hex_to_string(buf,buf1,8);
	buf[18] = 0;
	rt_kprintf("result :%s\n",buf);
}
FINSH_FUNCTION_EXPORT(hex_str_test,test fun);

#endif

