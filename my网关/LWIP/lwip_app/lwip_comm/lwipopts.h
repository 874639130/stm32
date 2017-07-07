#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

#include "includes.h"


#define SYS_LIGHTWEIGHT_PROT    1		//为1时使用实时操作系统的轻量级保护,保护关键代码不被中断打断


#define ETHARP_TRUST_IP_MAC     0
#define IP_REASSEMBLY           0
#define IP_FRAG                 0
#define ARP_QUEUEING            0
#define TCP_LISTEN_BACKLOG      1

#define NO_SYS                  0  		//使用UCOS操作系统
#define MEM_ALIGNMENT           4  		//使用4字节对齐模式
#define MEM_SIZE                (16000) 	//内存堆heap大小
#define MEMP_NUM_PBUF           20 	//MEMP_NUM_PBUF:memp结构的pbuf数量,如果应用从ROM或者静态存储区发送大量数据时,这个值应该设置大一点
#define MEMP_NUM_UDP_PCB        6		//MEMP_NUM_UDP_PCB:UDP协议控制块(PCB)数量.每个活动的UDP"连接"需要一个PCB.
#define MEMP_NUM_TCP_PCB        10		//MEMP_NUM_TCP_PCB:同时建立激活的TCP数量
#define MEMP_NUM_TCP_PCB_LISTEN 5		//MEMP_NUM_TCP_PCB_LISTEN:能够监听的TCP连接数量
#define MEMP_NUM_TCP_SEG        20		//MEMP_NUM_TCP_SEG:最多同时在队列中的TCP段数量
#define MEMP_NUM_SYS_TIMEOUT    10		//MEMP_NUM_SYS_TIMEOUT:能够同时激活的timeout个数

//pbuf选项
#define PBUF_POOL_SIZE          20		//PBUF_POOL_SIZE:pbuf内存池个数
#define PBUF_POOL_BUFSIZE       512		//PBUF_POOL_BUFSIZE:每个pbuf内存池大小

#define LWIP_TCP                1  		//使用TCP
#define TCP_TTL                 255		//生存时间


#define TCP_QUEUE_OOSEQ         0 		//当TCP的数据段超出队列时的控制位,当设备的内存过小的时候此项应为0
#define TCPIP_MBOX_SIZE         MAX_QUEUE_ENTRIES   //tcpip创建主线程时的消息邮箱大小


#define TCP_MSS                 (1500 - 40)	  	//最大TCP分段,TCP_MSS = (MTU - IP报头大小 - TCP报头大小
#define TCP_SND_BUF             (2*TCP_MSS)		//TCP发送缓冲区大小(bytes).
#define TCP_SND_QUEUELEN        (4* TCP_SND_BUF/TCP_MSS)	//TCP_SND_QUEUELEN: TCP发送缓冲区大小(pbuf).这个值最小为(2 * TCP_SND_BUF/TCP_MSS)
#define TCP_WND                 (2*TCP_MSS)		//TCP发送窗口
#define LWIP_ICMP               1 	//使用ICMP协议
#define LWIP_DHCP               1	//使用DHCP
#define LWIP_UDP                1 	//使用UDP服务
#define UDP_TTL                 255 //UDP数据包生存时间
#define LWIP_STATS 0
#define LWIP_PROVIDE_ERRNO              1
#define LWIP_NETIF_LINK_CALLBACK        1



//帧校验和选项，STM32F4x7允许通过硬件识别和计算IP,UDP和ICMP的帧校验和
#define CHECKSUM_BY_HARDWARE 
#ifdef CHECKSUM_BY_HARDWARE
  /* CHECKSUM_GEN_IP==0: Generate checksums by hardware for outgoing IP packets.*/
  #define CHECKSUM_GEN_IP                 0
  /* CHECKSUM_GEN_UDP==0: Generate checksums by hardware for outgoing UDP packets.*/
  #define CHECKSUM_GEN_UDP                0
  /* CHECKSUM_GEN_TCP==0: Generate checksums by hardware for outgoing TCP packets.*/
  #define CHECKSUM_GEN_TCP                0 
  /* CHECKSUM_CHECK_IP==0: Check checksums by hardware for incoming IP packets.*/
  #define CHECKSUM_CHECK_IP               0
  /* CHECKSUM_CHECK_UDP==0: Check checksums by hardware for incoming UDP packets.*/
  #define CHECKSUM_CHECK_UDP              0
  /* CHECKSUM_CHECK_TCP==0: Check checksums by hardware for incoming TCP packets.*/
  #define CHECKSUM_CHECK_TCP              0
  /* CHECKSUM_CHECK_ICMP==0: Check checksums by hardware for incoming ICMP packets.*/
  #define CHECKSUM_GEN_ICMP               0
#else
  /* CHECKSUM_GEN_IP==1: Generate checksums in software for outgoing IP packets.*/
  #define CHECKSUM_GEN_IP                 1
  /* CHECKSUM_GEN_UDP==1: Generate checksums in software for outgoing UDP packets.*/
  #define CHECKSUM_GEN_UDP                1
  /* CHECKSUM_GEN_TCP==1: Generate checksums in software for outgoing TCP packets.*/
  #define CHECKSUM_GEN_TCP                1
  /* CHECKSUM_CHECK_IP==1: Check checksums in software for incoming IP packets.*/
  #define CHECKSUM_CHECK_IP               1
  /* CHECKSUM_CHECK_UDP==1: Check checksums in software for incoming UDP packets.*/
  #define CHECKSUM_CHECK_UDP              1
  /* CHECKSUM_CHECK_TCP==1: Check checksums in software for incoming TCP packets.*/
  #define CHECKSUM_CHECK_TCP              1
  /* CHECKSUM_CHECK_ICMP==1: Check checksums by hardware for incoming ICMP packets.*/
  #define CHECKSUM_GEN_ICMP               1
#endif



#define LWIP_NETCONN                    1 	//LWIP_NETCONN==1:使能NETCON函数(要求使用api_lib.c)
#define LWIP_SOCKET                     1	//LWIP_SOCKET==1:使能Sicket API(要求使用sockets.c)
#define LWIP_COMPAT_MUTEX               1
#define LWIP_SO_RCVTIMEO                1 	//通过定义LWIP_SO_RCVTIMEO使能netconn结构体中recv_timeout,使用recv_timeout可以避免阻塞线程

//有关系统的选项

#define DEFAULT_UDP_RECVMBOX_SIZE       2000
#define DEFAULT_TCP_RECVMBOX_SIZE       2000  
#define DEFAULT_ACCEPTMBOX_SIZE         2000  
#define DEFAULT_THREAD_STACKSIZE        512

//LWIP调试选项
#define LWIP_DEBUG                    	 0	 //关闭DEBUG选项
#define ICMP_DEBUG                      LWIP_DBG_OFF //开启/关闭ICMPdebug

#endif /* __LWIPOPTS_H__ */

