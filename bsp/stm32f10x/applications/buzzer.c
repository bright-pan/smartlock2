#include "buzzer.h"



//蜂鸣器消息列队
static rt_mq_t  BZ_mq = RT_NULL;

rt_err_t buzzer_send_mail(BuzzerType type)
{
  rt_err_t result;
  
	if(BZ_mq == RT_NULL)
	{
    BZ_mq = rt_mq_create("buzzer",sizeof(BuzzerType),10,RT_IPC_FLAG_FIFO);
    RT_ASSERT(BZ_mq != RT_NULL);
	}

	result = rt_mq_send(BZ_mq,&type,sizeof(BuzzerType));
  
  return result;
}

//蜂鸣器功能函数
void buzzer_work(BuzzerType mode)
{
	switch(mode)
	{
		case BZ_TYPE_KEY:
		{
			buzzer_control(15);
			break;
		}
		case BZ_TYPE2:
		{
			rt_uint32_t i;
			rt_uint32_t j;
			rt_uint32_t z;
			
			buzzer_control(1000);
			
			for(i = 0 ;i < 3;i++)
			{
				for(j = 50 ;j < 950;j++)
				{
					if(i%2)
					{
            buzzer_pwm_set(j);
					}
					else
					{
            buzzer_pwm_set(1000-j);
					}
					rt_kprintf("i = %d\n",i);
					z = 0xfff;
          while(z--);
				}
			}
			buzzer_pwm_set(500);
			break;
		}
		case BZ_TYPE_LOCK:
		{
			rt_uint8_t i;
			
			for(i = 0 ;i < 5;i++)
			{
				buzzer_control(80);
				rt_thread_delay(10-i);
				buzzer_control(10);
				rt_thread_delay(9-i);
			}
			rt_thread_delay(120);
			break;
		}
		case BZ_TYPE_UNLOCK:
		{
			buzzer_pwm_set(500);
			buzzer_control(500);
			rt_thread_delay(5);
			break;
		}
		case BZ_TYPE_INIT:
		{
			buzzer_pwm_set(600);
			buzzer_control(800);
			buzzer_pwm_set(500);
			break;
		}
		case BZ_TYPE_ERROR1:
		{
			rt_uint32_t i;
			
			for(i = 0 ;i < 3;i++)
			{
				buzzer_pwm_set(400);
				buzzer_control(30);
				rt_thread_delay(10);
				buzzer_pwm_set(500);
			}
			break;
		}
		case BZ_TYPE_ERROR3:
		{
			rt_uint32_t i;
			
			for(i = 0 ;i < 20;i++)
			{
				buzzer_pwm_set(300+i*10);
				buzzer_control(50);
				rt_thread_delay(10);
				buzzer_pwm_set(600-i*10);
			}
			break;
		}

		default:
		{
			break;
		}
	}
}

void buzzer_thread_entry(void *arg)
{
	if(BZ_mq == RT_NULL)
	{
    BZ_mq = rt_mq_create("buzzer",sizeof(BuzzerType),10,RT_IPC_FLAG_FIFO);
    RT_ASSERT(BZ_mq != RT_NULL);
	}
	buzzer_send_mail(BZ_TYPE_INIT);
	while(1)
	{
		rt_err_t 		result;
		BuzzerType 	BZ_data;
		result = rt_mq_recv(BZ_mq,&BZ_data,sizeof(BuzzerType),10);
		if(result == RT_EOK)
		{
			buzzer_work(BZ_data);
		}
		else
		{
      rt_thread_delay(1);
		}
	}
}


int buzzer_thread_init(void)
{
	rt_thread_t thread_id;
	
  thread_id = rt_thread_create("buzzer",
		                           buzzer_thread_entry, 
		                           RT_NULL,256,103, 5);//优先级不能太高
  if(thread_id != RT_NULL)
  {
    rt_thread_startup(thread_id);
  }
	else
	{
		rt_kprintf("%s thread create fail\n",thread_id->name);
	}
  
	return 0;
}
INIT_APP_EXPORT(buzzer_thread_init);


#ifdef RT_USING_FINSH
#include <finsh.h>

FINSH_FUNCTION_EXPORT(buzzer_work, buzzer_work[mode]);

#endif

