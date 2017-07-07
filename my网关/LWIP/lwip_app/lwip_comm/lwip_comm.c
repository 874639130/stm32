#include "lwip_comm.h" 
#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "ethernetif.h" 
#include "lwip/timers.h"
#include "lwip/tcp_impl.h"
#include "lwip/ip_frag.h"
#include "lwip/tcpip.h" 
#include "lan8720.h"
#include "includes.h"
#include "tcp_client_demo.h"
#include "lwip/raw.h"
#include "lwip/inet_chksum.h"


#define GET_PHY_LINK_STATUS()		(ETH_ReadPHYRegister(LAN8720_PHY_ADDRESS, PHY_BSR) & 0x00000004)
uint16_t NET_LINK_STATUS(void)
{
 return  GET_PHY_LINK_STATUS();
}
__IO uint32_t  EthStatus = 0;

////////////////////////////////////////////////////////////////////////////////// 	   
__IO u8 lwipCoreErrReboot=1;   
__lwip_dev   lwipdev;						//lwip控制结构体 
struct netif lwip_netif;				//定义一个全局的网络接口

#ifdef USE_DHCP 
__IO uint8_t DHCP_state;
#endif

extern u32 memp_get_memorysize(void);	//在memp.c里面定义
extern u8_t *memp_memory;				//在memp.c里面定义.
extern u8_t *ram_heap;					//在mem.c里面定义.


/////////////////////////////////////////////////////////////////////////////////
//lwip两个任务定义(内核任务和DHCP任务)
 
OS_STK * TCPIP_THREAD_TASK_STK;

OS_STK * LWIP_DHCP_TASK_STK;	//lwip DHCP任务
//任务函数
void lwip_dhcp_task(void *pdata); 


//用于以太网中断调用
//用于以太网中断调用
void lwip_packet_handler(void)
{
	ethernetif_input(&lwip_netif);
}
//lwip内核部分,内存申请
//返回值:0,成功;
//    其他,失败
u8 lwip_comm_mem_malloc(void)
{
	u32 mempsize;
	u32 ramheapsize; 
	mempsize=memp_get_memorysize();			//得到memp_memory数组大小
	memp_memory=mymalloc(SRAMIN,mempsize);	//为memp_memory申请内存
	ramheapsize=LWIP_MEM_ALIGN_SIZE(MEM_SIZE)+2*LWIP_MEM_ALIGN_SIZE(4*3)+MEM_ALIGNMENT;//得到ram heap大小
	ram_heap=mymalloc(SRAMIN,ramheapsize);	//为ram_heap申请内存     
  TCPIP_THREAD_TASK_STK=mymalloc(SRAMIN,TCPIP_THREAD_STACKSIZE*4);//给内核任务申请堆栈 
	LWIP_DHCP_TASK_STK=mymalloc(SRAMIN,LWIP_DHCP_STK_SIZE*4);		//给dhcp任务堆栈申请内存空间
	if(!memp_memory||!ram_heap||!TCPIP_THREAD_TASK_STK||!LWIP_DHCP_TASK_STK)//有申请失败的
	{
		lwip_comm_mem_free();
		return 1;
	}
	return 0;	
}
//lwip内核部分,内存释放
void lwip_comm_mem_free(void)
{ 	
	myfree(SRAMIN,memp_memory);
	myfree(SRAMIN,ram_heap);
    myfree(SRAMIN,TCPIP_THREAD_TASK_STK);
	myfree(SRAMIN,LWIP_DHCP_TASK_STK);
}
//lwip 默认IP设置
//lwipx:lwip控制结构体指针
void lwip_comm_default_ip_set(__lwip_dev *lwipx)
{
	u32 sn0;
	sn0=*(vu32*)(0x1FFF7A10);//获取STM32的唯一ID的前24位作为MAC地址后三字节
	//默认远端IP为:192.168.1.100
	lwipx->remoteip[0]=192;	
	lwipx->remoteip[1]=168;
	lwipx->remoteip[2]=1;
	lwipx->remoteip[3]=102;
	//MAC地址设置(高三字节固定为:2.0.0,低三字节用STM32唯一ID)
	lwipx->mac[0]=2;//高三字节(IEEE称之为组织唯一ID,OUI)地址固定为:2.0.0
	lwipx->mac[1]=0;
	lwipx->mac[2]=0;
	lwipx->mac[3]=(sn0>>16)&0XFF;//低三字节用STM32的唯一ID
	lwipx->mac[4]=(sn0>>8)&0XFFF;;
	lwipx->mac[5]=sn0&0XFF; 
//	//默认本地IP为:192.168.1.30
//	lwipx->ip[0]=192;	
//	lwipx->ip[1]=168;
//	lwipx->ip[2]=1;
//	lwipx->ip[3]=30;
//	//默认子网掩码:255.255.255.0
//	lwipx->netmask[0]=255;	
//	lwipx->netmask[1]=255;
//	lwipx->netmask[2]=255;
//	lwipx->netmask[3]=0;
//	//默认网关:192.168.1.1
//	lwipx->gateway[0]=192;	
//	lwipx->gateway[1]=168;
//	lwipx->gateway[2]=10;
//	lwipx->gateway[3]=1;	
	lwipx->dhcpstatus=0;//没有DHCP	
} 

