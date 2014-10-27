/*********************************************************************
 * Filename:     eeprom.c
 * 
 *
 * Author:        wangzw <wangzw@yuettak.com>
 * Created at:    2013-12-11 14:33:16
 *
 *                
 * Change Log:This use STM32 of flash simulate EEPROM device 
 *
 * Copyright (C) 2013 Yuettak Co.,Ltd
 ********************************************************************/

#include "eeprom.h"
#include "rtthread.h"

 
static rt_mutex_t eeprom_mutex = RT_NULL;

static void eeprom_mutex_operate(rt_uint8_t   cmd)
{
  if(eeprom_mutex == RT_NULL)
  {
    rt_kprintf("motor_mutex_operate is NULL!!\n");
    
    return ;
  }
  if(cmd)
  {
    rt_mutex_take(eeprom_mutex,RT_WAITING_FOREVER);
  }
  else
  {
    rt_mutex_release(eeprom_mutex);
  }
}


/*read STM32 flash 16bit
  *return read 16bit data
  *faddr: read address
  */
static u16 STMFLASH_ReadHalfWord(u32 faddr)
{
  return *(vu16*)faddr; 
}
#if STM32_FLASH_WREN  //���ʹ����д   
/*������д��
 *WriteAddr:��ʼ��ַ
 *pBuffer:����ָ��
 *NumToWrite:����(16λ)��   
 */
static void STMFLASH_Write_NoCheck(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)   
{            
  u16 i;
  for(i=0;i<NumToWrite;i++)
  {
    FLASH_ProgramHalfWord(WriteAddr,pBuffer[i]);
      WriteAddr+=2;//��ַ����2.
  }  
} 

/*��ָ����ַ��ʼд��ָ�����ȵ�����
 *WriteAddr:��ʼ��ַ(�˵�ַ����Ϊ2�ı���!!)
 *pBuffer:����ָ��
 *NumToWrite:����(16λ)��(����Ҫд���16λ���ݵĸ���.)
 */
#if STM32_FLASH_SIZE<256
#define STM_SECTOR_SIZE 1024 //�ֽ�
#else 
#define STM_SECTOR_SIZE 2048
#endif     
//u16 STMFLASH_BUF[STM_SECTOR_SIZE/2];//�����2K�ֽ�
static void STMFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite) 
{
  u32 secpos;    //������ַ
  u16 secoff;    //������ƫ�Ƶ�ַ(16λ�ּ���)
  u16 secremain; //������ʣ���ַ(16λ�ּ���)    
  u16 i;    
  u32 offaddr;   //ȥ��0X08000000��ĵ�ַ
  u16 *STMFLASH_BUF = RT_NULL;

  STMFLASH_BUF = (u16 *)rt_malloc(STM_SECTOR_SIZE);
  RT_ASSERT(STMFLASH_BUF);
  rt_memset(STMFLASH_BUF,0,(STM_SECTOR_SIZE));
  
  if(WriteAddr<STM32_FLASH_BASE||(WriteAddr>=(STM32_FLASH_BASE+1024*STM32_FLASH_SIZE)))
  {
    return;                             //�Ƿ���ַ
  }
  FLASH_Unlock();                       //����
  offaddr=WriteAddr-STM32_FLASH_BASE;   //ʵ��ƫ�Ƶ�ַ.
  secpos=offaddr/STM_SECTOR_SIZE;       //������ַ  0~127 for STM32F103RBT6
  secoff=(offaddr%STM_SECTOR_SIZE)/2;   //�������ڵ�ƫ��(2���ֽ�Ϊ������λ.)
  secremain=STM_SECTOR_SIZE/2-secoff;   //����ʣ��ռ��С   
  if(NumToWrite<=secremain)secremain=NumToWrite;//�����ڸ�������Χ
  while(1) 
  { 
    STMFLASH_Read(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,
                  STMFLASH_BUF,STM_SECTOR_SIZE/2);//������������������
    for(i=0;i<secremain;i++)                      //У������
    {
      if(STMFLASH_BUF[secoff+i]!=0XFFFF)
      {
        break;                                    //��Ҫ���� 
      }     
    }
    if(i<secremain)                               //��Ҫ����
    {
      FLASH_ErasePage(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE);//�����������
      for(i=0;i<secremain;i++)                   //����
      {
        STMFLASH_BUF[i+secoff]=pBuffer[i];    
      }
      STMFLASH_Write_NoCheck(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,
                            STMFLASH_BUF,STM_SECTOR_SIZE/2);   //д����������  
    }
    else 
    { /* д�Ѿ������˵�,ֱ��д������ʣ������.  */
      STMFLASH_Write_NoCheck(WriteAddr,pBuffer,secremain);          
    }
    if(NumToWrite==secremain)
    {
      break;//д�������
    }
    else                           //д��δ����
    {
      secpos++;                    //������ַ��1
      secoff=0;                    //ƫ��λ��Ϊ0   
        pBuffer+=secremain;        //ָ��ƫ��
      WriteAddr+=secremain;        //д��ַƫ��    
        NumToWrite-=secremain;     //�ֽ�(16λ)���ݼ�
      if(NumToWrite>(STM_SECTOR_SIZE/2))secremain=STM_SECTOR_SIZE/2;//��һ����������д����
      else secremain=NumToWrite;                                     //��һ����������д����
    }  
  };  
  FLASH_Lock();//����
  
  rt_free(STMFLASH_BUF);
  STMFLASH_BUF = RT_NULL;
}
#endif

