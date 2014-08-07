#include "keyboard_t.h"
#include "board.h"

#define KEYBOARD_DEBUG_INFO			1
#define KEY_VALUE_MOV(A,B)      {(A) |= (0x01<<(B));}

static struct rt_device keyboard_dev;

static rt_bool_t keyboad_enter_check(void)
{
	rt_uint8_t in_data = 1;
	
  GPIO_ResetBits(GPIOD,GPIO_Pin_12| GPIO_Pin_11 | GPIO_Pin_10);
    
	in_data &= GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_15);
	//rt_kprintf("read pin1 %x\n",in_data);
	in_data &= GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_14);
	//rt_kprintf("read pin2 %x\n",in_data);
	in_data &= GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_13);
	//rt_kprintf("read pin3 %x\n",in_data);

	return (in_data == Bit_RESET)?RT_TRUE:RT_FALSE;
}

static rt_uint8_t keyboard_row(void)
{
	rt_uint8_t in_data = 0;
	rt_uint8_t value = 0; 

	//第一列
	in_data = GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_15);
	if(in_data == 0)
	{
		RT_DEBUG_LOG(KEYBOARD_DEBUG_INFO,("Row value1:%x\n",value));
		KEY_VALUE_MOV(value,0);
	}
	//第二列
	in_data = GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_14);
	if(in_data == 0)
	{
	  RT_DEBUG_LOG(KEYBOARD_DEBUG_INFO,("Row value2:%x\n",value));
	  KEY_VALUE_MOV(value,1);
	}
	//第三列
	in_data = GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_13);
  if(in_data == 0)
  {
  	RT_DEBUG_LOG(KEYBOARD_DEBUG_INFO,("Row value3:%x\n",value));
    KEY_VALUE_MOV(value,2);
  }
  
  RT_DEBUG_LOG(KEYBOARD_DEBUG_INFO,("Row value:%x\n",value));

  return value;
}

static rt_uint8_t keyboard_rank(rt_uint8_t row)
{
	rt_uint8_t value = 0;

  GPIO_SetBits(GPIOD,GPIO_Pin_11 | GPIO_Pin_10 |GPIO_Pin_12);
	if(keyboard_row() & row)
	{
		KEY_VALUE_MOV(value,0);
	}
	GPIO_SetBits(GPIOD,GPIO_Pin_11 | GPIO_Pin_12);
  GPIO_ResetBits(GPIOD,GPIO_Pin_10);
  if(keyboard_row() & row)
  {
    KEY_VALUE_MOV(value,1);
  }

  GPIO_SetBits(GPIOD,GPIO_Pin_10 | GPIO_Pin_12);
	GPIO_ResetBits(GPIOD,GPIO_Pin_11);
	if(keyboard_row() & row)
  {
    KEY_VALUE_MOV(value,2);
  }

  GPIO_SetBits(GPIOD,GPIO_Pin_10 | GPIO_Pin_11);
	GPIO_ResetBits(GPIOD,GPIO_Pin_12);
	if(keyboard_row() & row)
  {
    KEY_VALUE_MOV(value,3);
  }

	RT_DEBUG_LOG(KEYBOARD_DEBUG_INFO,("Rank value:%x\n",value));
	
	return value;
}

/*
KEY_IN1   PD15
KEY_IN2   PD14
KEY_IN3   PD13
KEY_SCAN1 PD12
KEY_SCAN2 PD11
KEY_SCAN3 PD10
KEY_SCAN4 PD
KEY_INT   PD9
*/
rt_err_t  keyboard_init(rt_device_t dev)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15 | GPIO_Pin_14 | GPIO_Pin_13 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOD,&GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12| GPIO_Pin_11 | GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOD,&GPIO_InitStructure);
	//GPIO_SetBits(GPIOD,GPIO_Pin_12| GPIO_Pin_11 | GPIO_Pin_10);
	return RT_EOK;
}

rt_size_t keyboard_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
	rt_bool_t result;
	
	result = keyboad_enter_check();
	if(result == RT_TRUE)
	{
		//消抖
		rt_thread_delay(10);
		result = keyboad_enter_check();
		if(result == RT_TRUE)
		{
			//真的有键按下
			rt_uint8_t row;
			rt_uint8_t rank;
			rt_uint8_t key_value;

			//获得行
			row = keyboard_row();
			if(row == 0)
			{
				*((rt_uint8_t *)buffer) = 0xff;
				rt_kprintf("key is row Invalid\n");
				
				return 0;
			}
			else
			{
				//获得列
				rank = keyboard_rank(row);
				if(rank == 0)
				{
          *((rt_uint8_t *)buffer) = 0xff;
          rt_kprintf("key is rank Invalid\n");
          
          return 0;
				}
				else
				{
					//获得键值
					key_value = row<<4;
					key_value += (rank & 0x0f);
					*((rt_uint8_t *)buffer) = key_value;
					rt_kprintf("key is value :%2x\n",key_value);

					//松手检测
					do
					{
            result = keyboad_enter_check();
            rt_thread_delay(1);
					}while(result == RT_TRUE);
					
					return 1;
				}
			}
		}
	}
	
	return 0;
}


int rt_hw_keyboard_register(void)
{
    struct rt_device *device = RT_NULL;

    device = &keyboard_dev;

    device->type        = RT_Device_Class_Char;
    device->rx_indicate = RT_NULL;
    device->tx_complete = RT_NULL;

    device->init        = keyboard_init;
    device->open        = RT_NULL;
    device->close       = RT_NULL;
    device->read        = keyboard_read;
    device->write       = RT_NULL;
    device->control     = RT_NULL;
    device->user_data   = RT_NULL;

    /* register a character device */
    rt_device_register(device, KEYBOARD_DEVICE_NAME, RT_DEVICE_FLAG_RDONLY | RT_DEVICE_FLAG_STANDALONE);

		return 0;
}

//INIT_DEVICE_EXPORT(rt_hw_keyboard_register);