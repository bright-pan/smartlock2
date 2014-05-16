/**
功能:摄像头拍照 红外补光
版本:v0.2
*/
#include "camera.h"
#include "usart.h"
#include "untils.h"
#include "gpio_adc.h"
#include "gpio_pwm.h"
#include "netfile.h"
#include "gprs.h"
#include "appconfig.h"

#define CAMERA_BUF_SIZE       	 512    //缓冲区大小
#define PIC_DATA_MAX_SIZE			 	 70000	//70K
#define PIC_DATA_MIN_SIZE				 10000	//10K
#define CM_LIGHT_LED_DELAY       100    //光敏采用样次数
#define LIGHT_SAMPLING_CNT		   100    //light sensor sampling number
#define LIGHT_LED_JOB_TIME       50000  //补光灯最长工作时间
#define LDR_MAX_VALUE            1000   //正常环境下的最亮时光照adc值

//打印信息
#define CM_DAT_DEAL_INFO         0   //printf data info
#define CM_POWER_INFO            1   //printf power info
#define CM_LIGHT_INFO            1   //printf LIGHT info

#define CM_LIGHTLED_OPEN         1
#define CM_LIGHTLED_CLOSE        !CM_LIGHTLED_OPEN

#define CM_LDR_IS_LIGTH          0
#define CM_LDR_IS_DARK           !CM_LDR_IS_LIGTH

#define CM_POWER_OPEN            1
#define CM_POWER_CLOSE           !CM_POWER_OPEN

#define DEVICE_NAME_CAMERA_UART  "uart4"
#define DEVICE_NAME_CAMERA_LHLED "cm_led"
#define DEVICE_NAME_CAMERA_POWER "cm_power"
#define DEVICE_NAME_CAMERA_LDR   "cm_light"

static rt_mq_t CameraMail_mq = RT_NULL;
static rt_sem_t Camera_sem = RT_NULL;

typedef enum 
{
	CM_NORMAL = 0,
	CM_HW_FAULT_ERR,
	CM_GET_SIZE_ERR,
	CM_GET_PACK_ERR,
	CM_GET_DATA_ERR
}CameraError;

//camera data deal 
typedef struct
{
	rt_device_t Uart;   
	rt_device_t Power; 
	rt_device_t LDR;    //light dependent resistor
	rt_device_t LightLED;   
	rt_uint8_t  data[CAMERA_BUF_SIZE];
	rt_size_t   PicSize;
	CameraError Error;
}CameraObj,*CameraObj_p;

//vc0706 cmd
//static const rt_uint8_t CM_Reset[] =	{0x56,0x00,0x26,0x00};
static const rt_uint8_t CM_WitchFrame[] = {0x56,0x00,0x36,0x01,0x03};
static const rt_uint8_t CM_UpdataFrame[] = {0x56,0x00,0x36,0x01,0x02};
static const rt_uint8_t CM_StopCurFrame[] =	{0x56,0x00,0x36,0x01,0x00};
static const rt_uint8_t CM_StopNextFrame[] = {0x56,0x00,0x36,0x01,0x01};
static const rt_uint8_t CM_GetCurFrameLen[] = {0x56,0x00,0x34,0x01,0x00};
//static const rt_uint8_t CM_GetNextFrameLen[] = {0x56,0x00,0x34,0x01,0x01};
rt_uint8_t CM_RecvOk[] = {0x76,0x00,0x32,0x00,0x00};
rt_uint8_t CM_ReadBuf[16] = {0x56,0x00,0x32,0x0C,0x00,0x0a,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0xff,0x00,0x00};


/*void get_pic_file_name(char *name)
{
	rt_sprintf(name,"/%d.jpg",Camera_sem->value);
}*/

/** 
@brief  图片文件信号量操作
@param  RT_TRUE :释放信号量
@retval RT_EOK  :超时/错误
*/
rt_err_t pic_file_sem_operate(rt_bool_t arg)
{
	rt_err_t result;
	
	if(arg == RT_TRUE)
	{
		result = rt_sem_take(Camera_sem,100);
	}
	else
	{
		result = rt_sem_release(Camera_sem);
	}

	return result;
}

