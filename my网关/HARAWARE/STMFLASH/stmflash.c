#include "stmflash.h"
#include "tcp_client_demo.h"
#include "lwip_comm.h" 
//��ȡָ����ַ����(32λ����) 
//faddr:����ַ 
//����ֵ:��Ӧ����.
u32 STMFLASH_ReadWord(u32 faddr)
{
	return *(vu32*)faddr; 
}  
//��ȡĳ����ַ���ڵ�flash����
//addr:flash��ַ
//����ֵ:0~11,��addr���ڵ�����
uint16_t STMFLASH_GetFlashSector(u32 addr)
{
	if(addr<ADDR_FLASH_SECTOR_1)return FLASH_Sector_0;
	else if(addr<ADDR_FLASH_SECTOR_2)return FLASH_Sector_1;
	else if(addr<ADDR_FLASH_SECTOR_3)return FLASH_Sector_2;
	else if(addr<ADDR_FLASH_SECTOR_4)return FLASH_Sector_3;
	else if(addr<ADDR_FLASH_SECTOR_5)return FLASH_Sector_4;
	else if(addr<ADDR_FLASH_SECTOR_6)return FLASH_Sector_5;
	else if(addr<ADDR_FLASH_SECTOR_7)return FLASH_Sector_6;
	else if(addr<ADDR_FLASH_SECTOR_8)return FLASH_Sector_7;
	else if(addr<ADDR_FLASH_SECTOR_9)return FLASH_Sector_8;
	else if(addr<ADDR_FLASH_SECTOR_10)return FLASH_Sector_9;
	else if(addr<ADDR_FLASH_SECTOR_11)return FLASH_Sector_10; 
	return FLASH_Sector_11;	
}


//��ָ����ַ��ʼ����ָ�����ȵ�����
//ReadAddr:��ʼ��ַ
//pBuffer:����ָ��
//NumToRead:��(4λ)��
void STMFLASH_Read(u32 ReadAddr,u32 *pBuffer,u32 NumToRead)   	
{
	u32 i;
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i]=STMFLASH_ReadWord(ReadAddr);//��ȡ4���ֽ�.
		ReadAddr+=4;//ƫ��4���ֽ�.	
	}
}


//��ָ����ַ��ʼд��ָ�����ȵ�����
//�ر�ע��:��ΪSTM32F4������ʵ��̫��,û�취���ر�����������,���Ա�����
//         д��ַ�����0XFF,��ô���Ȳ������������Ҳ�������������.����
//         д��0XFF�ĵ�ַ,�����������������ݶ�ʧ.����д֮ǰȷ��������
//         û����Ҫ����,��������������Ȳ�����,Ȼ����������д. 
//�ú�����OTP����Ҳ��Ч!��������дOTP��!
//OTP�����ַ��Χ:0X1FFF7800~0X1FFF7A0F(ע�⣺���16�ֽڣ�����OTP���ݿ�����������д����)
//WriteAddr:��ʼ��ַ(�˵�ַ����Ϊ4�ı���!!)
//pBuffer:����ָ��
//NumToWrite:��(32λ)��(����Ҫд���32λ���ݵĸ���.) 
void STMFLASH_Write(u32 WriteAddr,u32 *pBuffer,u32 NumToWrite)	
{ 
  FLASH_Status status = FLASH_COMPLETE;
	u32 addrx=0;
	u32 endaddr=0;	
  if(WriteAddr<STM32_FLASH_BASE||WriteAddr%4)return;	//�Ƿ���ַ
	FLASH_Unlock();									//���� 
  FLASH_DataCacheCmd(DISABLE);//FLASH�����ڼ�,�����ֹ���ݻ���
 		
	addrx=WriteAddr;				//д�����ʼ��ַ
	endaddr=WriteAddr+NumToWrite*4;	//д��Ľ�����ַ
	if(addrx<0X1FFF0000)			//ֻ�����洢��,����Ҫִ�в�������!!
	{
		while(addrx<endaddr)		//ɨ��һ���ϰ�.(�Է�FFFFFFFF�ĵط�,�Ȳ���)
		{
			if(STMFLASH_ReadWord(addrx)!=0XFFFFFFFF)//�з�0XFFFFFFFF�ĵط�,Ҫ�����������
			{   
				status=FLASH_EraseSector(STMFLASH_GetFlashSector(addrx),VoltageRange_3);//VCC=2.7~3.6V֮��!!
				if(status!=FLASH_COMPLETE)break;	//����������
			}else addrx+=4;
		} 
	}
	if(status==FLASH_COMPLETE)
	{
		while(WriteAddr<endaddr)//д����
		{
			if(FLASH_ProgramWord(WriteAddr,*pBuffer)!=FLASH_COMPLETE)//д������
			{ 
				break;	//д���쳣
			}
			WriteAddr+=4;
			pBuffer++;
		} 
	}
  FLASH_DataCacheCmd(ENABLE);	//FLASH��������,�������ݻ���
	FLASH_Lock();//����
} 


