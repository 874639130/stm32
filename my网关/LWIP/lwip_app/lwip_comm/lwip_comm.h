#ifndef _LWIP_COMM_H
#define _LWIP_COMM_H 
#include "lan8720.h"  
							  
//*******************************************************************************
//修改信息
//无
////////////////////////////////////////////////////////////////////////////////// 	   
 
#define ETH_LINK_FLAG           0x10 /* Ethernet Link Flag */ 
uint16_t NET_LINK_STATUS(void);  //检测网络链接状态 
extern __IO uint32_t  EthStatus; 
 
#define USE_DHCP       /* enable DHCP, if disabled static address is used */ 
 
 /* DHCP状态 */
#define DHCP_START                 1
#define DHCP_WAIT_ADDRESS          2
#define DHCP_ADDRESS_ASSIGNED      3
#define DHCP_TIMEOUT               4
#define DHCP_LINK_DOWN             5
 
#define LWIP_MAX_DHCP_TRIES		4   //DHCP服务器最大重试次数

/* 调试信息输出 */  
#define SERIAL_DEBUG 
///* 远端IP地址和端口 */
#define DEST_IP_ADDR0               192
#define DEST_IP_ADDR1               168
#define DEST_IP_ADDR2                10
#define DEST_IP_ADDR3               104
#define DEST_PORT                  10000

#define DESTMIRRORINT_PORT         10001



/* MAC地址：网卡地址 */
#define MAC_ADDR0                     2
#define MAC_ADDR1                     0
#define MAC_ADDR2                     0
#define MAC_ADDR3                     0
#define MAC_ADDR4                     0
#define MAC_ADDR5                     0

/*静态IP地址 */
#define IP_ADDR0                    192
#define IP_ADDR1                    168
#define IP_ADDR2                      1
#define IP_ADDR3                    122

/* 子网掩码 */
#define NETMASK_ADDR0               255
#define NETMASK_ADDR1               255
#define NETMASK_ADDR2               255
#define NETMASK_ADDR3                 0

/* 网关 */
#define GW_ADDR0                    192
#define GW_ADDR1                    168
#define GW_ADDR2                      1
#define GW_ADDR3                      1


/* 检测PHY链路状态的实际间隔(单位：ms) */
#ifndef LINK_TIMER_INTERVAL
#define LINK_TIMER_INTERVAL        1000
#endif

/* MII and RMII mode selection ***********/
#define RMII_MODE  
//#define MII_MODE

/* 在MII模式时，使能MCO引脚输出25MHz脉冲 */
#ifdef 	MII_MODE
 #define PHY_CLOCK_MCO
#endif

//lwip控制结构体
typedef struct  
{
	u8 mac[6];      //MAC地址
	u8 remoteip[4];	//远端主机IP地址 
	u8 ip[4];       //本机IP地址
	u8 netmask[4]; 	//子网掩码
	u8 gateway[4]; 	//默认网关的IP地址
	
	vu8 dhcpstatus;	//dhcp状态 
					//0,未获取DHCP地址;
					//1,成功获取DHCP地址
					//0XFF,获取失败.
}__lwip_dev;
extern __lwip_dev lwipdev;	//lwip控制结构体
extern struct netif lwip_netif;				//定义一个全局的网络接口 


//void lwip_pkt_handle(void);
void GetIpMessage(void);
void lwip_comm_default_ip_set(__lwip_dev *lwipx);
u8 lwip_comm_mem_malloc(void);
void lwip_comm_mem_free(void);
u8 lwip_comm_init(void);
void lwip_comm_dhcp_creat(void);
void lwip_comm_dhcp_delete(void);
void lwip_comm_destroy(void);
void lwip_comm_delete_next_timeout(void);

u8* getLocalIp(void);
u8* getLocalMac(void);
u8* getLocalNetmask(void);
void setLocalIp(u8 *ip,u8* netmask);
#endif













