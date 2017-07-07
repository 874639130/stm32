
#include "includes.h"
#include "tcp_client_demo.h"  
#include "lwip_comm.h" 
static TCommBuf ToUpperComputerDatatemp;
//static UpperComputerMessage touppercomputermessage;


static u16 DataConversionU16(u8 data[2]){
  u16 temp=0;
	temp=(data[1]<<8)+data[0];
	return temp;
}
 
static u32 DataConversionU32(u8 data[4]){
  u32 temp=0;
	temp=data[3]<<24;
	temp+=data[2]<<16;
	temp+=data[1]<<8;
	temp+=data[0];
	return temp;
}

static u8  setipFlag=0;
static u8  setLocalIpFlag=0;
void SendAckToUpperComputer(const UartxPortStru * pRev)
{
	u16 crc=0,i=0;
	u16 temp=0;
	//u8 *pCHARGINGPILENUM;
	
	if(pRev->revLen==UARTX_FRAM_NULL)return;
	if(Enable485PressureDebug())pRev->pSendDataHandle(pRev->revBuf,pRev->revLen,pRev->port) ;
	if(pRev->revLen<sizeof(UpperComputerMessage))return;

	UpperComputerMessage *pRecev =(UpperComputerMessage*)(pRev->revBuf);
	crc=Modbus_CRC16((u8*)pRev->revBuf,160);
	u8 *pid=NULL;
	pid=DI_Read();
	if(pRecev->addr!=*pid)return;
	//if(pRecev->addr!=1)return;
	if(pRecev->crc!=crc)return;
	if(pRecev->functioncode!=23)return;
	
	RtcData *rtctemp=getRtcData();
	if(setipFlag==0) //设置时间
	{
	  rtctemp->RTC_Year=pRecev->data[0];//年    0
	  rtctemp->RTC_Month=pRecev->data[1];//月    1
		rtctemp->RTC_Date=pRecev->data[2];//日    2
		rtctemp->RTC_Hours=pRecev->data[3];//时    3
	  rtctemp->RTC_Minutes=pRecev->data[4];//分    4
		rtctemp->RTC_Seconds=pRecev->data[5];//秒    5
    RTC_TimeAndDate_Set();		
	}
	
	
  SetRemoteIp(&(pRecev->data[6]));     //设置ip                6-9
	temp=DataConversionU16(&(pRecev->data[10]));
	SetRemotePort(&temp);//设置端口    10-11
	if(setipFlag==0){NETREADY;setipFlag=1;}//已经设置好远程ip
	
	
	setCHARGINGPILENUM(&(pRecev->data[16]),8);//设置充电桩编号 前8位
	setChargingPileType(pRecev->data[24]);  //充电类型 0：交流 1：直流     24
	//pRecev->data[25];//是否插抢 0:未插抢 1：已插抢  25
  setcarLinkStatus(pRecev->data[26]);     //车连接状态                   26
	      
	setStartSoc(pRecev->data[27]);  //开始soc              27   
	setCurrentSoc(pRecev->data[28]); //当前soc      28       										
         
	setBMSRequireVoltage(DataConversionU16(&(pRecev->data[29])));      //BMS需求电压                  29-30、

	setBMSRequireCurrent(DataConversionU16(&(pRecev->data[31])));//BMS需求电流                  31-32
	
	setdcChargingVoltage(DataConversionU16(&(pRecev->data[33])));//直流电压                     33-34
  setdcChargingCurrent(DataConversionU16(&(pRecev->data[35])));//直流电流                     35-36
  setacAphaseChargingVoltage(DataConversionU16(&(pRecev->data[37])));//交流A电压                    37-38
  setacBphaseChargingVoltage(DataConversionU16(&(pRecev->data[39])));//交流B电压                    39-40
  setacCphaseChargingVoltage(DataConversionU16(&(pRecev->data[41])));//电流C电压                    41-42
  setacAphaseChargingCurrent(DataConversionU16(&(pRecev->data[43])));//交流A电流                    43-44
  setacBphaseChargingCurrent(DataConversionU16(&(pRecev->data[45])));//交流B电流                    45-46
  setacCphaseChargingCurrent(DataConversionU16(&(pRecev->data[47])));//交流C电流                    47-48
	
	setChargingPower(DataConversionU32(&(pRecev->data[49])));//充电功率                         49-52
	
	setBeforeChargingMeterRecord(DataConversionU32(&(pRecev->data[53])));//开始电表读数           53-56
	setCurrentMeterRecord(DataConversionU32(&(pRecev->data[57])));//当前电表读数          57-60
	setChargingCurrent(DataConversionU32(&(pRecev->data[61])));//当前充电电量           61-64
	
	setChargingAmount(DataConversionU32(&(pRecev->data[65])));//当前充电金额           65-68
	setChargeTime(DataConversionU32(&(pRecev->data[69])));//充电时长              69-72
	setHeatTime(pRecev->data[73]);//心跳包时长  73
	setStateTime(pRecev->data[74]);//状态包时长 74
	
	//u8* Ip=getLocalIp();//本地ip
	//if((Ip[3]==0)||(Ip[3]!=pRecev->data[78]))
		if(setLocalIpFlag==0){
			setLocalIp(&(pRecev->data[75]),&(pRecev->data[79]));//设置本地ip	
			setLocalIpFlag=1;
		}	
	
	
	ToUpperComputerMessage *pSend =(ToUpperComputerMessage*)(ToUpperComputerDatatemp.Buf);
	
	pSend->addr          =pRecev->addr;
	pSend->functioncode  =pRecev->functioncode;
	pSend->readstartaddr =pRecev->readstartaddr;
	pSend->readlen       =pRecev->readlen;
	pSend->data[0]=getChargingState();//充电状态
	
	vu32 headsend =getHeatSendCount();
  vu32 headack  =getHeatAckCount();
  vu32 statesend=getStateSendCount();
  vu32 stateack =getStateAckCount();
  vu32 finishsend=getFinishSendCount();
  vu32 fineshack =getFinishAckCount();
  vu32 startcount=getStartCount();
  vu32 stopcount =getStopCount();
	
	
	pSend->data[1]=(u8)headsend;//发送心跳包个数
	pSend->data[2]=(u8)(headsend>>8);
	pSend->data[3]=(u8)(headsend>>16);
	pSend->data[4]=(u8)(headsend>>24);
	pSend->data[5]=(u8)headack;
	pSend->data[6]=(u8)(headack>>8);
	pSend->data[7]=(u8)(headack>>16);
	pSend->data[8]=(u8)(headack>>24);
	pSend->data[9]=(u8)statesend;//发送状态包
	pSend->data[10]=(u8)(statesend>>8);
	pSend->data[11]=(u8)(statesend>>16);
	pSend->data[12]=(u8)(statesend>>24);
	pSend->data[13]=(u8)stateack;
	pSend->data[14]=(u8)(stateack>>8);
	pSend->data[15]=(u8)(stateack>>16);
	pSend->data[16]=(u8)(stateack>>24);
	pSend->data[17]=(u8)startcount;//开启包个数
	pSend->data[18]=(u8)(startcount>>8);
	pSend->data[19]=(u8)(startcount>>16);
	pSend->data[20]=(u8)(startcount>>24);
	pSend->data[21]=(u8)stopcount;//停止包个数
	pSend->data[22]=(u8)(stopcount>>8);
	pSend->data[23]=(u8)(stopcount>>16);
	pSend->data[24]=(u8)(stopcount>>24);
	pSend->data[25]=(u8)finishsend;//完成报文
	pSend->data[26]=(u8)(finishsend>>8);
	pSend->data[27]=(u8)(finishsend>>16);
	pSend->data[28]=(u8)(finishsend>>24);
	pSend->data[29]=(u8)fineshack;//完成报文
	pSend->data[30]=(u8)(fineshack>>8);
	pSend->data[31]=(u8)(fineshack>>16);
	pSend->data[32]=(u8)(fineshack>>24);
	u8* mac=getLocalMac();//本地mac
	pSend->data[33]=mac[0];
	pSend->data[34]=mac[1];
	pSend->data[35]=mac[2];
	pSend->data[36]=mac[3];
	pSend->data[37]=mac[4];
	pSend->data[38]=mac[5];
	
	u8* ip=getLocalIp();//本地ip
  pSend->data[39]=ip[0];
	pSend->data[40]=ip[1];
	pSend->data[41]=ip[2];
	pSend->data[42]=ip[3];
	
	
//	for(i=1;i<DATALEN;i++){
//	  pSend->data[i]=pRecev->data[i];
//	}
	
	getStartpower();
	pSend->crc=Modbus_CRC16(ToUpperComputerDatatemp.Buf,156);
  pRev->pSendDataHandle(ToUpperComputerDatatemp.Buf,sizeof(ToUpperComputerMessage),pRev->port);
}




































