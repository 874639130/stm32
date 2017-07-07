#include "includes.h"
#include "lwip_comm.h"
#include "tcp_client_demo.h"



static u8 configIpFlag=0;
static u16 addtemp[4]={0};
static void strToIArray(char *cmd)
{
  int i=0,ipbit=0;
	char *pch=NULL;
	memset(addtemp,0,sizeof(addtemp));
	for(i=0,pch=cmd;i<strlen(cmd);i++,pch++)
	{
		 if(('0'<=(*pch))&&((*pch)<='9'))
		 {
				addtemp[ipbit]*=10;
				addtemp[ipbit]+=((*pch)-0x30);
		 }
		 else if((*pch)=='.')
		 {
				ipbit++;
		 }
		 else 
		 {
			 MESSAGE("\r\n*************命令格式错误***************\r\n");
			 return;
		 }
	}
	//MESSAGE("%d.%d.%d.%d\n",addtemp[0],addtemp[1],addtemp[2],addtemp[3]);	
}

static void IpDataChange(u8 *parr)
{
	u8 i=0;
	for(i=0;i<4;i++)
   parr[i]=(u8)addtemp[i];
}

static void ConfigIp(char *cmd)
{
	  u8 temp[4];
		if(strncmp(cmd,RIP,4)==0)
		{
		   strToIArray(&cmd[4]);
			 IpDataChange(temp);
			 SetRemoteIp(temp);     //设置ip
			 DataStore(RIP);
		}
		else if(strncmp(cmd,RPORT,6)==0)
		{
		   strToIArray(&cmd[6]);
			 SetRemotePort(addtemp);//设置端口
			 DataStore(RPORT);
		}
		else if(strncmp(cmd,RMIP,5)==0)
		{
		   strToIArray(&cmd[5]);
			 IpDataChange(temp);
			 SetMirroringIp(temp);
			 DataStore(RMIP);
		}
		else if(strncmp(cmd,RMPORT,7)==0)
		{
		   strToIArray(&cmd[7]);
			 SetMirroringPort(addtemp);
			 DataStore(RMPORT);
		}
		else if(strncmp(cmd,"exit",4)==0)	
		{configIpFlag=0;MESSAGE("\r\n*************退出集中器网络配置模式***************\r\n");}
		else MESSAGE("\r\n*************命令格式错误***************\r\n");	
}
static volatile u8 debug485=0;
static volatile u8 debugDICI=0;
static void CmdDeal(char *cmd)
{
	if((configIpFlag==1)&&(DEBUGMESSAGE==1))	{ConfigIp(cmd);return;}

	if(strncmp(cmd,"DEBUGMESSAGE_OPEN",17)==0){ 
		 DEBUGMESSAGE=1;MESSAGE("\r\n********debug message open success!!*********\n");
		 //DataStore("DEBUG"); 
	}
	else if(strncmp(cmd,"DEBUGMESSAGE_CLOSE",17)==0){
		 MESSAGE("\r\n***********debug message closed !!*********\n");DEBUGMESSAGE=0;
		 //DataStore("DEBUG");
	}
	else if(strncmp(cmd,"lwiprebootopen",14)==0)
	{lwipCoreErrReboot=1;MESSAGE("\r\nlwipCoreErrReboot open\r\n");}
	else if(strncmp(cmd,"lwiprebootclose",15)==0)
	{lwipCoreErrReboot=0;MESSAGE("\r\nlwipCoreErrReboot close\r\n");}
	else if((strncmp(cmd,"ifconfig",8)==0)&&(DEBUGMESSAGE==1)){GetIpMessage();}
	else if(strncmp(cmd,"sysreboot",9)==0){SCB->AIRCR =0X05FA0000|(u32)0x04;	} 
	else if(strncmp(cmd,"id:",3)==0){u8 *pid=DI_Read();MESSAGE("\r\n device id :%d \r\n",*pid);}
	else if(strncmp(cmd,"ENdebug485",8)==0){debug485=1;MESSAGE("ENdebug485");}
	else if(strncmp(cmd,"NOdebug485",8)==0){debug485=0;MESSAGE("NOdebug485");}
	else if(strncmp(cmd,"NOdebugDICI",11)==0){debugDICI=0;MESSAGE("NOdebugDICI");}
	else if(strncmp(cmd,"ENdebugDICI",11)==0){debugDICI=1;MESSAGE("ENdebugDICI");}	
	else if(strncmp(cmd,"configip",8)==0)
	{
		MESSAGE("\r\n*************集中器网络配置***************\r\n");
		//MESSAGE("集中器IP:--------------ip:xxx.xxx.xxx.xxx \r\n");
		//MESSAGE("集中器PORT-----------------------port:xxx \r\n");
		MESSAGE("REMOTEIP--------------rip:xxx.xxx.xxx.xxx \r\n");
		MESSAGE("REMOTEPORT----------------------rport:xxx \r\n");
		MESSAGE("REMOTEMirroringIP----rmip:xxx.xxx.xxx.xxx \r\n");  
		MESSAGE("REMOTEMirroringPORT------------rmport:xxx \r\n"); 
    MESSAGE("退出网络配置-------------------------exit \r\n"); 		
		configIpFlag=1;
	}
}
static void Lock_Comm_Task(void *parg)
{
	  UartxPortStru * pRev;
	  Lock_Comm_Data_Buf_Init();
	
	  while(1)
		{
			pRev=Get_Uartx_Port_Fram(0);
			if(pRev->revLen != UARTX_FRAM_NULL)
			  CmdDeal((char *)pRev->revBuf);
			if(debugDICI==1)
			{
			  Uart_Send_Data("debugIDCI",9,2);  //调试信息。。。
			}	
//			if(DEBUGMESSAGE!=1)
//			{
//				//Send_Cmd_To_Lock(pRev);// 
//				SendAckToUpperComputer(pRev);
//				memset((char *)pRev->revBuf,0,pRev->revLen);
//			}
			
		}
}

