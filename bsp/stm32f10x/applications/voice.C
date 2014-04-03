/**
功能:语音播放 
版本:

*/
#include "voice.h"
#include "comm.h"

#include <dfs.h>
#include <dfs_posix.h>


#define VOICE_FILE_NAME1      "/2_16k.WAV"
#define VOICE_FILE_NAME2      "/hi.wav"
#define VOICE_DEVICE_NAME     "PT8211"
#define WAV_PLAY_BUFFER       256

static rt_mq_t  voice_mq = RT_NULL; //语音消息队列

/* Audio file information structure */
typedef struct
{
  rt_uint8_t   RIFF[4];
  rt_uint32_t  FileSize;
  rt_uint8_t   WAVE[4];
  rt_uint8_t   fmt[4];
  rt_uint32_t  reserve;
  rt_uint16_t  FormatTag;
  rt_uint16_t  NumChannels;
  rt_uint16_t  SampleRate;
  rt_uint32_t  ByteRate;
  rt_uint16_t  BlockAlign;
  rt_uint16_t  Bits;
  //rt_uint16_t  reserve1;
  rt_uint8_t   DataFlag[4];
  rt_uint32_t  DataSize;
} WAVE_FormatTypeDef;

typedef struct 
{
  rt_uint16_t *buffer1;   //缓冲区1
  rt_uint16_t *buffer2;   //缓冲区2
  rt_uint16_t playpos;    //缓冲区中数据位置
  rt_uint16_t PlaySize1;  //播放的有效数据大小
  rt_uint16_t PlaySize2;  //播放的有效数据大小
  rt_uint16_t BufSize;    //缓冲区大小
  rt_uint8_t  playbuf;    //当前播放的buf
  rt_uint8_t  ReadyBuf1;  //准备播放的Buff
  rt_uint8_t  ReadyBuf2;  //准备播放的Buff
}VoicePlayData;


static void voice_open_amp(void)
{
	send_ctx_mail(COMM_TYPE_VOICE_AMP,0,200,"\x01",1);	
}

static void voice_close_amp(void)
{
  send_ctx_mail(COMM_TYPE_VOICE_AMP,0,200,"\x00",1);  
}

