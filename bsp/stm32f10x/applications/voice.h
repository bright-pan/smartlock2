#ifndef __VOICE_H__
#define __VOICE_H__
#include "rtthread.h"

/*
voice type
*/
typedef enum
{
  VOICE_TYPE_TEST = 0,					//������������
  VOICE_TYPE_HI,						//����
  VOICE_TYPE_CCDIR,					//�����ڵ�����ͷ������ϵͳ�Զ�����
  VOICE_TYPE_ALARM,					//���˺󣬷���ϵͳ�Զ�����
  VOICE_TYPE_KEY1_ERRPR,		//Կ�״�������������
  VOICE_TYPE_MANAGE1KEY1,		//���������Աָ��
  VOICE_TYPE_KEY1_OK,    		//�����ɹ�
  VOICE_TYPE_KEY1_INPUT, 		//������ָ��
  VOICE_TYPE_REGISTER_OK, 	//ע��ɹ�
  VOICE_TYPE_REGISTER_FIAL, //ע��ʧ��
  VOICE_TYPE_KEY1_OUTIME,   //���볬ʱ
  VOICE_TYPE_KEY_FULL, 			//Կ�׿�����
}VoiceType;

void send_voice_mail(VoiceType type);

#endif


