/*********************************************************************
 * Filename:      sh1106.c
 *
 * Description:
 *
 * Author:        wangzw <wangzw@yuettak.com>
 * Created at:    2013-04-22
 *
 * Modify:
 *
 * 2013-04-25 Bright Pan <loststriker@gmail.com>
 * 2013-04-27 Bright Pan <loststriker@gmail.com>
 *
 * Copyright (C) 2013 Yuettak Co.,Ltd
 ********************************************************************/
#include "oled.h"
#include "sh1106.h"
#include "font.h"
#include "untils.h"

#define DEVICE_NAME_OLED_SH1106 "sh1106"

//////////////////////////////////////////////////////////////////////////////////
//三线接口
#define SH1106_CS_PORT	GPIOA
#define SH1106_CS_PIN	GPIO_Pin_15

#define SH1106_RST_PORT	GPIOD
#define SH1106_RST_PIN	GPIO_Pin_6

#define SH1106_SI_PORT	GPIOB
#define SH1106_SI_PIN	GPIO_Pin_5

#define SH1106_SCL_PORT	GPIOB
#define SH1106_SCL_PIN	GPIO_Pin_3

//-----------------SH1106端口定义----------------

#define SH1106_CS_Clr()  GPIO_ResetBits(SH1106_CS_PORT,SH1106_CS_PIN)
#define SH1106_CS_Set()  GPIO_SetBits(SH1106_CS_PORT,SH1106_CS_PIN)

#define SH1106_RST_Clr() GPIO_ResetBits(SH1106_RST_PORT,SH1106_RST_PIN)
#define SH1106_RST_Set() GPIO_SetBits(SH1106_RST_PORT,SH1106_RST_PIN)

#define SH1106_SCLK_Clr() GPIO_ResetBits(SH1106_SCL_PORT,SH1106_SCL_PIN)
#define SH1106_SCLK_Set() GPIO_SetBits(SH1106_SCL_PORT,SH1106_SCL_PIN)

#define SH1106_SDIN_Clr() GPIO_ResetBits(SH1106_SI_PORT,SH1106_SI_PIN)
#define SH1106_SDIN_Set() GPIO_SetBits(SH1106_SI_PORT,SH1106_SI_PIN)

#define SH1106_CMD  0	//写命令
#define SH1106_DATA 1	//写数据

#define CACHE_X_SIZE 128
#define CACHE_Y_SIZE 64
#define CACHE_PAGES (CACHE_Y_SIZE >> 3)

#define pixel_set(x, y, cache)	(cache[(x)][(y) >> 3] |= bits_mask((y) - (((y) >> 3) << 3)))// y - ((y >> 4) << 4) <==> y % 16;
#define pixel_inverse(x, y) (cache[(x)][(y) >> 3] = (cache[(x)][(y) >> 3] & ~(bits_mask((y) - (((y) >> 3) << 3)))) | (~cache[(x)][(y) >> 4] & bits_mask((y) - (((y) >> 4) << 4))))
#define pixel_clear(x, y, cache)	(cache[(x)][(y) >> 3] &= ~(bits_mask((y) - (((y) >> 3) << 3))))// y - ((y >> 4) << 4) <==> y % 16;


