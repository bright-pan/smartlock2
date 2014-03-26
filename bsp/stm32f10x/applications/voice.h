#ifndef __VOICE_H__
#define __VOICE_H__
#include "rtthread.h"

/*
voice type
*/
typedef enum
{
  VOICE_TYPE_TEST,
}VoiceType;

void send_voice_mail(VoiceType type);

#endif


