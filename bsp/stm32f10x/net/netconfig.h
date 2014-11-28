#ifndef __NETCONFIG_H__
#define __NETCONFIG_H__

#ifdef   USEING_CAN_SET_DEBUG
#define SHOW_MSG_THREAD             1 //��ʾЭ���̴߳������Ϣ
#define SHOW_RECV_GSM_RST           2 //��ʾ���ձ��ĵĽ��
#define SHOW_RECV_MSG_INFO          3 //��ʾ���ձ�����Ϣ
#define SHOW_SEND_MSG_INFO          4 //��ʾ���ͱ�����Ϣ
#define SHOW_LENMAP_INFO            5 //��ʾ����ӳ������ϸ��Ϣ
#define SHOW_SEND_MODE_INFO         6 //��ʾ����ģʽ��Ϣ
#define SHOW_MEM_INFO               7 //ϵ�����ڴ���Ϣ
#define SHOW_WND_INFO               8 //��ʾ���ķ��ʹ��ڴ�����Ϣ
#define SHOW_SET_MSG_INOF           9 //��ʾ���ñ������ݹ��̵���Ϣ
#define SHOW_RECV_MAIL_ADDR         10 //��ʾ��������������ڴ��ַ
#define SHOW_NONE_ENC_DATA          11 //��ʾû�м��ܵ�����

//�����ļ�
#define SHOW_NFILE_CRC32            12 //��ʾ�ļ���crc32У��ֵ
#define SHOW_NFILE_SEND             13 //��ʾ�ļ����͹���
#define SHOW_NFILE_SRESULT          14 //��ʾ�ļ������͵Ľ��

#define SHOW_CRC16_INIF             15 //��ʾcrc16��Ϣ
#else
#define SHOW_MSG_THREAD             0 //��ʾЭ���̴߳������Ϣ
#define SHOW_RECV_GSM_RST           1 //��ʾ���ձ��ĵĽ��
#define SHOW_RECV_MSG_INFO          0 //��ʾ���ձ�����Ϣ
#define SHOW_SEND_MSG_INFO          1 //��ʾ���ͱ�����Ϣ
#define SHOW_LENMAP_INFO            0 //��ʾ����ӳ������ϸ��Ϣ
#define SHOW_SEND_MODE_INFO         0 //��ʾ����ģʽ��Ϣ
#define SHOW_MEM_INFO               0 //ϵ�����ڴ���Ϣ
#define SHOW_WND_INFO               0 //��ʾ���ķ��ʹ��ڴ�����Ϣ
#define SHOW_SET_MSG_INOF           0 //��ʾ���ñ������ݹ��̵���Ϣ
#define SHOW_RECV_MAIL_ADDR         0 //��ʾ��������������ڴ��ַ
#define SHOW_NONE_ENC_DATA          1 //��ʾû�м��ܵ�����

//�����ļ�
#define SHOW_NFILE_CRC32            0 //��ʾ�ļ���crc32У��ֵ
#define SHOW_NFILE_SEND             0 //��ʾ�ļ����͹���
#define SHOW_NFILE_SRESULT          1 //��ʾ�ļ������͵Ľ��

#define SHOW_CRC16_INIF             0 //��ʾcrc16��Ϣ
#endif

#endif

