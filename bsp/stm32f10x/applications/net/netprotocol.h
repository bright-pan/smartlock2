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

#define NET_WND_MAX_NUM             16  //���ڴ�С
#define NET_FILE_BUF_SIZE           512 //�ļ�buffer


#define NET_MESSAGE_STERN       0XAA55
#define NET_KEY1_LEN            8
#define NET_DEVICE_ID_LEN       8
#define NET_CHECK_LEN           2

#define NET_RECV_MSG_MAX        5

//��������
typedef enum
{
  NET_MSGTYPE_LANDED        = 0x00, //��½
  NET_MSGTYPE_LANDED_ACK    = 0X80, //��½Ӧ��
  NET_MSGTYPE_HEART         = 0x01, //����
  NET_MSGTYPE_HEART_ACK     = 0X81, //����Ӧ��
  NET_MSGTYPE_ALARM         = 0x02, //��������
  NET_MSGTYPE_ALARM_ACK     = 0X82, //��������Ӧ��
  NET_MSGTYPE_FAULT         = 0x03, //����
  NET_MSGTYPE_FAULT_ACK     = 0X83, //����Ӧ��
  NET_MSGTYPE_OPENDOOR      = 0x04, //����
  NET_MSGTYPE_OPENDOOR_ACK  = 0X84, //����Ӧ��
  NET_MSGTYPE_BATTERY       = 0x05, //���
  NET_MSGTYPE_BATTERY_ACK   = 0x85, //���Ӧ��
  NET_MSGTYPE_FILEREQUEST   = 0x06, //�ļ�����
  NET_MSGTYPE_FILEREQUE_ACK = 0X86, //�ļ�����Ӧ��
  NET_MSGTYPE_FILEDATA      = 0x07, //�ļ�����
  NET_MSGTYPE_FILEDATA_ACK  = 0X87, //�ļ�����Ӧ��
  NET_MSGTYPE_PHONEADD      = 0x0b, //��ӵ绰
  NET_MSGTYPE_PHONEADD_ACK  = 0X8B, //��ӵ绰Ӧ��
  NET_MSGTYPE_PHONEDELETE   = 0x0c, //ɾ���绰
  NET_MSGTYPE_PHONEDEL_ACK = 0X8C, //ɾ���绰Ӧ����Ϣ
  NET_MSGTYPE_ALARMARG      = 0x0d, //��������
  NET_MSGTYPE_ALARMARG_ACK  = 0X8d, //��������Ӧ��
  NET_MSGTYPE_LINK          = 0x0e, //����
  NET_MSGTYPE_LINK_ACK      = 0X8E, //����Ӧ��
  NET_MSGTYPE_KEYADD        = 0x10, //���Կ��
  NET_MSGTYPE_KEYADD_ACK    = 0X90, //���Կ��Ӧ��
  NET_MSGTYPE_KEYDELETE     = 0x12, //ɾ��Կ��
  NET_MSGTYPE_KEYDEL_ACK    = 0X92, //ɾ��Կ��Ӧ��
  NET_MSGTYPE_UPDATE        = 0x13, //�ļ�����
  NET_MSGTYPE_UPDATE_ACK    = 0X93, //�ļ�����Ӧ��
  NET_MSGTYPE_TIME          = 0x14, //ʱ��ͬ��
  NET_MSGTYPE_TIME_ACK      = 0X94, //ʱ��ͬ��Ӧ��
  NET_MSGTYPE_SETK0         = 0x15, //����K0
  NET_MSGTYPE_SETK0_ACK     = 0X95, //����K0Ӧ��
  NET_MSGTYPE_HTTPUPDATE    = 0x16, //HTTP����
  NET_MSGTYPE_HTTPUPDAT_ACK = 0X96, //HTTP����Ӧ��
  NET_MSGTYPE_MOTOR         = 0x17, //���
  NET_MSGTYPE_MOTOR_ACK     = 0X97, //���Ӧ��
  NET_MSGTYPE_DOORMODE      = 0x18, //���ŷ�ʽ
  NET_MSGTYPE_DOORMODE_ACK  = 0X98, //���ŷ�ʽӦ��
  NET_MSGTYPE_CAMERA        = 0x1d, //Զ������
  NET_MSGTYPE_CAMERA_ACK    = 0X9d, //Զ������Ӧ��
  NET_MSGTYPE_TERMINAL      = 0x1e, //�ն�ģ���ѯ
  NET_MSGTYPE_TERMINAL_ACK  = 0X9e, //�ն�ģ���ѯӦ��
  NET_MSGTYPE_DOMAIN        = 0x21, //��������
  NET_MSGTYPE_DOMAIN_ACK    = 0XA1, //��������Ӧ��
  NET_MSGTYPE_NULL          = 0XFF
}message_type;

