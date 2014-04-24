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

#define KB_DEBUG
#define KB_TIMEOUT 10
#define KB_VERIFY_TIMES 3
#define KBUF_MAX_SIZE KEY_KBOARD_CODE_SIZE

static rt_sem_t kb_sem;

__STATIC_INLINE uint8_t bit_to_index(uint16_t data)
{
    uint8_t result = 0;
    while (data)
    {
        result++;
        data >>= 1;
    }
    return result;
}

static const uint8_t char_remap[16] = {
    '?',
    '*', '0', '#',
    '7', '8', '9', 
    '4', '5', '6', 
    '1', '2', '3',
    '?', '?', '?',
};

typedef enum {
	KB_MODE_NORMAL_AUTH = 0,
	KB_MODE_SETTING_AUTH,
    KB_MODE_SETTING,
	KB_MODE_ADD_PASSWORD,
}KB_MODE_TYPEDEF;

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
    uint8_t normal_auth_times;
    uint8_t setting_auth_times;
}KB_DATA_TYPEDEF;

__STATIC_INLINE void
kb_data_init(KB_DATA_TYPEDEF *data)
{
	rt_memset(data, 0, sizeof(*data));
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
			*mode = KB_MODE_SETTING_AUTH;
            result = KB_ESWITCH;
        }
    } else {
        if (c == '*')
			result = KB_ECANCEL;
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
			if (rt_memcmp(device_config.param.password, buf->data, 6)) {
				/* TODO:  verify failure */
				result = KB_EVERIFY_FAILURE;
			} else {
				/* TODO:  verify success */
				*mode = KB_MODE_SETTING;
				result = KB_EVERIFY_SUCCESS;
			}

        } else {
			result = KB_EEMPTY;
        }
    } else {
        if (c == '*') {
			if (!buf->size) {
				*mode = KB_MODE_NORMAL_AUTH;
				result = KB_EEXIT;
			} else {
				result = KB_ECANCEL;
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
					case '1' : {
						rt_kprintf("input password: \n");
						*mode = KB_MODE_ADD_PASSWORD;
						result = KB_EVERIFY_SUCCESS;
						break;
					}
					case '2' : {
						rt_kprintf("setting 2\n");
						result = KB_EVERIFY_SUCCESS;
						break;
					}
					default : {
#if (defined RT_USING_FINSH) && (defined KB_DEBUG)
						rt_kprintf("invalid setting, please reinput\n");
#endif
						result = KB_EVERIFY_FAILURE;
						break;
					}
				}
            } else {
#if (defined RT_USING_FINSH) && (defined KB_DEBUG)
                rt_kprintf("invalid setting, please reinput\n");
#endif
				result = KB_EVERIFY_FAILURE;
            }

        } else {
			result = KB_EEMPTY;
        }
    } else {
        if (c == '*') {
			if (!buf->size) {
				*mode = KB_MODE_NORMAL_AUTH;
				result = KB_EEXIT;
			} else {
				result = KB_ECANCEL;
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
		if (buf->verify_status) {
#if (defined RT_USING_FINSH) && (defined KB_DEBUG)
            rt_kprintf("kb_add_password verify:");
            print_char(buf->data_verify, buf->size_verify);
#endif
			if (rt_memcmp(buf->data, buf->data_verify, 6)) {
				/* TODO:  verify failure */
				result = KB_EVERIFY_FAILURE;
			} else {
				/* TODO:  verify success */
				*mode = KB_MODE_NORMAL_AUTH;
				result = KB_EVERIFY_SUCCESS;
			}
		} else {
#if (defined RT_USING_FINSH) && (defined KB_DEBUG)
            rt_kprintf("kb_add_password :");
            print_char(buf->data, buf->size);
#endif
			buf->verify_status = 1;
		}
    } else {
		if (buf->verify_status) {
			if (c == '*') {
				if (!buf->size_verify) {
					*mode = KB_MODE_NORMAL_AUTH;
					result = KB_EEXIT;
				} else {
					result = KB_ECANCEL;
				}
			} else {
				kbuf_add_verify(buf, c);
			}
		} else {
			if (c == '*') {
				if (!buf->size) {
					*mode = KB_MODE_NORMAL_AUTH;
					result = KB_EEXIT;
				} else {
					result = KB_ECANCEL;
				}
			} else {
				kbuf_add(buf, c);
			}
		}
    }
    return result;
}

void
kb_thread_entry(void *parameters)
{
	rt_err_t result;
    rt_size_t size;
    uint16_t data;
    uint8_t c;
	KB_MODE_TYPEDEF mode = KB_MODE_NORMAL_AUTH;
    rt_device_t device = RT_NULL;
	KB_DATA_TYPEDEF kb_data;
	KB_ERROR_TYPEDEF kb_error;
	uint16_t key_id;
    uint8_t error_detect = 0;

	kb_data_init(&kb_data);

    device_enable(DEVICE_NAME_KEY);
    device = device_enable(DEVICE_NAME_KEYBOARD);

	while (1) {
		result = rt_sem_take(kb_sem, KB_TIMEOUT*RT_TICK_PER_SECOND);
		if (result == RT_EOK) {
            data = 0;
            size = rt_device_read(device, 0, &data, 2);
            if (size == 2) {
                error_detect = 0;
				// filter keyboard input
				if (data != 0x0100) {
					data &= 0xfeff;
                }
				__REV16(data);
                c = char_remap[bit_to_index(data&0x0fff)];
#if (defined RT_USING_FINSH) && (defined KB_DEBUG)
                rt_kprintf("keyboard value is %04X, %c\n", data, c);
#endif
				switch(mode) {
					case KB_MODE_NORMAL_AUTH: {
						kb_error = kb_normal_auth_process(&mode, &kb_data.normal_auth, c, &key_id);
						switch (kb_error) {
							case KB_EOK: {
								break;
							}
							case KB_ECANCEL:
							case KB_EEMPTY: {
								kbuf_init(&kb_data.normal_auth);
								break;
							}
							case KB_EVERIFY_SUCCESS: {
								rt_kprintf("normal verify success!, key_id = %d\n", key_id);
                                kb_data.normal_auth_times = 0;
                                kbuf_init(&kb_data.normal_auth);
								break;
							}
							case KB_EVERIFY_FAILURE: {
								rt_kprintf("normal verify failure, %d\n", kb_data.normal_auth_times);
								if (kb_data.normal_auth_times++ >= KB_VERIFY_TIMES) {
									/* TODO:  alarm */
									kb_data.normal_auth_times = 0;
									rt_kprintf("normal auth alarm\n");
								}
								kbuf_init(&kb_data.normal_auth);
								break;
							}
							case KB_EEXIT: {
                                kbuf_init(&kb_data.normal_auth);
								break;
							}
							default : {
								break;
							}
						}
						break;
					}
                    case KB_MODE_SETTING_AUTH: {
						kb_error = kb_setting_auth_process(&mode, &kb_data.setting_auth, c);
						switch (kb_error) {
							case KB_EOK: {
								break;
							}
							case KB_ECANCEL:
							case KB_EEMPTY: {
								kbuf_init(&kb_data.setting_auth);
								break;
							}
							case KB_EVERIFY_SUCCESS: {
								rt_kprintf("setting verify success!");
                                kb_data.setting_auth_times = 0;
                                kbuf_init(&kb_data.setting_auth);
								break;
							}
							case KB_EVERIFY_FAILURE: {
								rt_kprintf("setting verify failure, %d\n", kb_data.setting_auth_times);
								if (kb_data.setting_auth_times++ >= KB_VERIFY_TIMES) {
									/* TODO:  alarm */
									kb_data.setting_auth_times = 0;
									rt_kprintf("setting auth alarm\n");
								}
								kbuf_init(&kb_data.setting_auth);
								break;
							}
							case KB_EEXIT: {
                                kbuf_init(&kb_data.setting_auth);
								break;
							}
							default : {
								break;
							}
						}
                        break;
                    }
                    case KB_MODE_SETTING: {
						kb_error = kb_setting_process(&mode, &kb_data.setting, c);
						switch (kb_error) {
							case KB_EOK: {
								break;
							}
							case KB_ECANCEL:
							case KB_EEMPTY: {
								kbuf_init(&kb_data.setting);
								break;
							}
							case KB_EVERIFY_SUCCESS: {
                                kbuf_init(&kb_data.setting);
								break;
							}
							case KB_EVERIFY_FAILURE: {
								kbuf_init(&kb_data.setting);
								break;
							}
							case KB_EEXIT: {
                                kbuf_init(&kb_data.setting);
								break;
							}
							default : {
								break;
							}
						}
						break;
                    }
					case KB_MODE_ADD_PASSWORD: {
						kb_error = kb_add_password_process(&mode, &kb_data.password, c);
						switch (kb_error) {
							case KB_EOK: {
								break;
							}
							case KB_ECANCEL: {
								if (kb_data.password.verify_status) {
									kb_data.password.size_verify = 0;
								} else {
									kb_data.password.size = 0;
								}
								break;
							}
							case KB_EVERIFY_SUCCESS: {
								rt_kprintf("add password success : %s\n", kb_data.password.data);
								device_config_key_create(KEY_TYPE_KBOARD, kb_data.password.data);
                                kbuf_init(&kb_data.password);
								break;
							}
							case KB_EVERIFY_FAILURE: {
                                kbuf_init(&kb_data.password);
								rt_kprintf("add password failure!\n");
								break;
							}
							case KB_EEXIT: {
                                kbuf_init(&kb_data.password);
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
			} else {
#if (defined RT_USING_FINSH) && (defined KB_DEBUG)
				rt_kprintf("read key board failure!!!\n");
#endif
                if (error_detect++ > 2)
                {
                    rt_device_control(device, RT_DEVICE_CTRL_CONFIGURE, RT_NULL);
                    error_detect = 0;
                    
                }

			}

		} else { //time out
			mode = KB_MODE_NORMAL_AUTH;
			kb_data_init(&kb_data);
		}
	}
}

__INLINE
void kb_detect(void)
{
    rt_sem_release(kb_sem);
}

int
rt_keyboard_init(void)
{
	rt_thread_t kb_thread;

	//initial keyboard sem
    kb_sem = rt_sem_create("kboard", 0, RT_IPC_FLAG_FIFO);
	if (kb_sem == RT_NULL)
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

FINSH_FUNCTION_EXPORT(kb_detect, kb_detect[]);
#endif // RT_USING_FINSH