int Enable485PressureDebug(void)
{
  return debug485;
}
static void Charging485_Task(void *parg)
{
	u8 debugmessage=0;
	UartxPortStru * pRev;
  while(1)
	{
		pRev=Get_Uartx_Port_Fram(1);
		if(pRev->revLen != UARTX_FRAM_NULL)
		{
			//i++;printf("%d\n",i);
			SendAckToUpperComputer(pRev);
			memset((char *)pRev->revBuf,0,pRev->revLen);
			pRev->revLen=UARTX_FRAM_NULL;
		}
//		if(debug485==1)
//		{
//			debugmessage++;
//			pRev->pSendDataHandle("debug485",9,1) ;
//			if(debugmessage==10){debugmessage=0;debug485=0;}
//		}
	}
}
//定义缓冲区
//返回缓冲区

volatile u8 dicidatatmp[50]="fgdgsdfgfdsgfsdg";
volatile u16 dicilen=0;
void getDICIdata(vu8** pdata,vu16* len)
{
  *pdata=dicidatatmp;
	len=&dicilen;
}

static void DICI_Task(void *parg)
{
	UartxPortStru * pRev;
	
	  vu8 *data=NULL;
	  vu16 *len=NULL;
	
  while(1)
	{
		pRev=Get_Uartx_Port_Fram(2);
		if(pRev->revLen != UARTX_FRAM_NULL)
		{
			memcpy((char*)dicidatatmp,(char*)pRev->revBuf,pRev->revLen);
			dicilen=pRev->revLen;
			
			if(debugDICI==1)
			{
			  getDICIdata((vu8**)&data,len); 
		  	Uart_Send_Data((u8*)data,*len,1);  //调试信息。。。
			}
			
		  memset((char *)pRev->revBuf,0,pRev->revLen);
			pRev->revLen=UARTX_FRAM_NULL;
		}
	}
}
	

OS_STK gLockCommTaskStk[LOCK_COMM_TASK_STK_SIZE];
OS_STK gCharging485TaskStk[CHARGING_485_TASK_STK_SIZE];
OS_STK DICITaskStk[DICI_TASK_STK_SIZE];


void Creat_Lock_Comm_Task()
{
	OSTaskCreate(Lock_Comm_Task,(void*) 0,	(OS_STK* )&gLockCommTaskStk[LOCK_COMM_TASK_STK_SIZE - 1],(INT8U) LOCK_COMM_TASK_PRIO);
	OSTaskCreate(Charging485_Task,(void*) 0,	(OS_STK* )&gCharging485TaskStk[CHARGING_485_TASK_STK_SIZE - 1],(INT8U) CHARGING_485_TASK_PRIO);
	//OSTaskCreate(DICI_Task,(void*) 0,	(OS_STK* )&DICITaskStk[DICI_TASK_STK_SIZE - 1],(INT8U) DICI_TASK_PRIO);
}