typedef struct 
{
  rt_uint16_t data:10;  //�����ֽ���
  rt_uint16_t check:3;  //У����
  rt_uint16_t col:1;    //����ֽ���
  rt_uint16_t cmd:2;    //�����ֽ���
}net_lenmap_bit;

//����ӳ����
typedef  union 
{
  rt_uint16_t  bype;
  net_lenmap_bit bit;
}net_lenmap;

//��½
typedef struct 
{
  //rt_uint8_t info[16];
  rt_uint8_t k1[NET_KEY1_LEN];
  rt_uint8_t id[NET_DEVICE_ID_LEN];
  rt_uint8_t version;
}net_landed;

//����
typedef struct 
{
  rt_uint8_t door_status;
}net_heart;

//����
typedef struct 
{
  rt_uint8_t type;        //��������
  rt_uint8_t lock_status; //��״̬
  rt_uint8_t time[4];     //����ʱ��
}net_alarm;

// ����
typedef struct 
{
  rt_uint8_t type;       //��������
  rt_uint8_t time[4];    //����ʱ��
}net_fault;

//���ż�¼
typedef struct 
{
  rt_uint8_t type;      //���ŷ�ʽ    
  rt_uint8_t key[2];    //Կ�ױ��
  rt_uint8_t time[4];   //����ʱ��
}net_opendoor;

//�����Ϣ
typedef struct 
{
  rt_uint8_t status;    //���״̬
  rt_uint8_t capacity;  //�������
  rt_uint8_t time[4];   //�ϴ�ʱ��
}net_battery;


//�ļ�����
typedef struct 
{
  rt_uint8_t alarm;      //��������
  rt_uint8_t time[4];    //����ʱ��
  rt_uint8_t type;       //�ļ�����
  rt_uint8_t size[4];    //�ļ���С
  rt_uint8_t packsize;   //ÿһ����С
  rt_uint8_t packnum[4]; //�ܰ���
  rt_uint8_t crc32[4];     //CRC32
}net_filerequest;

//�ļ�����Ӧ��
typedef struct 
{
	rt_uint8_t result;
}net_filereq_ack;

//�ļ�����
typedef struct 
{
  //rt_uint8_t col[4];    //��ǰ�����
  rt_uint8_t *data;     //����
}net_filedata;

//�ļ�����Ӧ��
typedef struct 
{
	rt_uint8_t order[4];
	rt_uint8_t result;
}net_filedat_ack;

//��Ӧ����ֻ���
typedef struct 
{
	rt_uint8_t pos;
  rt_uint8_t data[12];
}net_phoneadd;

//����ֻ�Ӧ��
typedef struct 
{
	rt_uint8_t result;
	rt_uint8_t pos;
}net_phoneadd_ack;

//��Ӧɾ���ֻ���
typedef struct 
{
	rt_uint8_t pos;
}net_phonedelete;

//ɾ���ֻ�����Ӧ��
typedef struct 
{
	rt_uint8_t result;
	rt_uint8_t pos;
}net_phonedel_ack;

//��������
typedef struct 
{
  rt_uint8_t type;     //��������
  rt_uint8_t arg;      //��������
}net_alarmarg;

//����״̬
typedef struct 
{
  rt_uint8_t arg;//�����뻽�ѱ��ĵĲ���
}net_link;

