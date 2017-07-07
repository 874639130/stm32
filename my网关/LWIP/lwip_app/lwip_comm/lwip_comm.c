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
__lwip_dev   lwipdev;						//lwip���ƽṹ�� 
struct netif lwip_netif;				//����һ��ȫ�ֵ�����ӿ�

#ifdef USE_DHCP 
__IO uint8_t DHCP_state;
#endif

extern u32 memp_get_memorysize(void);	//��memp.c���涨��
extern u8_t *memp_memory;				//��memp.c���涨��.
extern u8_t *ram_heap;					//��mem.c���涨��.


/////////////////////////////////////////////////////////////////////////////////
//lwip����������(�ں������DHCP����)
 
OS_STK * TCPIP_THREAD_TASK_STK;

OS_STK * LWIP_DHCP_TASK_STK;	//lwip DHCP����
//������
void lwip_dhcp_task(void *pdata); 


//������̫���жϵ���
//������̫���жϵ���
void lwip_packet_handler(void)
{
	ethernetif_input(&lwip_netif);
}
//lwip�ں˲���,�ڴ�����
//����ֵ:0,�ɹ�;
//    ����,ʧ��
u8 lwip_comm_mem_malloc(void)
{
	u32 mempsize;
	u32 ramheapsize; 
	mempsize=memp_get_memorysize();			//�õ�memp_memory�����С
	memp_memory=mymalloc(SRAMIN,mempsize);	//Ϊmemp_memory�����ڴ�
	ramheapsize=LWIP_MEM_ALIGN_SIZE(MEM_SIZE)+2*LWIP_MEM_ALIGN_SIZE(4*3)+MEM_ALIGNMENT;//�õ�ram heap��С
	ram_heap=mymalloc(SRAMIN,ramheapsize);	//Ϊram_heap�����ڴ�     
  TCPIP_THREAD_TASK_STK=mymalloc(SRAMIN,TCPIP_THREAD_STACKSIZE*4);//���ں����������ջ 
	LWIP_DHCP_TASK_STK=mymalloc(SRAMIN,LWIP_DHCP_STK_SIZE*4);		//��dhcp�����ջ�����ڴ�ռ�
	if(!memp_memory||!ram_heap||!TCPIP_THREAD_TASK_STK||!LWIP_DHCP_TASK_STK)//������ʧ�ܵ�
	{
		lwip_comm_mem_free();
		return 1;
	}
	return 0;	
}
//lwip�ں˲���,�ڴ��ͷ�
void lwip_comm_mem_free(void)
{ 	
	myfree(SRAMIN,memp_memory);
	myfree(SRAMIN,ram_heap);
    myfree(SRAMIN,TCPIP_THREAD_TASK_STK);
	myfree(SRAMIN,LWIP_DHCP_TASK_STK);
}
//lwip Ĭ��IP����
//lwipx:lwip���ƽṹ��ָ��
void lwip_comm_default_ip_set(__lwip_dev *lwipx)
{
	u32 sn0;
	sn0=*(vu32*)(0x1FFF7A10);//��ȡSTM32��ΨһID��ǰ24λ��ΪMAC��ַ�����ֽ�
	//Ĭ��Զ��IPΪ:192.168.1.100
	lwipx->remoteip[0]=192;	
	lwipx->remoteip[1]=168;
	lwipx->remoteip[2]=1;
	lwipx->remoteip[3]=102;
	//MAC��ַ����(�����ֽڹ̶�Ϊ:2.0.0,�����ֽ���STM32ΨһID)
	lwipx->mac[0]=2;//�����ֽ�(IEEE��֮Ϊ��֯ΨһID,OUI)��ַ�̶�Ϊ:2.0.0
	lwipx->mac[1]=0;
	lwipx->mac[2]=0;
	lwipx->mac[3]=(sn0>>16)&0XFF;//�����ֽ���STM32��ΨһID
	lwipx->mac[4]=(sn0>>8)&0XFFF;;
	lwipx->mac[5]=sn0&0XFF; 
//	//Ĭ�ϱ���IPΪ:192.168.1.30
//	lwipx->ip[0]=192;	
//	lwipx->ip[1]=168;
//	lwipx->ip[2]=1;
//	lwipx->ip[3]=30;
//	//Ĭ����������:255.255.255.0
//	lwipx->netmask[0]=255;	
//	lwipx->netmask[1]=255;
//	lwipx->netmask[2]=255;
//	lwipx->netmask[3]=0;
//	//Ĭ������:192.168.1.1
//	lwipx->gateway[0]=192;	
//	lwipx->gateway[1]=168;
//	lwipx->gateway[2]=10;
//	lwipx->gateway[3]=1;	
	lwipx->dhcpstatus=0;//û��DHCP	
} 

