#ifndef __TCP_CLIENT_DEMO_H
#define __TCP_CLIENT_DEMO_H

#include "includes.h"
#include "sys_arch.h"
////////////////////////////////////////////////////////////////////////////////// 	   

 
#define TCP_CLIENT_RX_BUFSIZE	2000	//接收缓冲区长度
#define TCP_CLIENT_TX_BUFSIZE 2000  //发送缓冲区长度
#define TCP_MESSAGE_HEAD			        4    //定义有数据发送

#define NOTDATA               1



typedef struct { 
  sys_mbox_t   recvQ;
	sys_mbox_t   sendQ;
	u8  remoteip[4];	//远端主机IP地址 
	u16 remoteport;
	struct netconn *tcp_clientconn;	//TCP CLIENT网络连接结构体
	u8 tcp_client_recvbuf[TCP_CLIENT_RX_BUFSIZE];	//TCP客户端接收数据缓冲区
	u8 tcp_client_sendbuf[TCP_CLIENT_TX_BUFSIZE];	//TCP客户端接收数据缓冲区
	u8 tcp_client_flag;
	u8 tcp_thread_open;       //1开启  0关闭 
}TCPCLIENTMESSAGE;	

INT8U tcp_client_init(void);  //tcp客户端初始化(创建tcp客户端线程)
u32 SendNetDataToServer(u8 *pData,u32 sendLen);
u32 GetNetDataForServer(u8 **pData,u32 *recvLen,u32_t timeout);
u32 SendNetDataToMirroringServer(u8 *pData,u32 sendLen);
u32 GetNetDataForMirroringServer(u8 **pData,u32 *recvLen,u32_t timeout);

void SetMirroringIp(u8 *pip);
void SetRemoteIp(u8 *pip);
void SetMirroringPort(u16 *pport);
void SetRemotePort(u16 *pport);

void PrintMirroringAddr(void);
void PrintRemoteAddr(void);

u8 getIpFlag(void);

TCPCLIENTMESSAGE *GetRemoteAddrHandle(void);
TCPCLIENTMESSAGE *GetMirroringAddrHandle(void);
#endif