//���Կ��
typedef struct 
{
  rt_uint8_t col[2];			//���
  rt_uint8_t type;				//����
  rt_uint8_t createt[4];  //����ʱ��
  rt_uint8_t accredit;		//��Ȩ
  rt_uint8_t start_t[4];	//��ʼʹ��ʱ��
  rt_uint8_t stop_t[4];		//ֹͣʹ��ʱ��
  rt_uint8_t *data;				//Կ�ױ���
}net_keyadd;

//���Կ��Ӧ��
typedef struct 
{
	rt_uint8_t  result;			//�������
	rt_uint8_t  pos[2];			//Կ��λ��
}net_keyadd_ack;

//ɾ��Կ��
typedef struct 
{
	rt_uint8_t pos[2];
}net_keydelete;

//Կ��ɾ��Ӧ��
typedef struct 
{
	rt_uint8_t result;
	rt_uint8_t pos[2];
}net_keydel_ack;

//����
typedef struct 
{
  rt_uint8_t size[4];
  rt_uint8_t check[4];
}net_update;

//��ʱ
typedef struct 
{
  rt_uint8_t time[4];
}net_time;

//����k0
typedef struct 
{
  rt_uint8_t key0[8];
}net_setk0;

//http��ʽ����
typedef struct 
{
  rt_uint8_t check[4];
  rt_uint8_t *url;
}net_httpupdate;

//�������
typedef struct 
{
  rt_uint8_t operation;
}net_motor;

//���ŷ�ʽ
typedef struct 
{
  rt_uint8_t mode;
}net_doormode;

//����ͷ
typedef struct 
{
  rt_uint8_t operation;
}net_camera;

//�ն�״̬
typedef struct 
{
  rt_uint8_t status;
}net_terminal;

//����
typedef struct 
{
  rt_uint8_t col;
  rt_uint8_t *url;
  rt_uint8_t port[2];
}net_domain;

//����Ӧ��
typedef struct
{
	rt_uint8_t result;
}net_ack;

//������
typedef union 
{
  net_landed        landed;    		//��½
  net_heart         heart;     		//����
  net_alarm         alarm;     		//����
  net_fault         fault;     		//����
  net_opendoor      opendoor;  		//���ż�¼
  net_battery       battery;   		//�����Ϣ
  net_filerequest   filerequest; 	//�ļ�����
  net_filereq_ack   FileReqAck;   //�ļ�����Ӧ��
  net_filedata      filedata;   	//�ļ�����
  net_filedat_ack   FileDatAck;   //�ļ����ݴ���Ӧ��
  //net_phoneadd      phoneadd;   	//��ӵ绰
  net_phoneadd_ack  PhoneAddAck;  //
  //net_phonedelete   phonedelete; 	//ɾ���绰
  net_phonedel_ack  PhoneDelAck;  //ɾ���绰����Ӧ��
  net_alarmarg      alarmarg;  		//����ʱ�����
  net_ack           AlarmArgAck;	//��������Ӧ��
  net_link          link;      		//����״̬����
  net_ack           LinkAck;      //����״̬����Ӧ��
  net_keyadd        keyadd;    		//���Կ��
  net_keyadd_ack    KeyAddAck;    //���Կ��Ӧ��
  net_keydelete     keydelete;		//ɾ��Կ��
  net_keydel_ack    KeyDelAck; 		//Կ��ɾ��Ӧ��
  net_update        update;    		//���ļ�Э�����
  net_ack           UpDateAck; 		//���ļ�Э�����Ӧ��
  net_time          time;      		//ʱ��ͬ��
  net_setk0         set_k0;    		//����k0
  net_ack           SetK0Ack;     //����k0Ӧ��
  net_httpupdate    httpupdate;		//��http��ʽ����
 	net_ack           HttpUpDateAck;//http���·�ʽӦ��
  net_motor         motor;     		//���
  net_ack           MotorAck;     //���Ӧ��
  net_doormode      doormode;  		//����ģʽ
  net_ack           DoorModeAck;  //����ģʽӦ��
  net_camera        camera;    		//����ͷ
  net_ack           CameraAck;    //����ͷӦ��
  net_terminal      terminal;  		//�ն˲�ѯ
  net_ack           TerminalAck;  //�ն˲�ѯӦ��
  net_domain        domain;   		//��������
  net_ack           DomainAck;    //��������Ӧ��
}net_messge_data;