static void
sh1106_inverse(struct oled_device *, u8, u8, u8, u8);
/* lcd logo bmp */
const u8 logo_bmp[] = {

0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x20,0xE0,0xE0,0xE0,0xE0,0xE0,0xE0,
0xE0,0xE0,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x80,0xC0,0xC0,0xE0,0xE0,0xF0,0xF0,0x78,0x78,0x38,0x3C,0x3C,0x1C,0x0E,0x0E,0x0E,0x06,
0x06,0x06,0x07,0x07,0x03,0x03,0x03,0x03,0x03,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,
0x80,0x80,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xC0,0xC0,0xC0,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0xF0,0xF0,0xF0,0xF0,0x30,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0F,0x3F,0xFF,0x7F,0x3F,0x3F,0x9F,0x8F,0xE0,0xF0,0xF0,0xF8,0xFE,0xFE,0x7F,0x3F,0x1F,
0x0F,0x8F,0xF1,0xF1,0xF0,0xF0,0x70,0x00,0x00,0x00,0x00,0x00,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0x00,0x00,0x80,0xC0,0xE0,0xF0,0xF0,0x70,0x38,0x38,
0x38,0x38,0x78,0xF8,0xF0,0xF0,0xE0,0x00,0x00,0x00,0x70,0x70,0xF0,0xF8,0xFF,0xFF,0xFF,0xFF,0x7F,0x70,0x70,0x70,0x00,0x00,0x00,0x00,0x00,0x10,
0x18,0x98,0xFE,0xFF,0xFF,0xFF,0x1F,0x1E,0x18,0x18,0x08,0x00,0x00,0x00,0x18,0x18,0x08,0x0E,0x0E,0x0E,0x0E,0x0E,0x9E,0xFE,0xFE,0xF8,0xF8,0x70,
0x00,0x00,0x80,0xFF,0xFF,0xFF,0x3F,0x07,0x80,0x80,0xC0,0xE0,0xF0,0x78,0x3E,0x1E,0x0E,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xC0,0xE0,
0xF0,0xF8,0xFC,0xFE,0xFF,0xFF,0xFF,0x1F,0x0F,0x07,0x03,0x01,0x00,0x00,0x00,0x00,0xF8,0xFF,0xFF,0xFF,0xFF,0xE7,0xC0,0xC0,0xC0,0xC0,0xE0,0xF0,
0xFF,0xFF,0xFF,0xFF,0x0F,0x00,0x00,0x7F,0xFF,0xFF,0xFF,0xF3,0xC0,0xC6,0x86,0x86,0x86,0x86,0x87,0xC7,0xC7,0x07,0x07,0x00,0x00,0x00,0x00,0xF8,
0xFF,0xFF,0xFF,0xFF,0xCF,0xC0,0xC0,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0xF8,0xFF,0xFF,0xFF,0xDF,0x81,0x80,0x80,0x80,0x00,0xF0,0xF8,
0xFC,0xFC,0x1C,0x0E,0x06,0x06,0x06,0x07,0x03,0xE3,0xFF,0xFF,0xFF,0x7F,0x07,0x00,0x80,0xF8,0xFF,0xFF,0x3F,0x03,0x00,0x07,0x0F,0x3F,0xFF,0xFF,
0xF8,0xE0,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x70,0x78,0x7C,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x1F,0x07,0x01,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x01,0x03,0x03,0x03,0x03,0x03,0x01,0x01,0x00,0x00,0x03,0x03,0x03,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x03,
0x03,0x03,0x03,0x03,0x83,0x83,0x03,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x81,0x03,0x03,0x03,0x03,0x03,0x03,0x01,0x80,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x01,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x00,0x00,0x81,0x83,0x03,0x03,0x03,0x03,0x03,0x03,0x01,0x00,0x03,0x03,0x03,0x03,
0x03,0x00,0x00,0x00,0x03,0x03,0x03,0x01,0x00,0x00,0x00,0x00,0x00,0x80,0x00,0x01,0x03,0x03,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xF0,0x00,0xFF,0xFF,0x10,0x30,0x00,0xF8,0x0B,0x08,
0x08,0x08,0x0E,0x1B,0xF8,0x00,0x00,0x00,0x08,0x0C,0xC6,0x33,0x11,0x00,0xF2,0x92,0x82,0x92,0xFE,0x82,0xF2,0x82,0x82,0xF2,0x02,0x00,0x00,0x00,
0x00,0x10,0x93,0xD2,0x72,0x7E,0x52,0x92,0x92,0x10,0xFE,0xFE,0x82,0x82,0xFE,0xFE,0x00,0x00,0x00,0x00,0x98,0x9C,0x96,0x90,0x90,0x9C,0x18,0x00,
0x00,0x7F,0x40,0x44,0x44,0x42,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0xFF,0xFF,0x00,0x00,0x00,0x81,0x7F,0x01,0x01,0xFF,0xFF,0x01,0x01,0x80,0x80,0x00,0x06,0x01,0xFF,0x00,
0x00,0x86,0x66,0x06,0xE6,0x06,0x06,0x66,0x06,0x86,0x26,0x66,0xC0,0x00,0x00,0x00,0x00,0x00,0x01,0xFE,0x6E,0x26,0x26,0x26,0x27,0x26,0x26,0x26,
0xFE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x20,0x20,0x20,0xFF,0x00,0x00,0x00,0xFF,0x10,0x10,0x08,0x08,0x80,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x03,0x00,0x02,
0x03,0x01,0x00,0x00,0x00,0x01,0x01,0x03,0x03,0x01,0x00,0x00,0x00,0x00,0x03,0x00,0x00,0x01,0x00,0x00,0x01,0x01,0x02,0x02,0x00,0x01,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0x03,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x03,0x00,0x00,
0x02,0x03,0x00,0x00,0x00,0x03,0x02,0x02,0x02,0x03,0x00,0x00,0x00,0x00,0x00,0x00,/*"C:\Users\lenovo\Desktop\asdf.bmp",0*/

};
struct private_user_data
{
    const char name[RT_NAME_MAX];
    u8 cache[CACHE_X_SIZE][CACHE_PAGES];
};

/*
    sh1106 write command to device
    u8 dat, data
    u8 cmd, write mode
*/
__STATIC_INLINE void sh1106_write(u8 dat,u8 cmd)
{
	u8 i;

	SH1106_CS_Clr();
	if (cmd == SH1106_CMD) {
        SH1106_SCLK_Clr();
        delay_us(1);
        SH1106_SDIN_Clr();
        delay_us(1);
        SH1106_SCLK_Set();
	} else {
		SH1106_SCLK_Clr();
		delay_us(1);
		SH1106_SDIN_Set();
		delay_us(1);
		SH1106_SCLK_Set();
	}
	//delay_us(10);
	for (i = 0;i < 8; i++) {
		SH1106_SCLK_Clr();
		if (dat & 0x80)
            SH1106_SDIN_Set();
		else
            SH1106_SDIN_Clr();
        delay_us(1);
        SH1106_SCLK_Set();
        delay_us(1);
        dat <<= 1;
	}
	if (cmd == SH1106_CMD) {
        SH1106_SCLK_Clr();
        delay_us(1);
        SH1106_SDIN_Clr();
        delay_us(1);
        SH1106_SCLK_Set();
	} else {
		SH1106_SCLK_Clr();
		delay_us(1);
		SH1106_SDIN_Set();
		delay_us(1);
		SH1106_SCLK_Set();
	}
	SH1106_CS_Set();
}
const static u8 bit_map[8] = {

	0x00, 0x01, 0x03, 0x07,
	0x0f, 0x1f, 0x3f, 0x7f
};

