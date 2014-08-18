/*********************************************************************
 * Filename:			keyboard.c
 *
 * Description:
 *
 * Author:				BRIGHT PAN
 * Email:				bright_pan@yuettak.com
 * Date:				2014-04-18
 *
 * Modify:
 *
 * Copyright (C) 2014 Yuettak Co.,Ltd
 ********************************************************************/

#include "keyboard.h"
#include "kb_dev.h"
#include "untils.h"
#include "gpio_exti.h"
#include "gpio_pin.h"
#include "gpio_pwm.h"

#define KB_TIMEOUT 20
#define KB_VERIFY_TIMES 3
#define KBUF_MAX_SIZE KEY_KBOARD_CODE_SIZE
#define KB_MAIL_MAX_MSGS 5
#define VERIFY_STATUS_INPUT 0x01
#define VERIFY_STATUS_EXIT 0x02

typedef struct {
	uint16_t type;
	KB_MODE_TYPEDEF mode;
	uint8_t c;
}KB_MAIL_TYPEDEF;

typedef struct {
	uint8_t data[KBUF_MAX_SIZE];
	uint8_t size;
	uint8_t data_verify[KBUF_MAX_SIZE];
	uint8_t size_verify;
	uint8_t verify_status;
}KBUF_TYPEDEF;

typedef struct {
    KBUF_TYPEDEF normal_auth;
    KBUF_TYPEDEF setting_auth;
    KBUF_TYPEDEF setting;
	KBUF_TYPEDEF password;
	KBUF_TYPEDEF superpwd;
    KBUF_TYPEDEF fprint;
    uint8_t normal_auth_times;
    uint8_t setting_auth_times;
}KB_DATA_TYPEDEF;

static rt_mq_t kb_mq;
static KB_MODE_TYPEDEF kb_mode = KB_MODE_NORMAL_AUTH;
static KB_DATA_TYPEDEF kb_data;
static keyboard_call_back key_api_port = RT_NULL;

void key_api_port_callback(keyboard_call_back fun)
{
	if(fun != RT_NULL)
	{
		key_api_port = fun;
	}
}

__INLINE void
kb_data_init(void)
{
	rt_memset(&kb_data, 0, sizeof(kb_data));
}

__STATIC_INLINE void
kbuf_init(KBUF_TYPEDEF *buf)
{
	rt_memset(buf, 0, sizeof(*buf));
}

__STATIC_INLINE void
kbuf_add(KBUF_TYPEDEF *buf, uint8_t c)
{
	if (buf->size < KBUF_MAX_SIZE)
		buf->data[buf->size++] = c;
}
__STATIC_INLINE void
kbuf_add_verify(KBUF_TYPEDEF *buf, uint8_t c)
{
	if (buf->size_verify < KBUF_MAX_SIZE)
		buf->data_verify[buf->size_verify++] = c;
}

__STATIC_INLINE void
kb_normal_auth_reset(void)
{
	kb_data.normal_auth_times = 0;
}

__STATIC_INLINE uint8_t
kb_normal_auth_size(void)
{
	return kb_data.normal_auth.size;
}

__STATIC_INLINE int8_t
kb_normal_auth_failure(void)
{
	int8_t result = 0;

	rt_kprintf("normal verify failure, %d\n", kb_data.normal_auth_times);
	if (kb_data.normal_auth_times >= KB_VERIFY_TIMES) {
		/* TODO:  alarm */
		kb_normal_auth_reset();
		rt_kprintf("normal auth alarm\n");
		result = 1;
	}
	kb_data.normal_auth_times++;
	return result;
}

__STATIC_INLINE void
kb_setting_auth_reset(void)
{
	kb_data.setting_auth_times = 0;
}

__STATIC_INLINE uint8_t
kb_setting_auth_size(void)
{
	return kb_data.setting_auth.size;
}

__STATIC_INLINE int8_t
kb_setting_auth_failure(void)
{
	int8_t result = 0;

	rt_kprintf("setting verify failure, %d\n", kb_data.setting_auth_times);
	if (kb_data.setting_auth_times >= KB_VERIFY_TIMES) {
		/* TODO:  alarm */
		kb_setting_auth_reset();
		rt_kprintf("setting auth alarm\n");
		result = 1;
	}
	kb_data.setting_auth_times++;
	return result;
}

