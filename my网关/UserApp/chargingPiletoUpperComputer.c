
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
	if(setipFlag==0) //����ʱ��
	{
	  rtctemp->RTC_Year=pRecev->data[0];//��    0
	  rtctemp->RTC_Month=pRecev->data[1];//��    1
		rtctemp->RTC_Date=pRecev->data[2];//��    2
		rtctemp->RTC_Hours=pRecev->data[3];//ʱ    3
	  rtctemp->RTC_Minutes=pRecev->data[4];//��    4
		rtctemp->RTC_Seconds=pRecev->data[5];//��    5
    RTC_TimeAndDate_Set();		
	}
	
	
  SetRemoteIp(&(pRecev->data[6]));     //����ip                6-9
	temp=DataConversionU16(&(pRecev->data[10]));
	SetRemotePort(&temp);//���ö˿�    10-11
	if(setipFlag==0){NETREADY;setipFlag=1;}//�Ѿ����ú�Զ��ip
	
	
	setCHARGINGPILENUM(&(pRecev->data[16]),8);//���ó��׮��� ǰ8λ
	setChargingPileType(pRecev->data[24]);  //������� 0������ 1��ֱ��     24
	//pRecev->data[25];//�Ƿ���� 0:δ���� 1���Ѳ���  25
  setcarLinkStatus(pRecev->data[26]);     //������״̬                   26
	      
	setStartSoc(pRecev->data[27]);  //��ʼsoc              27   
	setCurrentSoc(pRecev->data[28]); //��ǰsoc      28       										
         
	setBMSRequireVoltage(DataConversionU16(&(pRecev->data[29])));      //BMS�����ѹ                  29-30��

	setBMSRequireCurrent(DataConversionU16(&(pRecev->data[31])));//BMS�������                  31-32
	
	setdcChargingVoltage(DataConversionU16(&(pRecev->data[33])));//ֱ����ѹ                     33-34
  setdcChargingCurrent(DataConversionU16(&(pRecev->data[35])));//ֱ������                     35-36
  setacAphaseChargingVoltage(DataConversionU16(&(pRecev->data[37])));//����A��ѹ                    37-38
  setacBphaseChargingVoltage(DataConversionU16(&(pRecev->data[39])));//����B��ѹ                    39-40
  setacCphaseChargingVoltage(DataConversionU16(&(pRecev->data[41])));//����C��ѹ                    41-42
  setacAphaseChargingCurrent(DataConversionU16(&(pRecev->data[43])));//����A����                    43-44
  setacBphaseChargingCurrent(DataConversionU16(&(pRecev->data[45])));//����B����                    45-46
  setacCphaseChargingCurrent(DataConversionU16(&(pRecev->data[47])));//����C����                    47-48
	
	setChargingPower(DataConversionU32(&(pRecev->data[49])));//��繦��                         49-52
	
	setBeforeChargingMeterRecord(DataConversionU32(&(pRecev->data[53])));//��ʼ������           53-56
	setCurrentMeterRecord(DataConversionU32(&(pRecev->data[57])));//��ǰ������          57-60
	setChargingCurrent(DataConversionU32(&(pRecev->data[61])));//��ǰ������           61-64
	
	setChargingAmount(DataConversionU32(&(pRecev->data[65])));//��ǰ�����           65-68
	setChargeTime(DataConversionU32(&(pRecev->data[69])));//���ʱ��              69-72
	setHeatTime(pRecev->data[73]);//������ʱ��  73
	setStateTime(pRecev->data[74]);//״̬��ʱ�� 74
	
	//u8* Ip=getLocalIp();//����ip
	//if((Ip[3]==0)||(Ip[3]!=pRecev->data[78]))
		if(setLocalIpFlag==0){
			setLocalIp(&(pRecev->data[75]),&(pRecev->data[79]));//���ñ���ip	
			setLocalIpFlag=1;
		}	
	
	
	ToUpperComputerMessage *pSend =(ToUpperComputerMessage*)(ToUpperComputerDatatemp.Buf);
	
	pSend->addr          =pRecev->addr;
	pSend->functioncode  =pRecev->functioncode;
	pSend->readstartaddr =pRecev->readstartaddr;
	pSend->readlen       =pRecev->readlen;
	pSend->data[0]=getChargingState();//���״̬
	
	vu32 headsend =getHeatSendCount();
  vu32 headack  =getHeatAckCount();
  vu32 statesend=getStateSendCount();
  vu32 stateack =getStateAckCount();
  vu32 finishsend=getFinishSendCount();
  vu32 fineshack =getFinishAckCount();
  vu32 startcount=getStartCount();
  vu32 stopcount =getStopCount();
	
	
	pSend->data[1]=(u8)headsend;//��������������
	pSend->data[2]=(u8)(headsend>>8);
	pSend->data[3]=(u8)(headsend>>16);
	pSend->data[4]=(u8)(headsend>>24);
	pSend->data[5]=(u8)headack;
	pSend->data[6]=(u8)(headack>>8);
	pSend->data[7]=(u8)(headack>>16);
	pSend->data[8]=(u8)(headack>>24);
	pSend->data[9]=(u8)statesend;//����״̬��
	pSend->data[10]=(u8)(statesend>>8);
	pSend->data[11]=(u8)(statesend>>16);
	pSend->data[12]=(u8)(statesend>>24);
	pSend->data[13]=(u8)stateack;
	pSend->data[14]=(u8)(stateack>>8);
	pSend->data[15]=(u8)(stateack>>16);
	pSend->data[16]=(u8)(stateack>>24);
	pSend->data[17]=(u8)startcount;//����������
	pSend->data[18]=(u8)(startcount>>8);
	pSend->data[19]=(u8)(startcount>>16);
	pSend->data[20]=(u8)(startcount>>24);
	pSend->data[21]=(u8)stopcount;//ֹͣ������
	pSend->data[22]=(u8)(stopcount>>8);
	pSend->data[23]=(u8)(stopcount>>16);
	pSend->data[24]=(u8)(stopcount>>24);
	pSend->data[25]=(u8)finishsend;//��ɱ���
	pSend->data[26]=(u8)(finishsend>>8);
	pSend->data[27]=(u8)(finishsend>>16);
	pSend->data[28]=(u8)(finishsend>>24);
	pSend->data[29]=(u8)fineshack;//��ɱ���
	pSend->data[30]=(u8)(fineshack>>8);
	pSend->data[31]=(u8)(fineshack>>16);
	pSend->data[32]=(u8)(fineshack>>24);
	u8* mac=getLocalMac();//����mac
	pSend->data[33]=mac[0];
	pSend->data[34]=mac[1];
	pSend->data[35]=mac[2];
	pSend->data[36]=mac[3];
	pSend->data[37]=mac[4];
	pSend->data[38]=mac[5];
	
	u8* ip=getLocalIp();//����ip
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




