void printf_data(rt_uint8_t data[],rt_size_t size)
{
	rt_size_t i;

	rt_kprintf("data:\n");
	for(i = 0; i < size;i++)
	{
		rt_kprintf("%02X",data[i]);
	}
	rt_kprintf("\n");
}
 
/** 
@brief  摄像头电源控制
@param  camera :摄像头对象
@param  status :新状态
@retval void
*/
static void camera_power_control(CameraObj_p camera,rt_uint8_t status)
{
	if(status == CM_POWER_OPEN)
	{
    rt_device_control(camera->Uart,RT_DEVICE_CTRL_SET_TX_GPIO,RT_NULL);
    RT_DEBUG_LOG(CM_POWER_INFO,("open camrea modlue \n"));
	}
	else
	{
    rt_device_control(camera->Uart,RT_DEVICE_CTRL_CLR_TX_GPIO,RT_NULL);
    RT_DEBUG_LOG(CM_POWER_INFO,("close camrea modlue \n"));
	}
	
	rt_device_write(camera->Power,0,&status,1);
}

/** 
@brief  获取ADC设备的adc值
@param  dev :ADC设备
@retval ADC的值
*/
static rt_uint16_t get_adc_devce_value(rt_device_t dev)
{
	rt_uint16_t ADC_Value;
	
  if(!(dev->open_flag & RT_DEVICE_OFLAG_OPEN))
  {
		rt_device_open(dev,RT_DEVICE_OFLAG_RDWR);
  }
  rt_device_control(dev, RT_DEVICE_CTRL_ENABLE_CONVERT,(void*)0);
  delay_us(120);
  rt_device_control(dev, RT_DEVICE_CTRL_GET_CONVERT_VALUE, (void *)&ADC_Value);
  rt_device_control(dev, RT_DEVICE_CTRL_DISABLE_CONVERT,(void*)0);

	//rt_kprintf("ADC_Value = %d\n",ADC_Value);
  return ADC_Value;
}

/** 
@brief  补光灯调光
@param  camera :摄像头对象
@param  value  :发光强度
@retval void
*/
static void camera_light_led_work(CameraObj_p camera,rt_uint16_t value)
{
	rt_uint16_t counts;
	rt_uint16_t pwmvalue;
	float       intensity;
	

	intensity = (float)value;
	intensity /= LDR_MAX_VALUE;
	if(CM_LIGHT_INFO)
	{
		char  *str;

		str = rt_calloc(1,20);
		RT_ASSERT(str != RT_NULL);
    sprintf(str,"%f",intensity);
    rt_kprintf("Light Intensity:%s ",str);
    rt_free(str);
	}
	counts = LIGHT_LED_JOB_TIME*(1-intensity);
	RT_DEBUG_LOG(CM_LIGHT_INFO,("PWM Counts:%d ",counts));
	if(intensity > 0.7)
	{
		counts = 10;
		pwmvalue = 10;
	}
	else
	{
    pwmvalue = 1800 * (1-intensity);
	}
	RT_DEBUG_LOG(CM_LIGHT_INFO,("PWM Duty Cycle:%d\n",pwmvalue));
	rt_device_control(camera->LightLED, RT_DEVICE_CTRL_SET_PULSE_VALUE,(void*)&pwmvalue);
	rt_device_control(camera->LightLED, RT_DEVICE_CTRL_SET_PULSE_COUNTS, (void *)&counts);
	rt_device_control(camera->LightLED, RT_DEVICE_CTRL_SEND_PULSE, (void *)0);
}

/** 
@brief  根据光敏控制补光灯
@param  camera :摄像头对象
@param  status :新状态
@retval void
*/
static void camera_light_control(CameraObj_p camera,rt_uint8_t status)
{
	rt_uint8_t sample_cnt = LIGHT_SAMPLING_CNT;
	rt_size_t sum = 0;

	RT_ASSERT(camera != RT_NULL);
	//supplementary lighting request 
	if(CM_LIGHTLED_OPEN== status)
	{
		while(sample_cnt--)
		{
      sum += get_adc_devce_value(camera->LDR);
		}
		sum /= LIGHT_SAMPLING_CNT;
		camera_light_led_work(camera,sum);
		rt_kprintf("LDRValue = %d\n",sum);
	}
	else if(CM_LIGHTLED_CLOSE == status)
	{
		rt_device_write(camera->LightLED,0,&status,1);
		rt_kprintf("close camera led \n");
	}
}

