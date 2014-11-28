/**
  ******************************************************************************
  * @file    eeprom_process.c 
  * @author  wangzw <wangzw@yuettak.com>
  * @version V1.1.0
  * @date    10/11/2014
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and peripherals
  *          interrupt service routine.
  ******************************************************************************
  * @copy
  *
  *
  * <h2><center>&copy; COPYRIGHT 2014 Yuettalk</center></h2>
  */ 

/* Includes ------------------------------------------------------------------*/

#include "eeprom_process.h"
#include <time.h>

//#ifdef   USEING_RAM_DEBUG
#include "untils.h" //��Ҫʹ������� rt_dprintf
//#endif

#ifndef USEING_RAM_DEBUG
#define rt_dprintf    RT_DEBUG_LOG
#endif



#define EEPROM_DEBUG_THREAD        25    //������Ϣ

#define SYSTEM_TIME_SAVE_ADDR    0       //ϵͳʱ�䱣���ַ
#define SYSTEM_TIME_SAVE_SIZE    sizeof(rt_uint32_t) //ϵͳʱ�䱣��ռ��С
#define SYSTEM_DEBUG_MAP_ADDR    (SYSTEM_TIME_SAVE_ADDR+SYSTEM_TIME_SAVE_SIZE)
#define SYSTEM_DEBUG_MAP_SIZE    128




/**
  * @brief  ��ȡ��ǰϵͳʱ��
  * @param  None
  * @retval ϵͳʱ��
  */
static rt_uint32_t sys_date_get(void)
{
  rt_device_t device;
  rt_uint32_t time=0;

  device = rt_device_find("rtc");
  if (device != RT_NULL)
  {
      rt_device_control(device, RT_DEVICE_CTRL_RTC_GET_TIME, &time);
  }

	rt_dprintf(EEPROM_DEBUG_THREAD,("System Time: %s",ctime((const time_t *)&time)));
  return time;
}

/**
  * @brief  ����ϵͳʱ��
  * @param  ϵͳʱ��
  * @retval None
  */
static void sys_date_set(rt_uint32_t date)
{
  rt_device_t device;

  device = rt_device_find("rtc");
  if (device != RT_NULL)
  {
  	if(!(device->open_flag & RT_DEVICE_OFLAG_OPEN))
  	{
      rt_device_open(device,RT_DEVICE_OFLAG_RDWR);
  	}
    rt_device_control(device, RT_DEVICE_CTRL_RTC_SET_TIME, &date);
    rt_device_close(device);
  }

	rt_dprintf(EEPROM_DEBUG_THREAD,("System Time: %s",ctime((const time_t *)&date)));	
}

/**
  * @brief  EEPROM �����򿪺���
  * @param  None
  * @retval ��������
  */
rt_device_t eeprom_device_open(void)
{
	rt_device_t device;

	device = rt_device_find("EEPROM");
  if (device != RT_NULL)
  {
  	if(!(device->open_flag & RT_DEVICE_OFLAG_OPEN))
  	{
      rt_device_open(device,RT_DEVICE_OFLAG_RDWR);
  	}
  }

  return device;
}

/**
  * @brief  EEPROM �����ر�
  * @param  EEPROM ��������
  * @retval None
  */
void eeprom_device_close(rt_device_t dev)
{
	if(dev != RT_NULL)
	{
		rt_device_close(dev);
	}
}

/**
  * @brief  ��EEPROM �б���ʱ��
  * @param  ʱ��
  * @retval RT_EOK �ɹ�
  * @retval RT_EERROR ʧ��
  */
rt_err_t eeprom_save_system_time(rt_uint32_t date)
{
	rt_device_t dev;
	
  dev = eeprom_device_open();
  if(dev == RT_NULL)
  {
		rt_kprintf("EEPROM Open Fail --%s\n",__FUNCTION__);
		return RT_ERROR;
  }
	rt_device_write(dev,SYSTEM_TIME_SAVE_ADDR,&date,SYSTEM_TIME_SAVE_SIZE);
	rt_device_close(dev);
	
	return RT_EOK;
}

/**
  * @brief  ��EEPROM �ж�ȡϵͳʱ��
  * @param  ʱ����յ�ַ
  * @retval RT_EOK �ɹ�
  * @retval RT_ERROR
  */