/*
    write cache to memory
    struct oled_device *oled, write device
    u8 x, u8 y, write coordination(x,y)
    u8 x_size, u8 y_size, write coordination size(x,y)
    const u8 *data_array, write data array
*/
static void
sh1106_write_cache(struct oled_device *oled,
                          u8 x, u8 y,
                          u8 x_size, u8 y_size,
                          const u8 *data_array)
{
	u8 col,cols;
	u8 page = 0;
	u8 pages = 0;
	u8 page_offset = 0;
	u8 data = 0;
	u8 ys_bytes_index = 0;
	u16 data_index = 0;
	u8 ys_bytes = 0;

    struct private_user_data *user = (struct private_user_data *)oled->parent.user_data;

    RT_ASSERT(x+x_size <= CACHE_X_SIZE);
    RT_ASSERT(y+y_size <= CACHE_Y_SIZE);

    ys_bytes = y_size >> 3;

	if(x_size)
		x_size--;
	if(y_size)
		y_size--;
	ys_bytes = (y_size >> 3) + 1;
	pages = (y + y_size) >> 3;
	cols = x + x_size;
	page = y >> 3;
	page_offset = y - ((y >> 3) << 3); // <==> page_offset = y % 8;
	col = x;
	if(ys_bytes > 1)
	{
		if(page_offset == 0)
		{
			for(; page < pages + 1; page++, ys_bytes_index++)
			{
				for(data_index = ys_bytes_index, col = x; col <= cols; col++, data_index += ys_bytes)
				{
					user->cache[col][page] = data_array[data_index];
				}
			}
		}
		else
		{
			for(; page < pages; page++, ys_bytes_index++)
			{
				for(data_index = ys_bytes_index, col = x; col <= cols; col++, data_index += ys_bytes)
				{
					data = data_array[data_index];
					user->cache[col][page] &= bit_map[page_offset];
					user->cache[col][page] |= data << page_offset;
					data >>= 8 - page_offset;
					user->cache[col][page + 1] &= ~bit_map[page_offset] << 1;
					user->cache[col][page + 1] |= data & ((bit_map[page_offset] << 1) + 1);
				}
			}
		}
	}
	else if(ys_bytes == 1)
	{
		if(page_offset == 0)
		{
				for(data_index = 0; col <= cols; col++, data_index++)
				{
					user->cache[col][page] = data_array[data_index];
				}
		}
		else
		{

				for(data_index = 0; col <= cols; col++, data_index++)
				{
					data = data_array[data_index];
					user->cache[col][page] &= bit_map[page_offset];
					user->cache[col][page] |= data << page_offset;
					data >>= 8 - page_offset;
					user->cache[col][page + 1] &= ~bit_map[page_offset] << 1;
					user->cache[col][page + 1] |= data & ((bit_map[page_offset] << 1) + 1);
				}
		}
	}

}
/*
    display string
    struct oled_device *oled, display device
    u8 x, u8 y, coordination(x,y)
    u8 *buf, u8 buf_size, string buffer and size
    u8 inverse_flag, 1,inverse,0,normal.
*/
static void
sh1106_display_string(struct oled_device *oled, u8 x, u8 y, u8 *buf, u8 buf_size, u8 inverse_flag)
{
	u8 index = 0;

	for(index = 0; index < buf_size; index++)
	{
		sh1106_write_cache(oled, x + index * 8, y, 8, 8, ASCII_FONT[*(buf + index) - 0x20]);
	}
    if (inverse_flag)
        sh1106_inverse(oled, x, y, buf_size * 8, 8);
}
/*
    display chinese
    struct oled_device *oled, display device
    u8 x, u8 y, coordination(x,y)
    u8 *buf, u8 buf_size, chinese buffer and size
    u8 inverse_flag, 1,inverse,0,normal.
*/
static void
sh1106_display_chinese(struct oled_device *oled, u8 x, u8 y, u8 *buf, u8 buf_size, u8 inverse_flag)
{
	u8 index = 0;

	for(index = 0; index < buf_size; index++)
	{
		if(*(buf + index) < 0x80)
        {
			sh1106_write_cache(oled, x + index * 7, y, 7, 16, chinese_font_search((u32)*(buf + index)));
            if (inverse_flag)
                sh1106_inverse(oled, x + index * 7, y, 7, 16);
        }
        else
		{
			sh1106_write_cache(oled, x + index * 7, y, 14, 16, chinese_font_search(((u32)*(buf + index + 1)<< 8) | (u32)*(buf + index)));
			index++;
            if (inverse_flag)
                sh1106_inverse(oled, x + index * 7, y, 14, 16);
		}
	}

}

