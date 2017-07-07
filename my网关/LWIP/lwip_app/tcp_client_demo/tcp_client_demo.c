#include "tcp_client_demo.h"
#include "lwip/opt.h"
#include "lwip_comm.h"
#include "lwip/lwip_sys.h"
#include "lwip/api.h"
#include "includes.h"
#include "httpd.h"

OS_STK TCPCLIENT_TASK_STK[TCPCLIENT_STK_SIZE];////�����ջ
OS_STK TCPCLIENTM_TASK_STK[TCPCLIENT_STK_SIZE];
OS_STK STK_SERVER_COMM_SEND[Server_Send_STACK_SIZE];
OS_STK STK_SERVER_COMM_RECVE[Server_Recve_STACK_SIZE];


static TCPCLIENTMESSAGE client,clientMirroring;

static u32_t GetNetData(TCPCLIENTMESSAGE *ptcpClientMessage,u8 **msg,u32 *recvLen, u32_t timeout)
{
  if(sys_arch_mbox_fetch(&(ptcpClientMessage->recvQ), (void **)msg,timeout)!=SYS_ARCH_TIMEOUT)
	{ 
			*recvLen=(u32)(((u8)(*msg)[0]<<24)|((u8)(*msg)[1]<<16)|((u8)(*msg)[2]<<8)|((u8)(*msg)[3]));
		  *msg= (u8*)(&(*msg)[4]);
		  return 0;
	}
	return 1;
}
static u8 netflag=0;
static u32_t SendNetData(TCPCLIENTMESSAGE *ptcpClientMessage,u8 *pData,u32 sendLen)
{
 //len=strlen((char *)pData);
	if((ptcpClientMessage->tcp_client_flag==0)||(netflag==1))return 1;
	memcpy(&ptcpClientMessage->tcp_client_sendbuf[4],pData,sendLen);
	ptcpClientMessage->tcp_client_sendbuf[0]=(u8)(sendLen>>24);
	ptcpClientMessage->tcp_client_sendbuf[1]=(u8)(sendLen>>16);
	ptcpClientMessage->tcp_client_sendbuf[2]=(u8)(sendLen>>8);
	ptcpClientMessage->tcp_client_sendbuf[3]=(u8)sendLen;
	if(ERR_OK!=sys_mbox_trypost(&(ptcpClientMessage->sendQ),(void *)ptcpClientMessage->tcp_client_sendbuf))return 1;
	return 0;
}
u32 SendNetDataToServer(u8 *pData,u32 sendLen)
{
  return SendNetData(&client,pData,sendLen);
}
u32 SendNetDataToMirroringServer(u8 *pData,u32 sendLen)
{
  return SendNetData(&clientMirroring,pData,sendLen);
}

u32 GetNetDataForServer(u8 **pData,u32 *recvLen,u32_t timeout)
{
	u8 *msg;
  if(!GetNetData(&client,(u8 **)&msg,recvLen,timeout))*pData=msg;//MESSAGE("%d %s\n",*recvLen,msg);
	else  return 1;
  return 0;
}
u32 GetNetDataForMirroringServer(u8 **pData,u32 *recvLen,u32_t timeout)
{
	u8 *msg;
  if(!GetNetData(&clientMirroring,(u8 **)&msg,recvLen,timeout))*pData=msg;//MESSAGE("%d %s\n",*recvLen,msg);
	else  return 1;
  return 0;
}

