#ifndef __NETPROTOCOL_H__
#define __NETPROTOCOL_H__
#include "rtthread.h"
#include "rthw.h"
#include "netconfig.h"

#include "netcomm.h"

#ifdef RT_USING_DFS
#include <dfs_fs.h>
#include <dfs.h>
#include <dfs_posix.h>
#endif

#define NET_WND_MAX_NUM             16  //窗口大小
#define NET_FILE_BUF_SIZE           512 //文件buffer


#define NET_MESSAGE_STERN       0XAA55
#define NET_KEY1_LEN            8
#define NET_DEVICE_ID_LEN       8
#define NET_CHECK_LEN           2

#define NET_RECV_MSG_MAX        5

//报文命令
typedef enum
{
  NET_MSGTYPE_LANDED        = 0x00, //登陆
  NET_MSGTYPE_LANDED_ACK    = 0X80, //登陆应答
  NET_MSGTYPE_HEART         = 0x01, //心跳
  NET_MSGTYPE_HEART_ACK     = 0X81, //心跳应答
  NET_MSGTYPE_ALARM         = 0x02, //工作报警
  NET_MSGTYPE_ALARM_ACK     = 0X82, //工作报警应答
  NET_MSGTYPE_FAULT         = 0x03, //故障
  NET_MSGTYPE_FAULT_ACK     = 0X83, //故障应答
  NET_MSGTYPE_OPENDOOR      = 0x04, //开门
  NET_MSGTYPE_OPENDOOR_ACK  = 0X84, //开门应答
  NET_MSGTYPE_BATTERY       = 0x05, //电池
  NET_MSGTYPE_BATTERY_ACK   = 0x85, //电池应答
  NET_MSGTYPE_FILEREQUEST   = 0x06, //文件请求
  NET_MSGTYPE_FILEREQUE_ACK = 0X86, //文件请求应答
  NET_MSGTYPE_FILEDATA      = 0x07, //文件数据
  NET_MSGTYPE_FILEDATA_ACK  = 0X87, //文件数据应答
  NET_MSGTYPE_PHONEADD      = 0x0b, //添加电话
  NET_MSGTYPE_PHONEADD_ACK  = 0X8B, //添加电话应答
  NET_MSGTYPE_PHONEDELETE   = 0x0c, //删除电话
  NET_MSGTYPE_PHONEDEL_ACK = 0X8C, //删除电话应答信息
  NET_MSGTYPE_ALARMARG      = 0x0d, //报警参数
  NET_MSGTYPE_ALARMARG_ACK  = 0X8d, //报警参数应答
  NET_MSGTYPE_LINK          = 0x0e, //休眠
  NET_MSGTYPE_LINK_ACK      = 0X8E, //休眠应答
  NET_MSGTYPE_KEYADD        = 0x10, //添加钥匙
  NET_MSGTYPE_KEYADD_ACK    = 0X90, //添加钥匙应答
  NET_MSGTYPE_KEYDELETE     = 0x12, //删除钥匙
  NET_MSGTYPE_KEYDEL_ACK    = 0X92, //删除钥匙应答
  NET_MSGTYPE_UPDATE        = 0x13, //文件更新
  NET_MSGTYPE_UPDATE_ACK    = 0X93, //文件更新应答
  NET_MSGTYPE_TIME          = 0x14, //时间同步
  NET_MSGTYPE_TIME_ACK      = 0X94, //时间同步应答
  NET_MSGTYPE_SETK0         = 0x15, //设置K0
  NET_MSGTYPE_SETK0_ACK     = 0X95, //设置K0应答
  NET_MSGTYPE_HTTPUPDATE    = 0x16, //HTTP更新
  NET_MSGTYPE_HTTPUPDAT_ACK = 0X96, //HTTP更新应答
  NET_MSGTYPE_MOTOR         = 0x17, //电机
  NET_MSGTYPE_MOTOR_ACK     = 0X97, //电机应答
  NET_MSGTYPE_DOORMODE      = 0x18, //开门方式
  NET_MSGTYPE_DOORMODE_ACK  = 0X98, //开门方式应答
  NET_MSGTYPE_CAMERA        = 0x1d, //远程拍照
  NET_MSGTYPE_CAMERA_ACK    = 0X9d, //远程拍照应答
  NET_MSGTYPE_TERMINAL      = 0x1e, //终端模块查询
  NET_MSGTYPE_TERMINAL_ACK  = 0X9e, //终端模块查询应答
  NET_MSGTYPE_DOMAIN        = 0x21, //域名管理
  NET_MSGTYPE_DOMAIN_ACK    = 0XA1, //域名管理应答
  NET_MSGTYPE_NULL          = 0XFF
}message_type;

