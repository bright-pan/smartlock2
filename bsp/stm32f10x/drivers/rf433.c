/*********************************************************************
 * Filename:			rf433.c
 *
 * Description:
 *
 * Author:				Bright Pan
 * Email:				bright_pan@yuettak.com
 * Date:				2014-10-14
 *
 * Modify:
 *
 * Copyright (C) 2014 Yuettak Co.,Ltd
 ********************************************************************/

#include <rthw.h>
#include <rtthread.h>
#include "stm32f10x.h"
#include "rf433.h"
#include "gpio_pin.h"
#include "gpio_exti.h"
#include "untils.h"
#include "config.h"
#include "sms.h"
#include "local.h"

#define RF433_DEBUG 1
#define RF433_TIMEOUT 1000
#define RF433_SMS_LIMITE 200

static volatile s32 time_out; // ms 计时变量
static volatile s32 time_cnt;
static volatile s32 status_cnt;

#define RF_DAT GPIO_ReadInputDataBit(user->gpiox,user->gpio_pinx)
#define START_TIME  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 , ENABLE);TIM_Cmd(TIM2, ENABLE)
#define STOP_TIME  TIM_Cmd(TIM2, DISABLE);RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 , DISABLE)
#define UNMASK_TIME TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE)
#define MASK_TIME TIM_ITConfig(TIM2,TIM_IT_Update,DISABLE)

#define RF433_MAIL_MAX_MSGS 1

typedef struct
{
    s32 cmd;
    u8 data[16];
}RF433_MAIL_TYPEDEF;

// rf433 msg queue for rf433 alarm
static rt_mq_t rf433_mq;
static rt_timer_t rf433_dat_scan_timer = RT_NULL;

static void
rf433_check_start(void)
{
    status_cnt = 0;
    rt_timer_start(rf433_dat_scan_timer);
    
}

static s32
rf433_check_stop(void)
{
    rt_timer_stop(rf433_dat_scan_timer);
    return status_cnt;
}

/*
 * 函数名：TIM2_NVIC_Configuration
 * 描述  ：TIM2中断优先级配置
 * 输入  ：无
 * 输出  ：无
 */
static void
TIM2_NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}


/*TIM_Period--1000   TIM_Prescaler--71 -->中断周期为10us*/
static void
TIM2_Configuration(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 , ENABLE);
    TIM_DeInit(TIM2);
    TIM_TimeBaseStructure.TIM_Period=720;		 								/* 自动重装载寄存器周期的值(计数值) */
    /* 累计 TIM_Period个频率后产生一个更新或者中断 */
    TIM_TimeBaseStructure.TIM_Prescaler= (1 - 1);				    /* 时钟预分频数 72M/72 */
    TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1; 		/* 采样分频 */
    TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; /* 向上计数模式 */
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    TIM_ClearFlag(TIM2, TIM_FLAG_Update);							    		/* 清除溢出中断标志 */
    TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);
    TIM_Cmd(TIM2, ENABLE);																		/* 开启时钟 */

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 , DISABLE);		/*先关闭等待使用*/
}

void TIM2_IRQHandler(void)
{
	if ( TIM_GetITStatus(TIM2 , TIM_IT_Update) != RESET )
	{
		TIM_ClearITPendingBit(TIM2 , TIM_FLAG_Update);
		--time_out;
        ++time_cnt;
	}
}

