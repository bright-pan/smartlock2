#ifndef __GB1616_H__
#define __GB1616_H__

// ------------------  汉字字模的数据结构定义 ------------------------ //
struct  typFNT_GB16                 // 汉字字模数据结构 
{
       unsigned char  Index[3];               // 汉字内码索引	“年”
       unsigned char   Msk[32];               // 点阵码数据  “年”后面的32个数据
};

/************************************************************************/
/*  汉字字模表                                                          */
/*   汉字库: 宋体16.dot,横向取模左高位,数据排列:从左到右从上到下        */                                                                    
/************************************************************************/
 const struct  typFNT_GB16 codeGB_16[] =          // 数据表 
{
  /*--  文字:  管  --*/
  /*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
  "管",
  0x20,0x80,0x3E,0xFC,0x51,0x20,0x8A,0x10,0x01,0x00,0x7F,0xFE,0x40,0x04,0x1F,0xE0,
  0x10,0x20,0x1F,0xE0,0x10,0x00,0x1F,0xF0,0x10,0x10,0x10,0x10,0x1F,0xF0,0x10,0x10,
  
  /*--  文字:  理  --*/
  /*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
  "理",
  0x00,0x00,0x03,0xFC,0xFA,0x44,0x22,0x44,0x23,0xFC,0x22,0x44,0xFA,0x44,0x23,0xFC,
  0x22,0x44,0x20,0x40,0x23,0xFC,0x38,0x40,0xC0,0x40,0x00,0x40,0x0F,0xFE,0x00,0x00,
  
  /*--  文字:  员  --*/
  /*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
  "员",
  0x0F,0xF0,0x08,0x10,0x08,0x10,0x0F,0xF0,0x00,0x00,0x1F,0xF8,0x10,0x08,0x11,0x08,
  0x11,0x08,0x11,0x08,0x11,0x08,0x12,0x88,0x02,0x40,0x04,0x30,0x08,0x18,0x30,0x10,
  
  /*--  文字:  登  --*/
  /*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
  "登",
  0x00,0x80,0x3E,0x90,0x02,0xA4,0x24,0x48,0x14,0x50,0x08,0x20,0x1F,0xF8,0x20,0x0E,
  0x4F,0xE4,0x88,0x20,0x08,0x20,0x0F,0xE0,0x00,0x00,0x08,0x20,0x04,0x40,0x7F,0xFC,
  
  /*--  文字:  陆  --*/
  /*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
  "陆",
  0x00,0x40,0xF8,0x40,0x88,0x40,0x97,0xFC,0xA0,0x40,0x90,0x40,0x88,0x40,0x8F,0xFE,
  0x88,0x40,0xA8,0x40,0x94,0x44,0x84,0x44,0x84,0x44,0x87,0xFC,0x84,0x04,0x80,0x00,
  
  /*--  文字:  密  --*/
  /*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
  "密",
  0x02,0x00,0x01,0x00,0x3F,0xFE,0x42,0x24,0x49,0x50,0x29,0x48,0x48,0xA4,0x0B,0x34,
  0x1F,0xE0,0xE0,0x00,0x41,0x00,0x11,0x08,0x11,0x08,0x11,0x08,0x1F,0xF8,0x00,0x00,
  
  /*--  文字:  码  --*/
  /*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
  "码",
  0x00,0x00,0xFD,0xF8,0x10,0x08,0x10,0x88,0x10,0x88,0x20,0x88,0x3C,0x88,0x64,0xFC,
  0xA4,0x04,0x24,0x04,0x25,0xF4,0x24,0x04,0x3C,0x04,0x24,0x04,0x20,0x28,0x00,0x10,
  
	/*--  文字:  用  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"用",
	0x00,0x00,0x1F,0xFC,0x10,0x84,0x10,0x84,0x10,0x84,0x1F,0xFC,0x10,0x84,0x10,0x84,
	0x10,0x84,0x1F,0xFC,0x10,0x84,0x10,0x84,0x20,0x84,0x20,0x84,0x40,0x94,0x80,0x88,

	/*--  文字:  户  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"户",
	0x01,0x00,0x00,0xC0,0x00,0x40,0x1F,0xFC,0x10,0x04,0x10,0x04,0x10,0x04,0x1F,0xFC,
	0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x20,0x00,0x20,0x00,0x40,0x00,0x80,0x00,

	/*--  文字:  系  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"系",
	0x00,0x7C,0x3F,0x80,0x02,0x20,0x04,0x20,0x08,0x40,0x1F,0x80,0x03,0x20,0x0C,0x10,
	0x3F,0xF8,0x10,0x8C,0x04,0xA0,0x08,0x90,0x10,0x88,0x20,0x84,0x42,0x84,0x01,0x00,

	/*--  文字:  统  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"统",
	0x10,0x40,0x10,0x20,0x23,0xFE,0x20,0x40,0x44,0x40,0xF8,0x88,0x09,0x04,0x13,0xFE,
	0x20,0x94,0x7C,0x90,0x00,0x90,0x00,0x90,0x1D,0x12,0xE1,0x12,0x02,0x0E,0x04,0x00,

	/*--  文字:  退  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"退",
	0x40,0x00,0x27,0xF0,0x24,0x10,0x07,0xF0,0x04,0x10,0x04,0x10,0xE7,0xF0,0x24,0x88,
	0x24,0x50,0x24,0x20,0x25,0x10,0x26,0x18,0x24,0x08,0x50,0x00,0x8F,0xFE,0x00,0x00,

	/*--  文字:  出  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"出",
	0x01,0x00,0x01,0x00,0x21,0x04,0x21,0x04,0x21,0x04,0x21,0x04,0x3F,0xFC,0x21,0x04,
	0x01,0x00,0x21,0x04,0x21,0x04,0x21,0x04,0x21,0x04,0x3F,0xFC,0x20,0x04,0x00,0x00,

	/*--  文字:  新  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"新",
	0x10,0x00,0x08,0x0E,0x7F,0x70,0x22,0x40,0x14,0x40,0xFF,0x7E,0x08,0x48,0x08,0x48,
	0xFF,0x48,0x08,0x48,0x2C,0x48,0x2A,0x88,0x4A,0x88,0x89,0x08,0x2A,0x08,0x10,0x08,

	/*--  文字:  建  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"建",
	0x00,0x40,0x78,0x40,0x0B,0xF8,0x10,0x48,0x17,0xFE,0x20,0x48,0x7B,0xF8,0x08,0x40,
	0x4B,0xFC,0x48,0x40,0x28,0x40,0x17,0xFC,0x28,0x40,0x46,0x40,0x81,0xFE,0x00,0x00,

  
  /*--  文字:  修  --*/
  /*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
  "修",
  0x11,0x00,0x19,0x00,0x31,0xF8,0x23,0x08,0x6A,0x90,0xAC,0x60,0x28,0x98,0x2B,0x4E,
  0x28,0xE4,0x29,0x98,0x26,0x60,0x21,0x8C,0x26,0x30,0x20,0xC0,0x27,0x00,0x00,0x00,
  
  /*--  文字:  改  --*/
  /*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
  "改",
  0x00,0x40,0x00,0x60,0x7C,0x40,0x04,0x40,0x04,0xFE,0x04,0x88,0x7D,0x88,0x42,0x88,
  0x40,0x50,0x40,0x50,0x44,0x20,0x48,0x60,0x50,0x90,0x63,0x0E,0x4C,0x04,0x00,0x00,
  
	/*--  文字:  本  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"本",
	0x01,0x00,0x01,0x00,0x01,0x00,0xFF,0xFE,0x03,0x80,0x03,0x40,0x05,0x40,0x05,0x20,
	0x09,0x10,0x11,0x18,0x2F,0xEE,0xC1,0x04,0x01,0x00,0x01,0x00,0x01,0x00,0x00,0x00,

	/*--  文字:  机  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"机",
	0x10,0x00,0x10,0xF8,0x10,0x88,0xFE,0x88,0x10,0x88,0x10,0x88,0x38,0x88,0x34,0x88,
	0x54,0x88,0x50,0x88,0x91,0x08,0x11,0x0A,0x12,0x0A,0x12,0x0A,0x14,0x06,0x10,0x00,

	/*--  文字:  信  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"信",
	0x08,0x80,0x0C,0x60,0x18,0x40,0x17,0xFE,0x30,0x00,0x33,0xF8,0x50,0x00,0x93,0xF8,
	0x10,0x00,0x13,0xF8,0x12,0x08,0x12,0x08,0x12,0x08,0x13,0xF8,0x12,0x08,0x00,0x00,

	/*--  文字:  息  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"息",
	0x01,0x00,0x02,0x00,0x1F,0xF0,0x10,0x10,0x1F,0xF0,0x10,0x10,0x1F,0xF0,0x10,0x10,
	0x1F,0xF0,0x00,0x00,0x09,0x00,0x28,0x84,0x28,0x92,0x68,0x12,0x07,0xF0,0x00,0x00,

	/*--  文字:  参  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"参",
	0x04,0x00,0x0C,0x20,0x11,0xF0,0x3F,0x10,0x02,0x00,0xFF,0xFC,0x04,0x40,0x09,0x20,
	0x32,0x18,0xCC,0x6E,0x31,0x84,0x06,0x18,0x18,0x60,0x03,0x80,0x7C,0x00,0x00,0x00,

	/*--  文字:  数  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"数",
	0x08,0x20,0x49,0x30,0x2A,0x20,0x1C,0x20,0xFF,0x7E,0x1C,0x44,0x2B,0x44,0x48,0xC4,
	0x08,0x28,0xFF,0x28,0x12,0x10,0x34,0x10,0x0C,0x28,0x32,0x4E,0xC0,0x84,0x00,0x00,

	/*--  文字:  设  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"设",
	0x40,0x00,0x21,0xF0,0x31,0x10,0x21,0x10,0x01,0x10,0x01,0x10,0xE2,0x0E,0x25,0xF8,
	0x21,0x08,0x21,0x08,0x20,0x90,0x20,0x90,0x28,0x60,0x30,0x90,0x23,0x0E,0x0C,0x04,

	/*--  文字:  置  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"置",
	0x3F,0xF8,0x24,0x48,0x24,0x48,0x3F,0xF8,0x01,0x00,0x7F,0xFC,0x02,0x00,0x1F,0xF0,
	0x10,0x10,0x1F,0xF0,0x10,0x10,0x1F,0xF0,0x10,0x10,0x1F,0xF0,0x10,0x10,0xFF,0xFE,

  /*--  文字:  指  --*/
  /*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
  "指",
  0x11,0x00,0x11,0x38,0x11,0xC0,0x11,0x04,0xFD,0x04,0x11,0xFC,0x14,0x00,0x19,0xFC,
  0x31,0x04,0xD1,0x04,0x11,0xFC,0x11,0x04,0x11,0x04,0x11,0x04,0x51,0xFC,0x20,0x00,
  
  /*--  文字:  纹  --*/
  /*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
  "纹",
  0x20,0x80,0x30,0x60,0x20,0x40,0x47,0xFE,0x48,0x10,0xF1,0x10,0x11,0x10,0x21,0x10,
  0x40,0xA0,0xF8,0xA0,0x00,0x40,0x00,0x40,0x1C,0xA0,0xE1,0x18,0x02,0x0E,0x04,0x04,
  
  /*--  文字:  电  --*/
  /*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
  "电",
  0x01,0x00,0x01,0x00,0x01,0x00,0x3F,0xF8,0x21,0x08,0x21,0x08,0x3F,0xF8,0x21,0x08,
  0x21,0x08,0x21,0x08,0x3F,0xF8,0x21,0x08,0x01,0x02,0x01,0x02,0x00,0xFE,0x00,0x00,
  
  /*--  文字:  话  --*/
  /*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
  "话",
  0x40,0x00,0x20,0x1C,0x33,0xE0,0x20,0x20,0x00,0x20,0x07,0xFE,0xF0,0x20,0x10,0x20,
  0x10,0x20,0x11,0xFC,0x11,0x04,0x11,0x04,0x15,0x04,0x19,0xFC,0x11,0x04,0x00,0x00,
  
  /*--  文字:  保  --*/
  /*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
  "保",
  0x10,0x00,0x1B,0xF8,0x12,0x08,0x22,0x08,0x32,0x08,0x63,0xF8,0xA0,0x40,0x2F,0xFE,
  0x20,0xE0,0x21,0x60,0x21,0x50,0x22,0x58,0x24,0x4E,0x28,0x44,0x20,0x40,0x00,0x00,
  
  /*--  文字:  存  --*/
  /*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
  "存",
  0x03,0x00,0x02,0x00,0x7F,0xFC,0x04,0x00,0x04,0x00,0x0B,0xF8,0x18,0x10,0x10,0x20,
  0x30,0x20,0x57,0xFE,0x90,0x20,0x10,0x20,0x10,0x20,0x10,0x20,0x10,0xA0,0x10,0x40,
  
  /*--  文字:  信  --*/
  /*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
  "信",
  0x08,0x80,0x0C,0x60,0x18,0x40,0x17,0xFE,0x30,0x00,0x33,0xF8,0x50,0x00,0x93,0xF8,
  0x10,0x00,0x13,0xF8,0x12,0x08,0x12,0x08,0x12,0x08,0x13,0xF8,0x12,0x08,0x00,0x00,
  
  /*--  文字:  息  --*/
  /*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
  "息",
  0x01,0x00,0x02,0x00,0x1F,0xF0,0x10,0x10,0x1F,0xF0,0x10,0x10,0x1F,0xF0,0x10,0x10,
  0x1F,0xF0,0x00,0x00,0x09,0x00,0x28,0x84,0x28,0x92,0x68,0x12,0x07,0xF0,0x00,0x00,

  /*--  文字:  编  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"编",
	0x10,0x80,0x18,0x40,0x23,0xFC,0x22,0x04,0x4A,0x04,0xFB,0xFC,0x12,0x00,0x23,0xFC,
	0x7B,0x54,0x03,0x54,0x05,0xFC,0x35,0x54,0xC5,0x54,0x09,0x54,0x11,0x0C,0x00,0x00,

	/*--  文字:  号  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"号",
	0x1F,0xF0,0x10,0x10,0x10,0x10,0x10,0x10,0x1F,0xF0,0x00,0x00,0xFF,0xFE,0x08,0x00,
	0x08,0x00,0x1F,0xF0,0x08,0x10,0x00,0x10,0x00,0x10,0x01,0x10,0x00,0xA0,0x00,0x40,

	/*--  文字:  再  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"再",
	0x7F,0xFC,0x01,0x00,0x01,0x00,0x1F,0xF0,0x11,0x10,0x11,0x10,0x1F,0xF0,0x11,0x10,
	0x11,0x10,0x11,0x10,0xFF,0xFE,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x50,0x10,0x20,

	/*--  文字:  次  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"次",
	0x01,0x00,0x41,0x00,0x25,0x00,0x25,0xFE,0x2A,0x44,0x0A,0x48,0x14,0x40,0x10,0x40,
	0x20,0xC0,0xE0,0xA0,0x41,0x20,0x42,0x10,0x44,0x08,0x18,0x0E,0x60,0x04,0x00,0x00,

	/*--  文字:  输  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"输",
	0x20,0x40,0x20,0x40,0xFC,0xA0,0x21,0x18,0x43,0xF6,0x54,0x00,0xFC,0x04,0x53,0xD4,
	0x12,0x54,0x1F,0xD4,0xF2,0x54,0x13,0xD4,0x12,0x54,0x12,0x54,0x13,0x44,0x12,0x8C,

	/*--  文字:  入  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"入",
	0x0C,0x00,0x06,0x00,0x02,0x00,0x01,0x00,0x03,0x00,0x02,0x80,0x02,0x80,0x04,0x40,
	0x04,0x20,0x08,0x20,0x08,0x10,0x10,0x08,0x20,0x0E,0x40,0x04,0x80,0x00,0x00,0x00,

	/*--  文字:  一  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"一",
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x7F,0xFE,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	
	/*--  文字:  请  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"请",
	0x00,0x40,0x47,0xFC,0x30,0x40,0x23,0xF8,0x00,0x40,0x07,0xFE,0xF0,0x00,0x13,0xF8,
	0x12,0x08,0x13,0xF8,0x12,0x08,0x13,0xF8,0x16,0x08,0x1A,0x08,0x12,0x28,0x02,0x10,

	/*--  文字:  返  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"返",
	0x00,0x08,0x43,0xFC,0x22,0x00,0x32,0x00,0x23,0xF8,0x02,0x08,0xE3,0x08,0x22,0x90,
	0x22,0x50,0x24,0x20,0x24,0x50,0x28,0x88,0x23,0x04,0x50,0x00,0x8F,0xFC,0x00,0x00,

	/*--  文字:  回  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"回",
	0x00,0x00,0x7F,0xFC,0x40,0x04,0x40,0x04,0x4F,0xE4,0x48,0x24,0x48,0x24,0x48,0x24,
	0x48,0x24,0x48,0x24,0x4F,0xE4,0x40,0x04,0x40,0x04,0x40,0x04,0x7F,0xFC,0x00,0x00,

	/*--  文字:  键  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"键",
	0x20,0x20,0x20,0x20,0x27,0x7C,0x39,0x24,0x41,0xFE,0x7A,0x24,0xA7,0x7C,0x21,0x20,
	0xF9,0x7C,0x25,0x20,0x23,0xFE,0x21,0x20,0x2A,0xA0,0x34,0x60,0x28,0x1E,0x00,0x00,

	/*--  文字:  错  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"错",
	0x10,0x90,0x10,0x90,0x3C,0x90,0x21,0xFC,0x40,0x90,0x7C,0x90,0x93,0xFE,0x10,0x00,
	0xFD,0xFC,0x11,0x04,0x11,0x04,0x11,0xFC,0x11,0x04,0x15,0x04,0x19,0xFC,0x11,0x04,

	/*--  文字:  误  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"误",
	0x43,0xF8,0x22,0x08,0x32,0x08,0x22,0x08,0x03,0xF8,0x00,0x00,0xE7,0xFC,0x20,0x40,
	0x20,0x40,0x27,0xFE,0x20,0x40,0x28,0xA0,0x31,0x10,0x22,0x0C,0x04,0x06,0x08,0x04,

	/*--  文字:  添  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"添",
	0x20,0x00,0x17,0xF8,0x10,0x80,0x00,0x80,0x8F,0xFE,0x41,0x40,0x51,0x20,0x12,0x18,
	0x24,0x8E,0x38,0x84,0xE0,0x80,0x24,0xC8,0x24,0xA4,0x28,0xA4,0x22,0x80,0x21,0x00,

	/*--  文字:  加  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"加",
	0x08,0x00,0x08,0x00,0x08,0x00,0x7F,0x7C,0x09,0x44,0x09,0x44,0x09,0x44,0x09,0x44,
	0x11,0x44,0x11,0x44,0x11,0x44,0x21,0x44,0x21,0x7C,0x45,0x44,0x82,0x40,0x00,0x00,

	/*--  文字:  失  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"失",
	0x01,0x00,0x09,0x00,0x09,0x00,0x11,0x00,0x1F,0xF8,0x21,0x00,0x41,0x00,0x01,0x00,
	0x7F,0xFE,0x01,0x00,0x02,0x80,0x02,0x40,0x04,0x20,0x08,0x30,0x10,0x1C,0x20,0x08,

	/*--  文字:  败  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"败",
	0x00,0x40,0x3E,0x60,0x22,0x40,0x22,0x80,0x2A,0xFE,0x2B,0x88,0x2A,0x88,0x2A,0x88,
	0x2A,0x50,0x08,0x50,0x14,0x20,0x12,0x20,0x23,0x50,0x22,0x88,0x41,0x0E,0x02,0x04,

	
  /*--  文字:  量  --*/
  /*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
  "量",
  0x1F,0xF0,0x10,0x10,0x1F,0xF0,0x10,0x10,0x1F,0xF0,0x00,0x00,0xFF,0xFE,0x11,0x10,
  0x1F,0xF0,0x11,0x10,0x1F,0xF0,0x01,0x00,0x1F,0xF8,0x01,0x00,0xFF,0xFE,0x00,0x00,

  /*--  文字:  按  --*/
  /*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
  "按",
  0x10,0x40,0x10,0x20,0x10,0x20,0x13,0xFE,0xFE,0x04,0x10,0x40,0x14,0x40,0x1B,0xFE,
  0x30,0x88,0xD1,0x10,0x11,0x90,0x10,0x60,0x10,0x58,0x11,0x8E,0x56,0x04,0x20,0x00,
  
  /*--  文字:  确  --*/
  /*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
  "确",
  0x00,0x80,0x00,0xFC,0x7C,0x88,0x11,0x10,0x13,0xFC,0x21,0x24,0x3D,0x24,0x65,0xFC,
  0xA5,0x24,0x25,0x24,0x25,0xFC,0x3D,0x24,0x22,0x24,0x22,0x24,0x04,0x14,0x08,0x08,
  
  /*--  文字:  定  --*/
  /*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
  "定",
  0x02,0x00,0x01,0x00,0x3F,0xFE,0x20,0x04,0x40,0x08,0x1F,0xF0,0x01,0x00,0x11,0x00,
  0x11,0x00,0x11,0xF0,0x11,0x00,0x29,0x00,0x25,0x00,0x43,0x00,0x81,0xFC,0x00,0x00,
  
  /*--  文字:  开  --*/
  /*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
  "开",
  0x00,0x00,0x7F,0xFE,0x04,0x20,0x04,0x20,0x04,0x20,0x04,0x20,0xFF,0xFE,0x04,0x20,
  0x04,0x20,0x04,0x20,0x08,0x20,0x08,0x20,0x10,0x20,0x20,0x20,0x40,0x20,0x00,0x00,
  
  /*--  文字:  始  --*/
  /*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
  "始",
  0x10,0x20,0x18,0x30,0x10,0x20,0x10,0x48,0xFC,0x84,0x25,0xFE,0x24,0x84,0x44,0x00,
  0x44,0xFC,0x28,0x84,0x18,0x84,0x14,0x84,0x22,0x84,0x42,0xFC,0x80,0x84,0x00,0x00,

  /*--  文字:  正  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"正",
	0x00,0x00,0x7F,0xFE,0x00,0x80,0x00,0x80,0x00,0x80,0x00,0x80,0x10,0x80,0x10,0xFC,
	0x10,0x80,0x10,0x80,0x10,0x80,0x10,0x80,0x10,0x80,0xFF,0xFE,0x00,0x00,0x00,0x00,

	/*--  文字:  在  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"在",
	0x02,0x00,0x02,0x00,0x7F,0xFE,0x04,0x00,0x04,0x00,0x08,0x40,0x18,0x40,0x17,0xFC,
	0x30,0x40,0x50,0x40,0x90,0x40,0x10,0x40,0x10,0x40,0x17,0xFE,0x10,0x00,0x00,0x00,

	/*--  文字:  采  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"采",
	0x00,0x00,0x01,0xF8,0x7E,0x00,0x00,0x10,0x11,0x10,0x08,0xA0,0x01,0x00,0x7F,0xFE,
	0x01,0x80,0x03,0x40,0x05,0x20,0x09,0x18,0x31,0x0E,0xC1,0x04,0x01,0x00,0x01,0x00,

	/*--  文字:  集  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"集",
	0x08,0x80,0x08,0x40,0x1F,0xF8,0x30,0x80,0x5F,0xF8,0x10,0x80,0x1F,0xF8,0x10,0x80,
	0x1F,0xFC,0x01,0x00,0x7F,0xFE,0x03,0x40,0x05,0x20,0x19,0x1C,0x61,0x08,0x01,0x00,

	/*--  文字:  操  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"操",
	0x21,0xF8,0x21,0x08,0x21,0xF8,0xF8,0x00,0x23,0xBC,0x22,0xA4,0x2A,0xA4,0x33,0xBC,
	0x60,0x40,0xA7,0xFE,0x20,0x60,0x20,0xD0,0x21,0x48,0x22,0x4E,0xAC,0x44,0x40,0x40,

	/*--  文字:  作  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"作",
	0x08,0x80,0x0C,0x80,0x09,0x00,0x13,0xFE,0x12,0x80,0x34,0x88,0x50,0xFC,0x90,0x80,
	0x10,0x80,0x10,0x84,0x10,0xFE,0x10,0x80,0x10,0x80,0x10,0x80,0x10,0x80,0x10,0x80,

	/*--  文字:  成  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"成",
	0x00,0xA0,0x00,0x90,0x00,0x80,0x3F,0xFE,0x20,0x80,0x20,0x80,0x3E,0x88,0x22,0x8C,
	0x22,0x48,0x22,0x50,0x22,0x20,0x2A,0x60,0x44,0x92,0x41,0x0A,0x86,0x06,0x00,0x02,

	/*--  文字:  功  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"功",
	0x00,0x20,0x00,0x20,0x7F,0x20,0x08,0x20,0x0B,0xFC,0x08,0x24,0x08,0x24,0x08,0x44,
	0x09,0x44,0x0E,0x44,0xF0,0x84,0x40,0x84,0x01,0x04,0x02,0x78,0x04,0x10,0x00,0x00,

	/*--  文字:  注  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"注",
	0x20,0x80,0x10,0x40,0x10,0x40,0x07,0xFE,0x80,0x40,0x48,0x40,0x48,0x40,0x10,0x40,
	0x13,0xFC,0x20,0x40,0xE0,0x40,0x20,0x40,0x20,0x40,0x20,0x40,0x2F,0xFE,0x20,0x00,

	/*--  文字:  册  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"册",
	0x3C,0x78,0x24,0x48,0x24,0x48,0x24,0x48,0x24,0x48,0x24,0x48,0xFF,0xFE,0x24,0x48,
	0x24,0x48,0x24,0x48,0x24,0x48,0x24,0x88,0x44,0x88,0x55,0x28,0x8A,0x10,0x00,0x00,

	/*--  文字:  找  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"找",
	0x10,0x40,0x10,0x50,0x10,0x48,0xFE,0x40,0x11,0xFE,0x10,0x40,0x14,0x40,0x18,0x48,
	0x30,0x28,0xD0,0x30,0x10,0x20,0x10,0x70,0x11,0x92,0x16,0x0A,0x50,0x06,0x20,0x02,

	/*--  文字:  不  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"不",
	0x00,0x04,0x7F,0xFE,0x01,0x80,0x01,0x00,0x01,0x00,0x01,0x00,0x03,0x60,0x05,0x30,
	0x09,0x18,0x11,0x0C,0x21,0x08,0x41,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x00,0x00,

	/*--  文字:  到  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"到",
	0x00,0x04,0x7F,0x84,0x08,0x04,0x12,0x24,0x21,0x24,0x7F,0xA4,0x25,0x24,0x04,0x24,
	0x04,0x24,0x7F,0xA4,0x04,0x24,0x04,0x24,0x05,0x84,0x0E,0x04,0x70,0x14,0x20,0x08,

	/*--  文字:  此  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"此",
	0x04,0x40,0x04,0x40,0x04,0x40,0x04,0x40,0x24,0x46,0x27,0x58,0x24,0x60,0x24,0x40,
	0x24,0x40,0x24,0x40,0x24,0x40,0x27,0x42,0x38,0x42,0xE0,0x42,0x00,0x3E,0x00,0x00,

	/*--  文字:  是  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"是",
	0x00,0x00,0x0F,0xF0,0x08,0x10,0x0F,0xF0,0x08,0x10,0x0F,0xF0,0x00,0x00,0xFF,0xFE,
	0x01,0x00,0x09,0x00,0x09,0xF8,0x09,0x00,0x15,0x00,0x23,0x00,0x40,0xFE,0x00,0x00,

	/*--  文字:  否  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"否",
	0x00,0x00,0x7F,0xFC,0x00,0x80,0x01,0x80,0x03,0x20,0x0D,0x18,0x19,0x0C,0x61,0x04,
	0x01,0x00,0x1F,0xF0,0x10,0x10,0x10,0x10,0x10,0x10,0x1F,0xF0,0x10,0x10,0x00,0x00,

	/*--  文字:  进  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"进",
	0x01,0x10,0x41,0x10,0x21,0x10,0x37,0xFC,0x21,0x10,0x01,0x10,0x01,0x10,0xF7,0xFE,
	0x11,0x10,0x11,0x10,0x12,0x10,0x12,0x10,0x14,0x10,0x28,0x00,0x47,0xFE,0x00,0x00,

	/*--  文字:  浏  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"浏",
	0x44,0x04,0x22,0x04,0x22,0x04,0x0F,0xD4,0x80,0x94,0x44,0x94,0x52,0x94,0x13,0x14,
	0x21,0x14,0x23,0x14,0xC2,0x94,0x44,0x94,0x44,0x84,0x48,0x04,0x50,0x14,0x40,0x08,

	/*--  文字:  览  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"览",
	0x04,0x80,0x24,0x80,0x24,0xFE,0x24,0xA0,0x25,0x18,0x06,0x10,0x1F,0xF0,0x10,0x10,
	0x11,0x10,0x11,0x10,0x11,0x10,0x11,0x10,0x12,0x84,0x04,0x84,0x18,0x86,0x60,0x7C,

	/*--  文字:  界  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"界",
	0x1F,0xF8,0x11,0x08,0x11,0x08,0x1F,0xF8,0x11,0x08,0x11,0x08,0x1F,0xF8,0x11,0x88,
	0x02,0x40,0x04,0x30,0x1A,0x4E,0x62,0x44,0x02,0x40,0x04,0x40,0x08,0x40,0x10,0x40,

	/*--  文字:  面  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"面",
	0x00,0x00,0x7F,0xFE,0x02,0x00,0x04,0x00,0x3F,0xFC,0x24,0x44,0x24,0x44,0x27,0xC4,
	0x24,0x44,0x24,0x44,0x27,0xC4,0x24,0x44,0x24,0x44,0x3F,0xFC,0x20,0x04,0x00,0x00,

	/*--  文字:  删  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"删",
	0x00,0x02,0x3B,0x82,0x2A,0x92,0x2A,0x92,0x2A,0x92,0x2A,0x92,0xFF,0xF2,0x2A,0x92,
	0x2A,0x92,0x2A,0x92,0x2A,0x92,0x4A,0x92,0x4A,0x82,0xBA,0x82,0x95,0x8E,0x00,0x04,

	/*--  文字:  除  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"除",
	0x00,0x80,0xF8,0x80,0x89,0x40,0x91,0x20,0xA2,0x10,0xA7,0xEC,0x98,0x80,0x90,0x80,
	0x97,0xF8,0xD0,0x80,0xA2,0xA0,0x82,0x90,0x84,0x8C,0x88,0x84,0x92,0x80,0x81,0x00,

	/*--  文字:  账  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"账",
	0x00,0x80,0x7C,0x88,0x44,0x8C,0x54,0x90,0x54,0xA0,0x54,0xC0,0x57,0xFE,0x54,0xA0,
	0x54,0xA0,0x54,0xA0,0x54,0x90,0x10,0x90,0x28,0xA8,0x46,0xC6,0x84,0x84,0x00,0x00,
	
  /*--  文字:  钥  --*/
  /*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
  "钥",
  0x10,0x00,0x10,0x7C,0x1F,0x44,0x10,0x44,0x20,0x7C,0x3E,0x44,0x48,0x44,0x88,0x44,
  0x7F,0x7C,0x08,0x44,0x08,0x44,0x08,0x44,0x0A,0x84,0x0C,0x94,0x09,0x08,0x00,0x00,
  
  /*--  文字:  匙  --*/
  /*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
  "匙",
  0x00,0x00,0x3F,0x20,0x21,0x20,0x3F,0x20,0x21,0x24,0x3F,0x38,0x00,0x20,0xFF,0xA0,
  0x04,0x24,0x24,0x24,0x27,0x24,0x24,0x1C,0x54,0x00,0x4C,0x00,0x83,0xFE,0x00,0x00,

  /*--  文字:  锁  --*/
	/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
	"锁",
	0x10,0x20,0x12,0x22,0x3D,0x24,0x20,0xA8,0x41,0xFC,0x7D,0x04,0x91,0x04,0x11,0x24,
	0xFD,0x24,0x11,0x24,0x11,0x24,0x11,0x44,0x14,0x50,0x18,0x88,0x13,0x06,0x00,0x00,
};


/*
管理员登陆密码用户系统退出新建修改本机信息参数设置指纹电话保存信息编号再次输入请返回键错添加失败
误量正在采集操作成功注册是否进删除账钥匙锁
*/
#endif

