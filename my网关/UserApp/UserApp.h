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
                       ����ͨ��Э��
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
                     ���� �������ͨ��Э�� 
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
                     ���׮ �������ͨ��Э�� 
**************************************************************************************/
#define CHARGINGPILEHEADLEN      8
#define CHARGINGPILEDATALEN      500

typedef struct 
{
   void*      func;			//����ָ��	 
	 const u16  cmd;
}CmdFunsNameTab;	




#pragma pack(1)
typedef struct 
{
	unsigned char  head[2];              //0xAA 0xF5
	unsigned short len;                  //�������ĵĳ���
	unsigned char  informationfield;     //��Ϣ��
	unsigned char  serialnumber;         //���к�
	volatile unsigned short cmd;                  //����
	unsigned char  data[CHARGINGPILEDATALEN];                //�������ݳ���
	unsigned char  check;                //����������������У���
}ChargingPileMessage;

#pragma pack(1)
typedef struct 
{
	u8            xxx1[2];
	u8            xxx2[2];
	u8            num[32];                //assic����
}identify1;

typedef struct 
{
	u8            num[32];                //assic����
	u8            xxx1[2];
	u8            xxx2[2];
}identify2;
typedef struct 
{
	u8            identify[36];                //assic����+4�����õ��ֽ�
	u8            flag;                   //bit0:0-��֧�ּ��� 1-֧�ּ���
	u8            vision[4];                 //100 00 �汾100.00
	u16           type;                   
	u32           bootTimes;              //�ն�ÿ����������������
  u8	          uploadMode;             //1.Ӧ��ģʽ  2�������ϱ�ģʽ
	u16           LoginIntervalTime;      //ǩ��ʱ�䵥λ����
	u8            runInternalVariable;    //0:��������ģʽ  1��IAPģʽ
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
  u8            identify[36];                //assic����+4�����õ��ֽ�
	u16           head;
}ChargingPileheatData;

typedef struct 
{
  u8            identify[36];                //assic����+4�����õ��ֽ�
	u8            gunNUM;	
	u8            chargingSlogan;         //���ں�
	u8            chargingPileType;       //���׮����
	u8            workState;              //����״̬
  u8            currentSoc;             //��ǰSOC%
  u32           currentHighestAlarmCode;//��ǰ��߸澯����
  u8            carLinkStatus;          //������״̬
  u8            chargingExpenses[4];	      //���γ���ۼƷ���
	u32           internalVariables2;     
	u32           internalVariables3;     //�ڲ�����3
  u16           dcChargingVoltage;      //ֱ������ѹ
	u16           dcChargingCurrent;      //ֱ��������
  u16           BMSRequireVoltage;      
  u16           BMSRequireCurrent;      //BMS�������
  u8            BMSChargingMode;        //BMS���ģʽ
	u16           acAphaseChargingVoltage;//����A�����ѹ
	u16           acBphaseChargingVoltage;//����B�����ѹ
	u16           acCphaseChargingVoltage;//����C�����ѹ
	u16           acAphaseChargingCurrent;//����A�������
	u16           acBphaseChargingCurrent;//����B�������
	u16           acCphaseChargingCurrent;//����C�������
	u16           remainingChargingTime;  //ʣ����ʱ��
  u32	          chargeTime;             //���ʱ��
	u32           chargingCurrent;        //���γ���ۼƳ�������0.01kwh��
  u32           beforeChargingMeterRecord;//���ǰ������
  u32           currentMeterRecord;     //��ǰ������
  u8            chargingBootMode;       //���������ʽ
  u8            ChargingStrategy;       //������
	u8            ChargingStrategyArgument[4];//�����Բ���
  u8            reservationFlag;        //ԤԼ��־
  u8            reservationCode[32];    //ԤԼ����
  u8            reservationOvertime;    //ԤԼ��ʱʱ��
  u8            reservationStartTime[8];//ԤԼ/��ʼ��翪ʼʱ��
  u32           beforeChargingBalance;  //���ǰ����� 	
	u32           xxx3;
	u32           chargingPower;          //��繦�� 
	u32           systemVariables3;       //ϵͳ����3
	u32           systemVariables4;       //ϵͳ����4
	u32           systemVariables5;       //ϵͳ����5  
	u8            airOutletTemperature;   //������¶�
	u8            environmentTemperature; //�����¶�
	u8            ChargingPileTemperature;//������¶�
}
ChargingPileStateData;
/**************************************************************************************
                     ���׮ �������ͨ��Э�� 
**************************************************************************************/



/**************************************************************************************
                     ���׮ ����λ��ͨ��Э�� 
**************************************************************************************/
#define DATALEN             150
typedef struct 
{
  u8   addr;                //��ַ
	u8   functioncode;        //������
	u16  writestartaddr;      //д��ʼ��ַ
  u16	 writelen;            //��λ�������������ݳ��� 
  u16  readstartaddr;
	u16	 readlen;
	u8   data[DATALEN];       //���ȣ�Ĭ��150��
	u16  crc;

}UpperComputerMessage;


typedef struct 
{
  u8   addr;                //��ַ
	u8   functioncode;        //������
  u16  readstartaddr;
	u16	 readlen;
  u8   data[DATALEN];       //���ȣ�Ĭ��150��
	u16  crc;
}ToUpperComputerMessage;



/**************************************************************************************
                     ���׮ ����λ��ͨ��Э�� 
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