typedef struct 
{
  rt_uint16_t data:10;  //数据字节数
  rt_uint16_t check:3;  //校验码
  rt_uint16_t col:1;    //序号字节数
  rt_uint16_t cmd:2;    //命令字节数
}net_lenmap_bit;

//长度映射域
typedef  union 
{
  rt_uint16_t  bype;
  net_lenmap_bit bit;
}net_lenmap;

//登陆
typedef struct 
{
  //rt_uint8_t info[16];
  rt_uint8_t k1[NET_KEY1_LEN];
  rt_uint8_t id[NET_DEVICE_ID_LEN];
  rt_uint8_t version;
}net_landed;

//心跳
typedef struct 
{
  rt_uint8_t door_status;
}net_heart;

//报警
typedef struct 
{
  rt_uint8_t type;        //报警类型
  rt_uint8_t lock_status; //锁状态
  rt_uint8_t time[4];     //发生时间
}net_alarm;

// 故障
typedef struct 
{
  rt_uint8_t type;       //故障类型
  rt_uint8_t time[4];    //发生时间
}net_fault;

//开门记录
typedef struct 
{
  rt_uint8_t type;      //开门方式    
  rt_uint8_t key[2];    //钥匙编号
  rt_uint8_t time[4];   //开门时间
}net_opendoor;

//电池信息
typedef struct 
{
  rt_uint8_t status;    //电池状态
  rt_uint8_t capacity;  //电池容量
  rt_uint8_t time[4];   //上传时间
}net_battery;


//文件请求
typedef struct 
{
  rt_uint8_t alarm;      //报警类型
  rt_uint8_t time[4];    //报警时间
  rt_uint8_t type;       //文件类型
  rt_uint8_t size[4];    //文件大小
  rt_uint8_t packsize;   //每一包大小
  rt_uint8_t packnum[4]; //总包数
  rt_uint8_t crc32[4];     //CRC32
}net_filerequest;

//文件请求应答
typedef struct 
{
	rt_uint8_t result;
}net_filereq_ack;

//文件数据
typedef struct 
{
  //rt_uint8_t col[4];    //当前包序号
  rt_uint8_t *data;     //数据
}net_filedata;

//文件传送应答
typedef struct 
{
	rt_uint8_t order[4];
	rt_uint8_t result;
}net_filedat_ack;

//响应添加手机号
typedef struct 
{
	rt_uint8_t pos;
  rt_uint8_t data[12];
}net_phoneadd;

//添加手机应答
typedef struct 
{
	rt_uint8_t result;
	rt_uint8_t pos;
}net_phoneadd_ack;

//响应删除手机号
typedef struct 
{
	rt_uint8_t pos;
}net_phonedelete;

//删除手机号码应答
typedef struct 
{
	rt_uint8_t result;
	rt_uint8_t pos;
}net_phonedel_ack;

//报警参数
typedef struct 
{
  rt_uint8_t type;     //报警类型
  rt_uint8_t arg;      //报警参数
}net_alarmarg;

//链接状态
typedef struct 
{
  rt_uint8_t arg;//休眠与唤醒报文的参数
}net_link;

//添加钥匙
typedef struct 
{
  rt_uint8_t col[2];			//序号
  rt_uint8_t type;				//类型
  rt_uint8_t createt[4];  //创建时间
  rt_uint8_t accredit;		//授权
  rt_uint8_t start_t[4];	//开始使用时间
  rt_uint8_t stop_t[4];		//停止使用时间
  rt_uint8_t *data;				//钥匙编码
}net_keyadd;

