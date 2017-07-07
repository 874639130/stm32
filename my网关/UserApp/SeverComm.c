
#include "tcp_client_demo.h"
#include <stdlib.h>
#include <stdio.h>
#define SERVER_REV_LEN     26
#define SERVER_BIT_TIME    5000


static unsigned char gCurrentNum=0;
static unsigned int gServerSendTime=0;
static const unsigned char DEVICE_NUM[DEVICE_NUM_LEN]="EBCCPAC001608001";
static unsigned char gServerRevErrDeviceNum[32];
static unsigned char gServerRevErrSign=0;
static unsigned int gServerRevErrCmd;


static unsigned char Server_Check(unsigned char *pBuf,unsigned char len)
{
	unsigned char index;
	unsigned char res=0;
	for(index=0;index<len;index++)
	{
		res+=pBuf[index];
	}
	return res;
}
TCommBuf gSeverSendComm;

/************************************************************************************************/
static void Server_Creat_Event_Cmd(TLockCommData *pLockCommData)
{
	unsigned char index;
	unsigned char len;
	TServerSendData *pSend =(TServerSendData*)(gSeverSendComm.Buf);
	pSend->head[0]=0xff;
	pSend->head[1]=0xff;
	len=sizeof(TServerSendData);
	pSend->len=len-5;
	pSend->cmd=0x1;
	
	memcpy(pSend->deviceNum,DEVICE_NUM,DEVICE_NUM_LEN-2);
	pSend->msgNum=LOCK_NUM;
	
	for(index=0;index<LOCK_NUM;index++)
	{
		 pSend->serverLockMsg[index].lockNum[0]=pLockCommData[index].lockNum[0];
		 pSend->serverLockMsg[index].lockNum[1]=pLockCommData[index].lockNum[1];
		 pSend->serverLockMsg[index].lockState=pLockCommData[index].lockState;
		 pSend->serverLockMsg[index].carState=pLockCommData[index].carState;
		 pSend->serverLockMsg[index].batteryState=pLockCommData[index].batteryState;
	}
	pSend->check=Server_Check((unsigned char *)(&(pSend->len)),len-3);
	gSeverSendComm.Len=len;
}
/************************************************************************************************
                            ChargingPile  充电桩
************************************************************************************************/

#define CMDSTOP                        5
#define CMDSTOPACK                     6
#define CMDSTART                       7   //开始充电
#define CMDSTARTACK                    8
#define CMDHEATACK                     101
#define CMDHEAT                        102 //
#define CMDSTATEACK                    103 //
#define CMDSTATE                       104 //
#define CMDLOGINACK                    105
#define CMDLOGIN                       106
#define CMDALARMACK                    107  //not use
#define CMDALARM                       108
#define CMDSTARTFINISHACK              109
#define CMDSTARTFINISH                 110
#define CMDFINISH                      202
#define CMDDICI                        2001
#define CMDDICIACK                     2002


static unsigned char CHARGINGPILENUM[32]="00160000\0";  
const unsigned char RESERVATIONNUM[32]="001500000\0"; 
static ChargingPileMessage chargingpiledata;

/************************************************************************************************/
volatile static u8 oldversion=0;
volatile static u8 headtime=0;
volatile static u8 statetime=0;
volatile static u8 linkStart=1;       
volatile static u8 chargingPileType;
volatile static u8 carLinkStatus;
volatile static u8 currentSoc;
volatile static u8 StartSoc;
volatile static u16 BMSRequireVoltage;
volatile static u16 BMSRequireCurrent;
volatile static u16 dcChargingVoltage;
volatile static u16 dcChargingCurrent;
volatile static u16 acAphaseChargingVoltage;
volatile static u16 acBphaseChargingVoltage;
volatile static u16 acCphaseChargingVoltage;
volatile static u16 acAphaseChargingCurrent;
volatile static u16 acBphaseChargingCurrent;
volatile static u16 acCphaseChargingCurrent;
volatile static u32 chargingPower;
volatile static u32 currentMeterRecord;
volatile static u32 beforeChargingMeterRecord;
volatile static u32 chargeTime;
volatile static u32 chargingCurrent;
volatile static u32 chargingAmount;

volatile static u32 heatsendcount=0;
volatile static u32 heatackcount=0;
volatile static u32 statesendcount=0;
volatile static u32 stateackcount=0;
volatile static u32 finishsendcount=0;
volatile static u32 finishackcount=0;
volatile static u32 startcount=0;
volatile static u32 stopcount=0;