typedef enum {
	KB_EOK = 0,
	KB_ECANCEL,
	KB_EEMPTY,
	KB_EINPUT,
	KB_ESWITCH,
	KB_EEXIT,
	KB_EVERIFY_SUCCESS,
	KB_EVERIFY_FAILURE,
}KB_ERROR_TYPEDEF;

__STATIC_INLINE KB_ERROR_TYPEDEF
kb_normal_auth_process(KB_MODE_TYPEDEF *mode, KBUF_TYPEDEF *buf, uint8_t c, uint16_t *key_id)
{
    KB_ERROR_TYPEDEF result = KB_EOK;
	int id;

    if (c == '#') {
		if (buf->size) {
#if (defined RT_USING_FINSH) && (defined KB_DEBUG)
            rt_kprintf("kb_normal :");
            print_char(buf->data, buf->size);
#endif
			id = device_config_key_verify(KEY_TYPE_KBOARD, buf->data);
			if (id >= 0) {
				/* TODO:  verify success */
                *key_id = (uint16_t)id;
				result = KB_EVERIFY_SUCCESS;
			} else {
				result = KB_EVERIFY_FAILURE;
			}
        } else {
			//send_kb_mail(KB_MAIL_TYPE_SET_MODE, KB_MODE_SETTING_AUTH, 0);
        }
		kbuf_init(buf);
    } else {
        if (c == '*') {
			kbuf_init(buf);
		}
        else
            kbuf_add(buf, c);
    }
    return result;
}

__STATIC_INLINE KB_ERROR_TYPEDEF
kb_setting_auth_process(KB_MODE_TYPEDEF *mode, KBUF_TYPEDEF *buf, uint8_t c)
{
    KB_ERROR_TYPEDEF result = KB_EOK;

    if (c == '#') {
        if (buf->size) {
#if (defined RT_USING_FINSH) && (defined KB_DEBUG)
            rt_kprintf("kb_setting :");
            print_char(buf->data, buf->size);
#endif
			if (device_config_superpwd_verify(buf->data) < 0) {
				/* TODO:  verify failure */
				result = KB_EVERIFY_FAILURE;
			} else {
				/* TODO:  verify success */
				send_kb_mail(KB_MAIL_TYPE_SETMODE, KB_MODE_SETTING, 0);
				result = KB_EVERIFY_SUCCESS;
			}
        } else {
			/* TODO:  EMPTY*/
        }
		kbuf_init(buf);
    } else {
        if (c == '*') {
			kbuf_init(buf);
			if (!buf->size) {
				//send_kb_mail(KB_MAIL_TYPE_SET_MODE, KB_MODE_NORMAL_AUTH, 0);
			}
		} else {
            kbuf_add(buf, c);
		}
    }
    return result;
}

__STATIC_INLINE KB_ERROR_TYPEDEF
kb_setting_process(KB_MODE_TYPEDEF *mode, KBUF_TYPEDEF *buf, uint8_t c)
{
    KB_ERROR_TYPEDEF result = KB_EOK;

    if (c == '#') {
        if (buf->size) {
            if (buf->size == 1) {
                switch (buf->data[0]) {
					case '1' : {// add password
						send_kb_mail(KB_MAIL_TYPE_SETMODE, KB_MODE_ADD_PASSWORD, 0);
						break;
					}
					case '2' : {// add fprint
						send_kb_mail(KB_MAIL_TYPE_SETMODE, KB_MODE_ADD_FPRINT, 0);
						break;
					}
					case '3' : {// modify super pwd
						send_kb_mail(KB_MAIL_TYPE_SETMODE, KB_MODE_MODIFY_SUPERPWD, 0);
						break;
					}
					default : {
#if (defined RT_USING_FINSH) && (defined KB_DEBUG)
						rt_kprintf("invalid setting, please reinput\n");
#endif
						//无效设置
						if(key_api_port != RT_NULL)
						{
							KEYBOARD_USER data;
							data.event = KEY_MODE_INPUT_ERROR;
							key_api_port(&data);
						}
						result = KB_EVERIFY_FAILURE;
						break;
					}
				}
            } else {
#if (defined RT_USING_FINSH) && (defined KB_DEBUG)
                rt_kprintf("invalid setting, please reinput\n");
#endif
            }

        } else {
			/* TODO:  EMPTY*/
        }
		kbuf_init(buf);
    } else {
        if (c == '*') {
			kbuf_init(buf);
			if (!buf->size) {
				//send_kb_mail(KB_MAIL_TYPE_SET_MODE, KB_MODE_NORMAL_AUTH, 0);
			}
		} else {
            kbuf_add(buf, c);
		}
    }

    return result;
}

