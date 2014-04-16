#ifndef __VOICE_H__
#define __VOICE_H__
#include "rtthread.h"

/*
voice type
*/
typedef enum
{
  VOICE_TYPE_TEST = 0,					//语音测试正常
  VOICE_TYPE_HI,						//您好
  VOICE_TYPE_CCDIR,					//请勿遮挡摄像头，否则系统自动报警
  VOICE_TYPE_ALARM,					//请退后，否则系统自动报警
  VOICE_TYPE_KEY1_ERRPR,		//钥匙错误请重新输入
  VOICE_TYPE_MANAGE1KEY1,		//请输入管理员指纹
  VOICE_TYPE_KEY1_OK,    		//解锁成功
  VOICE_TYPE_KEY1_INPUT, 		//请输入指纹
  VOICE_TYPE_REGISTER_OK, 	//注册成功
  VOICE_TYPE_REGISTER_FIAL, //注册失败
  VOICE_TYPE_KEY1_OUTIME,   //输入超时
  VOICE_TYPE_KEY_FULL, 			//钥匙库已满
}VoiceType;

void send_voice_mail(VoiceType type);

#endif