static void
rf433_thread_entry(void *parameter)
{
	rt_err_t result;
	RF433_MAIL_TYPEDEF rf433_mail_buf;
    u8 sum, i, flag = 0;
    s32 temp;
    TIM2_NVIC_Configuration();
    TIM2_Configuration();

	while (1)
	{
		// receive mail
		rt_memset(&rf433_mail_buf, 0, sizeof(rf433_mail_buf));
		result = rt_mq_recv(rf433_mq, &rf433_mail_buf, sizeof(rf433_mail_buf), RF433_TIMEOUT);
		if (result == RT_EOK)
        {
			switch (rf433_mail_buf.cmd)
			{
				case RF433_START:
                {
                    flag = 1;
                    gpio_pin_output(DEVICE_NAME_RF_ENABLE, 0, 1);
                    rf433_check_start();
                    break;
                }
				case RF433_VERIFY:
				{
                    sum = 0;
                    for (i = 0; i < 15; ++i)
                        sum += rf433_mail_buf.data[i];
                    if (sum == rf433_mail_buf.data[15]) {
                        RT_DEBUG_LOG(RF433_DEBUG,("accept rf433\n"));
                        temp = device_config_key_verify(KEY_TYPE_RF433, rf433_mail_buf.data, 4);
                        if (temp >= 0) {
                            //send_sms_mail(ALARM_TYPE_RFID_KEY_SUCCESS, 0, RT_NULL, 0);
                            union alarm_data *data;
                            flag = 0;
                            data = rt_calloc(1,sizeof(*data));
                            data->key.ID = temp;
                            data->key.Type = KEY_TYPE_RF433;
                            data->key.sms = 0;
                            send_local_mail(ALARM_TYPE_KEY_RIGHT,0,data);
                            rt_free(data);
                        }
                    } else {
                        RT_DEBUG_LOG(RF433_DEBUG,("rf433 verify error\n"));
                    }
					break;
				}
				case RF433_STOP:
				{
					//fprint_unlock_process(&local_mail_buf);
					
					break;
				}
				default :
                {
                    RT_DEBUG_LOG(RF433_DEBUG,("this rf433 cmd is not process...\n"));
                    break;
                };
			}
        } else {
            if (flag) {
                if (abs(rf433_check_stop()) < RF433_TIMEOUT - RF433_SMS_LIMITE)
                    send_sms_mail(ALARM_TYPE_SMS_RF433_ERROR, 0, RT_NULL, 0, PHONE_AUTH_SMS);
                flag = 0;
            }
        }
    }
}

void
send_rf433_mail(s32 cmd, u8 *data)
{
	RF433_MAIL_TYPEDEF buf;
	rt_err_t result;
	//send mail
	buf.cmd = cmd;
    rt_memcpy(buf.data, data, 16);
	if (rf433_mq != NULL) {
		result = rt_mq_send(rf433_mq, &buf, sizeof(RF433_MAIL_TYPEDEF));
		if (result == -RT_EFULL) {
            RT_DEBUG_LOG(RF433_DEBUG,("rf433_mq is full!!!\n"));
		}
	} else {
		RT_DEBUG_LOG(RF433_DEBUG,("rf433_mq is RT_NULL!!!\n"));
	}
}

