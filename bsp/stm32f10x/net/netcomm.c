/**
����:ʵ������Э�����õĵ�һЩ���ܺ���
�汾:0.1
����:wangzw <wangzw@yuettak.com>
*/
#include "netcomm.h"
#include <time.h>
//#include <cyg/crc/crc.h>

/*
����:��λ����
*/
void net_swap(rt_uint8_t *dat1,rt_uint8_t *dat2)
{
	rt_uint8_t tmp;

	tmp = *dat1;
  *dat1 = *dat2;
  *dat2 = tmp;
}

/*
����:��С��ת������
����:data :��Ҫת��������
����:ת��֮�������
*/
rt_uint16_t net_rev16(rt_uint16_t data)
{
  rt_uint16_t rev = 0;

  rev |= (data & 0x00ff);
  rev <<= 8;
  rev |= (data & 0xff00)>>8;

  return rev;
}

/*
����:��С��ת������
����:data :��Ҫת��������
����:ת��֮�������
*/
rt_uint32_t net_rev32(rt_uint32_t data)
{
	rt_uint32_t rev;
	rt_uint8_t  str[4];
	
	rt_memcpy((void *)str,(const void*)&data,sizeof(rt_uint32_t));
	net_swap(&str[0],&str[3]);
	net_swap(&str[1],&str[2]);
	rt_memcpy((void *)&rev,(const void*)str,sizeof(rt_uint32_t));
	
	return rev;
}

/*
����:16λ�����͸��Ƶ��ַ�����
*/
void net_uint16_copy_string(rt_uint8_t str[],rt_uint16_t data)
{
	str[0] = data>>8;
  str[1] = data>>0;
}

/*
����:32λ�����θ��Ƶ��ַ�����
*/
void net_uint32_copy_string(rt_uint8_t str[],rt_uint32_t data)
{
  str[0] = data>>24;
  str[1] = data>>16;
  str[2] = data>>8;
  str[3] = data>>0;
}

/*
����:�ַ���ת��Ϊ16λ����
*/
void net_string_copy_uint16(rt_uint16_t *data,rt_uint8_t str[])
{	
	rt_memcpy((void *)data,(const void*)str,sizeof(rt_uint16_t));
	*data = net_rev16(*data);
}


/*
����:�ַ���ת��Ϊ32λ����
*/
void net_string_copy_uint32(rt_uint32_t *data,rt_uint8_t str[])
{
	rt_memcpy((void *)data,(const void*)str,sizeof(rt_uint32_t));
	*data = net_rev32(*data);
}




/*
����:���һ���ļ��Ĵ�С
����:name �ļ���   size ��С
*/
rt_int8_t get_file_size(const char* name ,rt_uint32_t *size)
{
	struct stat status;

	if(stat(name,&status) >= 0)
	{
		*size = status.st_size;
	}
	else
	{
		rt_kprintf("Get File Info Error\n");
		*size = 0;
		return -1;
	}
	return 0;
}

/*
����:��õ�ǰʱ��
*/
rt_uint32_t net_get_date(void)
{
  rt_device_t device;
  rt_uint32_t time=0;

  device = rt_device_find("rtc");
  if (device != RT_NULL)
  {
      rt_device_control(device, RT_DEVICE_CTRL_RTC_GET_TIME, &time);
  }

	rt_kprintf("System Time: %s\n",ctime((const time_t *)time));
  return time;
}

void net_copy_date_str(rt_uint8_t *time)
{
	rt_uint32_t date;

	date = net_get_date();

	rt_kprintf("date = %x",date);
	net_uint32_copy_string(time,date);
}

/*
����:����һ���ļ���CRC32ֵ
*/
rt_int8_t file_get_crc32(rt_uint8_t *FileName,rt_uint32_t *crc32)
{
	int FileID;
	rt_uint8_t data;

	FileID = open((const char*)FileName,O_CREAT|O_RDONLY,0x777);
	if(FileID < 0)
	{
		return -1;
	}
	*crc32 = 0;
	while(1)
	{
		if(read(FileID,&data,1))
		{
			*crc32 = cyg_ether_crc32_accumulate(*crc32,&data,1);
		}
		else
		{
			break;
		}
	}
	close(FileID);
	return 0;
}



#ifdef RT_USING_FINSH
#include <finsh.h>

void testrev32(rt_uint32_t data)
{	
	rt_uint32_t rev;

	rt_kprintf("data = %8X",data);
	rev = net_rev32(data);
	rt_kprintf("data = %8X",rev);
}
FINSH_FUNCTION_EXPORT(testrev32,"(uint32)test testrev32 function");

#endif
