/**
功能:PT8211音频转换DAC驱动代码
版本:0.1V

*/
#include "pt8211.h"

static struct rt_device  pt8211;


/*
hardware configuration
*/
static void NVIC_Config(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  NVIC_InitStructure.NVIC_IRQChannel = SPI2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

static void I2S_GPIO_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

}

static void PT8211_init()
{
  I2S_InitTypeDef I2S_InitStructure; 
  
  NVIC_Config();
  
  I2S_GPIO_Config();

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
  SPI_I2S_DeInit(SPI2); 

  I2S_InitStructure.I2S_Mode = I2S_Mode_MasterTx;	           //主模式
  I2S_InitStructure.I2S_Standard = I2S_Standard_MSB;		     //对齐标准
  I2S_InitStructure.I2S_DataFormat = I2S_DataFormat_16b;     //数据格式
  I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Disable;		  
  I2S_InitStructure.I2S_AudioFreq = I2S_AudioFreq_8k;			   //采样频率
  I2S_InitStructure.I2S_CPOL = I2S_CPOL_Low;  			         //空闲时时钟极性
  I2S_Init(SPI2, &I2S_InitStructure);
 
  I2S_Cmd(SPI2, ENABLE);
}

void SPI2_IRQHandler(void)
{
  rt_interrupt_enter();
  
  if ((SPI_I2S_GetITStatus(SPI2, SPI_I2S_IT_TXE) == SET))//缓冲区为空中断
  {
    if(pt8211.tx_complete != RT_NULL)
    {
      pt8211.tx_complete(&pt8211,pt8211.user_data);
    }
  }
  
  rt_interrupt_leave();
}




/* RT-Thread Device Interface */

static rt_err_t rt_dac_init(struct rt_device *dev)
{
  rt_err_t result = RT_EOK;

  PT8211_init();
  return result;
}

static rt_err_t rt_dac_open(struct rt_device *dev, rt_uint16_t oflag)
{

  SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_TXE, ENABLE);     
  return RT_EOK;
}

static rt_err_t rt_dac_close(struct rt_device *dev)
{
  SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_TXE, DISABLE);   
  return RT_EOK;
}

static rt_size_t rt_dac_read(struct rt_device *dev,
                                rt_off_t          pos,
                                void             *buffer,
                                rt_size_t         size)
{
  return size;
}

static rt_size_t rt_dac_write(struct rt_device *dev,
                                 rt_off_t          pos,
                                 const void       *buffer,
                                 rt_size_t         size)
{
  rt_uint16_t data = *(rt_uint16_t *)buffer;

  //data += 500;
  SPI_I2S_SendData(SPI2,data); //关键函数
  return size;
}

static rt_err_t rt_dac_control(struct rt_device *dev,
                                  rt_uint8_t        cmd,
                                  void             *args)
{
  return RT_EOK;
}


/*
DAC register
*/
rt_err_t rt_hw_dac_register(void)
{
    pt8211.type        = RT_Device_Class_Char;
    pt8211.rx_indicate = RT_NULL;
    pt8211.tx_complete = RT_NULL;

    pt8211.init        = rt_dac_init;
    pt8211.open        = rt_dac_open;
    pt8211.close       = rt_dac_close;
    pt8211.read        = rt_dac_read;
    pt8211.write       = rt_dac_write;
    pt8211.control     = rt_dac_control;
    pt8211.user_data   = RT_NULL;

    /* register a character device */
    return rt_device_register(&pt8211, "PT8211", RT_DEVICE_FLAG_RDWR);
}


int rt_pt8211_init(void)
{
  rt_hw_dac_register();
  
  return 0;
}

//INIT_DEVICE_EXPORT(rt_pt8211_init);