void setHeatSendCount(u32 t){
   heatsendcount=t;
}
void setHeatAckCount(u32 t){
   heatackcount=t;
}
void setStateSendCount(u32 t){
   statesendcount=t;
}
void setStateAckCount(u32 t){
   stateackcount=t;
}
void setFinishSendCount(u32 t){
   finishsendcount=t;
}
void setFinishAckCount(u32 t){
   finishackcount=t;
}
void setStartCount(u32 t){
   startcount=t;
}
void setStopCount(u32 t){
   stopcount=t;
}


vu32 getHeatSendCount(void){
  return heatsendcount;
}

vu32 getHeatAckCount(void){
  return heatackcount;
}
vu32 getStateSendCount(void){
  return statesendcount;
}
vu32 getStateAckCount(void){
  return stateackcount;
}
vu32 getFinishSendCount(void){
  return finishsendcount;
}

vu32 getFinishAckCount(void){
  return finishackcount;
}
vu32 getStartCount(void){
  return startcount;
}
vu32 getStopCount(void){
  return stopcount;
}

//void setCHARGINGPILENUM(u8* pCHARGINGPILENUM,u8 setlen){//设置充电桩编号
//	 u8 i=0,*pid=NULL;
//	 u8 str[5]; 
//	 if(setlen>32)return;
//	 for(i=0;i<setlen;i++){
//		 CHARGINGPILENUM[i]=pCHARGINGPILENUM[i];
//		 pid=DI_Read();
//		 if(*pid==0)continue;
//		 if(*pid<10)
//     {
//			 if(i==7) CHARGINGPILENUM[i]=(*pid)+0x30;
//		 }
//		 else if(*pid<100)
//     {
//		   if(i==6)  
//			 {
//				 CHARGINGPILENUM[i]=((*pid)/10)+0x30;
//			   CHARGINGPILENUM[i+1]=((*pid)%10)+0x30;
//				 break;
//			 }
//		 }
//     else if(*pid<255)		 
//		 {
//			 if(i==5)
//			 {
//			   CHARGINGPILENUM[i]=((*pid)/100)+0x30;
//			   CHARGINGPILENUM[i+1]=(((*pid)%100)/10)+0x30;
//				 CHARGINGPILENUM[i+2]=(((*pid)%100)%10)+0x30;
//				 break;
//			 }
//		 }
//	 }
//	 linkStart=1;
//}

void setCHARGINGPILENUM(u8* pCHARGINGPILENUM,u8 setlen){//设置充电桩编号

	 u8 i=0; 
   if(setlen>32)return;
	 for(i=0;i<setlen;i++){
     CHARGINGPILENUM[i]=pCHARGINGPILENUM[i];
	 }
	 linkStart=1;
}
void setChargingPileType(const u8 type) {
  chargingPileType=type;            //充电桩类型
}
void setcarLinkStatus(const u8 linkstatus){
  carLinkStatus=linkstatus;          //车连接状态
}
void setCurrentSoc(u8 soc){          //当前soc
	currentSoc=soc;
}
void setStartSoc(u8 soc){
  StartSoc=soc;
}
void setBMSRequireCurrent(u16 Current){
  BMSRequireCurrent=Current*10;
}
void setBMSRequireVoltage(u16 Voltage){
	BMSRequireVoltage=Voltage*10;
}
	
void setdcChargingVoltage(const u16 voltage){
  dcChargingVoltage=voltage*10;          //直流充电电压
}
void setdcChargingCurrent(const u16 current){
  dcChargingCurrent=current*10;         //直流充电电流
}
void setacAphaseChargingVoltage(const u16 acav){
  acAphaseChargingVoltage=acav*10;//交流A相充电电压
}
void setacBphaseChargingVoltage(const u16 acbv){
  acBphaseChargingVoltage=acbv*10;//交流B相充电电压
}
void setacCphaseChargingVoltage(const u16 accv){
  acCphaseChargingVoltage=accv*10;//交流C相充电电压
}
void setacAphaseChargingCurrent(const u16 acai){
  acAphaseChargingCurrent=acai*10;//交流A相充电电流
}
void setacBphaseChargingCurrent(const u16 acbi){
  acBphaseChargingCurrent=acbi*10;//交流B相充电电流
}
void setacCphaseChargingCurrent(const u16 acci){
  acCphaseChargingCurrent=acci*10;//交流C相充电电流
}
void setChargingPower(u32 power){
	chargingPower=power;//          //充电功率  
}
void setCurrentMeterRecord(u32 record){//当前电表读数
  currentMeterRecord=record;//1212
}
void setBeforeChargingMeterRecord(u32 record){ //充电前电表读书
  beforeChargingMeterRecord=record;
}