//LWIP初始化(LWIP启动的时候使用)
//返回值:0,成功
//      1,内存错误
//      2,LAN8720初始化失败
//      3,网卡添加失败.
u8 lwip_comm_init(void)
{
	u8 retry=0;
	OS_CPU_SR cpu_sr;
	struct netif *Netif_Init_Flag;		//调用netif_add()函数时的返回值,用于判断网络初始化是否成功
	struct ip_addr ipaddr;  			//ip地址
	struct ip_addr netmask; 			//子网掩码
	struct ip_addr gw;      			//默认网关 
	
	if(ETH_Mem_Malloc())return 1;		//内存申请失败
	if(lwip_comm_mem_malloc())return 2;	//内存申请失败
	
	while(LAN8720_Init())		        //初始化LAN8720,如果失败的话就重试5次
	{
			retry++;
			if(retry>5) {retry=0;return 3;} //LAN8720初始化失败
	}
	
	tcpip_init(NULL,NULL);				//初始化tcp ip内核,该函数里面会创建tcpip_thread内核任务
	lwip_comm_default_ip_set(&lwipdev);	//设置默认IP等信息
#if LWIP_DHCP		//使用动态IP
	ipaddr.addr = 0;
	netmask.addr = 0;
	gw.addr = 0;
#else				//使用静态IP
	IP4_ADDR(&ipaddr,lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
	IP4_ADDR(&netmask,lwipdev.netmask[0],lwipdev.netmask[1] ,lwipdev.netmask[2],lwipdev.netmask[3]);
	IP4_ADDR(&gw,lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
	MESSAGE("网卡en的MAC地址为:................%d.%d.%d.%d.%d.%d\r\n",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);
	MESSAGE("静态IP地址........................%d.%d.%d.%d\r\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
	MESSAGE("子网掩码..........................%d.%d.%d.%d\r\n",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
	MESSAGE("默认网关..........................%d.%d.%d.%d\r\n",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
#endif
	Netif_Init_Flag=netif_add(&lwip_netif,&ipaddr,&netmask,&gw,NULL,&ethernetif_init,&tcpip_input);//向网卡列表中添加一个网口
	if(Netif_Init_Flag==NULL)return 4;//网卡添加失败 
	else//网口添加成功后,设置netif为默认值,并且打开netif网口
	{
		netif_set_default(&lwip_netif); //设置netif为默认网口
		netif_set_up(&lwip_netif);		//打开netif网口
	}
	return 0;//操作OK.
}   

void GetIpMessage(void)
{
  //if((lwipdev.dhcpstatus==1)||(lwipdev.dhcpstatus==0XFF))	
	{
		  MESSAGE("\r\n\n\n");
 			MESSAGE("MAC地址为:....................%d.%d.%d.%d.%d.%d\r\n",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);
			MESSAGE("IP地址........................%d.%d.%d.%d\r\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
			MESSAGE("子网掩码......................%d.%d.%d.%d\r\n",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
			MESSAGE("默认网关......................%d.%d.%d.%d\r\n\n\n",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
	    PrintMirroringAddr();
      PrintRemoteAddr();
	}
}

u8* getLocalMac(void){
  return lwipdev.mac;
}
u8* getLocalIp(void){
	return lwipdev.ip;
}
u8* getLocalNetmask(void){
	return lwipdev.netmask;
}
void setLocalIp(u8 *ip,u8* netmask)//设置静态ip
{
	
	lwipdev.ip[0]=lwipdev.gateway[0]=ip[0];
	lwipdev.ip[1]=lwipdev.gateway[1]=ip[1];
	lwipdev.ip[2]=lwipdev.gateway[2]=ip[2];
	lwipdev.ip[3]=ip[3];
	
	lwipdev.gateway[3]=1;
	
	lwipdev.netmask[0]=netmask[0];
	lwipdev.netmask[1]=netmask[1];
	lwipdev.netmask[2]=netmask[2];
	lwipdev.netmask[3]=netmask[3];
	
  IP4_ADDR(&(lwip_netif.ip_addr),lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
	IP4_ADDR(&(lwip_netif.netmask),lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
	IP4_ADDR(&(lwip_netif.gw),lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
	
	//DataStore(LOCALIP);//存储ip
	//DataStore(LOCALNETMASK);
	
	OSFlagPost (semFlag, (OS_FLAGS)1, OS_FLAG_SET,&semFlagErr);
}



int PingSend(u8 remoteip[])
{
	 static ip_addr_t ipaddr;
	 struct raw_pcb *ping_pcb;
	 struct pbuf *p;
	 struct icmp_echo_hdr *iecho; 
	 int i=0;
   u16_t ping_seq_num=0x5800;
	 
   ping_pcb=raw_new(IP_PROTO_ICMP);
   if(!ping_pcb)return 1;
   IP4_ADDR(&ipaddr,lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
	 ip_addr_set(&ping_pcb->local_ip,&ipaddr);
   
	 IP4_ADDR(&ipaddr,remoteip[0],remoteip[1],remoteip[2],remoteip[3]);//目标地址
	 ip_addr_set(&ping_pcb->remote_ip,&ipaddr);
	 
	 p=pbuf_alloc(PBUF_IP,sizeof(struct icmp_echo_hdr) + 8,PBUF_RAM);//申请pbuf ICMP_DEST_UNREACH_DATASIZE
	 if(!p)     //申请失败
	 {
	   printf("\r\nping_send failed.\r\n");
	 }
	 if((p->len==p->tot_len)&&(p->next==NULL))//数据有效
	 {
	   //格式话数据icmp头
	   iecho=(struct icmp_echo_hdr*)p->payload;
		 ICMPH_TYPE_SET(iecho,ICMP_ECHO); //类型
	   ICMPH_CODE_SET(iecho,0);         //代码
  	 iecho->id=htons(0x0200);            //标示符
		 iecho->seqno=htons(ping_seq_num++);//序列号

     for(i=0;i<8;i++)//填充data_len个字节的无效数据，默认32字节
		 {
			  ((char*)iecho)[sizeof(struct icmp_echo_hdr)+i] =(char)i;
		 }
		 iecho->chksum=0;                 //校验和清零
     iecho->chksum = inet_chksum(p->payload, p->len);//校验和
		 raw_sendto(ping_pcb,p,&ping_pcb->remote_ip);//发送	
     pbuf_free(p);
	 }  
	 return 0;
}



//如果使能了DHCP
#if LWIP_DHCP
//创建DHCP任务
void lwip_comm_dhcp_creat(void)
{
	OS_CPU_SR cpu_sr;
	OS_ENTER_CRITICAL();  //进入临界区
	OSTaskCreate(lwip_dhcp_task,(void*)0,(OS_STK*)&LWIP_DHCP_TASK_STK[LWIP_DHCP_STK_SIZE-1],LWIP_DHCP_TASK_PRIO);//创建DHCP任务 
	OS_EXIT_CRITICAL();  //退出临界区
}
//删除DHCP任务
void lwip_comm_dhcp_delete(void)
{
	dhcp_stop(&lwip_netif); 		//关闭DHCP
	OSTaskDel(LWIP_DHCP_TASK_PRIO);	//删除DHCP任务
}
//DHCP处理任务
void lwip_dhcp_task(void *pdata)
{
	u32 ip=0,netmask=0,gw=0;
	u8 netLinkstate=1;
	u8 autogetip=0;
	while(1)
	{ 
		if(NET_LINK_STATUS())EthStatus |= ETH_LINK_FLAG;
		else                 EthStatus &= ~ETH_LINK_FLAG;
		
		if(((EthStatus & ETH_LINK_FLAG)==ETH_LINK_FLAG)&&(netLinkstate==1))
		{
		  netLinkstate=0; 
			dhcp_start(&lwip_netif);//开启DHCP 
	    lwipdev.dhcpstatus=0;	//正在DHCP
	    MESSAGE("正在查找DHCP服务器,请稍等...........\r\n");   
			lwip_netif.ip_addr.addr=0;
			lwip_netif.netmask.addr=0;
			lwip_netif.gw.addr=0;
      autogetip=1;			
		}
		else if((EthStatus & ETH_LINK_FLAG)!=ETH_LINK_FLAG)netLinkstate=1;
		
		
		if(autogetip==1)
		{
			MESSAGE("获取动态地址中...\r\n");
			ip=0;
			ip=lwip_netif.ip_addr.addr;		//读取新IP地址
			netmask=lwip_netif.netmask.addr;//读取子网掩码
			gw=lwip_netif.gw.addr;			//读取默认网关 
			if(ip!=0)   					//当正确读取到IP地址的时候
			{
				lwipdev.dhcpstatus=1;	//DHCP成功
				MESSAGE("网卡en的MAC地址为:................%d.%d.%d.%d.%d.%d\r\n",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);
				//解析出通过DHCP获取到的IP地址
				lwipdev.ip[3]=(uint8_t)(ip>>24); 
				lwipdev.ip[2]=(uint8_t)(ip>>16);
				lwipdev.ip[1]=(uint8_t)(ip>>8);
				lwipdev.ip[0]=(uint8_t)(ip);
				MESSAGE("通过DHCP获取到IP地址..............%d.%d.%d.%d\r\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
				//解析通过DHCP获取到的子网掩码地址
				lwipdev.netmask[3]=(uint8_t)(netmask>>24);
				lwipdev.netmask[2]=(uint8_t)(netmask>>16);
				lwipdev.netmask[1]=(uint8_t)(netmask>>8);
				lwipdev.netmask[0]=(uint8_t)(netmask);
				MESSAGE("通过DHCP获取到子网掩码............%d.%d.%d.%d\r\n",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
				//解析出通过DHCP获取到的默认网关
				lwipdev.gateway[3]=(uint8_t)(gw>>24);
				lwipdev.gateway[2]=(uint8_t)(gw>>16);
				lwipdev.gateway[1]=(uint8_t)(gw>>8);
				lwipdev.gateway[0]=(uint8_t)(gw);
				MESSAGE("通过DHCP获取到的默认网关..........%d.%d.%d.%d\r\n",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
				OSFlagPost (semFlag, (OS_FLAGS)1, OS_FLAG_SET,&semFlagErr);
				OSTimeDly(500); //延时250ms
				autogetip=0;
				dhcp_stop(&lwip_netif); 		//关闭DHCP
			}
			else if(lwip_netif.dhcp->tries>LWIP_MAX_DHCP_TRIES) //通过DHCP服务获取IP地址失败,且超过最大尝试次数
			{  
				lwipdev.dhcpstatus=0XFF;//DHCP失败.
				//使用静态IP地址
				IP4_ADDR(&(lwip_netif.ip_addr),lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
				IP4_ADDR(&(lwip_netif.netmask),lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
				IP4_ADDR(&(lwip_netif.gw),lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
				MESSAGE("DHCP服务超时,使用静态IP地址!\r\n");
				MESSAGE("网卡en的MAC地址为:................%d.%d.%d.%d.%d.%d\r\n",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);
				MESSAGE("静态IP地址........................%d.%d.%d.%d\r\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
				MESSAGE("子网掩码..........................%d.%d.%d.%d\r\n",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
				MESSAGE("默认网关..........................%d.%d.%d.%d\r\n",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
				OSFlagPost (semFlag, (OS_FLAGS)1, OS_FLAG_SET,&semFlagErr);
				autogetip=0;
				dhcp_stop(&lwip_netif); 		//关闭DHCP
				//break;
			}
		}
		OSTimeDly(500); //延时250ms
	}
	lwip_comm_dhcp_delete();//删除DHCP任务 
}
#endif 




//		if(((EthStatus & ETH_INIT_FLAG)!=ETH_INIT_FLAG)&&((EthStatus & ETH_LINK_FLAG)==ETH_LINK_FLAG))//初始化是否成功
//		{
//			MESSAGE("初始化失败!\n");
//			ETH_BSP_Config();
//			//lwip_comm_init();
//		}






