//添加钥匙应答
typedef struct 
{
	rt_uint8_t  result;			//操作结果
	rt_uint8_t  pos[2];			//钥匙位置
}net_keyadd_ack;

//删除钥匙
typedef struct 
{
	rt_uint8_t pos[2];
}net_keydelete;

//钥匙删除应答
typedef struct 
{
	rt_uint8_t result;
	rt_uint8_t pos[2];
}net_keydel_ack;

//更新
typedef struct 
{
  rt_uint8_t size[4];
  rt_uint8_t check[4];
}net_update;

//较时
typedef struct 
{
  rt_uint8_t time[4];
}net_time;

//设置k0
typedef struct 
{
  rt_uint8_t key0[8];
}net_setk0;

//http方式更新
typedef struct 
{
  rt_uint8_t check[4];
  rt_uint8_t *url;
}net_httpupdate;

//电机控制
typedef struct 
{
  rt_uint8_t operation;
}net_motor;

//开门方式
typedef struct 
{
  rt_uint8_t mode;
}net_doormode;

//摄像头
typedef struct 
{
  rt_uint8_t operation;
}net_camera;

//终端状态
typedef struct 
{
  rt_uint8_t status;
}net_terminal;

//域名
typedef struct 
{
  rt_uint8_t col;
  rt_uint8_t *url;
  rt_uint8_t port[2];
}net_domain;

//发送应答
typedef struct
{
	rt_uint8_t result;
}net_ack;

//数据域
typedef union 
{
  net_landed        landed;    		//登陆
  net_heart         heart;     		//心跳
  net_alarm         alarm;     		//报警
  net_fault         fault;     		//故障
  net_opendoor      opendoor;  		//开门记录
  net_battery       battery;   		//电池信息
  net_filerequest   filerequest; 	//文件请求
  net_filereq_ack   FileReqAck;   //文件请求应答
  net_filedata      filedata;   	//文件数据
  net_filedat_ack   FileDatAck;   //文件数据传送应答
  //net_phoneadd      phoneadd;   	//添加电话
  net_phoneadd_ack  PhoneAddAck;  //
  //net_phonedelete   phonedelete; 	//删除电话
  net_phonedel_ack  PhoneDelAck;  //删除电话号码应答
  net_alarmarg      alarmarg;  		//报警时间参数
  net_ack           AlarmArgAck;	//报警参数应答
  net_link          link;      		//链接状态管理
  net_ack           LinkAck;      //链接状态管理应答
  net_keyadd        keyadd;    		//添加钥匙
  net_keyadd_ack    KeyAddAck;    //添加钥匙应答
  net_keydelete     keydelete;		//删除钥匙
  net_keydel_ack    KeyDelAck; 		//钥匙删除应答
  net_update        update;    		//用文件协议更新
  net_ack           UpDateAck; 		//用文件协议更新应答
  net_time          time;      		//时间同步
  net_setk0         set_k0;    		//设置k0
  net_ack           SetK0Ack;     //设置k0应答
  net_httpupdate    httpupdate;		//用http方式更新
 	net_ack           HttpUpDateAck;//http更新方式应答
  net_motor         motor;     		//电机
  net_ack           MotorAck;     //电机应答
  net_doormode      doormode;  		//开门模式
  net_ack           DoorModeAck;  //开门模式应答
  net_camera        camera;    		//摄像头
  net_ack           CameraAck;    //摄像头应答
  net_terminal      terminal;  		//终端查询
  net_ack           TerminalAck;  //终端查询应答
  net_domain        domain;   		//域名管理
  net_ack           DomainAck;    //域名管理应答
}net_messge_data;

//序号的位操作结构
typedef struct 
{
	rt_uint8_t resend:2;
	rt_uint8_t col:6;
}net_col_bit;

//包序号
typedef union
{
	net_col_bit bit;
	rt_uint8_t  byte;
}net_col;