/** 
@brief  复位摄像头
@param  camera :摄像头对象
@retval void
*/
/*static void camera_reset(CameraObj_p camera)
{
	rt_device_write(camera->Uart,0,CM_Reset,sizeof(CM_Reset));	
	rt_thread_delay(1);
	rt_device_read(camera->Uart,0,camera->data,CAMERA_BUF_SIZE);
}
*/

/** 
@brief  更新帧
@param  camera :摄像头对象
@retval void
*/
void camera_update_frame(CameraObj_p camera)
{
	rt_size_t length;
	
	rt_device_write(camera->Uart,0,CM_UpdataFrame,sizeof(CM_UpdataFrame));	
	
	rt_thread_delay(1);
	
	length = rt_device_read(camera->Uart,0,camera->data,CAMERA_BUF_SIZE);
	printf_data(camera->data,length);
}

/** 
@brief  停止当前帧
@param  camera :摄像头对象
@retval void
*/
void camera_stop_cur_frame(CameraObj_p camera)
{
	rt_size_t length;
	
	rt_device_write(camera->Uart,0,CM_StopCurFrame,sizeof(CM_StopCurFrame));	
	
	rt_thread_delay(1);
	
	length = rt_device_read(camera->Uart,0,camera->data,CAMERA_BUF_SIZE);
	printf_data(camera->data,length);
}

/** 
@brief  停止下一帧
@param  camera :摄像头对象
@retval void
*/
void camera_stop_next_frame(CameraObj_p camera)
{
	rt_size_t length;
	
	rt_device_write(camera->Uart,0,CM_StopNextFrame,sizeof(CM_StopNextFrame));	
	
	rt_thread_delay(1);
	
	length = rt_device_read(camera->Uart,0,camera->data,CAMERA_BUF_SIZE);
	printf_data(camera->data,length);
}

/** 
@brief  切换帧
@param  camera :摄像头对象
@retval void
*/
void camera_switch_frame(CameraObj_p camera)
{
	rt_size_t length;
	
	rt_device_write(camera->Uart,0,CM_WitchFrame,sizeof(CM_WitchFrame));	
	
	rt_thread_delay(1);
	
	length = rt_device_read(camera->Uart,0,camera->data,CAMERA_BUF_SIZE);
	printf_data(camera->data,length);
}

/** 
@brief  获取照片大小
@param  camera :摄像头对象
@retval -1 :获取数据错误
@retval 0  :获取成功
*/
rt_int8_t camera_get_cur_size(CameraObj_p camera)
{
	volatile rt_size_t length;
	rt_uint8_t max_cnt = 0;
	
	do
	{
		rt_device_write(camera->Uart,0,CM_GetCurFrameLen,sizeof(CM_GetCurFrameLen)); 
		
		rt_thread_delay(10);
		
		length = rt_device_read(camera->Uart,0,camera->data,CAMERA_BUF_SIZE);
		
		/*    calculate receive size	*/
		camera->PicSize= ((*(camera->data+5)) << 24) |((*(camera->data+6))  << 16) |\
		((*(camera->data+7))  << 8) |((*(camera->data+8)) ); 
		
		max_cnt++;
		if(max_cnt>10)
		{
			camera->Error|= CM_GET_SIZE_ERR;
			return -1;
		}
		rt_kprintf("size %d max_cnt = %d\n",camera->PicSize,max_cnt);
	}
	while((camera->PicSize >PIC_DATA_MAX_SIZE) ||(camera->PicSize < PIC_DATA_MIN_SIZE));

	length = rt_device_read(camera->Uart,0,camera->data,CAMERA_BUF_SIZE);
	
	return 0;
}

/** 
@brief  将32位数据转换到数据组
@param  array :数据地址
@param  data  :要转换的数据
@retval void
*/
static void uint32_to_array(rt_uint8_t *array,rt_uint32_t data)
{
	*array = (data>>24) & 0x000000ff;	
	array++;
	*array = (data>>16) & 0x000000ff;
	array++;
	*array = (data>>8) & 0x000000ff;
	array++;
	*array = (data>>0) & 0x000000ff;
}