/*
    display bmp
    struct oled_device *oled, display device
    u8 x, u8 y, coordination(x,y)
    u8 *buf, u8 buf_size, bmp buffer and size
    u8 inverse_flag, 1,inverse,0,normal.
*/
static void
sh1106_display_bmp(struct oled_device *oled, u8 x, u8 y, u8 x_size, const u8 *buf, u32 buf_size)
{
	u32 index = 0;
	u32 step = buf_size / x_size;
	for(index = 0; index < step; index++){

		sh1106_write_cache(oled, x, y + (index << 3), x_size, 8, buf + x_size * index);
	}
}
/* display logo */
__STATIC_INLINE void
sh1106_display_logo(struct oled_device *oled)
{
    sh1106_display_bmp(oled, 0,0, 128, logo_bmp, sizeof(logo_bmp));
}
/*
    更新显存到LCD
*/
static void
sh1106_refresh(struct oled_device *oled)
{
    struct private_user_data *user = (struct private_user_data *)oled->parent.user_data;
	u8 i,n;
	for(i = 0; i < CACHE_PAGES; i++)
	{
		sh1106_write(0xb0 + i, SH1106_CMD);    //设置页地址（0~7）
		sh1106_write(0x02, SH1106_CMD);      //设置显示位置—列低地址
		sh1106_write(0x10, SH1106_CMD);      //设置显示位置—列高地址
		for (n = 0; n < CACHE_X_SIZE; n++)
            sh1106_write(user->cache[n][i], SH1106_DATA);
	}
}
/*
    display to oled(x,y)
    x, rect coordinate x
    y, rect coordinate y
    x_size, rect coordinate x size
    y_size, rect coordinate y size
*/
static void
sh1106_display(struct oled_device *oled, u8 x, u8 y, u8 x_size, u8 y_size)
{
	u8 page, pages;
	u8 col, cols;
    struct private_user_data *user;

    RT_ASSERT(x+x_size <= CACHE_X_SIZE);
    RT_ASSERT(y+y_size <= CACHE_Y_SIZE);

    user = (struct private_user_data *)oled->parent.user_data;
	page = y >> 3;
	pages = (y + y_size) >> 3;
	cols = x + x_size;// <==> cols = x + x_size

    if (x == cols)
        ++cols;
    if (page == pages)
        ++pages;

	for(; page < pages; ++page)
	{
		sh1106_write(0xb0 + page, SH1106_CMD);//设置页地址（0~7）
        col = x;
		sh1106_write(0x00 | (x+2 & 0x0f), SH1106_CMD);//设置显示位置—列低地址
		sh1106_write(0x10 | (x+2 >> 4 & 0x0f), SH1106_CMD);//设置显示位置—列高地址
        for (; col < cols; ++col)
            sh1106_write(user->cache[col][page], SH1106_DATA);
	}
}