__STATIC_INLINE KB_ERROR_TYPEDEF
kb_add_password_process(KB_MODE_TYPEDEF *mode, KBUF_TYPEDEF *buf, uint8_t c)
{
    KB_ERROR_TYPEDEF result = KB_EOK;

    if (c == '#') {
		if (!(buf->verify_status & VERIFY_STATUS_EXIT)) {
			if (buf->verify_status & VERIFY_STATUS_INPUT) {
#if (defined RT_USING_FINSH) && (defined KB_DEBUG)
				rt_kprintf("kb_add_password verify:");
				print_char(buf->data_verify, buf->size_verify);
#endif
				if (buf->size != 6 || rt_memcmp(buf->data, buf->data_verify, 6)) {
					/* TODO:  verify failure */
					result = KB_EVERIFY_FAILURE;
				} else {
					/* TODO:  verify success */
					result = KB_EVERIFY_SUCCESS;
				}
				buf->verify_status |= VERIFY_STATUS_EXIT;
#if (defined RT_USING_FINSH) && (defined KB_DEBUG)
                rt_kprintf("* return setting mode, # input new pw once again.\n");
#endif
			} else { //VERIFY_STATUS_INPUT
#if (defined RT_USING_FINSH) && (defined KB_DEBUG)
				rt_kprintf("kb_add_password :");
				print_char(buf->data, buf->size);
#endif
				//请再输入一遍
				if(key_api_port != RT_NULL)
				{
					KEYBOARD_USER data;

					data.event = KEY_REINPUT_NEW_CODE;
					key_api_port(&data);
				}
				buf->verify_status |= VERIFY_STATUS_INPUT;
			}
		} else { //VERIFY_STATUS_EXIT
			// try add again
			kbuf_init(buf);
		}
    } else {
		if (!(buf->verify_status & VERIFY_STATUS_EXIT)) {
			if (buf->verify_status & VERIFY_STATUS_INPUT) {
				if (c == '*') {
					buf->size_verify = 0;
					if (!buf->size_verify) {

					}
				} else {
					kbuf_add_verify(buf, c);
				}
			} else { //VERIFY_STATUS_INPUT
				if (c == '*') {
					buf->size = 0;
					if (!buf->size) {

					}
				} else {
					kbuf_add(buf, c);
				}
			}
		} else { //VERIFY_STATUS_EXIT
			if (c == '*') {
				kbuf_init(buf);
				send_kb_mail(KB_MAIL_TYPE_SETMODE, KB_MODE_SETTING, 0);
			}
		}
    }
    return result;
}

__STATIC_INLINE KB_ERROR_TYPEDEF
kb_modify_superpwd_process(KB_MODE_TYPEDEF *mode, KBUF_TYPEDEF *buf, uint8_t c)
{
    KB_ERROR_TYPEDEF result = KB_EOK;

    if (c == '#') {
		if (!(buf->verify_status & VERIFY_STATUS_EXIT)) {
			if (buf->verify_status & VERIFY_STATUS_INPUT) {
#if (defined RT_USING_FINSH) && (defined KB_DEBUG)
				rt_kprintf("modify superpwd verify:");
				print_char(buf->data_verify, buf->size_verify);
#endif
				if (buf->size != 6 || rt_memcmp(buf->data, buf->data_verify, 6)) {
					/* TODO:  verify failure */
					result = KB_EVERIFY_FAILURE;
				} else {
					/* TODO:  verify success */
					result = KB_EVERIFY_SUCCESS;
				}
				buf->verify_status |= VERIFY_STATUS_EXIT;
#if (defined RT_USING_FINSH) && (defined KB_DEBUG)
                rt_kprintf("* return setting mode, # input superpwd once again.\n");
#endif
			} else { //VERIFY_STATUS_INPUT
#if (defined RT_USING_FINSH) && (defined KB_DEBUG)
				rt_kprintf("modify superpwd :");
				print_char(buf->data, buf->size);
#endif
				//请再输入一遍
				if(key_api_port != RT_NULL)
				{
					KEYBOARD_USER data;

					data.event = KEY_REINPUT_NEW_CODE;
					key_api_port(&data);
				}
				buf->verify_status |= VERIFY_STATUS_INPUT;
			}
		} else { //VERIFY_STATUS_EXIT
			// try add again
			kbuf_init(buf);
		}
    } else {
		if (!(buf->verify_status & VERIFY_STATUS_EXIT)) {
			if (buf->verify_status & VERIFY_STATUS_INPUT) {
				if (c == '*') {
					buf->size_verify = 0;
					if (!buf->size_verify) {

					}
				} else {
					kbuf_add_verify(buf, c);
				}
			} else { //VERIFY_STATUS_INPUT
				if (c == '*') {
					buf->size = 0;
					if (!buf->size) {

					}
				} else {
					kbuf_add(buf, c);
				}
			}
		} else { //VERIFY_STATUS_EXIT
			if (c == '*') {
				kbuf_init(buf);
				send_kb_mail(KB_MAIL_TYPE_SETMODE, KB_MODE_SETTING, 0);
			}
		}
    }
    return result;
}