/********************************************************************************************************************
                                          ***********************
********************************************************************************************************************/

//datalen �������С��ĳ���
//place   place������ݱ��ֲ��䣬place������step�ռ� 

static u32* SpaceScale( u32 *datatemp,u32 datalen,u32 place,u16 step,u8 flag){
	u32 *pdatatemp=datatemp;
	u32 index=0;
	if(flag==1){//�ռ��������
	  for(index=datalen-1;index>place+step;index--){
		   pdatatemp[index]=pdatatemp[index-step];
		}
		for(;index>place;index--){//����չ�Ŀռ�������
      pdatatemp[index]=0xffffffff;
    }
	}
	else if(flag==0){ //�ռ������С
		for(index=place;index<datalen;index++){
		   pdatatemp[index-step]=pdatatemp[index]; 
		}
  }
	return pdatatemp;
}



#define TABLESTARTADDR   1           //�洢���λ�õ���ʼλ��
#define TABLENAME       "TABLE"
#define TABLENAMELEN    5
DataStoreInode datatable[TABLELEN];  //�洢 ��һ����ͳ��
u32            DATAPACKAGECOUNT=TABLESTARTADDR;//���ݰ�ͳ��  ��һ�������ڴ����ݱ�������ݰ��� �����ݵ��ܳ���
u32            USEVOLUMN=0;       //�����ܳ���
u32            OLDUSEVOLUMN=0;    
u32            *pDatatemp;

static void dataStore(u16 index,u32 *data)
{
	u32 i=0;
	u32 place;
	u16 step;
  	//�洢����
  //�ж��ռ��Ƿ���Ҫ��������  //������ݱ����д����ݲŶ����ݽ������ݿռ������ж�
  if(OLDUSEVOLUMN<USEVOLUMN){ //����洢����������ԭ����Ҫ��  ����USEVOLUMN-OLDUSEVOLUMN���ֽ� 
	  	pDatatemp=(u32*)malloc(sizeof(u32)*USEVOLUMN);        //�����ڴ�
	    STMFLASH_Read(DATASTORE_ADD,pDatatemp,OLDUSEVOLUMN);  //��ȡ�ϵ�����
		
  		place=datatable[index].addr+datatable[index].len;  //��һ�����ݵ���ʼ��ַ
		  step =USEVOLUMN-OLDUSEVOLUMN;                                            //��Ҫ��������ݿռ�
			pDatatemp=SpaceScale(pDatatemp,USEVOLUMN,place,step,1);	//�ռ�����

      memcpy(&pDatatemp[datatable[index].addr],data,datatable[index].len*4);		
		  //�������ݱ��и������ݵ�ƫ�Ƶ�ַ
		  
		//  STMFLASH_Read(TABLESTORE_ADD,(u32*)datatable,sizeof(DataStoreInode));//��ȡ��ͳ�ư�
		  
		  for(i=index;i<DATAPACKAGECOUNT;i++){
			  datatable[i].addr+=step;
			}
			strcpy(datatable[0].chr,TABLENAME); 
	    datatable[0].len=DATAPACKAGECOUNT;  //װ�����ݰ���
	    datatable[0].addr= USEVOLUMN;        //װ�������ܳ���
			STMFLASH_Write(TABLESTORE_ADD,(u32*)datatable,sizeof(DataStoreInode)*DATAPACKAGECOUNT);	//����������
			memset(datatable[0].chr,0,DATANAMELEN);   //���ݱ����¹�����Ҫ���¼���
	}
	else if(OLDUSEVOLUMN>USEVOLUMN){               //����洢����������ԭ������ �����ݿ�����ѹ�� �ڴ洢
		  pDatatemp=(u32*)malloc(sizeof(u32)*OLDUSEVOLUMN);        //�����ڴ�
	    STMFLASH_Read(DATASTORE_ADD,pDatatemp,OLDUSEVOLUMN);  //��ȡ�ϵ�����
		
		  memcpy(&pDatatemp[datatable[index].addr],data,datatable[index].len*4);//���ݿ���
		
		  step =OLDUSEVOLUMN-USEVOLUMN;                                    //����ѹ��
	    place=datatable[index].addr+datatable[index].len;
		  pDatatemp=SpaceScale(pDatatemp,USEVOLUMN,place,step,0);		      //�ռ���С
		  //�������ݱ��и������ݵ�ƫ�Ƶ�ַ
		  
		  for(i=index;i<DATAPACKAGECOUNT;i++){
			  datatable[i].addr-=step;
			}
			strcpy(datatable[0].chr,TABLENAME); 
	    datatable[0].len=DATAPACKAGECOUNT;  //װ�����ݰ���
	    datatable[0].addr= USEVOLUMN;        //װ�������ܳ���
			STMFLASH_Write(TABLESTORE_ADD,(u32*)datatable,sizeof(DataStoreInode)*DATAPACKAGECOUNT);	// ����������
			memset(datatable[0].chr,0,DATANAMELEN);   //���ݱ����¹�����Ҫ���¼���
	}
	else{
	    pDatatemp=(u32*)malloc(sizeof(u32)*USEVOLUMN);        //�����ڴ�
	    STMFLASH_Read(DATASTORE_ADD,pDatatemp,USEVOLUMN);  //��ȡ�ϵ�����
		  memcpy(&pDatatemp[datatable[index].addr],data,datatable[index].len*4);//���ݿ���
		 // printf("%x",pDatatemp[datatable[index].addr]);
	}		
		
	STMFLASH_Write(DATASTORE_ADD,pDatatemp,USEVOLUMN);
	free(pDatatemp);//�ͷſռ�
	OLDUSEVOLUMN=USEVOLUMN;
}