void setChargeTime(u32 time){//充电时长
	chargeTime=time;//
}
void setChargingCurrent(u32 current){//本次充电累计充电电量（0.01kwh）
  chargingCurrent=current;
}
void setChargingAmount(u32 amount){  //本次充电金额
  chargingAmount=amount;
}
	

void setHeatTime(u8 time){//心跳包时长  
 headtime=time;
}
void setStateTime(u8 time){//状态包时长 
 statetime=time;
}
/************************************************************************************************/



ChargingPileMessage* GetChargingPileHandle(){
   return &chargingpiledata;
}

static u16 StructureLoginDataPackage()
{
  ChargingPileLoginData *pdata=(ChargingPileLoginData *)(GetChargingPileHandle()->data);

 
	u16 len=0;
	//u8 temp[8]="11111111";
	if(oldversion)memcpy(((identify1*)pdata->identify)->num,CHARGINGPILENUM,32);

	else          memcpy(((identify2*)pdata->identify)->num,CHARGINGPILENUM,32);

	
	//pdata->flag=0; 
	
	pdata->vision[0]=(u8)(10410&0xff);
	pdata->vision[1]=(u8)((10410>>8)&0xff);
	pdata->vision[2]=(u8)((10410>>16)&0xff);
	pdata->vision[3]=(u8)((10410>>24)&0xff);
	
  //pdata->type=0;
	//pdata->bootTimes=1;
	//pdata->uploadMode=2;
	pdata->LoginIntervalTime=10;
	//pdata->runInternalVariable=0;
	//pdata->gunNUM=1;
	//pdata->heatCycle=5;
	//pdata->heatTimeout=5;
	//pdata->rechargeRecord=5;
	
	//memcpy(pdata->sysTime,temp,8);
	len=sizeof(ChargingPileLoginData);
  return len;
}
static u16 sendHeatSum=0;
static int sendHeatNum=0;                                                                         
static u16 StructureHEATDataPackage()
{
	u16 len;

  ChargingPileheatData *pdata=(ChargingPileheatData *)(GetChargingPileHandle()->data);
	
	if(oldversion)memcpy(((identify1*)pdata->identify)->num,CHARGINGPILENUM,32);
	else          memcpy(((identify2*)pdata->identify)->num,CHARGINGPILENUM,32);
	
	if(sendHeatSum>65535)sendHeatNum=1;
	pdata->head=sendHeatNum;
	sendHeatSum++;
	sendHeatNum++;
	if(sendHeatNum>3)
	{
	  sendHeatNum=0;
	}
	len=sizeof(ChargingPileheatData);
	return len;
}

