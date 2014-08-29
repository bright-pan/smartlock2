#include "menu_3.h"
#define LCD_LINE_MAX_LEN					17							//留出一个结束符的位置

static const rt_uint8_t LocalDevInofText[][LCD_LINE_MAX_LEN] =
{
	{"ID:123456789"},
	{"MAC:123456789"},
};

static const rt_uint8_t SystmeArgText[][LCD_LINE_MAX_LEN] = 
{
	{"报警时间间隔"},
};

//显示本机信息
void menu_20_processing(void)
{
  gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
  gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(0),LocalDevInofText[0],GUI_WIHIT);
	gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(1),LocalDevInofText[1],GUI_WIHIT);
  gui_display_update();
}
void menu_21_processing(void)
{
  gui_clear(0,0,LCD_X_MAX,LCD_Y_MAX);
  gui_display_string(SHOW_X_ROW8(0),SHOW_Y_LINE(0),SystmeArgText[0],GUI_WIHIT);
  gui_display_update();
}