/*
    inverse display to oled(x,y)
    x, rect coordinate x
    y, rect coordinate y
    x_size, rect coordinate x size
    y_size, rect coordinate y size
*/
static void
sh1106_inverse(struct oled_device *oled, u8 x, u8 y, u8 x_size, u8 y_size)
{
	u8 col, cols;
	u8 page, pages;
	u8 page_offset, pages_offet;
 	u8 data;
    struct private_user_data *user;

    RT_ASSERT(x+x_size <= CACHE_X_SIZE);
    RT_ASSERT(y+y_size <= CACHE_Y_SIZE);

	if(x_size)
		x_size--;
	if(y_size)
		y_size--;

	pages = (y + y_size) >> 3;
	pages_offet = (y + y_size) - (((y + y_size) >> 3) << 3);// <==> pages_offet = (y + y_size) % 8;
	cols = x + x_size;
	page = y >> 3;
	page_offset = y - ((y >> 3) << 3); // <==> page_offset = y % 8;

    user = (struct private_user_data *)oled->parent.user_data;

	if(page != pages)
	{
		for(; page < pages; page++)
		{
			for(col = x; col <= cols; col++)
			{
				data = ~user->cache[col][page] & ~bit_map[page_offset];
				user->cache[col][page] &= bit_map[page_offset];
				user->cache[col][page] |= data;
				page_offset = 0;
			}

		}
		for(col = x; col <= cols; col++)
		{
			data = ~user->cache[col][page] & ~(bit_map[page_offset] | ((~bit_map[pages_offet]) << 1));
			user->cache[col][page] &= bit_map[page_offset] | ((~bit_map[pages_offet]) << 1);
			user->cache[col][page] |= data;
		}

	}
	else
	{
		for(col = x; col <= cols; col++)
		{
			data = ~user->cache[col][page] & ~(bit_map[page_offset] | ((~bit_map[pages_offet]) << 1));
			user->cache[col][page] &= bit_map[page_offset] | ((~bit_map[pages_offet]) << 1);
			user->cache[col][page] |= data;
		}

	}
}
/*
    clear display to oled(x,y)
    x, rect coordinate x
    y, rect coordinate y
    x_size, rect coordinate x size
    y_size, rect coordinate y size
*/
static void
sh1106_clear(struct oled_device *oled, u8 x, u8 y, u8 x_size, 	u8 y_size)
{
	u8 col, cols;
	u8 page, pages;
	u8 page_offset, pages_offet;
	struct private_user_data *user;

    RT_ASSERT(x+x_size <= CACHE_X_SIZE);
    RT_ASSERT(y+y_size <= CACHE_Y_SIZE);

	if(x_size)
		x_size--;
	if(y_size)
		y_size--;

	pages = (y + y_size) >> 3;
	pages_offet = (y + y_size) - (((y + y_size) >> 3) << 3);// <==> pages_offet = (y + y_size) % 16;
	cols = x + x_size;
	page = y >> 3;
	page_offset = y - ((y >> 3) << 3); // <==> page_offset = y % 16;

    user = (struct private_user_data *)oled->parent.user_data;

	if(page != pages)
	{
		for(; page < pages; page++)
		{
			for(col = x; col <= cols; col++)
			{
				user->cache[col][page] &= bit_map[page_offset];
				page_offset = 0;
			}

		}
		for(col = x; col <= cols; col++)
		{
			user->cache[col][page] &= bit_map[page_offset] | ((~bit_map[pages_offet]) << 1);
		}

	}
	else
	{
		for(col = x; col <= cols; col++)
		{
			user->cache[col][page] &= bit_map[page_offset] | ((~bit_map[pages_offet]) << 1);
		}

	}
}
/*
    set pixer display to oled(x,y)
    x, coordinate x
    y, coordinate y
*/
__STATIC_INLINE void
sh1106_set_pixel(struct oled_device *oled, u8 x, u8 y)
{
    struct private_user_data *user = (struct private_user_data *)oled->parent.user_data;
    RT_ASSERT(x < CACHE_X_SIZE && y < CACHE_Y_SIZE);
    pixel_set(x, y, user->cache);
}
/*
    clear pixer display to oled(x,y)
    x, coordinate x
    y, coordinate y
*/
__STATIC_INLINE void
sh1106_clear_pixel(struct oled_device *oled, u8 x, u8 y)
{
    struct private_user_data *user = (struct private_user_data *)oled->parent.user_data;
    RT_ASSERT(x < CACHE_X_SIZE && y < CACHE_Y_SIZE);
    pixel_clear(x, y, user->cache);
}

static rt_err_t 
sh1106_sleep_configure(struct oled_device *oled)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	struct private_user_data *user = (struct private_user_data *)oled->parent.user_data;

	rt_memset(user->cache, 0x00, sizeof(user->cache));

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOD, ENABLE);  //使能PC,D,G端口时钟
	GPIO_StructInit(&GPIO_InitStructure);
	//cs
	GPIO_InitStructure.GPIO_Pin = SH1106_CS_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(SH1106_CS_PORT, &GPIO_InitStructure);
	GPIO_SetBits(SH1106_CS_PORT,SH1106_CS_PIN);

	//rst
	GPIO_InitStructure.GPIO_Pin = SH1106_RST_PIN;
	GPIO_Init(SH1106_RST_PORT, &GPIO_InitStructure);
	GPIO_SetBits(SH1106_RST_PORT,SH1106_RST_PIN);

	//si
	GPIO_InitStructure.GPIO_Pin = SH1106_SI_PIN;
	GPIO_Init(SH1106_SI_PORT, &GPIO_InitStructure);
	GPIO_SetBits(SH1106_SI_PORT,SH1106_SI_PIN);

	//scl
	GPIO_InitStructure.GPIO_Pin = SH1106_SCL_PIN;
	GPIO_Init(SH1106_SCL_PORT, &GPIO_InitStructure);
	GPIO_SetBits(SH1106_SCL_PORT,SH1106_SCL_PIN);
}

/*
 * gpio pin ops configure
 */
