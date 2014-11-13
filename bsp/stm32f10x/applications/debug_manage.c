#include "debug_manage.h"
#include "eeprom_process.h"


#ifdef USEING_RAM_DEBUG
MapByteDef_p SystemPrintMap = RT_NULL;

//创建系统打印映射域
int system_printf_map_byte(void)
{
	SystemPrintMap = map_byte_create(64);

	eeprom_debugmap_manage(SystemPrintMap,0);
	
	rt_kprintf("sizeof(MapByteDef_p) = %d\n",sizeof(*SystemPrintMap));
	RT_ASSERT(SystemPrintMap != RT_NULL);

	
	return 0;
}
INIT_APP_EXPORT(system_printf_map_byte);

rt_bool_t debug_check(rt_uint32_t flag)
{
	return map_byte_bit_get(SystemPrintMap,flag);
}

/*
int debug_init(void)
{
	sys_printf(3,22);

	return 0;
}
INIT_APP_EXPORT(debug_init);
*/













#ifdef RT_USING_FINSH
#include <finsh.h>

#define SYS_PRINTF_TEXT_MAXLEN  50
static const char SysPintfText[][SYS_PRINTF_TEXT_MAXLEN] = 
{
  "USEING_GPRS_DEBUG",
	"NET_MSG_THREAD",
	"NET_RECV_MSG_TYP",
	"NET_RECV_MSG_INFO",
	"NET_SEND_MSG_INFO",
	"NET_LENMAP_INFO",
	"NET_SEND_MODE_INFO",
	"NET_MEM_USE_INFO",
	"NET_WND_INFO",
	"NET_SET_MSG_INOF",
	"NET_RECV_MAIL_ADDR",
	"NET_NONE_DES_RWDATA",
	"NET_NFILE_CRC32",
	"NET_NFILE_SEND_INFO",
	"NET_NFILE_SRESULT",
	"NET_CRC16_INIF",
	"LOCAL_DEBUG_THREAD",
	"LOCAL_DEBUG_MAIL",
	"BT_DEBUG_THREAD",
	"BT_DEBUG_RCVDAT",
	"BT_DEBUG_SENDDAT",
	"NETPY_DEBUG_THREAD",
	"NETMAILCLASS_DEBUG",
	"MENU_DEBUG_THREAD",
	"MENU_DEBUG_THREAD",
	"EEPROM_DEBUG_THREAD",
	"BUZZER_DEBUG_THREAD",
	"NET_RECV_ENC_DATA",
	"NET_SEND_MSG_TYPE",
	"NET_SEND_DES_DATA",
};


void sys_printf(rt_uint8_t cmd,rt_uint8_t data)
{
	switch(cmd)
	{
		case 0:
		{
			rt_uint8_t i;
			
			rt_kprintf("--help\n");
			rt_kprintf("cmd:1 Set Printf Debug Type\n");
			rt_kprintf("cmd:2 Clear Printf Debug Type\n");
			rt_kprintf("cmd:3 Set 0~data Printf output\n");
			rt_kprintf("cmd:4 Set 0~data Printf close\n");

			for(i = 0;i<sizeof(SysPintfText)/SYS_PRINTF_TEXT_MAXLEN;i++)
			{
        rt_kprintf("data=%02d >> %s\n",i,SysPintfText[i]);
			}

			break;
		}
		case 1:
		{
			//设置
			map_byte_bit_set(SystemPrintMap,data,RT_TRUE);
			eeprom_debugmap_manage(SystemPrintMap,1);
			break;
		}
		case 2:
		{
			//清除
			map_byte_bit_set(SystemPrintMap,data,RT_FALSE);
			eeprom_debugmap_manage(SystemPrintMap,1);
			break;
		}
		case 3:
		{
			//设置0到data调试信息输出
			rt_uint8_t i;

			for(i = 0;i < data;i++)
			{
				map_byte_bit_set(SystemPrintMap,i,RT_TRUE);
			}
			rt_kprintf("Set 0~%d Printf output\n",data);
			break;
		}
		case 4:
		{
			//清除0到data的调试信息关闭
			rt_uint8_t i;

			for(i = 0;i < data;i++)
			{
				map_byte_bit_set(SystemPrintMap,i,RT_FALSE);
			}
			rt_kprintf("Clear 0~%d  Printf close\n",data);
			break;
		}
		case 5:
		{
			rt_uint8_t i;

			for(i = 0; i < SystemPrintMap->BitMaxNum;i++)
			{
			if(i%10 == 0)
			{
				rt_kprintf("\n");
			}
			rt_kprintf("%01d ",map_byte_bit_get(SystemPrintMap,i));
		}
		rt_kprintf("\n");
			//显示所有已经设置的调试选项
			
			break;
		}
		default:
		{
			break;
		}
	}
}
FINSH_FUNCTION_EXPORT(sys_printf,(cmd data) system debug printf );

#endif











#endif