void DataWrite(char *dataname,u32 *data,u32 datalen)//
{	
	u16 index=0;//,i=0
	if(strlen(dataname)<2||strlen(dataname)>DATANAMELEN)return;
	if(datalen<1)         return;
  for(index=TABLESTARTADDR;index<TABLELEN;index++){
	   if(strcmp(datatable[index].chr,dataname)==0){  //������ļ�������Ҫ�����ļ�������
		    if(datatable[index].len!=datalen){
				   USEVOLUMN=USEVOLUMN+datalen-datatable[index].len;//�����ܳ���
				   datatable[index].len=datalen;                    //���½ڵ㳤��
				}		
	      dataStore(index,data);                              //�洢����
				return;
		 }
	}
	 //�����ļ����������洢����  
   
	strcpy(datatable[DATAPACKAGECOUNT].chr,dataname);//�����������ڲ������ݽṹ��
	datatable[DATAPACKAGECOUNT].len=datalen;         //���ݳ��������ݵ�ַ���ڲ�������
	if(DATAPACKAGECOUNT==TABLESTARTADDR)datatable[DATAPACKAGECOUNT].addr=0;//��Ե�ַ
	else
	datatable[DATAPACKAGECOUNT].addr=datatable[DATAPACKAGECOUNT-1].addr +datatable[DATAPACKAGECOUNT-1].len; 
 
	DATAPACKAGECOUNT++;   //���ݰ���һ 
	USEVOLUMN+=datalen;   //���������ܳ���
	strcpy(datatable[0].chr,TABLENAME); 
	datatable[0].len=DATAPACKAGECOUNT;  //װ�����ݰ���
	datatable[0].addr= USEVOLUMN;        //װ�������ܳ���
	STMFLASH_Write(TABLESTORE_ADD,(u32*)datatable,sizeof(DataStoreInode)*DATAPACKAGECOUNT);	//����ʹ�� ����������
	
	
	pDatatemp=(u32*)malloc(sizeof(u32)*USEVOLUMN);        //�����ڴ�
	STMFLASH_Read(DATASTORE_ADD,pDatatemp,USEVOLUMN);  //��ȡ�ϵ�����
	memcpy(&pDatatemp[datatable[DATAPACKAGECOUNT-1].addr],data,datatable[DATAPACKAGECOUNT-1].len*4);//���ݿ���
	
	STMFLASH_Write(DATASTORE_ADD,pDatatemp,USEVOLUMN);
	free(pDatatemp);//�ͷſռ�
	OLDUSEVOLUMN=USEVOLUMN;
}	