static void
rf433_dat_check(void *parameters)
{

    rt_device_t dev = device_enable(DEVICE_NAME_RF_DAT);
    gpio_device *gpio = (gpio_device *)dev;
    struct gpio_pin_user_data* user = (struct gpio_pin_user_data *)gpio->parent.user_data;
    u8 b, i = 0;
    u8 dat, byte = 0;
    u8 data[16] = {0,};
    if (gpio_pin_input(DEVICE_NAME_HALL, 0) != HALL_STATUS) {
        --status_cnt;
        return;
    } else {
        ++status_cnt;
    }
    //等一次跳变到来
    START_TIME;
    MASK_TIME;
    time_out=100;//超时设定1ms
    time_cnt=0;
    UNMASK_TIME;
    
    if(RF_DAT)
        while(RF_DAT)
        {
            if(time_out<=0)
                goto __exit;
        }
    else
        while(!RF_DAT)
        {
            if(time_out<=0)
                goto __exit;
        }


    //抓一个完整的高脉冲或低脉冲
    MASK_TIME;
    time_out=100;//超时设定1ms
    time_cnt=0;
    UNMASK_TIME;
    if(RF_DAT)
        while(RF_DAT)  {
            if(time_out<=0)
                goto __exit;
        }
    else
        while(!RF_DAT)
        {
            if(time_out<=0)
                goto __exit;
        }
    //此时time_cnt内就是脉冲宽度了。
    if((time_cnt>60)||(time_cnt<40))
        goto __exit;
    //抓一个完整的高脉冲或低脉冲
    MASK_TIME;
    time_out=100;//超时设定1ms
    time_cnt=0;
    UNMASK_TIME;
    if(RF_DAT)
        while(RF_DAT)  {
            if(time_out<=0)
                goto __exit;
        }
    else
        while(!RF_DAT)
        {
            if(time_out<=0)
                goto __exit;
        }
    //此时time_cnt内就是脉冲宽度了。
    if((time_cnt>60)||(time_cnt<40))
        goto __exit;

    //这里继续判断引导码，直到抓到1000us的low pulse
    while (1)
    {
        MASK_TIME;
        time_out=110;//超时设定1ms
        time_cnt=0;
        UNMASK_TIME;
        if(RF_DAT)
            while((dat = RF_DAT) == 1)  {
                if(time_out<=0)
                    goto __exit;
            }
        else
            while((dat = RF_DAT) == 0) {
                if(time_out<=0)
                    goto __exit;
            }
        //此时time_cnt内就是脉冲宽度了。
        if (dat && time_cnt > 90)
            break;
        if((time_cnt>60)||(time_cnt<40))
            goto __exit;
    }

    //接收n字节数据

    for (b = 0; b < 16; ++b) {
        byte = 0;
        for (i = 0; i < 8; ++i)
        {
            MASK_TIME;
            time_out=110;//超时设定1ms
            time_cnt=0;
            UNMASK_TIME;
            if(RF_DAT)
                while((dat = RF_DAT) == 1)  {
                    if(time_out<=0)
                        goto __exit;
                }
            else
                goto __exit;
            //此时time_cnt内就是脉冲宽度了。
            if((time_cnt>85)||(time_cnt<65)) {
                if((time_cnt>35)||(time_cnt<15)) {
                    goto __exit;
                } else {
                    if(!RF_DAT)
                        while((dat = RF_DAT) == 0) {
                            if(time_out<=0)
                                break;
						}
                }
            } else {
                byte |= 1<< i;
                if(!RF_DAT)
                    while((dat = RF_DAT) == 0) {
                        if(time_out<=0)
                            break;
					}
            }
        }
        data[b] = byte;
    }
    send_rf433_mail(RF433_VERIFY, data);
    //判断数据是否正确
__exit:
    STOP_TIME;
}

static int
rt_rf433_init(void)
{
	rt_thread_t rf433_thread;
    
    rf433_dat_scan_timer = rt_timer_create("t_rfdat", rf433_dat_check, RT_NULL, 1,  RT_TIMER_CTRL_SET_PERIODIC | RT_TIMER_FLAG_SOFT_TIMER);
    // initial local msg queue
	rf433_mq = rt_mq_create("local", sizeof(RF433_MAIL_TYPEDEF),
							RF433_MAIL_MAX_MSGS, RT_IPC_FLAG_FIFO);
    if (rf433_mq == RT_NULL)
        return -1;

    // init local thread
    rf433_thread = rt_thread_create("rf433",
									rf433_thread_entry, RT_NULL,
									1024, 102, 5);
    if (rf433_thread == RT_NULL)
        return -1;

    rt_thread_startup(rf433_thread);
    return 0;

}
INIT_APP_EXPORT(rt_rf433_init);

#ifdef RT_USING_FINSH
#include <finsh.h>
FINSH_FUNCTION_EXPORT_ALIAS(rf433_check_start, rf_cst, rf_check_start);
FINSH_FUNCTION_EXPORT_ALIAS(rf433_check_stop, rf_csp, rf_check_stop);
FINSH_FUNCTION_EXPORT(send_rf433_mail, send_rf433_mail[cmd buf]);
#endif
