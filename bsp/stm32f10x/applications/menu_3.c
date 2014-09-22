#include "menu_3.h"
#define LCD_LINE_MAX_LEN					17							//留出一个结束符的位置

static const rt_uint8_t LocalDevInofText[][LCD_LINE_MAX_LEN] =
{
	{"ID:123456789"},
	{"MAC:"},
};

static const rt_uint8_t SystmeArgText[][LCD_LINE_MAX_LEN] = 
{
	{"报警时间间隔"},
};

//显示本机信息
void menu_20_processing(void)
{
	rt_device_t dev;
	rt_uint8_t	*mac;

  gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
  gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(0),LocalDevInofText[0],GUI_WIHIT);
	gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(1),LocalDevInofText[1],GUI_WIHIT);

	mac = rt_calloc(1,13);
	dev = rt_device_find("Blooth");
	if(!(dev->open_flag & RT_DEVICE_OFLAG_OPEN))
	{
		rt_kprintf("open blooth module\n");
		rt_device_open(dev,RT_DEVICE_OFLAG_OPEN);
	}
	rt_device_control(dev,4,mac);
	gui_display_string(SHOW_X_ROW8(rt_strlen(LocalDevInofText[1])),SHOW_Y_LINE(1),mac,GUI_WIHIT);
  gui_display_update();
 	rt_free(mac);
}
void menu_21_processing(void)
{
  gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
  gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(0),SystmeArgText[0],GUI_WIHIT);
  gui_display_update();
}