void DataRead(char *dataname,u32 *data)
{
	u16 index=0;
	if(strncmp(datatable[0].chr,TABLENAME,TABLENAMELEN)!=0){
		STMFLASH_Read(TABLESTORE_ADD,(u32*)datatable,sizeof(DataStoreInode));//��ȡ��ͳ�ư�
		if(strncmp(datatable[0].chr,TABLENAME,TABLENAMELEN)==0){//��ȡ����ı�����
			 DATAPACKAGECOUNT=datatable[0].len;  //�������ݰ���
		   USEVOLUMN=datatable[0].addr;        //���������ܳ���
		   OLDUSEVOLUMN=USEVOLUMN;
		   STMFLASH_Read(TABLESTORE_ADD,(u32*)datatable,sizeof(DataStoreInode)*DATAPACKAGECOUNT);//���û�ж�ȡ�����ݱ���ȡ���ݱ�
		}
		else return;  //û�д洢����
	}
	if(strlen(dataname)<2||strlen(dataname)>DATANAMELEN)return;
	
	for(index=TABLESTARTADDR;index<DATAPACKAGECOUNT;index++){
	    if(strcmp(datatable[index].chr,dataname)==0){     //�Ƚ������� //����������
				STMFLASH_Read((datatable[index].addr*4)+DATASTORE_ADD,data,datatable[index].len);//ͨ�����ݳ��Ⱥ����ַ ��ȡ����
				break;
			}
	}
		
}


/********************************************************************************************************************
                                          ***********************
************************** ******************************************************************************************/
#define DEBUG
static u8 syserror=0;
static u32 syserrorcount=0;
void setSysError(u8 temp){
	syserror=temp;
	syserrorcount++;
}
void DataStore(char *dataname)
{
		
  TCPCLIENTMESSAGE *ptcp;
	u32_t temp=0;
  if(strcmp(DATA1CH,dataname)==0)
     DataWrite(DATA1CH,(u32*)&DEBUGMESSAGE,1);
  else if(strcmp(RIP,dataname)==0)//"rip:"
	{
		ptcp=GetRemoteAddrHandle();
		temp= (((u32_t)(ptcp->remoteip[3])  << 24)|((u32_t)(ptcp->remoteip[2])  << 16)|((u32_t)(ptcp->remoteip[1])<< 8)|((u32_t)ptcp->remoteip[0]));  
		DataWrite(RIP,(u32*)&temp,1);
  }
	else if(strcmp(RPORT,dataname)==0)//"rport:"
	{
	  ptcp=GetRemoteAddrHandle();
		temp=(u32)ptcp->remoteport;
		DataWrite(RPORT,(u32*)&temp,1);
	}
	else if(strcmp(RMIP,dataname)==0)//"rmip:"
	{
	  ptcp=GetMirroringAddrHandle();
		temp= (((u32_t)(ptcp->remoteip[3])  << 24)|((u32_t)(ptcp->remoteip[2])  << 16)|((u32_t)(ptcp->remoteip[1])<< 8)|((u32_t)ptcp->remoteip[0]));  
		DataWrite(RMIP,(u32*)&temp,1);
	}
	else if(strcmp(RMPORT,dataname)==0)//"rmport:"
	{
	  ptcp=GetMirroringAddrHandle();
		temp=(u32)ptcp->remoteport;
		DataWrite(RMPORT,(u32*)&temp,1);
	}
	else if(strcmp(LOCALIP,dataname)==0)
	{
	  u8* ip=getLocalIp();
		temp= (((u32_t)(ip[3])  << 24)|((u32_t)(ip[2])  << 16)|((u32_t)(ip[1])<< 8)|((u32_t)ip[0]));  
		DataWrite(LOCALIP,(u32*)&temp,1);
	}
	else if(strcmp(LOCALNETMASK,dataname)==0)
	{
	  u8* netmask=getLocalNetmask();
		temp= (((u32_t)(netmask[3])  << 24)|((u32_t)(netmask[2])  << 16)|((u32_t)(netmask[1])<< 8)|((u32_t)netmask[0])); 
		DataWrite(LOCALNETMASK,(u32*)&temp,1);
	}
#ifdef DEBUG
	else if(strcmp(SYSERROR,dataname)==0)//syserror  
	{
		temp=syserror;
	  DataWrite(SYSERROR,(u32*)&temp,1);
		DataWrite(SYSEEORCOUNT,(u32*)&syserrorcount,1);
		
		vu32 headsend =getHeatSendCount();
    vu32 headack  =getHeatAckCount();
    vu32 statesend=getStateSendCount();
    vu32 stateack =getStateAckCount();
    vu32 finishsend=getFinishSendCount();
    vu32 fineshack =getFinishAckCount();
    vu32 startcount=getStartCount();
    vu32 stopcount =getStopCount();
		DataWrite(HEADSEND,(u32*)&headsend,1);
		DataWrite(HEADACK,(u32*)&headack,1);
		DataWrite(STATESEND,(u32*)&statesend,1);
		DataWrite(STATEACK,(u32*)&stateack,1);
		DataWrite(FINISHSEND,(u32*)&finishsend,1);
		DataWrite(FINISHACK,(u32*)&fineshack,1);
		DataWrite(STARTCOUNT,(u32*)&startcount,1);
		DataWrite(STOPCOUNT,(u32*)&stopcount,1);
	}
#endif
}


