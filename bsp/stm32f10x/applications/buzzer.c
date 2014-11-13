#include "buzzer.h"
#include "untils.h"
#define BUZZER_DEBUG_THREAD     26



void buzzer_control(rt_uint16_t Ferq,rt_uint16_t pwm,rt_size_t num)
{
	rt_device_t dev;
	rt_uint32_t delay;
	
	dev = rt_device_find(DEVICE_NAME_SPEAK);
	if (dev != RT_NULL)
  {
  	if(!(dev->open_flag & RT_DEVICE_OFLAG_OPEN))
  	{
      rt_device_open(dev,RT_DEVICE_OFLAG_RDWR);
  	}
  }
  rt_device_control(dev, RT_DEVICE_CTRL_SET_RELOAD_VALUE, (void *)&Ferq);
  rt_device_control(dev, RT_DEVICE_CTRL_SET_PULSE_VALUE, (void *)&pwm);
  rt_device_control(dev, RT_DEVICE_CTRL_SET_PULSE_COUNTS, (void *)&num);
  rt_device_control(dev, RT_DEVICE_CTRL_SEND_PULSE, (void *)0);
  
  delay = Ferq*num/10000+1;
  rt_thread_delay(delay);
  
  rt_dprintf(BUZZER_DEBUG_THREAD,("play delay %d\n",delay));
}


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
			//按键声音
			buzzer_control(1000,500,15);
			break;
		}
		case BZ_TYPE2:
		{
			buzzer_control(800,600,500);
			
			break;
		}
		case BZ_TYPE_LOCK:
		{
			//上锁
			rt_uint8_t i;
			
			for(i = 0 ;i < 2;i++)
			{
				buzzer_control(500,600,500);
				buzzer_control(800,600,300);
				//rt_thread_delay(15);
			}
			break;
		}
		case BZ_TYPE_UNLOCK:
		{
			//解锁成功
			buzzer_control(900,450,500);
			break;
		}
		case BZ_TYPE_INIT:
		{
			//开机提示
			buzzer_control(800,600,500);
			break;
		}
		case BZ_TYPE_INPUT_ERROR:
		{
			//操作错误
			rt_uint32_t i;
			
			/*for(i = 0 ;i < 3;i++)
			{
				buzzer_control(500,450,100);
				buzzer_control(600,450,100);
			}*/
			buzzer_control(450,200,600);
			
			break;
		}
		case BZ_TYPE_KEY_ERROR:
		{
			//钥匙错误
			rt_uint32_t i;
			
			for(i = 0 ;i < 20;i++)
			{
				buzzer_control(800,500,300);
				buzzer_control(400,200,200);
			}
			break;
		}
		case BZ_TYPE_OPOK:
		{
			//操作成功
			buzzer_control(1000,500,500);
			break;
		}
		case BZ_TYPE_RF433_STRART:
		{
			//433启动
			buzzer_control(1200,600,500);
			break;
		}
		default:
		{
			rt_kprintf("buzzer type is error!!!\n");
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
		                           RT_NULL,512,103, 5);//优先级不能太高
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

void buzzer_ch(rt_uint32_t f,rt_uint32_t t,rt_uint16_t T)
{
	buzzer_control(f,t,T);
}
FINSH_FUNCTION_EXPORT(buzzer_ch, buzzer_change(f t T));

/*
void budebug(rt_int8_t cmd,rt_size_t f,rt_size_t t,rt_size_t T)
{
	typedef struct
	{
		rt_uint16_t Ferq;
		rt_uint16_t Pwm;
		rt_uint16_t Delay;
	}BuDebugDef,*BuDebugDef_p;
	rt_uint32_t i,j;
	static BuDebugDef_p *arg;

	if(cmd == 0)
	{
		arg = rt_calloc(1,f);
		RT_ASSERT(arg != RT_NULL);
	}

	arg[cmd][0] = f;
	arg[cmd][1] = t;
	arg[cmd][2] = T;
	
	for(i = 0 ;i < 20;i++)
	{
		for(j = 0;j<sizeof(arg))
		{

		}
		buzzer_control(800,500,300);
	}

}*/
#endif