//��ŵ�λ�����ṹ
typedef struct 
{
	rt_uint8_t resend:2;
	rt_uint8_t col:6;
}net_col_bit;

//�����
typedef union
{
	net_col_bit bit;
	rt_uint8_t  byte;
}net_col;


//��Ҫ���ܵ����ݽṹ
typedef struct 
{
  net_lenmap      lenmap;  //����ӳ����
  rt_uint8_t      cmd;     //����
  net_col         col;     //���
  net_messge_data data;    //����
  rt_uint8_t      check[NET_CHECK_LEN];   //У��
}net_encrypt,*net_encrypt_p;


//���͸����������ݽṹ
typedef struct 
{
  rt_uint16_t length; //������(���ǰʹ��)
  rt_uint8_t  *buffer;//�������Ĵ���������
  rt_sem_t    sendsem;//��Դ�����ź�
  //rt_uint16_t stern;
  
}net_message,*net_message_p;


/**
����:һ�½ṹ�����ڴ�����ձ���
˵��:�����ڲ�ͬ�������ڶ���crc16
     ����ŵ����սṹ���ж���ֻ��
     ��İ��ܻ�ȡcrc16
*/
typedef struct 
{
	rt_uint8_t result;
	rt_uint8_t time[4];
	rt_uint8_t crc16[2];
}net_recv_landed;

//������Ϊ��
typedef struct 
{
	rt_uint8_t crc16[2];
}net_recv_null;

//����ֻ�н��
typedef struct 
{
	rt_uint8_t result;
	rt_uint8_t crc16[2];
}net_recv_result;

//�����ļ���Ӧ��
typedef struct 
{
  rt_uint8_t order[4];
  rt_uint8_t result;
  rt_uint8_t crc16[2];
}net_recv_filedatack;

//���յ��ֻ��б�
typedef struct 
{
  rt_uint8_t pos;
  rt_uint8_t data[12];
  rt_uint8_t crc16[2];
}net_recv_phoneadd;

//ɾ���ֻ�����
typedef struct 
{
	rt_uint8_t pos;
	rt_uint8_t crc16[2];
}net_recv_phonedel;

//�ļ�����
typedef struct 
{
	net_filerequest request;
	rt_uint8_t crc16[2];
}net_recv_filerq;

//�ļ���
typedef struct 
{
	rt_uint8_t pos[4];
	rt_uint8_t data[600];
}net_recv_filedata;

//��������
typedef struct 
{
	net_alarmarg arg;
	rt_uint8_t crc16[2];
}net_recv_alarmarg;

//Կ�����
typedef struct 
{
	net_keyadd key;
	rt_uint8_t crc16[2];
}net_recv_keyadd;

//Կ�����Ӧ��
typedef struct 
{
	net_keyadd_ack keyAck;
	rt_uint8_t crc16[2];
}net_recv_keyadd_ack;

//Կ��ɾ��
typedef struct 
{
	net_keydelete key;
	rt_uint8_t crc16[2];
}net_recv_keydel;

//Կ��ɾ��Ӧ��
typedef struct 
{
	net_keydelete keyAck;
	rt_uint8_t crc16[2];
}net_recv_keydel_ack;

//���ձ��ĵ�������
typedef union 
{
  net_recv_landed 		landed_ack;   //��½
  net_recv_null   		heart_ack;    //����
  net_recv_null   		alarm_ack;    //����
  net_recv_null   		fault_ack;    //����
  net_recv_null   		opendoor_ack; //����
  net_recv_null   		battery_ack;  //���
  net_recv_filerq 		filerq;       //�ļ�����
  net_recv_null       filerq_ack;   //�ļ�����Ӧ��
  net_recv_filedata 	filedata;     //�ļ�����
  net_recv_filedatack filedata_ack; //�ļ�����Ӧ��
  net_recv_phoneadd 	phoneadd;			//����ֻ�����
  net_recv_phonedel   phonedel;     //ɾ���ֻ�����
  net_recv_alarmarg   AlarmArg;     //��������
  net_recv_result     AlarmArgAck;  //��������Ӧ��
  net_recv_keyadd     keyadd;				//Կ�����
  net_recv_keyadd_ack KeyAddAck;    //Կ�����Ӧ��
  net_recv_keydel     keydel;       //Կ��ɾ��
  net_recv_keyadd_ack KeyDelAck;    //Կ��ɾ��Ӧ��
}net_recv_data;