static u8 stopSign=0;
static volatile RtcData startRtcData,endRtcData,timingStartTime;
static u8 startChargingFlag=0;
static u16 StructureStateDataPackage()
{
  u16 len;

	short resTime=50;
	u32 chongKw;
	ChargingPileStateData *pdata=(ChargingPileStateData *)(GetChargingPileHandle()->data);
	
	if(oldversion)memcpy(((identify1*)pdata->identify)->num,CHARGINGPILENUM,32);
	else memcpy(((identify2*)pdata->identify)->num,CHARGINGPILENUM,32);
	
	pdata->gunNUM=1;	
	pdata->chargingSlogan=1;         //充电口号
	pdata->chargingPileType=chargingPileType;       //充电抢类型
	pdata->workState=stopSign;//              //工作状态
  pdata->currentSoc=currentSoc;             //当前SOC%
  pdata->currentHighestAlarmCode=0;//当前最高告警编码
  pdata->carLinkStatus=carLinkStatus;//; //车连接状态
	
	pdata->chargingExpenses[0]=(u8)(chargingAmount&0x000000ff);
	pdata->chargingExpenses[1]=(u8)((chargingAmount>>8)&0x000000ff);
	pdata->chargingExpenses[2]=(u8)((chargingAmount>>16)&0x000000ff);
	pdata->chargingExpenses[3]=(u8)((chargingAmount>>24)&0x000000ff);
	
  pdata->dcChargingVoltage=dcChargingVoltage;      //直流充电电压
	pdata->dcChargingCurrent=dcChargingCurrent;      //直流充电电流
  pdata->BMSRequireVoltage=BMSRequireVoltage;      
  pdata->BMSRequireCurrent=BMSRequireCurrent;      //BMS需求电流
  pdata->BMSChargingMode=1;        //BMS充电模式
	pdata->acAphaseChargingVoltage=acAphaseChargingVoltage;//交流A相充电电压   
	pdata->acBphaseChargingVoltage=acBphaseChargingVoltage;//交流B相充电电压
	pdata->acCphaseChargingVoltage=acCphaseChargingVoltage;//交流C相充电电压
	pdata->acAphaseChargingCurrent=acAphaseChargingCurrent;//交流A相充电电流
	pdata->acBphaseChargingCurrent=acBphaseChargingCurrent;//交流B相充电电流
	pdata->acCphaseChargingCurrent=acCphaseChargingCurrent;//交流C相充电电流
	pdata->remainingChargingTime=resTime;  //剩余充电时间
  pdata->chargeTime=chargeTime;             //充电时长
	pdata->chargingCurrent=chargingCurrent;        //本次充电累计充电电量（0.01kwh）
  pdata->beforeChargingMeterRecord=beforeChargingMeterRecord;//充电前电表读书
  pdata->currentMeterRecord=currentMeterRecord;     //当前电表读数
  pdata->chargingBootMode=1;       //充电启动方式
  pdata->ChargingStrategy=3;       //充电策略
	pdata->ChargingStrategyArgument[0]=(u8)(chongKw&0xff);//充电策略参数
	pdata->ChargingStrategyArgument[1]=(u8)((chongKw>>8)&0xff);
	pdata->ChargingStrategyArgument[2]=(u8)((chongKw>>16)&0xff);
	pdata->ChargingStrategyArgument[3]=(u8)((chongKw>>24)&0xff);
  pdata->reservationFlag=1;        //预约标志
	memcpy(pdata->reservationCode,RESERVATIONNUM,32);//预约卡号
  pdata->reservationOvertime=0;    //预约超时时间
	
	if(startChargingFlag==2)
	{
		startChargingFlag=0;
		RtcData *rtctemp=getRtcData();
		startRtcData.RTC_Month=rtctemp->RTC_Month+(rtctemp->RTC_Month/10)*6;//rtctemp->RTC_Month+0x30;//开始充电时间
		startRtcData.RTC_Date=rtctemp->RTC_Date+(rtctemp->RTC_Date/10)*6;//rtctemp->RTC_Date+0x30;
		startRtcData.RTC_Hours=rtctemp->RTC_Hours+(rtctemp->RTC_Hours/10)*6;//rtctemp->RTC_Hours+0x30;
		startRtcData.RTC_Minutes=rtctemp->RTC_Minutes+(rtctemp->RTC_Minutes/10)*6;//;rtctemp->RTC_Minutes+0x30;
		startRtcData.RTC_Seconds=rtctemp->RTC_Seconds+(rtctemp->RTC_Seconds/10)*6;//rtctemp->RTC_Seconds+0x30;
	  

	  pdata->reservationStartTime[0]=0x20;
	  pdata->reservationStartTime[1]=0x17;//预约/开始充电开始时间
		
	}
	pdata->reservationStartTime[2]=startRtcData.RTC_Month;
	pdata->reservationStartTime[3]=startRtcData.RTC_Date;
	pdata->reservationStartTime[4]=startRtcData.RTC_Hours;
	pdata->reservationStartTime[5]=startRtcData.RTC_Minutes;
	pdata->reservationStartTime[6]=startRtcData.RTC_Seconds;
	pdata->reservationStartTime[7]=0xff;//保留位
	
	//memcpy(pdata->reservationStartTime,temp,8);
	
  pdata->beforeChargingBalance=180000;  //充电前卡余额 	
	pdata->chargingPower=chargingPower;          //充电功率  
	pdata->airOutletTemperature=50;   //出风口温度
	pdata->environmentTemperature=50; //环境温度
	pdata->ChargingPileTemperature=50;//充电抢温度
	
	
	len=sizeof(ChargingPileStateData);
	return len;
}
static u32 ChargingEndReason;
static u16 sendFinish()     //上报充电记录信息 （ 后台停止充电后发送）
{
  u16 len;
	u8 *pdata=(u8 *)(GetChargingPileHandle()->data);
	
  if(oldversion) memcpy(&pdata[4],CHARGINGPILENUM,32);
	else           memcpy(&pdata[0],CHARGINGPILENUM,32);
	
	pdata[36]=0;//充电抢位置类型 44    -8
	pdata[37]=0;//充电抢口       45
	memcpy(&pdata[38],RESERVATIONNUM,32);//预约卡号//充电卡号       46-77
	
	pdata[70]=0x20;//充电开始时间   78-85
  pdata[71]=0x17;
	pdata[72]=startRtcData.RTC_Month;
	pdata[73]=startRtcData.RTC_Date;
	pdata[74]=startRtcData.RTC_Hours;
	pdata[75]=startRtcData.RTC_Minutes;
	pdata[76]=startRtcData.RTC_Seconds;
	pdata[77]=0xff;//保留位
	
	pdata[78]=0x20;//充电结束时间   86-93
	pdata[79]=0x17;
	
	RtcData *rtctemp=getRtcData();
	endRtcData.RTC_Month=rtctemp->RTC_Month+(rtctemp->RTC_Month/10)*6;//rtctemp->RTC_Month+0x30;//开始充电时间
	endRtcData.RTC_Date=rtctemp->RTC_Date+(rtctemp->RTC_Date/10)*6;//rtctemp->RTC_Date+0x30;
	endRtcData.RTC_Hours=rtctemp->RTC_Hours+(rtctemp->RTC_Hours/10)*6;//rtctemp->RTC_Hours+0x30;
	endRtcData.RTC_Minutes=rtctemp->RTC_Minutes+(rtctemp->RTC_Minutes/10)*6;//;rtctemp->RTC_Minutes+0x30;
	endRtcData.RTC_Seconds=rtctemp->RTC_Seconds+(rtctemp->RTC_Seconds/10)*6;//rtctemp->RTC_Seconds+0x30;
	
	pdata[80]=endRtcData.RTC_Month;
	pdata[81]=endRtcData.RTC_Date;
	pdata[82]=endRtcData.RTC_Hours;
	pdata[83]=endRtcData.RTC_Minutes;
	pdata[84]=endRtcData.RTC_Seconds;
	pdata[85]=0xff;//保留位
	
	
	pdata[86]=chargeTime;//充电时间长度   94-97
	pdata[90]=StartSoc;//开始SOC        98
	
	pdata[91]=currentSoc;//结束SOC        99
	pdata[92]=ChargingEndReason;//充电结束原因   100-103
	pdata[93]=ChargingEndReason>>8;
	pdata[94]=ChargingEndReason>>16;
	pdata[95]=ChargingEndReason>>24;
	pdata[96]=chargingCurrent;//本次充电电量   104-107
	pdata[97]=chargingCurrent>>8;
	pdata[98]=chargingCurrent>>16;
	pdata[99]=chargingCurrent>>24;
	pdata[100]=beforeChargingMeterRecord;//充电前电表读数 108-111
	pdata[101]=beforeChargingMeterRecord>>8;
	pdata[102]=beforeChargingMeterRecord>>16;
	pdata[103]=beforeChargingMeterRecord>>24;
	pdata[104]=currentMeterRecord;//充电后电表读数 112-115
	pdata[105]=currentMeterRecord>>8;
	pdata[106]=currentMeterRecord>>16;
	pdata[107]=currentMeterRecord>>24;
	pdata[108]=chargingAmount;//本次充电金额   116-119
	pdata[109]=chargingAmount>>8;
	pdata[110]=chargingAmount>>16;
	pdata[111]=chargingAmount>>24;
	pdata[116]=0;//充电前卡余额   124-127  //只用在刷卡启动才有效
	pdata[120]=0;//当前充电记录索引 128-131
	pdata[124]=0;//总充电记录索引   132-135
	pdata[129]=0;//充电策略         137
	pdata[130]=0;//充电策略参数     138-141
	pdata[134]=0;//车辆VIN          142-158
	pdata[151]=0;//车牌号           159-166
	
	len=288;
	return len;
}
static u16 revStopDeal(void) ;
static u16 revStartDeal(void) ;


