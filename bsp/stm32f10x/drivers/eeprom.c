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
#if STM32_FLASH_WREN  //如果使能了写   
/*不检查的写入
 *WriteAddr:起始地址
 *pBuffer:数据指针
 *NumToWrite:半字(16位)数   
 */
static void STMFLASH_Write_NoCheck(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)   
{            
  u16 i;
  for(i=0;i<NumToWrite;i++)
  {
    FLASH_ProgramHalfWord(WriteAddr,pBuffer[i]);
      WriteAddr+=2;//地址增加2.
  }  
} 

/*从指定地址开始写入指定长度的数据
 *WriteAddr:起始地址(此地址必须为2的倍数!!)
 *pBuffer:数据指针
 *NumToWrite:半字(16位)数(就是要写入的16位数据的个数.)
 */
#if STM32_FLASH_SIZE<256
#define STM_SECTOR_SIZE 1024 //字节
#else 
#define STM_SECTOR_SIZE 2048
#endif     
//u16 STMFLASH_BUF[STM_SECTOR_SIZE/2];//最多是2K字节
static void STMFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite) 
{
  u32 secpos;    //扇区地址
  u16 secoff;    //扇区内偏移地址(16位字计算)
  u16 secremain; //扇区内剩余地址(16位字计算)    
  u16 i;    
  u32 offaddr;   //去掉0X08000000后的地址
  u16 *STMFLASH_BUF = RT_NULL;

  STMFLASH_BUF = (u16 *)rt_malloc(STM_SECTOR_SIZE);
  RT_ASSERT(STMFLASH_BUF);
  rt_memset(STMFLASH_BUF,0,(STM_SECTOR_SIZE));
  
  if(WriteAddr<STM32_FLASH_BASE||(WriteAddr>=(STM32_FLASH_BASE+1024*STM32_FLASH_SIZE)))
  {
    return;                             //非法地址
  }
  FLASH_Unlock();                       //解锁
  offaddr=WriteAddr-STM32_FLASH_BASE;   //实际偏移地址.
  secpos=offaddr/STM_SECTOR_SIZE;       //扇区地址  0~127 for STM32F103RBT6
  secoff=(offaddr%STM_SECTOR_SIZE)/2;   //在扇区内的偏移(2个字节为基本单位.)
  secremain=STM_SECTOR_SIZE/2-secoff;   //扇区剩余空间大小   
  if(NumToWrite<=secremain)secremain=NumToWrite;//不大于该扇区范围
  while(1) 
  { 
    STMFLASH_Read(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,
                  STMFLASH_BUF,STM_SECTOR_SIZE/2);//读出整个扇区的内容
    for(i=0;i<secremain;i++)                      //校验数据
    {
      if(STMFLASH_BUF[secoff+i]!=0XFFFF)
      {
        break;                                    //需要擦除 
      }     
    }
    if(i<secremain)                               //需要擦除
    {
      FLASH_ErasePage(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE);//擦除这个扇区
      for(i=0;i<secremain;i++)                   //复制
      {
        STMFLASH_BUF[i+secoff]=pBuffer[i];    
      }
      STMFLASH_Write_NoCheck(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,
                            STMFLASH_BUF,STM_SECTOR_SIZE/2);   //写入整个扇区  
    }
    else 
    { /* 写已经擦除了的,直接写入扇区剩余区间.  */
      STMFLASH_Write_NoCheck(WriteAddr,pBuffer,secremain);          
    }
    if(NumToWrite==secremain)
    {
      break;//写入结束了
    }
    else                           //写入未结束
    {
      secpos++;                    //扇区地址增1
      secoff=0;                    //偏移位置为0   
        pBuffer+=secremain;        //指针偏移
      WriteAddr+=secremain;        //写地址偏移    
        NumToWrite-=secremain;     //字节(16位)数递减
      if(NumToWrite>(STM_SECTOR_SIZE/2))secremain=STM_SECTOR_SIZE/2;//下一个扇区还是写不完
      else secremain=NumToWrite;                                     //下一个扇区可以写完了
    }  
  };  
  FLASH_Lock();//上锁
  
  rt_free(STMFLASH_BUF);
  STMFLASH_BUF = RT_NULL;
}
#endif

static void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead)    
{
  u16 i;
  for(i=0;i<NumToRead;i++)
  {
    pBuffer[i]=STMFLASH_ReadHalfWord(ReadAddr);//读取2个字节.
    ReadAddr+=2;//偏移2个字节.  
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

















