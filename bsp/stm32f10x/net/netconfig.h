#ifndef __NETCONFIG_H__
#define __NETCONFIG_H__

#ifdef   USEING_RAM_DEBUG
#define NET_MSG_THREAD             1 //显示协议线程处理的信息
#define NET_RECV_MSG_TYP           2 //显示接收报文的报文类型
#define NET_RECV_MSG_INFO          3 //显示接收报文信息
#define NET_SEND_MSG_INFO          4 //显示发送报文信息
#define NET_LENMAP_INFO            5 //显示长度映射域详细信息
#define NET_SEND_MODE_INFO         6 //显示发送模式信息
#define NET_MEM_USE_INFO               7 //系那是内存信息
#define NET_WND_INFO               8 //显示报文发送窗口处理信息
#define NET_SET_MSG_INOF           9 //显示设置报文数据过程的信息
#define NET_RECV_MAIL_ADDR         10 //显示接收数据邮箱的内存地址
#define NET_NONE_DES_RWDATA          11 //显示没有加密的数据
#define NET_RECV_ENC_DATA          27 //显示接收到的密码
#define NET_SEND_MSG_TYPE          28 //显示发送报文类型
#define NET_SEND_DES_DATA          29 //显示发送报文的密文
//网络文件
#define NET_NFILE_CRC32            12 //显示文件的crc32校验值
#define NET_NFILE_SEND_INFO             13 //显示文件发送过程
#define NET_NFILE_SRESULT          14 //显示文件包发送的结果

#define NET_CRC16_INIF             15 //显示crc16信息
#else
#define NET_MSG_THREAD             0 //显示协议线程处理的信息
#define SHOW_RECV_GSM_RST           1 //显示接收报文的结果
#define NET_RECV_MSG_INFO          0 //显示接收报文信息
#define NET_SEND_MSG_INFO          1 //显示发送报文信息
#define NET_LENMAP_INFO            0 //显示长度映射域详细信息
#define NET_SEND_MODE_INFO         0 //显示发送模式信息
#define NET_MEM_USE_INFO               0 //系那是内存信息
#define NET_WND_INFO               0 //显示报文发送窗口处理信息
#define NET_SET_MSG_INOF           0 //显示设置报文数据过程的信息
#define NET_RECV_MAIL_ADDR         0 //显示接收数据邮箱的内存地址
#define NET_NONE_DES_RWDATA          1 //显示没有加密的数据

//网络文件
#define NET_NFILE_CRC32            0 //显示文件的crc32校验值
#define NET_NFILE_SEND_INFO             0 //显示文件发送过程
#define NET_NFILE_SRESULT          1 //显示文件包发送的结果

#define NET_CRC16_INIF             0 //显示crc16信息
#endif

#endif