__STATIC_INLINE KB_ERROR_TYPEDEF
kb_add_fprint_process(KB_MODE_TYPEDEF *mode, KBUF_TYPEDEF *buf, uint8_t c)
{
    KB_ERROR_TYPEDEF result = KB_EOK;

    if (c == '#') {
		if (!(buf->verify_status & VERIFY_STATUS_EXIT)) {
			result = KB_EVERIFY_FAILURE;
			result = KB_EVERIFY_SUCCESS;
			buf->verify_status |= VERIFY_STATUS_EXIT;
#if (defined RT_USING_FINSH) && (defined KB_DEBUG)
			rt_kprintf("* return setting mode, # add fprint once again.\n");
#endif
		} else { //VERIFY_STATUS_EXIT
			// try add again
			kbuf_init(buf);
		}
    } else {
		if (!(buf->verify_status & VERIFY_STATUS_EXIT)) {

		} else { //VERIFY_STATUS_EXIT
			if (c == '*') {
				kbuf_init(buf);
				send_kb_mail(KB_MAIL_TYPE_SETMODE, KB_MODE_SETTING, 0);
			}
		}
    }
    return result;
}

void
kb_thread_entry(void *parameters)
{
	rt_err_t result;
	KB_ERROR_TYPEDEF kb_error;
	KB_MAIL_TYPEDEF mail;

    uint8_t c;
	uint16_t key_id;

	kb_data_init();

	while (1) {
		rt_memset(&mail, 0, sizeof(mail));
		result = rt_mq_recv(kb_mq, &mail, sizeof(mail), KB_TIMEOUT*RT_TICK_PER_SECOND);
		if (result == RT_EOK) {
			if (mail.type == KB_MAIL_TYPE_TIMEOUT) {
				kb_mode = mail.mode;
                rt_kprintf("input timeout, return normal auth mode.\n");
			}
			if (mail.type == KB_MAIL_TYPE_SETMODE) {
				kb_mode = mail.mode;
				switch (kb_mode) {
					case KB_MODE_NORMAL_AUTH: {
                        rt_kprintf("return normal auth mode.\n");
                        /*
						  if(key_api_port != RT_NULL)
						  {
						  KEYBOARD_USER data;

						  data.event = KEY_NORMAL_MODE;
						  key_api_port(&data);
						  }
                        */
						break;
					}
					case KB_MODE_SETTING_AUTH: {
                        rt_kprintf("enter setting auth mode.\n");
						//切换到设置模式
						if(key_api_port != RT_NULL)
						{
							KEYBOARD_USER data;

							data.event = KEY_SET_MODE;
							key_api_port(&data);
						}
						break;
					}
					case KB_MODE_SETTING: {
                        rt_kprintf("enter setting mode.\n");
						//请选择模式
						if(key_api_port != RT_NULL)
						{
							KEYBOARD_USER data;

							data.event = KEY_CHOOSE_MODE;
							key_api_port(&data);
						}
						break;
					}
					case KB_MODE_ADD_PASSWORD: {
                        rt_kprintf("enter add password mode.\n");
                        //提示输入新钥匙
						if(key_api_port != RT_NULL)
						{
							KEYBOARD_USER data;
							data.event = KEY_INPUT_NEW_CODE;
							key_api_port(&data);
						}
						break;
					}
					case KB_MODE_MODIFY_SUPERPWD: {
                        rt_kprintf("enter modify super password mode.\n");
						/* TODO:
                        //提示输入新钥匙
						if(key_api_port != RT_NULL)
						{
							KEYBOARD_USER data;
							data.event = KEY_INPUT_NEW_CODE;
							key_api_port(&data);
						}
						*/
						break;
					}
					case KB_MODE_ADD_FPRINT: {
                        rt_kprintf("enter add fprint mode.\n");
						/* TODO:
                        //提示输入新钥匙
						if(key_api_port != RT_NULL)
						{
							KEYBOARD_USER data;
							data.event = KEY_INPUT_NEW_CODE;
							key_api_port(&data);
						}
						*/
						break;
					}
					default :{
						break;
					}
				}
			}
			if (mail.type == KB_MAIL_TYPE_INPUT) {
                c = mail.c;
				//按键声音
				if(key_api_port != RT_NULL)
				{
					KEYBOARD_USER data;

					data.event = KEY_NOTIFYSOUND;
					key_api_port(&data);
				}
//                gpio_pin_output(DEVICE_NAME_LOGO_LED, 1);
				switch(kb_mode) {
					case KB_MODE_NORMAL_AUTH: {
						kb_error = kb_normal_auth_process(&kb_mode, &kb_data.normal_auth, c, &key_id);
						switch (kb_error) {
							case KB_EOK: {
								break;
							}
							case KB_EVERIFY_SUCCESS: {
								rt_kprintf("normal verify success!, key_id = %d\n", key_id);
								kb_normal_auth_reset();
								//密码解锁成功
								if(key_api_port != RT_NULL)
								{
									KEYBOARD_USER data;

									data.event = KEY_UNLOCK_OK;
									data.KeyPos = key_id;
									key_api_port(&data);
								}
								break;
							}
							case KB_EVERIFY_FAILURE: {
								if (kb_normal_auth_failure()) {
									//开门失败
									if(key_api_port != RT_NULL)
									{
										KEYBOARD_USER data;

										data.event = KEY_UNLOCK_FAIL;
										key_api_port(&data);
									}
								}
								else
								{
									//密码错误
									if(key_api_port != RT_NULL)
									{
										KEYBOARD_USER data;

										data.event = KEY_CODE_ERROR;
										key_api_port(&data);
									}
								}
								break;
							}
							default : {
								break;
							}
						}
						break;
					}
					case KB_MODE_SETTING_AUTH: {
						kb_error = kb_setting_auth_process(&kb_mode, &kb_data.setting_auth, c);
						switch (kb_error) {
							case KB_EOK: {
								break;
							}
							case KB_EVERIFY_SUCCESS: {
								rt_kprintf("setting verify success!");
								kb_setting_auth_reset();
								break;
							}
							case KB_EVERIFY_FAILURE: {
								if (kb_setting_auth_failure()) {
                                    send_kb_mail(KB_MAIL_TYPE_TIMEOUT, KB_MODE_NORMAL_AUTH, 0);
									//报警
									if(key_api_port != RT_NULL)
									{
										KEYBOARD_USER data;

										data.event = KEY_UNLOCK_FAIL;
										key_api_port(&data);
									}
								}
								else
								{
									//超级密码错误
									if(key_api_port != RT_NULL)
									{
										KEYBOARD_USER data;

										data.event = KEY_CODE_ERROR;
										key_api_port(&data);
									}
								}
								break;
							}
							default : {
								break;
							}
						}
						break;
					}
					case KB_MODE_SETTING: {
						kb_error = kb_setting_process(&kb_mode, &kb_data.setting, c);
						switch (kb_error) {
							case KB_EOK: {
								break;
							}
							default : {
								break;
							}
						}
						break;
					}
					case KB_MODE_ADD_PASSWORD: {
						kb_error = kb_add_password_process(&kb_mode, &kb_data.password, c);
						switch (kb_error) {
							case KB_EOK: {
								break;
							}
							case KB_EVERIFY_SUCCESS: {
								int NewKeyPos;
								rt_kprintf("add password success : %s\n", kb_data.password.data);
								NewKeyPos = device_config_key_create(KEY_TYPE_KBOARD, kb_data.password.data);
								rt_kprintf("%d\n",NewKeyPos);
								//注册成功
								if((NewKeyPos >= 0) && (NewKeyPos < KEY_NUMBERS))
								{
									if(key_api_port != RT_NULL)
									{
										KEYBOARD_USER data;
										data.event = KEY_REGISTER_OK;
										data.KeyPos = NewKeyPos;
										key_api_port(&data);
									}
								}
								else
								{
									//钥匙库已满
									if(key_api_port != RT_NULL)
									{
										KEYBOARD_USER data;
										data.event = KEY_LIB_FULL;
										key_api_port(&data);
									}
								}
								break;
							}
							case KB_EVERIFY_FAILURE: {
								rt_kprintf("add password failure!\n");
								//注册失败
								if(key_api_port != RT_NULL)
								{
									KEYBOARD_USER data;

									data.event = KEY_REGISTER_FAIL;
									key_api_port(&data);
								}
								break;
							}
							default : {
								break;
							}
						}
						break;
					}
					case KB_MODE_MODIFY_SUPERPWD: {
						kb_error = kb_modify_superpwd_process(&kb_mode, &kb_data.superpwd, c);
						switch (kb_error) {
							case KB_EOK: {
								break;
							}
							case KB_EVERIFY_SUCCESS: {
								rt_kprintf("modify superpwd success : ");
                                print_char(kb_data.superpwd.data, kb_data.superpwd.size);
								device_config_superpwd_save(kb_data.superpwd.data);
								break;
							}
							case KB_EVERIFY_FAILURE: {
								rt_kprintf("modify superpwd failure!\n");
								/* TODO:
								//注册失败
								if(key_api_port != RT_NULL)
								{
									KEYBOARD_USER data;

									data.event = KEY_REGISTER_FAIL;
									key_api_port(&data);
								}
								*/
								break;
							}
							default : {
								break;
							}
						}
						break;
					}
					case KB_MODE_ADD_FPRINT: {
						kb_error = kb_add_fprint_process(&kb_mode, &kb_data.fprint, c);
						switch (kb_error) {
							case KB_EOK: {
								break;
							}
							case KB_EVERIFY_SUCCESS: {
								rt_kprintf("add fprint success \n");
								break;
							}
							case KB_EVERIFY_FAILURE: {
								rt_kprintf("add fprint failure!\n");
								/* TODO:
								//注册失败
								if(key_api_port != RT_NULL)
								{
									KEYBOARD_USER data;

									data.event = KEY_REGISTER_FAIL;
									key_api_port(&data);
								}
								*/
								break;
							}
							default : {
								break;
							}
						}
						break;
					}
					default : {
						break;
					}
				}
			}
		} else { //time out
            //gpio_pin_output(DEVICE_NAME_LOGO_LED, 0);
			if (kb_mode != KB_MODE_NORMAL_AUTH && kb_mode != KB_MODE_ADD_FPRINT) {
				send_kb_mail(KB_MAIL_TYPE_TIMEOUT, KB_MODE_NORMAL_AUTH, 0);
				kb_data_init();
			}
		}
	}
}