/** 
@brief  从缓冲区中获得数据
@param  camera :摄像头对象
@param  pack :包序号
@param  PackSize :包大小
@retval void
*/
void camera_get_cur_pack(CameraObj_p camera,rt_size_t pack,rt_size_t PackSize)
{
	uint32_to_array(&CM_ReadBuf[10],PackSize);
	
	uint32_to_array(&CM_ReadBuf[6],(pack - 1) * CAMERA_BUF_SIZE);
	CM_ReadBuf[4] = 0;
	RT_DEBUG_LOG(CM_DAT_DEAL_INFO,("Get Pack %d\n",pack));
	//need delay 1-2ms
	if(CM_DAT_DEAL_INFO == 0)
	{
    delay_us(1000);
	}
	rt_device_write(camera->Uart,0,CM_ReadBuf,sizeof(CM_ReadBuf));
}

/** 
@brief  从串口中读取数据
@param  dev :串口设备
@param  data :保存的地址
@param  size :读取数据大小
@retval -1 :超时
@retval  0 :成功
*/
rt_int8_t uart_buf_read_data(rt_device_t dev,rt_uint8_t *data,rt_size_t size)
{
	volatile rt_size_t ReadResult = 0;
	rt_uint8_t *buf;
	rt_size_t  outtime = 0;

	buf = data;
	while(size)
	{
		ReadResult = rt_device_read(dev,0,buf,size);
		if(ReadResult > 0)
		{
			buf += ReadResult;
			size -= ReadResult;
		}
		else
		{
			outtime++;
			if(outtime > 0x1ffff)
			{
				rt_kprintf("read data outime size = %d\n",size);
				return -1;
			}
		}
	}
	//rt_kprintf("cur outime %d\n",outtime);
	return 0;
}

/** 
@brief  把缓冲区中的数据保存到文件
@param  FileID :文件索引
@param  camera :摄像头对象
@retval 0 :成功
@retval -1:失败
*/
rt_int8_t camera_get_buf_data(int FileID,CameraObj_p camera)
{
	rt_size_t PackNum;
	rt_size_t LastPackSize;
	rt_size_t CurPack;
	rt_size_t ReadSize;
	rt_int8_t UartResult;
	rt_uint8_t OutTime = 0;

	PackNum = camera->PicSize / CAMERA_BUF_SIZE;
	LastPackSize = camera->PicSize % CAMERA_BUF_SIZE;
	if(LastPackSize != 0)
	{
		PackNum++;
	}
	rt_kprintf("PackNum = %d\n",PackNum);
	rt_kprintf("LastPackSize = %d\n",LastPackSize);
	CurPack = 1;
	camera_get_cur_pack(camera,CurPack,CAMERA_BUF_SIZE);
	while(CurPack <= PackNum)
	{
		if(CurPack == PackNum)
		{
			ReadSize = LastPackSize;
		}
		else
		{
			ReadSize = CAMERA_BUF_SIZE;
		}
		if(OutTime > 10)
		{
			camera->Error = CM_GET_PACK_ERR;
			return -1;
		}
		UartResult = uart_buf_read_data(camera->Uart,camera->data,5);
		if(UartResult < 0)
		{
			rt_kprintf("CurPack = %d\n",CurPack);
			
			camera_get_cur_pack(camera,CurPack,ReadSize);
			OutTime++;
			continue;
		}
		UartResult = uart_buf_read_data(camera->Uart,camera->data,ReadSize);
		if(UartResult < 0)
		{
			rt_kprintf("CurPack = %d\n",CurPack);
			camera_get_cur_pack(camera,CurPack,ReadSize);
			OutTime++;
			continue;
		}
		CurPack++;
		if(CurPack <= PackNum)
		{
      camera_get_cur_pack(camera,CurPack,ReadSize);
		}
		//rt_kprintf("write %d pack\n",CurPack-1);
		write(FileID,camera->data,ReadSize);
		uart_buf_read_data(camera->Uart,camera->data,5);
	}
	rt_kprintf("write ok\n");

	return 0;
}