#define ALL             (strcmp("all",ch)==0)
#define NOTFLASHDATA    ((temp==0XFFFFFFFF)||(temp==0))

void ConfigurationLoaded(char *ch)
{
	u8 i=0;
  u32_t temp=0;
	TCPCLIENTMESSAGE *ptcp;
	
	if((strcmp(DATA1CH,ch)==0)||ALL)
	{
//#ifdef DEBUG		
		DataRead(DATA1CH,(u32*)&DEBUGMESSAGE);
//#else
//    STMFLASH_Write(DATA1_ADD,(u32*)&temp,1);
//#endif			
	 
	}
  if((strcmp(RIP,ch)==0)||ALL)//"rip:"
  {
//#ifdef DEBUG		
		DataRead(RIP,(u32*)&temp);
//#else
//    STMFLASH_Write(DATA2_ADD,(u32*)&temp,1);
//#endif		
		ptcp=GetRemoteAddrHandle();
		ptcp->tcp_thread_open=1;
		if(NOTFLASHDATA)
		{
		  ptcp->remoteip[0]=DEST_IP_ADDR0;
		  ptcp->remoteip[1]=DEST_IP_ADDR1;
		  ptcp->remoteip[2]=DEST_IP_ADDR2;
		  ptcp->remoteip[3]=DEST_IP_ADDR3;
		}
		else 
		{
		  for(i=0;i<4;i++) ptcp->remoteip[i]=((u8*)&temp)[i];
		}
		temp=0;
	}
	if((strcmp(RPORT,ch)==0)||ALL)//"rport:"
	{
//#ifdef DEBUG		
		DataRead(RPORT,(u32*)&temp);
//#else
//    STMFLASH_Write(DATA3_ADD,(u32*)&temp,1);
//#endif			
		ptcp=GetRemoteAddrHandle();
		if(NOTFLASHDATA) ptcp->remoteport=DEST_PORT;
		else                 ptcp->remoteport=temp;
		temp=0;
	}
	if((strcmp(RMIP,ch)==0)||ALL)//"rmip:"
	{
//#ifdef DEBUG		
		DataRead(RMIP,(u32*)&temp);
//#else
//    STMFLASH_Write(DATA4_ADD,(u32*)&temp,1);
//#endif			
		ptcp=GetMirroringAddrHandle();
		ptcp->tcp_thread_open=0;//Ĭ�Ϲر�
	  if(NOTFLASHDATA)
		{
		  ptcp->remoteip[0]=DEST_IP_ADDR0;
		  ptcp->remoteip[1]=DEST_IP_ADDR1;
		  ptcp->remoteip[2]=DEST_IP_ADDR2;
		  ptcp->remoteip[3]=DEST_IP_ADDR3;
		}
		else
		{
			for(i=0;i<4;i++) ptcp->remoteip[i]=((u8*)&temp)[i];
		}
		temp=0;
	}
	if((strcmp(RMPORT,ch)==0)||ALL)//"rmport:"
	{
//#ifdef DEBUG		
		DataRead(RMPORT,(u32*)&temp);
//#else
//    STMFLASH_Write(DATA5_ADD,(u32*)&temp,1);
//#endif		
		ptcp=GetMirroringAddrHandle();
		if(NOTFLASHDATA) ptcp->remoteport=DESTMIRRORINT_PORT;
		else             ptcp->remoteport=temp;
		temp=0;
	}
	if((strcmp(LOCALIP,ch)==0)||ALL)
	{
		//temp= (((u32_t)(ip[3])  << 24)|((u32_t)(ip[2])  << 16)|((u32_t)(ip[1])<< 8)|((u32_t)ip[0]));  
		DataRead(LOCALIP,(u32*)&temp);
		u8* ip=getLocalIp();
		if(NOTFLASHDATA) for(i=0;i<4;i++) ip[i]=0;	
		else{ 
		   for(i=0;i<4;i++) ip[i]=((u8*)&temp)[i];  
		   OSFlagPost (semFlag, (OS_FLAGS)1, OS_FLAG_SET,&semFlagErr);
		}
		temp=0;
	}
	if((strcmp(LOCALNETMASK,ch)==0)||ALL)
	{
		DataRead(LOCALNETMASK,(u32*)&temp);
		u8* netmask=getLocalNetmask();
		if(NOTFLASHDATA) for(i=0;i<4;i++) netmask[i]=0;	
	  else for(i=0;i<4;i++) netmask[i]=((u8*)&temp)[i];	 
    temp=0;		
	}
	
	
#ifdef DEBUG	
	if((strcmp(SYSERROR,ch)==0)||ALL)//syserror  
	{
		DataRead(SYSERROR,(u32*)&temp);
		DataRead(SYSEEORCOUNT,(u32*)&syserrorcount);
		if(temp==1)
		{
			setSysError(0);
		  DataStore(SYSERROR);
      DataRead(HEADSEND,(u32*)&temp);//"headsend"
			setHeatSendCount(temp);
      DataRead(HEADACK,(u32*)&temp);//"headack"
      setHeatAckCount(temp);
			DataRead(STATESEND,(u32*)&temp);//"statesend"
			setStateSendCount(temp);
			DataRead(STATEACK,(u32*)&temp);//"stateack"
			setStateAckCount(temp);
      DataRead(FINISHSEND,(u32*)&temp);//"finishsend"
			setFinishSendCount(temp);
      DataRead(FINISHACK,(u32*)&temp);//"finishack"
			setFinishAckCount(temp);
      DataRead(STARTCOUNT,(u32*)&temp);//"startcount"
			setStartCount(temp);
      DataRead(STOPCOUNT,(u32*)&temp);//"stopcount"		
			setStopCount(temp);
		}
		temp=0;
	}
#endif	
}