static u16 DICIDataPackage()
{
	vu8 *data=NULL;
	vu16 *len=NULL;
	u8 *pdata=(u8 *)(GetChargingPileHandle()->data);
	
	if(oldversion) memcpy(&pdata[4],CHARGINGPILENUM,32);
	else           memcpy(&pdata[0],CHARGINGPILENUM,32);
	
	
	getDICIdata((vu8**)&data,len);
	memcpy(&pdata[36],(char*)data,*len);

	Uart_Send_Data(&pdata[36],*len,0);  //调试信息。。。
	
  return *len;
}


CmdFunsNameTab cmdfunsnametab[]=
{
	(void *)StructureLoginDataPackage,CMDLOGIN,
  (void *)StructureHEATDataPackage, CMDHEAT,  
  (void *)StructureStateDataPackage,CMDSTATE,		
	(void *)revStartDeal,CMDSTARTACK,
	(void *)revStopDeal,CMDSTOPACK,
	(void *)sendFinish,CMDFINISH,
	(void *)DICIDataPackage,CMDDICI,	
	
};
static void SetChargingPileCmd(u16 cmd){ 
  chargingpiledata.cmd=cmd;
}

static u16 GetChargingPileCmd(){
  return chargingpiledata.cmd;
}
static u32 netsendconut=0;
static u32 netrecevconut=0;

u32 getSendChargingNetPackegeCount(void){ //发送给后台的充电桩包的总数
   return netsendconut;
}
u32 getReceChargingNetPackegeConut(void){ //接收后台发送的充电桩包的总数
	 return netrecevconut;
}

