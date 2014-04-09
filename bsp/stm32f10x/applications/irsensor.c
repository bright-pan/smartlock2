/*
功能:红外传感器检测遮挡物
*/
#include "irsensor.h"

#include "alarm.h"

#define DEVICE_NAME_INFRA_PULSE_PWM_IC "infra_ic"
#define DEVICE_NAME_INFRA_PULSE        "infra_s"

rt_uint8_t ir_check_process(void)
{
	extern void pwm_ic_start(char *str);	
	extern void pwm_ic_stop(char *str);
	extern rt_uint32_t pwm_ic_get(char *str);
	extern void pwm_set_counts(char *str, rt_uint16_t counts);
	extern void pwm_send_pulse(char *str);
	extern rt_uint32_t 	pwm_ic_time(void);
	volatile rt_uint32_t	rev_time = 0;
	rt_uint16_t						rev_num = 0;
	rt_uint32_t						run = 10;

	while(run--)
	{
		//open Input capture function 
		pwm_ic_start(DEVICE_NAME_INFRA_PULSE_PWM_IC);

		//send 500ms of 38kHz PWM pulse 
		pwm_set_counts(DEVICE_NAME_INFRA_PULSE,500); //1
		pwm_send_pulse(DEVICE_NAME_INFRA_PULSE);

		//analysis result
		rt_thread_delay(10);
		pwm_ic_stop(DEVICE_NAME_INFRA_PULSE_PWM_IC);
		rev_time = pwm_ic_time();
		//rt_kprintf("rev_time = %d\n",rev_time);
		if(rev_time > 10000 && rev_time < 20000)
		{
			rev_num++;
			if(rev_num > 5)
			{
				return 1;
			}
		}
	}

	return 0;
}

void ir_cover_process(void)
{
	static rt_uint8_t	ok_num = 0;
	
  if(ir_check_process())
  {
    ok_num++;
    if(ok_num == 2)
    {
      ok_num = 3;
      /* send camera infrared alarm information */      
      // sms
      send_alarm_mail(ALARM_TYPE_CAMERA_IRDASENSOR,ALARM_PROCESS_FLAG_LOCAL,0,0);
      rt_kprintf("start send SMS\n");
    }
    else if(ok_num > 2)
    {
      /* Did not leave marks */
      ok_num = 3;
    }
  }
  else
  {
    /* Leave marks */
    if(ok_num == 3)
    {
      /* send sem camera */
      //camera
      camera_send_mail(ALARM_TYPE_CAMERA_IRDASENSOR,sys_cur_date());
      rt_kprintf("start make picture\n");
    }
    ok_num = 0;
  }
}

void ir_thread_entry(void *arg)
{
	while(1)
	{
		ir_cover_process();
		rt_thread_delay(100);
	}
}


int ir_thread_init(void)
{
	rt_thread_t id;
	
	id = rt_thread_create("IRCheck",
	                      ir_thread_entry,
	                      RT_NULL,
	                      512,
	                      101,
	                      30);
	if(RT_NULL == id )
	{
	  rt_kprintf("IRCheck thread create fail\n\n");
	  
	  return -1;
	}
	rt_thread_startup(id);
	
	return 0;
}

INIT_APP_EXPORT(ir_thread_init);