//需要加密的数据结构
typedef struct 
{
  net_lenmap      lenmap;  //长度映射域
  rt_uint8_t      cmd;     //命令
  net_col         col;     //序号
  net_messge_data data;    //数据
  rt_uint8_t      check[NET_CHECK_LEN];   //校验
}net_encrypt,*net_encrypt_p;


//发送给物理层的数据结构
typedef struct 
{
  rt_uint16_t length; //包长度(打包前使用)
  rt_uint8_t  *buffer;//整个报文打包后的数据
  rt_sem_t    sendsem;//资源管理信号
  //rt_uint16_t stern;
  
}net_message,*net_message_p;


/**
作用:一下结构体用于处理接收报文
说明:必须在不同数据域内定义crc16
     如果放到接收结构体中定义只有
     最长的包能获取crc16
*/
typedef struct 
{
	rt_uint8_t result;
	rt_uint8_t time[4];
	rt_uint8_t crc16[2];
}net_recv_landed;

//数据域为空
typedef struct 
{
	rt_uint8_t crc16[2];
}net_recv_null;

//数据只有结果
typedef struct 
{
	rt_uint8_t result;
	rt_uint8_t crc16[2];
}net_recv_result;

//接收文件包应答
typedef struct 
{
  rt_uint8_t order[4];
  rt_uint8_t result;
  rt_uint8_t crc16[2];
}net_recv_filedatack;

//接收到手机列表
typedef struct 
{
  rt_uint8_t pos;
  rt_uint8_t data[12];
  rt_uint8_t crc16[2];
}net_recv_phoneadd;

//删除手机号码
typedef struct 
{
	rt_uint8_t pos;
	rt_uint8_t crc16[2];
}net_recv_phonedel;

//文件请求
typedef struct 
{
	net_filerequest request;
	rt_uint8_t crc16[2];
}net_recv_filerq;

//文件包
typedef struct 
{
	rt_uint8_t pos[4];
	rt_uint8_t data[600];
}net_recv_filedata;

//报警参数
typedef struct 
{
	net_alarmarg arg;
	rt_uint8_t crc16[2];
}net_recv_alarmarg;

//钥匙添加
typedef struct 
{
	net_keyadd key;
	rt_uint8_t crc16[2];
}net_recv_keyadd;

//钥匙添加应答
typedef struct 
{
	net_keyadd_ack keyAck;
	rt_uint8_t crc16[2];
}net_recv_keyadd_ack;

//钥匙删除
typedef struct 
{
	net_keydelete key;
	rt_uint8_t crc16[2];
}net_recv_keydel;

//钥匙删除应答
typedef struct 
{
	net_keydelete keyAck;
	rt_uint8_t crc16[2];
}net_recv_keydel_ack;

//接收报文的数据域
typedef union 
{
  net_recv_landed 		landed_ack;   //登陆
  net_recv_null   		heart_ack;    //报警
  net_recv_null   		alarm_ack;    //报警
  net_recv_null   		fault_ack;    //报警
  net_recv_null   		opendoor_ack; //报警
  net_recv_null   		battery_ack;  //电池
  net_recv_filerq 		filerq;       //文件请求
  net_recv_null       filerq_ack;   //文件请求应答
  net_recv_filedata 	filedata;     //文件数据
  net_recv_filedatack filedata_ack; //文件数据应答
  net_recv_phoneadd 	phoneadd;			//添加手机号码
  net_recv_phonedel   phonedel;     //删除手机号码
  net_recv_alarmarg   AlarmArg;     //报警参数
  net_recv_result     AlarmArgAck;  //报警参数应答
  net_recv_keyadd     keyadd;				//钥匙添加
  net_recv_keyadd_ack KeyAddAck;    //钥匙添加应答
  net_recv_keydel     keydel;       //钥匙删除
  net_recv_keyadd_ack KeyDelAck;    //钥匙删除应答
}net_recv_data;