//LWIP��ʼ��(LWIP������ʱ��ʹ��)
//����ֵ:0,�ɹ�
//      1,�ڴ����
//      2,LAN8720��ʼ��ʧ��
//      3,�������ʧ��.
u8 lwip_comm_init(void)
{
	u8 retry=0;
	OS_CPU_SR cpu_sr;
	struct netif *Netif_Init_Flag;		//����netif_add()����ʱ�ķ���ֵ,�����ж������ʼ���Ƿ�ɹ�
	struct ip_addr ipaddr;  			//ip��ַ
	struct ip_addr netmask; 			//��������
	struct ip_addr gw;      			//Ĭ������ 
	
	if(ETH_Mem_Malloc())return 1;		//�ڴ�����ʧ��
	if(lwip_comm_mem_malloc())return 2;	//�ڴ�����ʧ��
	
	while(LAN8720_Init())		        //��ʼ��LAN8720,���ʧ�ܵĻ�������5��
	{
			retry++;
			if(retry>5) {retry=0;return 3;} //LAN8720��ʼ��ʧ��
	}
	
	tcpip_init(NULL,NULL);				//��ʼ��tcp ip�ں�,�ú�������ᴴ��tcpip_thread�ں�����
	lwip_comm_default_ip_set(&lwipdev);	//����Ĭ��IP����Ϣ
#if LWIP_DHCP		//ʹ�ö�̬IP
	ipaddr.addr = 0;
	netmask.addr = 0;
	gw.addr = 0;
#else				//ʹ�þ�̬IP
	IP4_ADDR(&ipaddr,lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
	IP4_ADDR(&netmask,lwipdev.netmask[0],lwipdev.netmask[1] ,lwipdev.netmask[2],lwipdev.netmask[3]);
	IP4_ADDR(&gw,lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
	MESSAGE("����en��MAC��ַΪ:................%d.%d.%d.%d.%d.%d\r\n",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);
	MESSAGE("��̬IP��ַ........................%d.%d.%d.%d\r\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
	MESSAGE("��������..........................%d.%d.%d.%d\r\n",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
	MESSAGE("Ĭ������..........................%d.%d.%d.%d\r\n",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
#endif
	Netif_Init_Flag=netif_add(&lwip_netif,&ipaddr,&netmask,&gw,NULL,&ethernetif_init,&tcpip_input);//�������б������һ������
	if(Netif_Init_Flag==NULL)return 4;//�������ʧ�� 
	else//������ӳɹ���,����netifΪĬ��ֵ,���Ҵ�netif����
	{
		netif_set_default(&lwip_netif); //����netifΪĬ������
		netif_set_up(&lwip_netif);		//��netif����
	}
	return 0;//����OK.
}   

void GetIpMessage(void)
{
  //if((lwipdev.dhcpstatus==1)||(lwipdev.dhcpstatus==0XFF))	
	{
		  MESSAGE("\r\n\n\n");
 			MESSAGE("MAC��ַΪ:....................%d.%d.%d.%d.%d.%d\r\n",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);
			MESSAGE("IP��ַ........................%d.%d.%d.%d\r\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
			MESSAGE("��������......................%d.%d.%d.%d\r\n",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
			MESSAGE("Ĭ������......................%d.%d.%d.%d\r\n\n\n",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
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
void setLocalIp(u8 *ip,u8* netmask)//���þ�̬ip
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
	
	//DataStore(LOCALIP);//�洢ip
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
   
	 IP4_ADDR(&ipaddr,remoteip[0],remoteip[1],remoteip[2],remoteip[3]);//Ŀ���ַ
	 ip_addr_set(&ping_pcb->remote_ip,&ipaddr);
	 
	 p=pbuf_alloc(PBUF_IP,sizeof(struct icmp_echo_hdr) + 8,PBUF_RAM);//����pbuf ICMP_DEST_UNREACH_DATASIZE
	 if(!p)     //����ʧ��
	 {
	   printf("\r\nping_send failed.\r\n");
	 }
	 if((p->len==p->tot_len)&&(p->next==NULL))//������Ч
	 {
	   //��ʽ������icmpͷ
	   iecho=(struct icmp_echo_hdr*)p->payload;
		 ICMPH_TYPE_SET(iecho,ICMP_ECHO); //����
	   ICMPH_CODE_SET(iecho,0);         //����
  	 iecho->id=htons(0x0200);            //��ʾ��
		 iecho->seqno=htons(ping_seq_num++);//���к�

     for(i=0;i<8;i++)//���data_len���ֽڵ���Ч���ݣ�Ĭ��32�ֽ�
		 {
			  ((char*)iecho)[sizeof(struct icmp_echo_hdr)+i] =(char)i;
		 }
		 iecho->chksum=0;                 //У�������
     iecho->chksum = inet_chksum(p->payload, p->len);//У���
		 raw_sendto(ping_pcb,p,&ping_pcb->remote_ip);//����	
     pbuf_free(p);
	 }  
	 return 0;
}



//���ʹ����DHCP
#if LWIP_DHCP
//����DHCP����
void lwip_comm_dhcp_creat(void)
{
	OS_CPU_SR cpu_sr;
	OS_ENTER_CRITICAL();  //�����ٽ���
	OSTaskCreate(lwip_dhcp_task,(void*)0,(OS_STK*)&LWIP_DHCP_TASK_STK[LWIP_DHCP_STK_SIZE-1],LWIP_DHCP_TASK_PRIO);//����DHCP���� 
	OS_EXIT_CRITICAL();  //�˳��ٽ���
}
//ɾ��DHCP����
void lwip_comm_dhcp_delete(void)
{
	dhcp_stop(&lwip_netif); 		//�ر�DHCP
	OSTaskDel(LWIP_DHCP_TASK_PRIO);	//ɾ��DHCP����
}
//DHCP��������
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
			dhcp_start(&lwip_netif);//����DHCP 
	    lwipdev.dhcpstatus=0;	//����DHCP
	    MESSAGE("���ڲ���DHCP������,���Ե�...........\r\n");   
			lwip_netif.ip_addr.addr=0;
			lwip_netif.netmask.addr=0;
			lwip_netif.gw.addr=0;
      autogetip=1;			
		}
		else if((EthStatus & ETH_LINK_FLAG)!=ETH_LINK_FLAG)netLinkstate=1;
		
		
		if(autogetip==1)
		{
			MESSAGE("��ȡ��̬��ַ��...\r\n");
			ip=0;
			ip=lwip_netif.ip_addr.addr;		//��ȡ��IP��ַ
			netmask=lwip_netif.netmask.addr;//��ȡ��������
			gw=lwip_netif.gw.addr;			//��ȡĬ������ 
			if(ip!=0)   					//����ȷ��ȡ��IP��ַ��ʱ��
			{
				lwipdev.dhcpstatus=1;	//DHCP�ɹ�
				MESSAGE("����en��MAC��ַΪ:................%d.%d.%d.%d.%d.%d\r\n",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);
				//������ͨ��DHCP��ȡ����IP��ַ
				lwipdev.ip[3]=(uint8_t)(ip>>24); 
				lwipdev.ip[2]=(uint8_t)(ip>>16);
				lwipdev.ip[1]=(uint8_t)(ip>>8);
				lwipdev.ip[0]=(uint8_t)(ip);
				MESSAGE("ͨ��DHCP��ȡ��IP��ַ..............%d.%d.%d.%d\r\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
				//����ͨ��DHCP��ȡ�������������ַ
				lwipdev.netmask[3]=(uint8_t)(netmask>>24);
				lwipdev.netmask[2]=(uint8_t)(netmask>>16);
				lwipdev.netmask[1]=(uint8_t)(netmask>>8);
				lwipdev.netmask[0]=(uint8_t)(netmask);
				MESSAGE("ͨ��DHCP��ȡ����������............%d.%d.%d.%d\r\n",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
				//������ͨ��DHCP��ȡ����Ĭ������
				lwipdev.gateway[3]=(uint8_t)(gw>>24);
				lwipdev.gateway[2]=(uint8_t)(gw>>16);
				lwipdev.gateway[1]=(uint8_t)(gw>>8);
				lwipdev.gateway[0]=(uint8_t)(gw);
				MESSAGE("ͨ��DHCP��ȡ����Ĭ������..........%d.%d.%d.%d\r\n",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
				OSFlagPost (semFlag, (OS_FLAGS)1, OS_FLAG_SET,&semFlagErr);
				OSTimeDly(500); //��ʱ250ms
				autogetip=0;
				dhcp_stop(&lwip_netif); 		//�ر�DHCP
			}
			else if(lwip_netif.dhcp->tries>LWIP_MAX_DHCP_TRIES) //ͨ��DHCP�����ȡIP��ַʧ��,�ҳ�������Դ���
			{  
				lwipdev.dhcpstatus=0XFF;//DHCPʧ��.
				//ʹ�þ�̬IP��ַ
				IP4_ADDR(&(lwip_netif.ip_addr),lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
				IP4_ADDR(&(lwip_netif.netmask),lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
				IP4_ADDR(&(lwip_netif.gw),lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
				MESSAGE("DHCP����ʱ,ʹ�þ�̬IP��ַ!\r\n");
				MESSAGE("����en��MAC��ַΪ:................%d.%d.%d.%d.%d.%d\r\n",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);
				MESSAGE("��̬IP��ַ........................%d.%d.%d.%d\r\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
				MESSAGE("��������..........................%d.%d.%d.%d\r\n",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
				MESSAGE("Ĭ������..........................%d.%d.%d.%d\r\n",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
				OSFlagPost (semFlag, (OS_FLAGS)1, OS_FLAG_SET,&semFlagErr);
				autogetip=0;
				dhcp_stop(&lwip_netif); 		//�ر�DHCP
				//break;
			}
		}
		OSTimeDly(500); //��ʱ250ms
	}
	lwip_comm_dhcp_delete();//ɾ��DHCP���� 
}
#endif 




//		if(((EthStatus & ETH_INIT_FLAG)!=ETH_INIT_FLAG)&&((EthStatus & ETH_LINK_FLAG)==ETH_LINK_FLAG))//��ʼ���Ƿ�ɹ�
//		{
//			MESSAGE("��ʼ��ʧ��!\n");
//			ETH_BSP_Config();
//			//lwip_comm_init();
//		}






