//��ָ����ַ��ʼд��ָ�����ȵ�����

//�ú�����OTP����Ҳ��Ч!��������дOTP��!
//OTP�����ַ��Χ:0X1FFF7800~0X1FFF7A0F
//WriteAddr:��ʼ��ַ(�˵�ַ����Ϊ4�ı���!!)
//pBuffer:����ָ��
//NumToWrite:��(32λ)��(����Ҫд���32λ���ݵĸ���.) 

//#define STM_SECTOR_SIZE	(1024*128)

//void STMFLASH_Write(DataStoreTable *pDataList)	
//{ 
//  FLASH_Status status = FLASH_COMPLETE;
//	u32 addrx=0;
//  u8 size=0;	
//  if(pDataList->addr<STM32_FLASH_BASE||pDataList->addr%4)return;	//�Ƿ���ַ
//	FLASH_Unlock();									//���� 
//  FLASH_DataCacheCmd(DISABLE);//FLASH�����ڼ�,�����ֹ���ݻ���
// 		
//	addrx=pDataList->addr;				//д�����ʼ��ַ
//	if(addrx<0X1FFF0000){
//		status=FLASH_EraseSector(STMFLASH_GetFlashSector(addrx),VoltageRange_3);//VCC=2.7~3.6V֮��!! 
//	 if(status!=FLASH_COMPLETE)return;	//����������
//	}

//	if(status==FLASH_COMPLETE)
//	{
//		for(;((pDataList!=NULL)&&(pDataList->addr<(DATASTORE_ADD+STM_SECTOR_SIZE)));pDataList=pDataList->next){
//			for(size=0;size<DATANAMELEN;size++)
//			{
//			   if(FLASH_ProgramWord(pDataList->addr,pDataList->chr[size])!=FLASH_COMPLETE)break;
//			}
//			if(FLASH_ProgramWord(pDataList->addr,pDataList->data)!=FLASH_COMPLETE)break;//д������ //д���쳣�˳�
//		} 
//	}
//  FLASH_DataCacheCmd(ENABLE);	//FLASH��������,�������ݻ���
//	FLASH_Lock();//����
//} 