static u8 ChargingPileUploadMessage()
{
	  static u8 heatOder=1;
	  u16 len,i=0,id=255;
	  u32 checksum=0;
	  if(GetChargingPileCmd()==0)return 1;
  	ChargingPileMessage *pSend =(ChargingPileMessage*)(gSeverSendComm.Buf);
	  memset(gSeverSendComm.Buf,0,MAX_COM_STREAM_SIZE); 
    pSend->head[0]=0xAA;
	  pSend->head[1]=0xF5;
	  for(i=0;i<(sizeof(cmdfunsnametab)/sizeof(CmdFunsNameTab));i++){
		  if(GetChargingPileCmd()==cmdfunsnametab[i].cmd){
				id=i;break;
			}
		}
		if(id==255)return 2;
	  len=(*(u16(*)())cmdfunsnametab[id].func)()+1;
	  ((u8*)&(pSend->len))[0]=(u8)(CHARGINGPILEHEADLEN+len); 
	  ((u8*)&(pSend->len))[1]=(u8)((CHARGINGPILEHEADLEN+len)>>8); 
    pSend->informationfield=0x10;  //chargingpiledata.informationfield
		heatOder++;if(heatOder>255)heatOder=1;
	  pSend->serialnumber=heatOder;      //chargingpiledata.serialnumber
	  pSend->cmd=chargingpiledata.cmd ;             //104
	  
	  memcpy(pSend->data,chargingpiledata.data,len);//数据拷贝
	  
	  checksum=pSend->cmd;
	  for(i=0;i<len-1;i++)
	  {
	    checksum+=pSend->data[i];
	  }
	  pSend->data[len-1]=(u8)(checksum&0x000000ff);//校验位
	  
	  gSeverSendComm.Len=CHARGINGPILEHEADLEN+len;
	  if(SendNetDataToServer(gSeverSendComm.Buf,gSeverSendComm.Len))
		{
		  MESSAGE("数据发送失败\r\n");return 3;
		}
		netsendconut++;
		return 0;
}

	
/************************************************************************************************
                                    接收处理      
************************************************************************************************/
static u8 *precdata;
static u16 revStartDeal()                   //充电开始应答包
{
	u16 len;
	u8 *pdata=(u8 *)(GetChargingPileHandle()->data);
	if(oldversion)memcpy(&pdata[4],CHARGINGPILENUM,32);
	else          memcpy(&pdata[0],CHARGINGPILENUM,32);
	pdata[36]=precdata[4];            //充电桩枪口
	pdata[37]=0;            //命令执行结果
	pdata[38]=0;
	pdata[39]=0;
	pdata[40]=0;

	len=41;
	return len;
}

static u16 revStopDeal()                 //充电停止           
{
  u16 len;
	u8 *pdata=(u8 *)(GetChargingPileHandle()->data);
  if(oldversion)memcpy(&pdata[4],CHARGINGPILENUM,32);
	else          memcpy(&pdata[0],CHARGINGPILENUM,32);
	pdata[36]=precdata[4];          //充电桩枪口
	pdata[37]=precdata[5];       //命令启始标志   
	pdata[38]=precdata[6];
	pdata[39]=precdata[7];
	pdata[40]=precdata[8];
	pdata[41]=precdata[9];        //命令个数
	pdata[42]=0;           //命令执行结果
	len=43;
	return len;
}

static u32 startpower; 
u32 getStartpower(void){
  return startpower;
}
u8 getChargingState(void){
  return stopSign;
}
/************************************************************************************************
                                  END
************************************************************************************************/
static vu8 netLock=0;
void NetLockOn(void){
  netLock=1;
}
void NetLockOff(void){
	netLock=0;
}
u8 getNetLock(void){
  return netLock;
}

