#ifndef __NETCONFIG_H__
#define __NETCONFIG_H__

#ifdef   USEING_CAN_SET_DEBUG
#define SHOW_MSG_THREAD             1 //显示协议线程处理的信息
#define SHOW_RECV_GSM_RST           2 //显示接收报文的结果
#define SHOW_RECV_MSG_INFO          3 //显示接收报文信息
#define SHOW_SEND_MSG_INFO          4 //显示发送报文信息
#define SHOW_LENMAP_INFO            5 //显示长度映射域详细信息
#define SHOW_SEND_MODE_INFO         6 //显示发送模式信息
#define SHOW_MEM_INFO               7 //系那是内存信息
#define SHOW_WND_INFO               8 //显示报文发送窗口处理信息
#define SHOW_SET_MSG_INOF           9 //显示设置报文数据过程的信息
#define SHOW_RECV_MAIL_ADDR         10 //显示接收数据邮箱的内存地址
#define SHOW_NONE_ENC_DATA          11 //显示没有加密的数据

//网络文件
#define SHOW_NFILE_CRC32            12 //显示文件的crc32校验值
#define SHOW_NFILE_SEND             13 //显示文件发送过程
#define SHOW_NFILE_SRESULT          14 //显示文件包发送的结果

#define SHOW_CRC16_INIF             15 //显示crc16信息
#else
#define SHOW_MSG_THREAD             0 //显示协议线程处理的信息
#define SHOW_RECV_GSM_RST           1 //显示接收报文的结果
#define SHOW_RECV_MSG_INFO          0 //显示接收报文信息
#define SHOW_SEND_MSG_INFO          1 //显示发送报文信息
#define SHOW_LENMAP_INFO            0 //显示长度映射域详细信息
#define SHOW_SEND_MODE_INFO         0 //显示发送模式信息
#define SHOW_MEM_INFO               0 //系那是内存信息
#define SHOW_WND_INFO               0 //显示报文发送窗口处理信息
#define SHOW_SET_MSG_INOF           0 //显示设置报文数据过程的信息
#define SHOW_RECV_MAIL_ADDR         0 //显示接收数据邮箱的内存地址
#define SHOW_NONE_ENC_DATA          1 //显示没有加密的数据

//网络文件
#define SHOW_NFILE_CRC32            0 //显示文件的crc32校验值
#define SHOW_NFILE_SEND             0 //显示文件发送过程
#define SHOW_NFILE_SRESULT          1 //显示文件包发送的结果

#define SHOW_CRC16_INIF             0 //显示crc16信息
#endif

#endif

