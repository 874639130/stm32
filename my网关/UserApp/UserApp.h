#ifndef __COMMM_PRO_H
#define __COMMM_PRO_H

#define FLASH_SECTOR_LEN    1024
#define DEVICE_NOMBER_ADDR   0X0800F800  
#define LOCK_REV_LEN         6
#define LOCK_NUM             10
#define CMD_MAX_NUM          5
#define LOCK_OFF_LINE_SEND_MAX_NUM 3
#define DEFAULT_CMD                1
#define DEVICE_NUM_LEN             16
#include "includes.h"
#include "UartDriver.h"
#define  MAX_COM_STREAM_SIZE  UARTX_SEND_DMA_SIZE  

typedef struct
{
	 unsigned char  Buf[MAX_COM_STREAM_SIZE] ;
	 unsigned int   Timer;
	 unsigned short us_tick;
	 unsigned short	Len;
	
}TCommBuf;


typedef struct tagTLockCommData
{
	 unsigned char lockNum[2];
	 unsigned int sendDataTime; 
	 unsigned char cmd; 
	 unsigned char curentCmd;
	 unsigned char offLineSign;
	 unsigned char  lockState;
	 unsigned char  carState;
	 unsigned char  batteryState;
	 unsigned char  newLockState;
	 unsigned char  newCarState;
	 unsigned char  newBatteryState;
	 unsigned char newCmd;
	 unsigned char sendNum;
	 unsigned char offLine;
	 unsigned char cmdOpFilishSign;
	 unsigned char revNewCmd;
	 unsigned char numErr;
	 unsigned char sendNewState;
	 unsigned char revNewCmdAck;
	 
}TLockCommData;


/**************************************************************************************
                       与锁通信协议
**************************************************************************************/
typedef struct tagTLockSendData
{
	unsigned char lockNum[2];
	unsigned char cmd;
	unsigned char lockState;
	unsigned char carState;
	unsigned char batteryState;
	unsigned char crc[2];
}TLockSendData;
typedef struct tagTLockRevData
{
	unsigned char lockNum[2];
	unsigned char cmd;
	unsigned char lockState;
	unsigned char carState;
	unsigned char batteryState;
	unsigned char writeNumSign;
	unsigned char res;
	unsigned char crc[2];
}TLockRevData;



/**************************************************************************************
                     地锁 与服务器通信协议 
**************************************************************************************/
#pragma pack(1)
typedef struct tagTServerLockMsg	
{
	unsigned char lockNum[2];
	unsigned char lockState;
	unsigned char carState;
	unsigned char batteryState;
}TServerLockMsg;

#pragma pack(1)
typedef struct tagTServerSendData
{
	unsigned char head[2];
	unsigned short len;
	unsigned char cmd;
	unsigned char ack;
	unsigned char res[2];
	unsigned char deviceNum[DEVICE_NUM_LEN-2];
	unsigned short msgNum;
  TServerLockMsg serverLockMsg[LOCK_NUM];
	unsigned char check;
}TServerSendData;


typedef struct tagTServerRevData
{
	unsigned char head[2];
	unsigned short len;
	unsigned char cmd;
	unsigned char ack;
	unsigned char res[2];
	unsigned char deviceNum[DEVICE_NUM_LEN-2];
	unsigned char lockNum[2];
	unsigned char controlNum;
	unsigned char res2[4];
	unsigned char check;
}TServerRevData;


/**************************************************************************************
                     充电桩 与服务器通信协议 
**************************************************************************************/
#define CHARGINGPILEHEADLEN      8
#define CHARGINGPILEDATALEN      500

typedef struct 
{
   void*      func;			//函数指针	 
	 const u16  cmd;
}CmdFunsNameTab;	




#pragma pack(1)
typedef struct 
{
	unsigned char  head[2];              //0xAA 0xF5
	unsigned short len;                  //整个报文的长度
	unsigned char  informationfield;     //信息域
	unsigned char  serialnumber;         //序列号
	volatile unsigned short cmd;                  //命令
	unsigned char  data[CHARGINGPILEDATALEN];                //最大的数据长度
	unsigned char  check;                //命令代码与数据域的校验和
}ChargingPileMessage;

#pragma pack(1)
typedef struct 
{
	u8            xxx1[2];
	u8            xxx2[2];
	u8            num[32];                //assic编码
}identify1;

typedef struct 
{
	u8            num[32];                //assic编码
	u8            xxx1[2];
	u8            xxx2[2];
}identify2;
typedef struct 
{
	u8            identify[36];                //assic编码+4个无用的字节
	u8            flag;                   //bit0:0-不支持加密 1-支持加密
	u8            vision[4];                 //100 00 版本100.00
	u16           type;                   
	u32           bootTimes;              //终端每次启动，计数保存
  u8	          uploadMode;             //1.应答模式  2：主动上报模式
	u16           LoginIntervalTime;      //签到时间单位分钟
	u8            runInternalVariable;    //0:正常工作模式  1：IAP模式
	u8            gunNUM;
	u8            heatCycle;
	u8            heatTimeout;
	u32           rechargeRecord;
	u8            sysTime[8];
	u8            xxx3[8];
	u8            xxx4[8];
	u8            xxx5[8];
}ChargingPileLoginData;


typedef struct 
{
  u8            identify[36];                //assic编码+4个无用的字节
	u16           head;
}ChargingPileheatData;