static rt_err_t wav_data_send(rt_device_t dev, void *buffer)
{
  VoicePlayData *WavDat = (VoicePlayData *)buffer; 
  if(WavDat->playbuf == 1)
  {
    if(WavDat->ReadyBuf2 == 0)
    {
      WavDat->ReadyBuf2 = 1;
    }
    rt_device_write(dev,0,(void *)&WavDat->buffer1[WavDat->playpos++],2);
    if(WavDat->playpos >= WavDat->PlaySize1)
    {
      WavDat->playpos = 0;
      WavDat->playbuf = 2;
       WavDat->ReadyBuf2 = 0;
    }
  }
  else if(WavDat->playbuf == 2)
  {
    if(WavDat->ReadyBuf1 == 0)
    {
      WavDat->ReadyBuf1 = 1;
    }
    rt_device_write(dev,0,(void *)&WavDat->buffer2[WavDat->playpos++],2);
    if(WavDat->playpos >= WavDat->PlaySize2)
    {
      WavDat->playpos = 0;
      WavDat->playbuf = 1;
       WavDat->ReadyBuf1 = 0;
    }
  }
  return RT_EOK;
}
void printf_wavdata(VoicePlayData *WavDat)
{
  rt_kprintf("bufsize = %d\n",WavDat->BufSize);
}
static rt_int8_t create_play_data(VoicePlayData *WavDat)
{ 
  WavDat->BufSize = WAV_PLAY_BUFFER;
  WavDat->playpos = 0;
  WavDat->playbuf = 1;

  WavDat->buffer1 = (rt_uint16_t *)rt_calloc(1,WavDat->BufSize*2);
  RT_ASSERT(WavDat->buffer1 != RT_NULL);
 
  WavDat->buffer2 = (rt_uint16_t *)rt_calloc(1,WavDat->BufSize*2);
  RT_ASSERT(WavDat->buffer2 != RT_NULL);
  printf_wavdata(WavDat);
  
  return 0;
}
static rt_int8_t delete_play_data(VoicePlayData *WavDat)
{
  rt_free(WavDat->buffer1);
  rt_free(WavDat->buffer2);

  return 0;
}
static void voice_file_process(const char *file)
{
  int FileID;
  WAVE_FormatTypeDef wav;
  rt_device_t dev;
  VoicePlayData WavDat = {0,};

	rt_kprintf("VoicePlayData      = %d\n",sizeof(VoicePlayData));
	rt_kprintf("WAVE_FormatTypeDef = %d\n",sizeof(WAVE_FormatTypeDef));
  //初始化缓冲区控制块
  if(create_play_data(&WavDat) < 0)
  {
    rt_kprintf("create wav play data fail\n");
    return ;
  }
  printf_wavdata(&WavDat);
  FileID = open(file,O_RDONLY,0x777);
  if(FileID < 0)
  {
    rt_kprintf("can`t open %s file\n",file);
    delete_play_data(&WavDat);
    return ;
  }
 	read(FileID,(void *)&wav,sizeof(WAVE_FormatTypeDef));

  //处理wav音频数据
  dev = rt_device_find(VOICE_DEVICE_NAME);
  dev->tx_complete = wav_data_send;
  dev->user_data = (void*)&WavDat;
  printf_wavdata(&WavDat);

  WavDat.PlaySize1 = read(FileID,(void *)WavDat.buffer1,WavDat.BufSize*2);
  if(WavDat.PlaySize1 == WavDat.BufSize*2)
  {
    WavDat.playbuf = 1;
    WavDat.PlaySize1 /= 2;
    if(!(dev->flag & RT_DEVICE_OFLAG_OPEN))
    {
      rt_device_open(dev,RT_DEVICE_OFLAG_OPEN);
    }
  }
  else
  {
    rt_kprintf("wav read 1 buffer is fail\n");
  }
  
  while(1)
  {
    if(WavDat.ReadyBuf1 == 1)
    {
       WavDat.PlaySize1 = read(FileID,(void *)WavDat.buffer1,WavDat.BufSize*2);
       if(WavDat.PlaySize1 != WavDat.BufSize*2)
       {
          rt_device_close(dev);
          break;
       }
       WavDat.PlaySize1 /= 2;
       WavDat.ReadyBuf1 = 2;
    }
    else if(WavDat.ReadyBuf2 == 1)
    {
      WavDat.PlaySize2 = read(FileID,(void *)WavDat.buffer2,WavDat.BufSize*2);
      if(WavDat.PlaySize2 != WavDat.BufSize*2)
      {
        rt_device_close(dev);
        break;
      }
      WavDat.PlaySize2 /= 2;
      WavDat.ReadyBuf2 = 2;
    }
  }
  
  rt_kprintf("RIFFchunksize %c%c%c%c\n",wav.RIFF[0],wav.RIFF[1],wav.RIFF[2],wav.RIFF[3]);
  rt_kprintf("FileSize      %d\n",wav.FileSize);
  rt_kprintf("RIFFchunksize %c%c%c%c\n",wav.WAVE[0],wav.WAVE[1],wav.WAVE[2],wav.WAVE[3]);
  rt_kprintf("RIFFchunksize %c%c%c%c\n",wav.fmt[0],wav.fmt[1],wav.fmt[2],wav.fmt[3]);
  rt_kprintf("reserve       %s\n",wav.reserve);
  rt_kprintf("FormatTag     %d\n",wav.FormatTag);
  rt_kprintf("NumChannels   %d\n",wav.NumChannels);
  rt_kprintf("SampleRate    %d\n",wav.SampleRate);
  rt_kprintf("ByteRate      %d\n",wav.ByteRate);
  rt_kprintf("BlockAlign    %d\n",wav.BlockAlign);
  rt_kprintf("Bits          %d\n",wav.Bits);
  rt_kprintf("DataFlag      %c%c%c%c\n",wav.DataFlag[0],wav.DataFlag[1],wav.DataFlag[2],wav.DataFlag[3]);
  rt_kprintf("DataSize      %d\n",wav.DataSize);

  //释放内存资源
  close(FileID);
  delete_play_data(&WavDat);
}

static voice_recv_mail_process(void)
{
  VoiceType type;
  rt_err_t  RecvResult;
  
  RecvResult = rt_mq_recv(voice_mq,&type,sizeof(VoiceType),RT_WAITING_FOREVER);
  if(RecvResult == RT_EOK)
  {
    voice_open_amp();
    switch(type)
    {
      case VOICE_TYPE_TEST:
      {
        voice_file_process(VOICE_FILE_NAME1);
        break;
      }
      default:
      {
        break;
      }
    }
    voice_close_amp();
  }
}
void voice_thread_entry(void *arg)
{
  RT_ASSERT(voice_mq);
  while(1)
  {
    voice_recv_mail_process();
    rt_thread_delay(1);
  }
}

extern void send_voice_mail(VoiceType type);
int voice_thread_init(void)
{
	rt_thread_t thread_id;

	voice_mq = rt_mq_create("voice",sizeof(VoiceType),10,RT_IPC_FLAG_FIFO);
  RT_ASSERT(voice_mq);
	
  thread_id = rt_thread_create("voice",
		                           voice_thread_entry, 
		                           RT_NULL,1024, 21, 20);//优先级不能太高
  if(thread_id != RT_NULL)
  {
    rt_thread_startup(thread_id);
  }
	else
	{
		rt_kprintf("voice thread create fail\n");
	}
  
	return 0;
}
INIT_APP_EXPORT(voice_thread_init);

/*
功能:发送语音播放邮件
参数:type  语音播放类型
*/
void send_voice_mail(VoiceType type)
{
  if(voice_mq != RT_NULL)
  {
    rt_mq_send(voice_mq,(void *)&type,sizeof(VoiceType));
  }
  else
  {
    rt_kprintf("Now voice_mq is NULL\n");
  }
}

#ifdef RT_USING_FINSH
#include <finsh.h>

void voice(void)
{
  send_voice_mail(VOICE_TYPE_TEST);
}
FINSH_FUNCTION_EXPORT(voice,send voice cmd);


#endif