static void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead)    
{
  u16 i;
  for(i=0;i<NumToRead;i++)
  {
    pBuffer[i]=STMFLASH_ReadHalfWord(ReadAddr);//��ȡ2���ֽ�.
    ReadAddr+=2;//ƫ��2���ֽ�.  
  }
}


static rt_size_t rt_eeprom_read(rt_device_t dev, 
                                      rt_off_t pos, 
                                      void *buffer, 
                                      rt_size_t size)
{
  rt_uint32_t i;
  rt_uint16_t data;
  rt_uint8_t  *tmp = (rt_uint8_t *)buffer;
  
  RT_ASSERT(dev != RT_NULL);

  eeprom_mutex_operate(RT_TRUE);
  for(i = 0;i<size/2;i++)
  {
    STMFLASH_Read(FLASH_EEPROM_ADDR+pos,&data,1);
    *tmp = (data >> 8);
    tmp++;
    *tmp = (data & 0x00ff);
    tmp++;
    pos+=2;
  }
  if(size % 2 != 0)
  {
    STMFLASH_Read(FLASH_EEPROM_ADDR+pos,&data,1);
    *tmp = (data >> 8);
  }

  eeprom_mutex_operate(RT_FALSE);
  return size;
}
    
static rt_size_t rt_eeprom_write(rt_device_t dev, 
                                      rt_off_t pos, 
                                      const void *buffer, 
                                      rt_size_t size)
{
  rt_uint32_t i;
  rt_uint16_t data;
  rt_uint8_t  *tmp = (rt_uint8_t *)buffer;

  RT_ASSERT(dev != RT_NULL);

  eeprom_mutex_operate(RT_TRUE);
  for(i = 0;i<size/2;i++)
  {
    data = *tmp;
    data <<= 8;
    tmp++;
    data += *tmp;
    tmp++;
    STMFLASH_Write(FLASH_EEPROM_ADDR+pos,&data,1);
    pos += 2;
  }
  if(size % 2 != 0)
  {
    data = *tmp;
    data <<= 8;
    data += 0xff;
    STMFLASH_Write(FLASH_EEPROM_ADDR+pos,&data,1);
  }

  eeprom_mutex_operate(RT_FALSE);
  return size;
}

static struct rt_device eeprom;
void rt_hw_eeprom_register(void)
{     
  
  eeprom.type = RT_Device_Class_Char;
  /* register eeprom device */
  eeprom.init   = RT_NULL;
  eeprom.open   = RT_NULL;
  eeprom.close  = RT_NULL;
  eeprom.read   = rt_eeprom_read;
  eeprom.write  = rt_eeprom_write;
  eeprom.control = RT_NULL;

  /* no private */
  eeprom.user_data = RT_NULL;
  rt_device_register(&eeprom, "EEPROM", RT_DEVICE_FLAG_RDWR);
  eeprom_mutex = rt_mutex_create("EEPROM", RT_IPC_FLAG_FIFO);
}

