static rt_err_t
sh1106_configure(struct oled_device *oled)
{
 	GPIO_InitTypeDef  GPIO_InitStructure;
    struct private_user_data *user = (struct private_user_data *)oled->parent.user_data;

    rt_memset(user->cache, 0x00, sizeof(user->cache));

 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOD, ENABLE);	 //使能PC,D,G端口时钟
    GPIO_StructInit(&GPIO_InitStructure);
	//cs
	GPIO_InitStructure.GPIO_Pin = SH1106_CS_PIN;
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(SH1106_CS_PORT, &GPIO_InitStructure);
 	GPIO_SetBits(SH1106_CS_PORT,SH1106_CS_PIN);

	//rst
 	GPIO_InitStructure.GPIO_Pin = SH1106_RST_PIN;
 	GPIO_Init(SH1106_RST_PORT, &GPIO_InitStructure);
 	GPIO_SetBits(SH1106_RST_PORT,SH1106_RST_PIN);

	//si
 	GPIO_InitStructure.GPIO_Pin = SH1106_SI_PIN;
 	GPIO_Init(SH1106_SI_PORT, &GPIO_InitStructure);
 	GPIO_SetBits(SH1106_SI_PORT,SH1106_SI_PIN);

	//scl
	GPIO_InitStructure.GPIO_Pin = SH1106_SCL_PIN;
 	GPIO_Init(SH1106_SCL_PORT, &GPIO_InitStructure);
 	GPIO_SetBits(SH1106_SCL_PORT,SH1106_SCL_PIN);

    SH1106_RST_Clr();
    delay_us(100);
	SH1106_RST_Set();

    sh1106_write(0xAE,SH1106_CMD);     //Set Display Off
	sh1106_write(0xD5,SH1106_CMD);     //Display divide ratio/osc. freq. mode
	sh1106_write(0x80,SH1106_CMD);      //
	sh1106_write(0xA8,SH1106_CMD);      //Multiplex ration mode:63
	sh1106_write(0x3F,SH1106_CMD);
	sh1106_write(0xD3,SH1106_CMD);      //Set Display Offset
	sh1106_write(0x00,SH1106_CMD);

	sh1106_write(0x40,SH1106_CMD);       //Set Display Start Line
	sh1106_write(0xAD,SH1106_CMD);      //DC-DC Control Mode Set
	sh1106_write(0x8A,SH1106_CMD);      //DC-DC ON/OFF Mode Set
	sh1106_write(0x32,SH1106_CMD);      //Set Pump voltage value
	//sh1106_write(0xA1,SH1106_CMD);      //Segment Remap
	//sh1106_write(0xC8,SH1106_CMD);      //Set COM Output Scan Direction
	sh1106_write(0xA0,SH1106_CMD);      //Segment Remap
	sh1106_write(0xC0,SH1106_CMD);      //Set COM Output Scan Direction
	sh1106_write(0xDA,SH1106_CMD);     //Common pads hardware: alternative
	sh1106_write(0x12,SH1106_CMD);
	sh1106_write(0x81,SH1106_CMD);      //Contrast control
	sh1106_write(0xAA,SH1106_CMD);
	sh1106_write(0xD9,SH1106_CMD);     //Set pre-charge period
	sh1106_write(0x22,SH1106_CMD);
	sh1106_write(0xDB,SH1106_CMD);     //VCOM deselect level mode
	sh1106_write(0x18,SH1106_CMD);

	sh1106_write(0x81,SH1106_CMD); //对比度设置
	sh1106_write(0xEF,SH1106_CMD); // 1~255;默认0X7F (亮度设置,越大越亮)
	sh1106_write(0xD9,SH1106_CMD); //设置预充电周期
	sh1106_write(0xf1,SH1106_CMD); //[3:0],PHASE 1;[7:4],PHASE 2;
	sh1106_write(0xDB,SH1106_CMD); //设置VCOMH 电压倍率
	sh1106_write(0x30,SH1106_CMD); //[6:4] 000,0.65*vcc;001,0.77*vcc;011,0.83*vcc;
	
	sh1106_write(0xA4,SH1106_CMD);     //Set Entire Display On/Off
	sh1106_write(0xA6,SH1106_CMD);     //Set Normal Display
	sh1106_write(0xAF,SH1106_CMD);     //Set Display On

    sh1106_refresh(oled);
    /*
    rt_memset(user->cache, 0xFF, sizeof(user->cache));
    sh1106_display(oled,0,0,2,2);
    sh1106_display_chinese(oled, 2,2,"遥sdsds",7);
    sh1106_display_string(oled,16,16,"asdf",4);
    sh1106_display(oled,0,0,128,64);

    sh1106_display_bmp(oled, 0,0, 128, logo_bmp, sizeof(logo_bmp));
        sh1106_display(oled,0,0,128,64);
    */
    return RT_EOK;
}

enum {
    SH1106_CMD_DISPLAY_STRING = 0x50,
    SH1106_CMD_DISPLAY_CHINESE,
    SH1106_CMD_DISPLAY_BMP,
    SH1106_CMD_DISPLAY_LOGO,
    SH1106_CMD_DISPLAY,
    SH1106_CMD_SET_PIXER,
    SH1106_CMD_CLEAR_PIXER,
    SH1106_CMD_INIT,
    SH1106_CMD_ON,
    SH1106_CMD_OFF,
    SH1106_CMD_CLEAR,
    SH1106_CMD_INVERSE,
};

