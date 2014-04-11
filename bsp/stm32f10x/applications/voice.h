#ifndef __VOICE_H__
#define __VOICE_H__
#include "rtthread.h"

/*
voice type
*/
typedef enum
{
  VOICE_TYPE_TEST,
  VOICE_TYPE_HI,
  VOICE_TYPE_CCDIR,
  VOICE_TYPE_ALARM,
  VOICE_TYPE_KEY1_ERRPR,
  VOICE_TYPE_MANAGE1KEY1,
  VOICE_TYPE_KEY1_OK
}VoiceType;

void send_voice_mail(VoiceType type);

#endif