//���ձ��ĵ������ṹ��
typedef struct 
{
	rt_uint16_t   length;  //������
	net_lenmap    lenmap;  //����ӳ����
  message_type  cmd;     //����
  net_col       col;     //���
  rt_uint16_t   reserve; //���ڽṹ�����
  net_recv_data data;    //���ֱ��ĵ�����
}net_recvmsg,*net_recvmsg_p;



//���ķ��͵�ģʽ
typedef enum
{
  SYNC_MODE,    //ͬ��
  ASYN_MODE,    //�첽
  INIT_MODE     //��ʼ��ֵ
}net_msgmode;

/**
����:�ʼ�ת���㷢�ͱ����ʼ��Ŀ��ƽṹ
*/
typedef struct
{
  rt_uint16_t  outtime; //��ʱʱ��
  rt_uint8_t   type;    //��������
  net_msgmode  sendmode;//����ģʽ
  net_col      col;     //�����
  rt_uint32_t  time;    //����ʱ��
  rt_uint8_t   resend;  //�ط�����
  void         *user;   //����˽������
}net_msgmail,*net_msgmail_p;

//���ʹ��ڿ��ƽṹ
typedef struct _NET_SENDWND_LIST_
{
  net_msgmail  mail;      //ԭʼ�ı��������ʼ�
  rt_uint16_t  curtime;   //��ǰ��ʱ��
  volatile rt_int8_t permission; //���ڵķ���Ȩ��
}net_sendwnd,*net_sendwnd_p;


/**
Э��ʹ�õ����¼�����
*/
#define  NET_ENVET_RELINK              0X01<<0 //�������������
#define  NET_ENVET_ONLINE              0X01<<1 //����
#define  NET_ENVET_FILERQ              0x01<<2 //�ļ�����
#define  NET_EVENT_ALL                 0XFFFFFFFF


/**
Ӧ�ò�������������������ݽṹ ����ͬ����ʽ���ͱ��ĵ�
˽�������б�����net_send_result �ṹ
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

//�ļ�������Ӧ��
typedef struct
{
	net_filereq_ack result;
}net_filereq_ack_user;

//�ļ�����
typedef struct 
{
  net_send_result result;
  net_filedata    data;
  rt_uint16_t     length;
  rt_uint8_t      sendresult;
}net_filedata_user;

//�ļ���Ӧ�� �첽
typedef struct 
{
	net_filedat_ack fileack;
}net_filedata_ack_user;

//Կ�����
typedef struct 
{
	net_send_result result;
	net_keyadd data;
	rt_uint16_t DataLen;
}net_keyadd_user;

//Կ��ɾ��
typedef struct 
{
  net_send_result result;
	net_keydelete data;
}net_keydelete_user;

//��ʱ
typedef struct 
{
	net_send_result result;
	net_time date;
}net_time_user;

//�û��澯����
typedef struct 
{
	net_send_result result;
	net_alarmarg args;
}net_alarmarg_user;



//�����
extern net_col net_order;

//���������ӿ�
extern rt_mq_t net_datsend_mq;     //Э��㷢�͸����������
extern rt_mailbox_t net_datrecv_mb;//��������


void net_msg_send_mail(net_msgmail_p mail);
void Net_Set_MsgRecv_Callback(rt_uint8_t (*Callback)(net_recvmsg_p Mail,void *UserData));
void Net_NetMsg_thread_callback(void (*Callback)(void));


rt_uint8_t net_event_process(rt_uint8_t mode,rt_uint32_t type);
rt_uint8_t get_msg_new_order(rt_bool_t flag);//��ñ��ĵ������
rt_bool_t net_mail_crc16_check(net_recvmsg_p Mail);

rt_uint32_t net_get_wnd_user(net_recvmsg_p msg);

#endif