static void tcp_client_thread(void *arg)
{
	OS_CPU_SR cpu_sr;
	struct pbuf *q;
	err_t err,recv_err;
	static ip_addr_t server_ipaddr,loca_ipaddr;
	static u16_t 		 loca_port;
	TCPCLIENTMESSAGE *threadarg=(TCPCLIENTMESSAGE*)arg;
	threadarg->tcp_client_flag=0;
  OSFlagPend (semFlag, (OS_FLAGS)1, OS_FLAG_WAIT_SET_ALL, 0, &semFlagErr);//����ip�Ƿ��ȡ
	OSFlagPend (semFlag, (OS_FLAGS)2, OS_FLAG_WAIT_SET_ALL, 0, &semFlagErr);//��һ�����
	//IP4_ADDR(&server_ipaddr, threadarg->remoteip[0],threadarg->remoteip[1], threadarg->remoteip[2],threadarg->remoteip[3]);	
	while (1) 
	{ 
		while(threadarg->tcp_thread_open==0)OSTimeDly(100); //�ͻ��˹ر�
		IP4_ADDR(&server_ipaddr, threadarg->remoteip[0],threadarg->remoteip[1], threadarg->remoteip[2],threadarg->remoteip[3]);	
	  threadarg->tcp_clientconn = netconn_new(NETCONN_TCP);  //����һ��TCP����
		err = netconn_connect(threadarg->tcp_clientconn,&server_ipaddr,threadarg->remoteport);//���ӷ�����
		if(err != ERR_OK)
		{
			OSTimeDly(100); 
			netconn_delete(threadarg->tcp_clientconn); //����ֵ������ERR_OK,ɾ��tcp_clientconn����
		}
  	else if (err == ERR_OK)    //���������ӵ�����
		{ 
			struct netbuf *recvbuf;
			u8 *pSendBuf=NULL;
			u32 SendBufLen=0;
			u32 recvBuflen=0;
			u32 data_len = TCP_MESSAGE_HEAD;//ǰ�ĸ��ֽ�
			threadarg->tcp_clientconn->recv_timeout = 10;//���ճ�ʱʱ��
			netconn_getaddr(threadarg->tcp_clientconn,&loca_ipaddr,&loca_port,1); //��ȡ����IP����IP��ַ�Ͷ˿ں�
			MESSAGE("�����Ϸ�����%d.%d.%d.%d,�����˿ں�Ϊ:%d\r\n",threadarg->remoteip[0],threadarg->remoteip[1], threadarg->remoteip[2],threadarg->remoteip[3],loca_port);
			netflag=0;
			while(1)
			{
				threadarg->tcp_client_flag=1;
				if(sys_arch_mbox_fetch(&(threadarg->sendQ), (void **)&pSendBuf,1)!=SYS_ARCH_TIMEOUT)
				{
					SendBufLen=(u32)(((u8)pSendBuf[0]<<24)|((u8)pSendBuf[1]<<16)|((u8)pSendBuf[2]<<8)|((u8)pSendBuf[3]));
					if((EthStatus & ETH_LINK_FLAG)==ETH_LINK_FLAG)
					{
					  err = netconn_write(threadarg->tcp_clientconn ,&pSendBuf[TCP_MESSAGE_HEAD],SendBufLen,NETCONN_COPY); //��������
					  if(err != ERR_OK) 
					  {
						  MESSAGE("TCP�ں˷���ʧ��\r\n");break;
					  }
					}
				}
					
				if((recv_err = netconn_recv(threadarg->tcp_clientconn,&recvbuf)) == ERR_OK)  //���յ�����
				{	
					OS_ENTER_CRITICAL(); //���ж�
					memset(threadarg->tcp_client_recvbuf,0,TCP_CLIENT_RX_BUFSIZE);  //���ݽ��ջ���������
					for(q=recvbuf->p;q!=NULL;q=q->next)  //����������pbuf����
					{
						//�ж�Ҫ������TCP_CLIENT_RX_BUFSIZE�е������Ƿ����TCP_CLIENT_RX_BUFSIZE��ʣ��ռ䣬�������
						//�Ļ���ֻ����TCP_CLIENT_RX_BUFSIZE��ʣ�೤�ȵ����ݣ�����Ļ��Ϳ������е�����
						if(q->len > (TCP_CLIENT_RX_BUFSIZE-data_len)) memcpy(threadarg->tcp_client_recvbuf+data_len,q->payload,(TCP_CLIENT_RX_BUFSIZE-data_len));//��������
						else memcpy(threadarg->tcp_client_recvbuf+data_len,q->payload,q->len);
						data_len += q->len;  	
						if(data_len > TCP_CLIENT_RX_BUFSIZE) break; //����TCP�ͻ��˽�������,����	
					}
					OS_EXIT_CRITICAL();  //���ж�
					recvBuflen=data_len-TCP_MESSAGE_HEAD;
					data_len=TCP_MESSAGE_HEAD;  //������ɺ�data_lenҪ���㡣			
					threadarg->tcp_client_recvbuf[0]=(u8)(recvBuflen>>24);
					threadarg->tcp_client_recvbuf[1]=(u8)(recvBuflen>>16);
					threadarg->tcp_client_recvbuf[2]=(u8)(recvBuflen>>8);
					threadarg->tcp_client_recvbuf[3]=(u8)recvBuflen;	
          sys_mbox_trypost(&(threadarg->recvQ),(void *)threadarg->tcp_client_recvbuf);	    
					netbuf_delete(recvbuf);
				}
				else if(recv_err == ERR_CLSD)  //�ر�����
				{
					netconn_close(threadarg->tcp_clientconn);
					netconn_delete(threadarg->tcp_clientconn);
					threadarg->tcp_client_flag=0;
					MESSAGE("������%d.%d.%d.%d  %d�Ͽ�����\r\n",threadarg->remoteip[0],threadarg->remoteip[1], threadarg->remoteip[2],threadarg->remoteip[3],threadarg->remoteport);
					break;
				}
				if(netflag==1){//�����ر�����
				  netconn_close(threadarg->tcp_clientconn);
					netconn_delete(threadarg->tcp_clientconn);
					threadarg->tcp_client_flag=0;
					MESSAGE("�Ͽ��������%d.%d.%d.%d  %d����\r\n",threadarg->remoteip[0],threadarg->remoteip[1], threadarg->remoteip[2],threadarg->remoteip[3],threadarg->remoteport);
				  break;
				}
			}
		}
	}
}


