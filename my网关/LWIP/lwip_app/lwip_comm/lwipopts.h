#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

#include "includes.h"


#define SYS_LIGHTWEIGHT_PROT    1		//Ϊ1ʱʹ��ʵʱ����ϵͳ������������,�����ؼ����벻���жϴ��


#define ETHARP_TRUST_IP_MAC     0
#define IP_REASSEMBLY           0
#define IP_FRAG                 0
#define ARP_QUEUEING            0
#define TCP_LISTEN_BACKLOG      1

#define NO_SYS                  0  		//ʹ��UCOS����ϵͳ
#define MEM_ALIGNMENT           4  		//ʹ��4�ֽڶ���ģʽ
#define MEM_SIZE                (16000) 	//�ڴ��heap��С
#define MEMP_NUM_PBUF           20 	//MEMP_NUM_PBUF:memp�ṹ��pbuf����,���Ӧ�ô�ROM���߾�̬�洢�����ʹ�������ʱ,���ֵӦ�����ô�һ��
#define MEMP_NUM_UDP_PCB        6		//MEMP_NUM_UDP_PCB:UDPЭ����ƿ�(PCB)����.ÿ�����UDP"����"��Ҫһ��PCB.
#define MEMP_NUM_TCP_PCB        10		//MEMP_NUM_TCP_PCB:ͬʱ���������TCP����
#define MEMP_NUM_TCP_PCB_LISTEN 5		//MEMP_NUM_TCP_PCB_LISTEN:�ܹ�������TCP��������
#define MEMP_NUM_TCP_SEG        20		//MEMP_NUM_TCP_SEG:���ͬʱ�ڶ����е�TCP������
#define MEMP_NUM_SYS_TIMEOUT    10		//MEMP_NUM_SYS_TIMEOUT:�ܹ�ͬʱ�����timeout����

//pbufѡ��
#define PBUF_POOL_SIZE          20		//PBUF_POOL_SIZE:pbuf�ڴ�ظ���
#define PBUF_POOL_BUFSIZE       512		//PBUF_POOL_BUFSIZE:ÿ��pbuf�ڴ�ش�С

#define LWIP_TCP                1  		//ʹ��TCP
#define TCP_TTL                 255		//����ʱ��


#define TCP_QUEUE_OOSEQ         0 		//��TCP�����ݶγ�������ʱ�Ŀ���λ,���豸���ڴ��С��ʱ�����ӦΪ0
#define TCPIP_MBOX_SIZE         MAX_QUEUE_ENTRIES   //tcpip�������߳�ʱ����Ϣ�����С


#define TCP_MSS                 (1500 - 40)	  	//���TCP�ֶ�,TCP_MSS = (MTU - IP��ͷ��С - TCP��ͷ��С
#define TCP_SND_BUF             (2*TCP_MSS)		//TCP���ͻ�������С(bytes).
#define TCP_SND_QUEUELEN        (4* TCP_SND_BUF/TCP_MSS)	//TCP_SND_QUEUELEN: TCP���ͻ�������С(pbuf).���ֵ��СΪ(2 * TCP_SND_BUF/TCP_MSS)
#define TCP_WND                 (2*TCP_MSS)		//TCP���ʹ���
#define LWIP_ICMP               1 	//ʹ��ICMPЭ��
#define LWIP_DHCP               1	//ʹ��DHCP
#define LWIP_UDP                1 	//ʹ��UDP����
#define UDP_TTL                 255 //UDP���ݰ�����ʱ��
#define LWIP_STATS 0
#define LWIP_PROVIDE_ERRNO              1
#define LWIP_NETIF_LINK_CALLBACK        1



//֡У���ѡ�STM32F4x7����ͨ��Ӳ��ʶ��ͼ���IP,UDP��ICMP��֡У���
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



#define LWIP_NETCONN                    1 	//LWIP_NETCONN==1:ʹ��NETCON����(Ҫ��ʹ��api_lib.c)
#define LWIP_SOCKET                     1	//LWIP_SOCKET==1:ʹ��Sicket API(Ҫ��ʹ��sockets.c)
#define LWIP_COMPAT_MUTEX               1
#define LWIP_SO_RCVTIMEO                1 	//ͨ������LWIP_SO_RCVTIMEOʹ��netconn�ṹ����recv_timeout,ʹ��recv_timeout���Ա��������߳�

//�й�ϵͳ��ѡ��

#define DEFAULT_UDP_RECVMBOX_SIZE       2000
#define DEFAULT_TCP_RECVMBOX_SIZE       2000  
#define DEFAULT_ACCEPTMBOX_SIZE         2000  
#define DEFAULT_THREAD_STACKSIZE        512

//LWIP����ѡ��
#define LWIP_DEBUG                    	 0	 //�ر�DEBUGѡ��
#define ICMP_DEBUG                      LWIP_DBG_OFF //����/�ر�ICMPdebug

#endif /* __LWIPOPTS_H__ */