//接收报文的描述结构体
typedef struct 
{
	rt_uint16_t   length;  //包长度
	net_lenmap    lenmap;  //长度映射域
  message_type  cmd;     //命令
  net_col       col;     //序号
  rt_uint16_t   reserve; //用于结构体对齐
  net_recv_data data;    //各种报文的数据
}net_recvmsg,*net_recvmsg_p;



//报文发送的模式
typedef enum
{
  SYNC_MODE,    //同步
  ASYN_MODE,    //异步
  INIT_MODE     //初始化值
}net_msgmode;

/**
作用:邮件转发层发送报文邮件的控制结构
*/
typedef struct
{
  rt_uint16_t  outtime; //超时时间
  rt_uint8_t   type;    //发送类型
  net_msgmode  sendmode;//发送模式
  net_col      col;     //包序号
  rt_uint32_t  time;    //发送时间
  rt_uint8_t   resend;  //重发次数
  void         *user;   //报文私有数据
}net_msgmail,*net_msgmail_p;

//发送窗口控制结构
typedef struct _NET_SENDWND_LIST_
{
  net_msgmail  mail;      //原始的报文描述邮件
  rt_uint16_t  curtime;   //当前计时器
  volatile rt_int8_t permission; //窗口的发送权限
}net_sendwnd,*net_sendwnd_p;


/**
协议使用到的事件类型
*/
#define  NET_ENVET_RELINK              0X01<<0 //物理层重新链接
#define  NET_ENVET_ONLINE              0X01<<1 //在线
#define  NET_ENVET_FILERQ              0x01<<2 //文件请求
#define  NET_EVENT_ALL                 0XFFFFFFFF


/**
应用层用于设置数据域的数据结构 所有同步方式发送报文的
私有数据中必须有net_send_result 结构
*/
typedef struct 
{
  rt_uint8_t result;
  rt_sem_t   complete;
}net_send_result,*net_send_result_p;


typedef struct
{
  net_send_result result;
  net_alarm       alarm;
}net_alarm_user;

typedef struct 
{
  net_send_result result;
  net_fault       fault;
}net_fault_user;

typedef struct
{
  net_send_result result;
  net_opendoor    opendoor;
}net_opendoor_user;

typedef struct 
{
  net_send_result result;
  net_battery     battery;
}net_battery_user;

typedef struct 
{
  net_send_result result;
  net_filerequest file;
}net_filerequest_user;

//文件包请求应答
typedef struct
{
	net_filereq_ack result;
}net_filereq_ack_user;

//文件数据
typedef struct 
{
  net_send_result result;
  net_filedata    data;
  rt_uint16_t     length;
  rt_uint8_t      sendresult;
}net_filedata_user;

//文件包应答 异步
typedef struct 
{
	net_filedat_ack fileack;
}net_filedata_ack_user;

//钥匙添加
typedef struct 
{
	net_send_result result;
	net_keyadd data;
	rt_uint16_t DataLen;
}net_keyadd_user;

//钥匙删除
typedef struct 
{
  net_send_result result;
	net_keydelete data;
}net_keydelete_user;

//较时
typedef struct 
{
	net_send_result result;
	net_time date;
}net_time_user;

//用户告警参数
typedef struct 
{
	net_send_result result;
	net_alarmarg args;
}net_alarmarg_user;



//包序号
extern net_col net_order;

//网络物理层接口
extern rt_mq_t net_datsend_mq;     //协议层发送给物理网络层
extern rt_mailbox_t net_datrecv_mb;//接收邮箱


void net_msg_send_mail(net_msgmail_p mail);
void Net_Set_MsgRecv_Callback(rt_uint8_t (*Callback)(net_recvmsg_p Mail,void *UserData));
void Net_NetMsg_thread_callback(void (*Callback)(void));


rt_uint8_t net_event_process(rt_uint8_t mode,rt_uint32_t type);
rt_uint8_t get_msg_new_order(rt_bool_t flag);//获得报文的新序号
rt_bool_t net_mail_crc16_check(net_recvmsg_p Mail);

rt_uint32_t net_get_wnd_user(net_recvmsg_p msg);

#endif