/** 
@brief 创建摄像头所获取的资源
@param  camera :摄像头对象
@retval 对象地址
*/
static CameraObj_p camera_obj_create(void)
{
	CameraObj_p camera = RT_NULL;

	camera = (CameraObj_p)rt_calloc(1,sizeof(CameraObj));
	RT_ASSERT(camera != RT_NULL);
	
	//usart
	camera->Uart = rt_device_find(DEVICE_NAME_CAMERA_UART);
	RT_ASSERT(camera->Uart != RT_NULL);
	if(!(camera->Uart->open_flag & RT_DEVICE_OFLAG_OPEN))
	{
		rt_kprintf("open %s device\n",camera->Uart->parent.name);
		rt_device_open(camera->Uart,RT_DEVICE_OFLAG_RDWR);
	}
	
	//Fill Light
	camera->LightLED = rt_device_find(DEVICE_NAME_CAMERA_LHLED);
	RT_ASSERT(camera->LightLED != RT_NULL);
	if(!(camera->LightLED->open_flag & RT_DEVICE_OFLAG_OPEN))
	{
		rt_kprintf("open %s device\n",camera->LightLED->parent.name);
		rt_device_open(camera->LightLED,RT_DEVICE_OFLAG_RDWR);
	}

	//power
	camera->Power = rt_device_find(DEVICE_NAME_CAMERA_POWER);
	RT_ASSERT(camera->Power != RT_NULL);
	if(!(camera->Power->open_flag & RT_DEVICE_OFLAG_OPEN))
	{
		rt_kprintf("open %s device\n",camera->Power->parent.name);
		rt_device_open(camera->Power,RT_DEVICE_OFLAG_RDWR);
	}

	//light dependent resistor
	camera->LDR= rt_device_find(DEVICE_NAME_CAMERA_LDR);
	RT_ASSERT(camera->LDR != RT_NULL);
	if(!(camera->LDR->open_flag & RT_DEVICE_OFLAG_OPEN))
	{
		rt_kprintf("open %s device\n",camera->LDR->parent.name);
		rt_device_open(camera->LDR,RT_DEVICE_OFLAG_RDWR);
	}
	camera->PicSize = 0;
	camera->Error = CM_NORMAL;

	return camera;
}

/** 
@brief  删除摄像头所获取的资源
@param  camera :摄像头对象
@retval void
*/
void camera_obj_delete(CameraObj_p camera)
{
	rt_free(camera);
}

void camera_timer(void *parameter)
{
	rt_uint32_t *UseTime = (rt_uint32_t *)parameter;

	(*UseTime)++;
}

/** 
@brief  摄像头数据处理
@param  camera :摄像头参数结构
@param  mail :报警邮件参数结构
@param  FileName :目标文件名字
@retval void
*/
void camera_data_process(CameraObj_p camera,CameraMail_p mail,char *FileName)
{
	int FileID;
	rt_timer_t timer;
	rt_uint32_t UseTime = 0;

	timer = rt_timer_create("camera",
													camera_timer,
													(void *)&UseTime,
													10,
													RT_TIMER_FLAG_PERIODIC);
	rt_timer_start(timer);
	camera_update_frame(camera);
	camera_stop_cur_frame(camera);
	if(camera_get_cur_size(camera) == -1)
	{
		return ;
	}

	unlink((const char*)FileName);
	FileID = open((const char*)FileName,O_CREAT | O_RDWR, 0x777);
	if(FileID < 0)
	{
		rt_kprintf("File Create Fail\n");
	}
	camera_get_buf_data(FileID,camera);
	close(FileID);

	rt_kprintf("UseTime = %d\n",UseTime);
	rt_timer_stop(timer);
	rt_timer_delete(timer);
}

/** 
@brief  获得报警事件在报文中对应的类型
@param  alarm :报警类型
@retval 报文中的数据
*/
rt_uint8_t get_file_alarm_type(ALARM_TYPEDEF alarm)
{
	rt_uint8_t result = 0;

	switch(alarm)
	{
		case ALARM_TYPE_CAMERA_IRDASENSOR:
		{
			result = 0;
			break;
		}
		case ALARM_TYPE_RFID_KEY_ERROR:
		{
			result = 2;
			break;
		}
		case ALARM_TYPE_GPRS_CAMERA_OP:
		{
			result = 11; 
			break;
		}
		default:
		{
			break;
		}
	}

	return result;
}