static vu32  ChargingType=0;
static void ChargingPileMessageDeal(u8 *pData,u32 recvLen)
{
	  ChargingPileMessage *pRevData;
	  u16 len=0;
    pRevData=(ChargingPileMessage *)pData;
    if(pRevData->head[0]!=0xAA&&pRevData->head[1]!=0xF5)return;
	  len=(u16)((((u8*)&(pRevData->len))[1]<<8)+((u8*)&(pRevData->len))[0]);
	  if(pRevData->len!=recvLen)return;
    netrecevconut++;//统计接收后台发过来的充电桩的包
	  //处理相应的逻辑
	  NetLockOn();
	  switch(pRevData->cmd)
		{
			case CMDLOGINACK: 
				   
			     break;
			case CMDHEATACK:
				   if(sendHeatNum>1)
					 {
						 sendHeatNum--;
					 }
					 sendHeatNum=0; 
					 heatackcount++;
		       break;
			case CMDSTOP: 
				   startChargingFlag=1;
				   stopSign=0;
			     LED2_OFF;
			     ChargingEndReason=101006;   //后台主动停止
			     precdata=pRevData->data;
				   SetChargingPileCmd(CMDSTOPACK);
           if(!ChargingPileUploadMessage())ChargingPileUploadMessage();
			     stopcount++;
			     break;
			 case CMDSTART:                       //开始充电
				   
			     precdata=pRevData->data;
			     ChargingType=(precdata[8]<<24)+(precdata[7]<<16)+(precdata[6]<<8)+precdata[5];//充电生效类型
			     startpower  =(precdata[20]<<24)+(precdata[19]<<16)+(precdata[18]<<8)+precdata[17];//充电策略参数
			
					 timingStartTime.RTC_Month   =  precdata[23];
					 timingStartTime.RTC_Date    =	precdata[24];		 
					 timingStartTime.RTC_Hours   =  precdata[25];
					 timingStartTime.RTC_Minutes =  precdata[26];
					 timingStartTime.RTC_Seconds =  precdata[27];
			 
			     if(ChargingType==0)//定时启动充电
					 {
              stopSign=2;   
              startChargingFlag=2;			 
			        LED2_ON;
					 }
    			 SetChargingPileCmd(CMDSTARTACK);
				   if(!ChargingPileUploadMessage())ChargingPileUploadMessage();
			     startcount++;
				   break;
			 case CMDSTATEACK:
				   stateackcount++;
				   break;
		   case 201:
				   finishackcount++;
				   stopSign=0;
			     break;
			 case CMDDICIACK:
			     break;
			 default:
				   break;
		}
		NetLockOff();
}



/************************************************************************************************
                     END   ChargingPile  充电桩
************************************************************************************************/


static void Server_Creat_Send_Cmd(TLockCommData *pLockCommData)
{
		Server_Creat_Event_Cmd(pLockCommData);
	  SendNetDataToServer(gSeverSendComm.Buf,gSeverSendComm.Len);
}


/************************************************************************************************/

static void Server_Rev_Ack_Deal(TServerRevData *pRev,TLockCommData *pLockComm)
{
	pLockComm->sendNewState=0;
}
static void Server_Rev_Cmd_Deal( TServerRevData *pRevData,TLockCommData *pLockComm)
{
	pLockComm->newCmd=pRevData->controlNum;
	pLockComm->revNewCmd=1;
	pLockComm->offLine=0;
	pLockComm->sendNum=0;
	pLockComm->revNewCmdAck=1;
}