struct sh1106_param {
    u8 x;
    u8 y;
    u8 x_size;
    u8 y_size;
    u8 *buf;
    u32 buf_size;
    u8 inverse_flag;
};
/* sh1106 control */
static rt_err_t
sh1106_control(struct oled_device *oled, rt_uint8_t cmd, void *arg)
{
	//struct private_user_data *user = (struct private_user_data*)oled->parent.user_data;
    struct sh1106_param *lpm = (struct sh1106_param *)arg;
	switch(cmd)
	{
    case SH1106_CMD_INIT:
        {
            sh1106_configure(oled);     //Set Display On
            break;
        }
    case SH1106_CMD_ON:
        {
            sh1106_write(0xAF,SH1106_CMD);     //Set Display On
            sh1106_configure(oled);     //Set Display On
            break;
        }
    case SH1106_CMD_OFF:
        {
            sh1106_write(0xAE,SH1106_CMD);     //Set Display On
            sh1106_sleep_configure(oled);
            break;
        }
    case SH1106_CMD_CLEAR:
        {
            sh1106_clear(oled,lpm->x, lpm->y, lpm->x_size, lpm->y_size);
            break;
        }
    case SH1106_CMD_INVERSE:
        {
            sh1106_inverse(oled,lpm->x, lpm->y, lpm->x_size, lpm->y_size);
            break;
        }
    case SH1106_CMD_DISPLAY:
        {
            sh1106_display(oled,lpm->x, lpm->y, lpm->x_size, lpm->y_size);
            break;
        }
    case SH1106_CMD_DISPLAY_STRING:
        {
            sh1106_display_string(oled,lpm->x, lpm->y,lpm->buf, lpm->buf_size, lpm->inverse_flag);
            break;
        }
    case SH1106_CMD_DISPLAY_CHINESE:
        {
            sh1106_display_chinese(oled,lpm->x, lpm->y, lpm->buf, lpm->buf_size, lpm->inverse_flag);
            break;
        }
    case SH1106_CMD_SET_PIXER:
        {
            sh1106_set_pixel(oled,lpm->x, lpm->y);
            break;
        }
    case SH1106_CMD_CLEAR_PIXER:
        {
            sh1106_clear_pixel(oled,lpm->x, lpm->y);
            break;
        }
    case SH1106_CMD_DISPLAY_BMP:
        {
            sh1106_display_bmp(oled, lpm->x, lpm->y, lpm->x_size, lpm->buf, lpm->buf_size);
            break;
        }
    case SH1106_CMD_DISPLAY_LOGO:
        {
            sh1106_display_logo(oled);
        }
    default:
		{
			break;
		}
	}
  return RT_EOK;
}

static void
sh1106_out(struct oled_device *oled, rt_uint8_t data)
{
    //struct private_user_data *user = (struct private_user_data*)oled->parent.user_data;
}

static rt_uint8_t
sh1106_intput(struct oled_device *oled)
{
    return 0;
}

struct rt_oled_ops sh1106_user_ops=
{
    sh1106_configure,
    sh1106_control,
    sh1106_out,
    sh1106_intput
};

/* sh1106_device device */
struct private_user_data sh1106_user_data;

struct oled_device sh1106_device;

static int
rt_hw_sh1106_register(void)
{
    struct oled_device *device = &sh1106_device;
    struct private_user_data *user_data = &sh1106_user_data;
    rt_memcpy((void *)user_data->name, DEVICE_NAME_OLED_SH1106, sizeof(DEVICE_NAME_OLED_SH1106));

    device->ops = &sh1106_user_ops;
    rt_hw_oled_register(device, user_data->name, (RT_DEVICE_FLAG_RDWR), user_data);
    return 0;
}

INIT_DEVICE_EXPORT(rt_hw_sh1106_register);
/*
    lcd dispaly string (x,y)
    x, rect coordinate x
    y, rect coordinate y
    u8 *buf, buffer
    u8 *buf_size, buffer size
    u8 inverse_flag, 1,inverse,0,normal.
*/
void
lcd_display_string(u8 x, u8 y, u8 *buf, u8 buf_size, u8 inverse_flag)
{
    rt_device_t dev;
    struct sh1106_param lpm;

    if ((dev = device_enable(DEVICE_NAME_OLED_SH1106)) == RT_NULL)
        return;
    lpm.x = x;
    lpm.y = y;
    lpm.buf = buf;
    lpm.buf_size = buf_size;
    lpm.inverse_flag = inverse_flag;
    rt_device_control(dev, SH1106_CMD_DISPLAY_STRING, &lpm);

}
/*
    lcd dispaly chinese (x,y)
    x, rect coordinate x
    y, rect coordinate y
    u8 *buf, buffer
    u8 *buf_size, buffer size
    u8 inverse_flag, 1,inverse,0,normal.
*/
void
lcd_display_chinese(u8 x, u8 y, u8 *buf, u8 buf_size, u8 inverse_flag)
{
    rt_device_t dev;
    struct sh1106_param lpm;

    if ((dev = device_enable(DEVICE_NAME_OLED_SH1106)) == RT_NULL)
        return;
    lpm.x = x;
    lpm.y = y;
    lpm.buf = buf;
    lpm.buf_size = buf_size;
    lpm.inverse_flag = inverse_flag;
    rt_device_control(dev, SH1106_CMD_DISPLAY_CHINESE, &lpm);

}