rt_err_t eeprom_read_system_time(rt_uint32_t *date)
{
	rt_device_t dev;
	
  dev = eeprom_device_open();
  if(dev == RT_NULL)
  {
		rt_kprintf("EEPROM Open Fail --%s\n",__FUNCTION__);
		return RT_ERROR;
  }
	rt_device_read(dev,SYSTEM_TIME_SAVE_ADDR,date,SYSTEM_TIME_SAVE_SIZE);
	rt_device_close(dev);

	return RT_EOK;
}
/**
  * @brief  ���浱ǰϵͳʱ�䵽EEPROMָ��λ��
  					��ϵͳ�ϵ���ʱʹ�á�
  * @param  None
  * @retval RT_EOK �ɹ�
  * @retval RT_ERROR ʧ��
  */
rt_err_t system_time_save(void)
{
	return eeprom_save_system_time(sys_date_get());
}

/**
  * @brief  ϵͳʱ�������ϵͳ�����������⵽ʱ1970�꣬
  					ϵͳ���Զ���EEPROM�ж�ȡʱ���������ʱ��У׼��
  * @param  None
  * @retval 0
  */
#ifdef RT_USING_COMPONENTS_INIT
int system_time_error_process(void)
{
	rt_uint32_t date;

	//���ϵͳʱ��
	date = sys_date_get();

	//�ж�date�Ƿ�С��100
	if(date < 3)
	{
		//ʱ�䶪ʧ
		rt_kprintf("system time error date:%d!!! ",date);
		eeprom_read_system_time(&date);
		sys_date_set(date);
	}
	else
	{
		//ʱ������
	}
	//eeprom_save_system_time(sys_date_get());
	return 0;
}
INIT_APP_EXPORT(system_time_error_process);
#endif


/**
  * @brief  ������Ϣӳ�������				
  * @param  cmd = 1 дӳ����EEPROM
            cmd = 0 ��EEPROMӳ����
  * @retval RT_EOK �����ɹ�
  					RT_EEROR ����ʧ��
  */
rt_err_t eeprom_debugmap_manage(MapByteDef_p map,rt_uint8_t cmd)
{
	rt_size_t   Addr;
	rt_device_t dev = RT_NULL;
	rt_uint8_t  i;

	RT_ASSERT(map!=RT_NULL);

	if(cmd == 1)
	{
		//����
		dev = eeprom_device_open();
		if(dev == RT_NULL)
		{
			rt_kprintf("EEPROM Open Fail --%s\n",__FUNCTION__);
			return RT_ERROR;
		}
		//�����������ݴ�С
		Addr = SYSTEM_DEBUG_MAP_ADDR;
		rt_device_write(dev,Addr,&map->ByteSize,sizeof(map->ByteSize));

		//��������
		Addr += sizeof(map->ByteSize);
		rt_device_write(dev,Addr,map->data,map->ByteSize);

		rt_device_close(dev);
	}
	else
	{
		//��ȡ
		rt_size_t temp;
		
		dev = eeprom_device_open();
		if(dev == RT_NULL)
		{
			rt_kprintf("EEPROM Open Fail --%s\n",__FUNCTION__);
			return RT_ERROR;
		}

		//��ȡ���ݴ�С
		Addr = SYSTEM_DEBUG_MAP_ADDR;
		rt_device_read(dev,Addr,&temp,sizeof(map->ByteSize));
		if(temp != map->ByteSize)
		{
			rt_kprintf("\nSystem debug in eeprom none init\n");
			return RT_ERROR;
		}
		//��ȡ����
		Addr += sizeof(map->ByteSize);
		rt_device_read(dev,Addr,map->data,map->ByteSize);

		rt_kprintf("map->ByteSize = %d\n",map->ByteSize);
		rt_kprintf("map->BitMaxNum = %d\n",map->BitMaxNum);
		for(i = 0; i < map->BitMaxNum;i++)
		{
			if(i%10 == 0)
			{
				rt_kprintf("\n");
			}
			rt_kprintf("%01d ",map_byte_bit_get(map,i));
		}
		rt_kprintf("\n");
		
		rt_device_close(dev);
	}
		
	return RT_EOK;
}

/**
  * @}
  */

/**
  * @}
  */

/******************* (C) COPYRIGHT 2014 Yuettalk *****END OF FILE****/