static void Server_Rev_Comm_Deal(u32_t timeout)
{
	 TServerRevData *pRevData;
	 unsigned char index;
	 u32_t error=0;
	 u8    *pData=NULL;
	 u32 recvLen=0;
	 error=GetNetDataForServer((u8**)&pData,&recvLen,timeout);
	
	 if(error==NOTDATA)return;
   if(recvLen!=SERVER_REV_LEN)
	 {
	   ChargingPileMessageDeal(pData,recvLen); //充电桩处理函数
	   return;
	 }
	
	 pRevData=(TServerRevData *)pData;
	 if(pRevData->head[0]!=0xff||pRevData->head[1]!=0xff)
	 {
		 return;
	 }
	 for(index=0;index<DEVICE_NUM_LEN-2;index++)
	 {
		 if(pRevData->deviceNum[index]!=DEVICE_NUM[index])
		 {
			 break;
		 }
	 }
	 if(index==DEVICE_NUM_LEN)
	 {
		 return;
	 }
	 
	for(index=0;index<LOCK_NUM;index++)
	{
		if(pRevData->lockNum[0]==LockCommDataBuf[index].lockNum[0]&&pRevData->lockNum[1]==LockCommDataBuf[index].lockNum[1])
		{
			break;
		}
	}
	if(index==LOCK_NUM)
	{
		OSTimeDly(5000);
		return;
	}
	 
	 switch(pRevData->cmd)
	 {
		 case 0x01:
			 Server_Rev_Ack_Deal(pRevData,&LockCommDataBuf[index]);
			 break;
		 case 0x11:
			 if(pRevData->controlNum!=0)
			 {
					Server_Rev_Cmd_Deal(pRevData,&LockCommDataBuf[index]);
			 }
			 else
			 {
				 //OSTimeDly(5000);
			 }
			 OSTimeDly(5000);
			 break;
	 }
}
void Server_Comm_Recve_Task(void *parg)
{
    WAITNETREADY;
    while(1)
		{
		   Server_Rev_Comm_Deal(500);
			 if(ChargingType==2)
			 {
				 RtcData *rtctemp=getRtcData();
				 
//				 MESSAGE("The Date :   M:%0.2x - D:%0.2x ", 
//			   (rtctemp->RTC_Month+(rtctemp->RTC_Month/10)*6), 
//			   (rtctemp->RTC_Date+(rtctemp->RTC_Date/10)*6));
//		
//		     MESSAGE("The Time :  %0.2x:%0.2x \r\n\r\n", 
//		    	(rtctemp->RTC_Hours+(rtctemp->RTC_Hours/10)*6), 
//			    (rtctemp->RTC_Minutes+(rtctemp->RTC_Minutes/10)*6));
				 
				 
				 if(timingStartTime.RTC_Month==(rtctemp->RTC_Month+(rtctemp->RTC_Month/10)*6))//到达时间
				 {
					 if(timingStartTime.RTC_Date==(rtctemp->RTC_Date+(rtctemp->RTC_Date/10)*6))
					 { 
				    if(timingStartTime.RTC_Hours==(rtctemp->RTC_Hours+(rtctemp->RTC_Hours/10)*6))
					  {
						 if(timingStartTime.RTC_Minutes<=(rtctemp->RTC_Minutes+(rtctemp->RTC_Minutes/10)*6))
						 {
							 stopSign=2;   
							 startChargingFlag=2;			 
							 LED2_ON;
						 }
				    }
				  }
			  }
			 }
			 OSTimeDly(500);
			// Server_Creat_Send_Cmd(LockCommDataBuf);
		}
}
void Server_Comm_Send_Task(void *parg)
{
	 u32 times=0;
	 u8  sendFlag=0;
	 vu16 cmd;
	 WAITNETREADY;
	 
	 OSTimeDly(2000);
	 SetChargingPileCmd(CMDLOGIN);
	 ChargingPileUploadMessage(); 
	 OSTimeDly(2000);
	 SetChargingPileCmd(CMDLOGIN);
	 ChargingPileUploadMessage(); 
	 OSTimeDly(2000);
 
	 while(1)
	 {
		 SendNetDataToServer("fdsafdsa",5);
//		 times++;
//		 times=times%62;
//		 if((headtime<=0)||(headtime>62))headtime=7;
//		 if((statetime<=0)||(statetime>62))statetime=11;
//		  
//		 if((times%statetime)==0){
//			 SetChargingPileCmd(CMDSTATE);
//			 sendFlag=1;
//		 }
//		 else if((times%headtime)==0){
//			 SetChargingPileCmd(CMDHEAT);
//			 sendFlag=1;
//		 }
//		 else if((times%15)==0){
//			 SetChargingPileCmd(CMDDICI);
//			 sendFlag=1;
//		 }
//		 else if((times%61)==0){
//		   SetChargingPileCmd(CMDLOGIN);
//			 sendFlag=1;
//		 }
//		 if((((stopSign!=0)&&(carLinkStatus==0))||(currentSoc>=98)||(ChargingEndReason==101006)
//			 //正在充点电被拔出抢                         soc大于98            后台主动停止
//			    ||((acAphaseChargingCurrent==0)&&(chargingPileType==2))||(chargingCurrent>startpower))&&((times%3)==0))//
//		                    //交流电下             A相电流为0         充电电量达到设定值          //没接到应答报文
//		 {
//			 if(chargingCurrent>startpower)ChargingEndReason=101009;// 充电电量达到设定值  
//			 if((acAphaseChargingCurrent==0)&&(chargingPileType==2))ChargingEndReason=101015;//电池充满
//			 if(currentSoc>=98)ChargingEndReason=316;//BMS的SOC满中止
//			 if(carLinkStatus==0)ChargingEndReason=300;//充电机主动停止
//			 stopSign=0;LED2_OFF;
//			 SetChargingPileCmd(CMDFINISH);
//			 sendFlag=1;
//		 }
//		 
//		 if(getNetLock())goto loop;
//		 if(sendFlag==1)
//		 {
//			 if(linkStart)
//			 {
//				 if(!ChargingPileUploadMessage())
//				 {
//				   cmd=GetChargingPileCmd();
//					 switch(cmd)
//					 {
//						 case CMDHEAT: heatsendcount++;break;
//						 case CMDSTATE:statesendcount++;break;
//						 case CMDFINISH:finishsendcount++;break;
//						 default:break;
//					 }	
//				 }					 
//			 } 
//			 ChargingEndReason=0;
//		 }
//		 sendFlag=0;
//loop:		 	
		 OSTimeDly(500);
	 }
}