__INLINE rt_err_t
send_kb_mail(uint16_t type, KB_MODE_TYPEDEF mode, uint8_t c)
{
	rt_err_t result = -RT_EFULL;
	KB_MAIL_TYPEDEF mail;

	if (kb_mq != RT_NULL)
	{
		mail.type = type;
		mail.mode = mode;
		mail.c = c;
		result = rt_mq_send(kb_mq, &mail, sizeof(mail));
		if (result == -RT_EFULL)
		{
#if (defined RT_USING_FINSH) && (defined KB_DEBUG)
			rt_kprintf("kb_mq is full!!!\n");
#endif
		}
	}
	else
	{
#if (defined RT_USING_FINSH) && (defined KB_DEBUG)
		rt_kprintf("kb_mq is RT_NULL!!!!\n");
#endif
	}
    return result;
}

int
rt_keyboard_init(void)
{
	rt_thread_t kb_thread;

	//initial keyboard mq
    kb_mq = rt_mq_create("kboard", sizeof(KB_MAIL_TYPEDEF),
						 KB_MAIL_MAX_MSGS, RT_IPC_FLAG_FIFO);
	if (kb_mq == RT_NULL)
		return -1;
	//key board thread
	kb_thread = rt_thread_create("kboard", kb_thread_entry,
								 RT_NULL, 1024, 99, 5);
	if (kb_thread == RT_NULL)
		return -1;

	rt_thread_startup(kb_thread);
	return 0;
}

INIT_APP_EXPORT(rt_keyboard_init);

#ifdef RT_USING_FINSH
#include <finsh.h>

FINSH_FUNCTION_EXPORT(send_kb_mail, send_kb_mail[type mode c]);
#endif // RT_USING_FINSH