/*
    lcd display rect(x,y,x_size,y_size)
    x, rect coordinate x
    y, rect coordinate y
    x_size, rect coordinate x size
    y_size, rect coordinate y size
*/
void
lcd_display(u8 x, u8 y, u8 x_size, u8 y_size)
{
    rt_device_t dev;
    struct sh1106_param lpm;

    if ((dev = device_enable(DEVICE_NAME_OLED_SH1106)) == RT_NULL)
        return;
    lpm.x = x;
    lpm.y = y;
    lpm.x_size = x_size;
    lpm.y_size = y_size;
    rt_device_control(dev, SH1106_CMD_DISPLAY, &lpm);
}
/*
    lcd clear rect(x,y,x_size,y_size)
    x, rect coordinate x
    y, rect coordinate y
    x_size, rect coordinate x size
    y_size, rect coordinate y size
*/
void
lcd_clear(u8 x, u8 y, u8 x_size, u8 y_size)
{
    rt_device_t dev;
    struct sh1106_param lpm;

    if ((dev = device_enable(DEVICE_NAME_OLED_SH1106)) == RT_NULL)
        return;
    lpm.x = x;
    lpm.y = y;
    lpm.x_size = x_size;
    lpm.y_size = y_size;
    rt_device_control(dev, SH1106_CMD_CLEAR, &lpm);
}
/*
    lcd inverse rect(x,y,x_size,y_size)
    x, rect coordinate x
    y, rect coordinate y
    x_size, rect coordinate x size
    y_size, rect coordinate y size
*/
void
lcd_inverse(u8 x, u8 y, u8 x_size, u8 y_size)
{
    rt_device_t dev;
    struct sh1106_param lpm;

    if ((dev = device_enable(DEVICE_NAME_OLED_SH1106)) == RT_NULL)
        return;
    lpm.x = x;
    lpm.y = y;
    lpm.x_size = x_size;
    lpm.y_size = y_size;
    rt_device_control(dev, SH1106_CMD_INVERSE, &lpm);
}
/* display logo */
void
lcd_display_logo(void)
{
    rt_device_t dev;

    if ((dev = device_enable(DEVICE_NAME_OLED_SH1106)) == RT_NULL)
        return;
    rt_device_control(dev, SH1106_CMD_DISPLAY_LOGO, RT_NULL);
}
/*
    lcd bmp (x,y)
    x, rect coordinate x
    y, rect coordinate y
    x_size, rect coordinate x size
    u8 *buf, bmp buffer
    u8 *buf, bmp buffer size
*/
void
lcd_display_bmp(u8 x, u8 y, u8 x_size, u8 *buf, u8 buf_size)
{
    rt_device_t dev;
    struct sh1106_param lpm;

    if ((dev = device_enable(DEVICE_NAME_OLED_SH1106)) == RT_NULL)
        return;
    lpm.x = x;
    lpm.y = y;
    lpm.x_size = x_size;
    lpm.buf = buf;
    lpm.buf_size = buf_size;
    rt_device_control(dev, SH1106_CMD_DISPLAY_BMP, &lpm);

}
/*
    lcd set pixer(x,y) to [0,1]
    x, rect coordinate x
    y, rect coordinate y
    u8 flag: 0,clear;1,set;
*/
void
lcd_pixer(u8 x, u8 y, u8 flag)
{
    rt_device_t dev;
    struct sh1106_param lpm;

    if ((dev = device_enable(DEVICE_NAME_OLED_SH1106)) == RT_NULL)
        return;
    lpm.x = x;
    lpm.y = y;
    if (flag)
        rt_device_control(dev, SH1106_CMD_SET_PIXER, &lpm);
    else
        rt_device_control(dev, SH1106_CMD_CLEAR_PIXER, &lpm);
}

void 
lcd_on_off(rt_bool_t cmd)
{
	rt_device_t dev;
	dev = device_enable(DEVICE_NAME_OLED_SH1106);
	RT_ASSERT(dev);
	
	if(cmd == RT_FALSE)
	{
		//关闭
		rt_device_control(dev,SH1106_CMD_OFF,0);
	}
	else
	{
		//打开
		//rt_device_control(dev,SH1106_CMD_INIT,0);
		rt_device_control(dev,SH1106_CMD_ON,0);
	}
}
#ifdef RT_USING_FINSH
#include <finsh.h>

FINSH_FUNCTION_EXPORT_ALIAS(lcd_display_string, lcd_str, [x y buf size inverse_flag]);
FINSH_FUNCTION_EXPORT_ALIAS(lcd_display, lcd_ds, [x y x_size y_size]);
FINSH_FUNCTION_EXPORT_ALIAS(lcd_clear, lcd_clr, [x y x_size y_size]);
FINSH_FUNCTION_EXPORT_ALIAS(lcd_inverse, lcd_ivs, [x y x_size y_size]);
FINSH_FUNCTION_EXPORT_ALIAS(lcd_display_chinese, lcd_chs, [x y buf size inverse_flag]);
FINSH_FUNCTION_EXPORT_ALIAS(lcd_display_logo, lcd_logo, [x y x_size y_size]);
FINSH_FUNCTION_EXPORT_ALIAS(lcd_display_bmp, lcd_bmp, [x y buf size]);
FINSH_FUNCTION_EXPORT_ALIAS(lcd_pixer, lcd_pixer, [x y]);
#endif