INT8U tcp_client_init(void)
{
	INT8U res;
	OS_CPU_SR cpu_sr;
	
	OS_ENTER_CRITICAL();	//���ж�	
	
	if(sys_mbox_new(&(client.recvQ) , MAX_QUEUE_ENTRIES)!=ERR_OK){MESSAGE("client.recvQ\n");}
	if(sys_mbox_new(&(client.sendQ) , MAX_QUEUE_ENTRIES)!=ERR_OK){MESSAGE("client.sendQ\n");}
	res = OSTaskCreate(tcp_client_thread,(void*)&client,(OS_STK*)&TCPCLIENT_TASK_STK[TCPCLIENT_STK_SIZE-1],TCPCLIENT_PRIO);
	
	
	if(sys_mbox_new(&(clientMirroring.recvQ) , MAX_QUEUE_ENTRIES)!=ERR_OK){MESSAGE("clientMirroring.recvQ\n");}
	if(sys_mbox_new(&(clientMirroring.sendQ) , MAX_QUEUE_ENTRIES)!=ERR_OK){MESSAGE("clientMirroring.sendQ\n");}
	res = OSTaskCreate(tcp_client_thread,(void*)&clientMirroring,(OS_STK*)&TCPCLIENTM_TASK_STK[TCPCLIENTM_STK_SIZE-1],TCPCLIENTM_PRIO);
	 
	httpd_init(); 
	OSTaskCreate(Server_Comm_Send_Task,(void*)0,	(OS_STK* )&STK_SERVER_COMM_SEND[Server_Send_STACK_SIZE - 1],SERVERSEND_PRIO);
	OSTaskCreate(Server_Comm_Recve_Task,(void*)0,	(OS_STK* )&STK_SERVER_COMM_RECVE[Server_Recve_STACK_SIZE - 1],SERVERRECVE_PRIO);
	
	OS_EXIT_CRITICAL();		//���ж�
	
	return res;
}

void SetMirroringIp(u8 *pip)
{
	u8 i=0;
	MESSAGE("rmip:%d.%d.%d.%d\n",pip[0],pip[1],pip[2],pip[3]);	
	for(i=0;i<4;i++)
	{
		if(clientMirroring.remoteip[i]!=pip[i])netflag=1;
    clientMirroring.remoteip[i]=pip[i];
  }
}
void SetRemoteIp(u8 *pip)
{
  u8 i=0;
	MESSAGE("rip:%d.%d.%d.%d\n",pip[0],pip[1],pip[2],pip[3]);	
	for(i=0;i<4;i++)
	{
		if(client.remoteip[i]!=pip[i])netflag=1;
    client.remoteip[i]=pip[i];
  }
}
void SetMirroringPort(u16 *pport)
{
	if(*pport==clientMirroring.remoteport)return;
	MESSAGE("rmport:%d\n",*pport);
  clientMirroring.remoteport=*pport;
	netflag=1;
}
void SetRemotePort(u16 *pport)
{
	if(*pport==client.remoteport)return;
	MESSAGE("rport:%d\n",*pport);
  client.remoteport=*pport;
	netflag=1;
}	


void PrintRemoteAddr(void)
{
  MESSAGE("rip:%d.%d.%d.%d rport:%d\r\n",client.remoteip[0],client.remoteip[1],
	client.remoteip[2],client.remoteip[3],client.remoteport);
}	
void PrintMirroringAddr(void)
{
  MESSAGE("rmip:%d.%d.%d.%d rmport:%d\r\n",clientMirroring.remoteip[0],clientMirroring.remoteip[1],
	clientMirroring.remoteip[2],clientMirroring.remoteip[3],clientMirroring.remoteport);
}

TCPCLIENTMESSAGE *GetRemoteAddrHandle(void){
  return &client;
}
TCPCLIENTMESSAGE *GetMirroringAddrHandle(void){
  return &clientMirroring;
}
static u8 ipflag=0;
void WaitNetReady(){
 OSFlagPend (semFlag, (OS_FLAGS)1, OS_FLAG_WAIT_SET_ALL, 0, &semFlagErr);//�Ƿ����ñ���ip
 OSFlagPend (semFlag, (OS_FLAGS)2, OS_FLAG_WAIT_SET_ALL, 0, &semFlagErr);//�Ƿ�����Զ��ip
 ipflag=1;//ip���� 
}

u8 getIpFlag(void){
  return ipflag;
}

void NetReady(){               //�Ѿ�����Զ��ip
 OSFlagPost (semFlag, (OS_FLAGS)2, OS_FLAG_SET,&semFlagErr);
}

