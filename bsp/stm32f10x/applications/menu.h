#ifndef __MENU_H__
#define __MENU_H__
#include "rtthread.h"
//#include "commfun.h"
#include "gui.h"
#define KEY_MAX_MENU_NUM					50

typedef struct 
{
	rt_uint8_t StateIndex;					//��ǰ״̬������
	rt_uint8_t DnState;							//���¡����¡���ʱת���״̬������
	rt_uint8_t UpState;							//���¡����ϡ���ʱת���״̬������
	rt_uint8_t SureState;						//���¡��س�����ʱת���״̬������
	rt_uint8_t BackState;						//���¡��˻ء���ʱת���״̬������
	void(*CurrentOperate)(void);		//��ǰ״̬Ӧ��ִ�е��ܲ���
}KbdTabStruct;

extern rt_uint8_t KeyFuncIndex;
extern KbdTabStruct	KeyTab[KEY_MAX_MENU_NUM];

void key_input_processing(void);

rt_err_t menu_key_value_acquire(rt_uint8_t *KeyValue);

#endif