typedef struct 
{
  u8            identify[36];                //assic编码+4个无用的字节
	u8            gunNUM;	
	u8            chargingSlogan;         //充电口号
	u8            chargingPileType;       //充电桩类型
	u8            workState;              //工作状态
  u8            currentSoc;             //当前SOC%
  u32           currentHighestAlarmCode;//当前最高告警编码
  u8            carLinkStatus;          //车连接状态
  u8            chargingExpenses[4];	      //本次充电累计费用
	u32           internalVariables2;     
	u32           internalVariables3;     //内部变量3
  u16           dcChargingVoltage;      //直流充电电压
	u16           dcChargingCurrent;      //直流充电电流
  u16           BMSRequireVoltage;      
  u16           BMSRequireCurrent;      //BMS需求电流
  u8            BMSChargingMode;        //BMS充电模式
	u16           acAphaseChargingVoltage;//交流A相充电电压
	u16           acBphaseChargingVoltage;//交流B相充电电压
	u16           acCphaseChargingVoltage;//交流C相充电电压
	u16           acAphaseChargingCurrent;//交流A相充电电流
	u16           acBphaseChargingCurrent;//交流B相充电电流
	u16           acCphaseChargingCurrent;//交流C相充电电流
	u16           remainingChargingTime;  //剩余充电时间
  u32	          chargeTime;             //充电时长
	u32           chargingCurrent;        //本次充电累计充电电量（0.01kwh）
  u32           beforeChargingMeterRecord;//充电前电表读书
  u32           currentMeterRecord;     //当前电表读数
  u8            chargingBootMode;       //充电启动方式
  u8            ChargingStrategy;       //充电策略
	u8            ChargingStrategyArgument[4];//充电策略参数
  u8            reservationFlag;        //预约标志
  u8            reservationCode[32];    //预约卡号
  u8            reservationOvertime;    //预约超时时间
  u8            reservationStartTime[8];//预约/开始充电开始时间
  u32           beforeChargingBalance;  //充电前卡余额 	
	u32           xxx3;
	u32           chargingPower;          //充电功率 
	u32           systemVariables3;       //系统变量3
	u32           systemVariables4;       //系统变量4
	u32           systemVariables5;       //系统变量5  
	u8            airOutletTemperature;   //出风口温度
	u8            environmentTemperature; //环境温度
	u8            ChargingPileTemperature;//充电抢温度
}
ChargingPileStateData;
/**************************************************************************************
                     充电桩 与服务器通信协议 
**************************************************************************************/



/**************************************************************************************
                     充电桩 与上位机通信协议 
**************************************************************************************/
#define DATALEN             150
typedef struct 
{
  u8   addr;                //地址
	u8   functioncode;        //功能码
	u16  writestartaddr;      //写开始地址
  u16	 writelen;            //上位机发下来的数据长度 
  u16  readstartaddr;
	u16	 readlen;
	u8   data[DATALEN];       //长度（默认150）
	u16  crc;

}UpperComputerMessage;


typedef struct 
{
  u8   addr;                //地址
	u8   functioncode;        //功能码
  u16  readstartaddr;
	u16	 readlen;
  u8   data[DATALEN];       //长度（默认150）
	u16  crc;
}ToUpperComputerMessage;



/**************************************************************************************
                     充电桩 与上位机通信协议 
**************************************************************************************/




int Enable485PressureDebug(void);
extern TLockCommData LockCommDataBuf[LOCK_NUM];
unsigned short Modbus_CRC16(unsigned char *puchMsg, unsigned int usDataLen);
//int Modbus_CRCVerify(unsigned char *pBuf,unsigned int Len);
/***********CommPro.c************************/
void Lock_Comm_Data_Buf_Init(void);
void Send_Cmd_To_Lock(const UartxPortStru * pRev);

/***********LockComm.c************************/
void Creat_Lock_Comm_Task(void);
void getDICIdata(vu8** pdata,vu16* len);


/***********SeverComm.c************************/
void setHeatSendCount(u32 t);
void setHeatAckCount(u32 t);
void setStateSendCount(u32 t);
void setStateAckCount(u32 t);
void setFinishSendCount(u32 t);
void setFinishAckCount(u32 t);
void setStartCount(u32 t);
void setStopCount(u32 t);

vu32 getHeatSendCount(void);
vu32 getHeatAckCount(void);
vu32 getStateSendCount(void);
vu32 getStateAckCount(void);
vu32 getFinishSendCount(void);
vu32 getFinishAckCount(void);
vu32 getStartCount(void);
vu32 getStopCount(void);

void setChargingPileType(const u8 type) ;
void setcarLinkStatus(const u8 linkstatus);
void setCurrentSoc(u8 soc);
void setStartSoc(u8 soc);
void setBMSRequireVoltage(u16 Voltage);
void setBMSRequireCurrent(u16 Current);
void setdcChargingVoltage(const u16 voltage);
void setdcChargingCurrent(const u16 current);
void setacAphaseChargingVoltage(const u16 acav);
void setacBphaseChargingVoltage(const u16 acbv);
void setacCphaseChargingVoltage(const u16 accv);
void setacAphaseChargingCurrent(const u16 acai);
void setacBphaseChargingCurrent(const u16 acbi);
void setacCphaseChargingCurrent(const u16 acci);
void setCHARGINGPILENUM(u8* pCHARGINGPILENUM,u8 setlen);
void Server_Comm_Send_Task(void *parg);
void Server_Comm_Recve_Task(void *parg);
void setChargeTime(u32 time);
void setBeforeChargingMeterRecord(u32 record);
void setChargingPower(u32 power);
void setCurrentMeterRecord(u32 record);
void setChargingCurrent(u32 current);
void setChargeTime(u32 time);
void setChargingAmount(u32 amount);

u32 getReceChargingNetPackegeConut(void);
u32 getSendChargingNetPackegeCount(void);
void setHeatTime(u8 time);
void setStateTime(u8 time);
//////////////////////////////////////////////////////////////
u32 getStartpower(void);
u8 getChargingState(void);
/***********chargingPiletoUpperComputer.c************************/
void SendAckToUpperComputer(const UartxPortStru * pRev);
 
#endif




