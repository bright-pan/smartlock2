#ifndef __NETCONFIG_H__
#define __NETCONFIG_H__

#ifdef   USEING_RAM_DEBUG
#define NET_MSG_THREAD             1 //��ʾЭ���̴߳������Ϣ
#define NET_RECV_MSG_TYP           2 //��ʾ���ձ��ĵı�������
#define NET_RECV_MSG_INFO          3 //��ʾ���ձ�����Ϣ
#define NET_SEND_MSG_INFO          4 //��ʾ���ͱ�����Ϣ
#define NET_LENMAP_INFO            5 //��ʾ����ӳ������ϸ��Ϣ
#define NET_SEND_MODE_INFO         6 //��ʾ����ģʽ��Ϣ
#define NET_MEM_USE_INFO               7 //ϵ�����ڴ���Ϣ
#define NET_WND_INFO               8 //��ʾ���ķ��ʹ��ڴ�����Ϣ
#define NET_SET_MSG_INOF           9 //��ʾ���ñ������ݹ��̵���Ϣ
#define NET_RECV_MAIL_ADDR         10 //��ʾ��������������ڴ��ַ
#define NET_NONE_DES_RWDATA          11 //��ʾû�м��ܵ�����
#define NET_RECV_ENC_DATA          27 //��ʾ���յ�������
#define NET_SEND_MSG_TYPE          28 //��ʾ���ͱ�������
#define NET_SEND_DES_DATA          29 //��ʾ���ͱ��ĵ�����
//�����ļ�
#define NET_NFILE_CRC32            12 //��ʾ�ļ���crc32У��ֵ
#define NET_NFILE_SEND_INFO             13 //��ʾ�ļ����͹���
#define NET_NFILE_SRESULT          14 //��ʾ�ļ������͵Ľ��

#define NET_CRC16_INIF             15 //��ʾcrc16��Ϣ
#else
#define NET_MSG_THREAD             0 //��ʾЭ���̴߳������Ϣ
#define SHOW_RECV_GSM_RST           1 //��ʾ���ձ��ĵĽ��
#define NET_RECV_MSG_INFO          0 //��ʾ���ձ�����Ϣ
#define NET_SEND_MSG_INFO          1 //��ʾ���ͱ�����Ϣ
#define NET_LENMAP_INFO            0 //��ʾ����ӳ������ϸ��Ϣ
#define NET_SEND_MODE_INFO         0 //��ʾ����ģʽ��Ϣ
#define NET_MEM_USE_INFO               0 //ϵ�����ڴ���Ϣ
#define NET_WND_INFO               0 //��ʾ���ķ��ʹ��ڴ�����Ϣ
#define NET_SET_MSG_INOF           0 //��ʾ���ñ������ݹ��̵���Ϣ
#define NET_RECV_MAIL_ADDR         0 //��ʾ��������������ڴ��ַ
#define NET_NONE_DES_RWDATA          1 //��ʾû�м��ܵ�����

//�����ļ�
#define NET_NFILE_CRC32            0 //��ʾ�ļ���crc32У��ֵ
#define NET_NFILE_SEND_INFO             0 //��ʾ�ļ����͹���
#define NET_NFILE_SRESULT          1 //��ʾ�ļ������͵Ľ��

#define NET_CRC16_INIF             0 //��ʾcrc16��Ϣ
#endif

#endif