/** 
@brief  摄像头处理线程
@param  arg :私有数据
@retval void
*/
void camera_thread_entry(void *arg)
{
	rt_err_t    result;
	CameraMail  CameraMail;
	CameraObj_p CameraDat = RT_NULL;
	UploadFile_p FileInfo;

	CameraDat = camera_obj_create();
	camera_power_control(CameraDat,CM_POWER_CLOSE);
	camera_obj_delete(CameraDat);
	while(1)
	{
		result =  rt_mq_recv(CameraMail_mq,&CameraMail,sizeof(CameraMail),24*360000);

		result = pic_file_sem_operate(RT_TRUE);		
		if(RT_EOK == result)		
		{
			CameraDat = camera_obj_create();
			camera_power_control(CameraDat,CM_POWER_OPEN);
			camera_light_control(CameraDat,CM_LIGHTLED_OPEN);
			rt_thread_delay(50);
			rt_device_read(CameraDat->Uart,0,CameraDat->data,CAMERA_BUF_SIZE);
			
			camera_data_process(CameraDat,&CameraMail,CM_MAKE_PIC_NAME);
			
			camera_light_control(CameraDat,CM_LIGHTLED_CLOSE);
			camera_power_control(CameraDat,CM_POWER_CLOSE);

			//发送报警信息
			FileInfo = rt_calloc(1,sizeof(*FileInfo));
			RT_ASSERT(FileInfo != RT_NULL);
			rt_memcpy(FileInfo->name,CM_MAKE_PIC_NAME,rt_strlen(CM_MAKE_PIC_NAME));
			FileInfo->AlarmType = get_file_alarm_type(CameraMail.AlarmType);//报警类型有待有修改
			FileInfo->FileType = 1;
			FileInfo->time = CameraMail.time;
			send_gprs_mail(ALARM_TYPE_GPRS_UPLOAD_PIC,CameraMail.time,FileInfo);
			
			camera_obj_delete(CameraDat);
		}
	}
}

void mq(rt_uint32_t time);

/** 
@brief  本次图片文件处理完成
@param  user :私有数据
@retval void
*/
static void pic_file_process_complete(void *user)
{
	rt_int8_t SendResult;

	SendResult = *(rt_int8_t *)user;

	if(SendResult == -1)
	{
		rt_kprintf("Picture send failure\n");
	}
	else
	{
		rt_kprintf("Picture send succeed\n");
	}
	pic_file_sem_operate(RT_FALSE);
	{
		//mq(10);
	}
	
}

/** 
@brief  初始化摄像头任务
@param  void
@retval 0	:初始化成功
@retval 1 :初始化失败
*/
int camera_thread_init(void)
{
	rt_thread_t id;
	
  net_upload_complete_Callback(pic_file_process_complete);

	//camrea in anytime only make a photo
	CameraMail_mq = rt_mq_create("CMMail",
																sizeof(CameraMail),
																3,
																RT_IPC_FLAG_FIFO);
	RT_ASSERT(CameraMail_mq != RT_NULL);

	Camera_sem = rt_sem_create("picfile",1,RT_IPC_FLAG_FIFO);
	RT_ASSERT(Camera_sem != RT_NULL);
	
	id = rt_thread_create("Camera",
												camera_thread_entry,
												RT_NULL,
												1024,
												104,
												100);
	if(RT_NULL == id )
	{
		rt_kprintf("camera thread create fail\n");
		return 1;
	}
	rt_thread_startup(id);
	return 0;
}
INIT_APP_EXPORT(camera_thread_init);

/** 
@brief  摄像头对外邮件接口
@param  alarm_type :报警事件
@param  time :报警事件
@retval void
*/
void camera_send_mail(ALARM_TYPEDEF alarm_type,rt_uint32_t time)
{
	CameraMail mail;
	
#ifdef PIC_CMAERA_MAKE
	if(PIC_CMAERA_MAKE == 0)
	{
		rt_kprintf("System camera function is close\n");
		return ;
	}
#endif

	if(CameraMail_mq != RT_NULL)
	{
		mail.AlarmType = alarm_type;
		mail.time = time;
		rt_mq_send(CameraMail_mq,&mail,sizeof(CameraMail));
	}
}



#ifdef RT_USING_FINSH
#include <finsh.h>

void mq(rt_uint32_t time)//(rt_uint8_t time,rt_uint8_t *file_name)
{
  camera_send_mail(ALARM_TYPE_CAMERA_IRDASENSOR,0);
}

FINSH_FUNCTION_EXPORT(mq, mq(time,name));

#endif

