#include "stmflash.h"
#include "tcp_client_demo.h"
#include "lwip_comm.h" 
//读取指定地址的字(32位数据) 
//faddr:读地址 
//返回值:对应数据.
u32 STMFLASH_ReadWord(u32 faddr)
{
	return *(vu32*)faddr; 
}  
//获取某个地址所在的flash扇区
//addr:flash地址
//返回值:0~11,即addr所在的扇区
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


//从指定地址开始读出指定长度的数据
//ReadAddr:起始地址
//pBuffer:数据指针
//NumToRead:字(4位)数
void STMFLASH_Read(u32 ReadAddr,u32 *pBuffer,u32 NumToRead)   	
{
	u32 i;
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i]=STMFLASH_ReadWord(ReadAddr);//读取4个字节.
		ReadAddr+=4;//偏移4个字节.	
	}
}


//从指定地址开始写入指定长度的数据
//特别注意:因为STM32F4的扇区实在太大,没办法本地保存扇区数据,所以本函数
//         写地址如果非0XFF,那么会先擦除整个扇区且不保存扇区数据.所以
//         写非0XFF的地址,将导致整个扇区数据丢失.建议写之前确保扇区里
//         没有重要数据,最好是整个扇区先擦除了,然后慢慢往后写. 
//该函数对OTP区域也有效!可以用来写OTP区!
//OTP区域地址范围:0X1FFF7800~0X1FFF7A0F(注意：最后16字节，用于OTP数据块锁定，别乱写！！)
//WriteAddr:起始地址(此地址必须为4的倍数!!)
//pBuffer:数据指针
//NumToWrite:字(32位)数(就是要写入的32位数据的个数.) 
void STMFLASH_Write(u32 WriteAddr,u32 *pBuffer,u32 NumToWrite)	
{ 
  FLASH_Status status = FLASH_COMPLETE;
	u32 addrx=0;
	u32 endaddr=0;	
  if(WriteAddr<STM32_FLASH_BASE||WriteAddr%4)return;	//非法地址
	FLASH_Unlock();									//解锁 
  FLASH_DataCacheCmd(DISABLE);//FLASH擦除期间,必须禁止数据缓存
 		
	addrx=WriteAddr;				//写入的起始地址
	endaddr=WriteAddr+NumToWrite*4;	//写入的结束地址
	if(addrx<0X1FFF0000)			//只有主存储区,才需要执行擦除操作!!
	{
		while(addrx<endaddr)		//扫清一切障碍.(对非FFFFFFFF的地方,先擦除)
		{
			if(STMFLASH_ReadWord(addrx)!=0XFFFFFFFF)//有非0XFFFFFFFF的地方,要擦除这个扇区
			{   
				status=FLASH_EraseSector(STMFLASH_GetFlashSector(addrx),VoltageRange_3);//VCC=2.7~3.6V之间!!
				if(status!=FLASH_COMPLETE)break;	//发生错误了
			}else addrx+=4;
		} 
	}
	if(status==FLASH_COMPLETE)
	{
		while(WriteAddr<endaddr)//写数据
		{
			if(FLASH_ProgramWord(WriteAddr,*pBuffer)!=FLASH_COMPLETE)//写入数据
			{ 
				break;	//写入异常
			}
			WriteAddr+=4;
			pBuffer++;
		} 
	}
  FLASH_DataCacheCmd(ENABLE);	//FLASH擦除结束,开启数据缓存
	FLASH_Lock();//上锁
} 


/********************************************************************************************************************
                                          ***********************
********************************************************************************************************************/

//datalen 扩充或缩小后的长度
//place   place后的数据保持不变，place后扩充step空间 

static u32* SpaceScale( u32 *datatemp,u32 datalen,u32 place,u16 step,u8 flag){
	u32 *pdatatemp=datatemp;
	u32 index=0;
	if(flag==1){//空间进行扩充
	  for(index=datalen-1;index>place+step;index--){
		   pdatatemp[index]=pdatatemp[index-step];
		}
		for(;index>place;index--){//对扩展的空间进行清空
      pdatatemp[index]=0xffffffff;
    }
	}
	else if(flag==0){ //空间进行缩小
		for(index=place;index<datalen;index++){
		   pdatatemp[index-step]=pdatatemp[index]; 
		}
  }
	return pdatatemp;
}



#define TABLESTARTADDR   1           //存储表的位置的起始位置
#define TABLENAME       "TABLE"
#define TABLENAMELEN    5
DataStoreInode datatable[TABLELEN];  //存储 第一个包统计
u32            DATAPACKAGECOUNT=TABLESTARTADDR;//数据包统计  第一个包用于存数据表的总数据包数 和数据的总长度
u32            USEVOLUMN=0;       //数据总长度
u32            OLDUSEVOLUMN=0;    
u32            *pDatatemp;

static void dataStore(u16 index,u32 *data)
{
	u32 i=0;
	u32 place;
	u16 step;
  	//存储数据
  //判读空间是否需要进行伸缩  //如果数据表中有此数据才对数据进行数据空间伸缩判断
  if(OLDUSEVOLUMN<USEVOLUMN){ //如果存储的数据量比原来的要多  扩充USEVOLUMN-OLDUSEVOLUMN个字节 
	  	pDatatemp=(u32*)malloc(sizeof(u32)*USEVOLUMN);        //分配内存
	    STMFLASH_Read(DATASTORE_ADD,pDatatemp,OLDUSEVOLUMN);  //读取老的数据
		
  		place=datatable[index].addr+datatable[index].len;  //下一个数据的起始地址
		  step =USEVOLUMN-OLDUSEVOLUMN;                                            //需要扩充的数据空间
			pDatatemp=SpaceScale(pDatatemp,USEVOLUMN,place,step,1);	//空间扩充

      memcpy(&pDatatemp[datatable[index].addr],data,datatable[index].len*4);		
		  //更新数据表中各个数据的偏移地址
		  
		//  STMFLASH_Read(TABLESTORE_ADD,(u32*)datatable,sizeof(DataStoreInode));//读取表统计包
		  
		  for(i=index;i<DATAPACKAGECOUNT;i++){
			  datatable[i].addr+=step;
			}
			strcpy(datatable[0].chr,TABLENAME); 
	    datatable[0].len=DATAPACKAGECOUNT;  //装载数据包数
	    datatable[0].addr= USEVOLUMN;        //装载数据总长度
			STMFLASH_Write(TABLESTORE_ADD,(u32*)datatable,sizeof(DataStoreInode)*DATAPACKAGECOUNT);	//更新索引表
			memset(datatable[0].chr,0,DATANAMELEN);   //数据表被更新过，需要重新加载
	}
	else if(OLDUSEVOLUMN>USEVOLUMN){               //如果存储的数据量比原来的少 先数据拷贝后压缩 在存储
		  pDatatemp=(u32*)malloc(sizeof(u32)*OLDUSEVOLUMN);        //分配内存
	    STMFLASH_Read(DATASTORE_ADD,pDatatemp,OLDUSEVOLUMN);  //读取老的数据
		
		  memcpy(&pDatatemp[datatable[index].addr],data,datatable[index].len*4);//数据拷贝
		
		  step =OLDUSEVOLUMN-USEVOLUMN;                                    //数据压缩
	    place=datatable[index].addr+datatable[index].len;
		  pDatatemp=SpaceScale(pDatatemp,USEVOLUMN,place,step,0);		      //空间缩小
		  //更新数据表中各个数据的偏移地址
		  
		  for(i=index;i<DATAPACKAGECOUNT;i++){
			  datatable[i].addr-=step;
			}
			strcpy(datatable[0].chr,TABLENAME); 
	    datatable[0].len=DATAPACKAGECOUNT;  //装载数据包数
	    datatable[0].addr= USEVOLUMN;        //装载数据总长度
			STMFLASH_Write(TABLESTORE_ADD,(u32*)datatable,sizeof(DataStoreInode)*DATAPACKAGECOUNT);	// 更新索引表
			memset(datatable[0].chr,0,DATANAMELEN);   //数据表被更新过，需要重新加载
	}
	else{
	    pDatatemp=(u32*)malloc(sizeof(u32)*USEVOLUMN);        //分配内存
	    STMFLASH_Read(DATASTORE_ADD,pDatatemp,USEVOLUMN);  //读取老的数据
		  memcpy(&pDatatemp[datatable[index].addr],data,datatable[index].len*4);//数据拷贝
		 // printf("%x",pDatatemp[datatable[index].addr]);
	}		
		
	STMFLASH_Write(DATASTORE_ADD,pDatatemp,USEVOLUMN);
	free(pDatatemp);//释放空间
	OLDUSEVOLUMN=USEVOLUMN;
}

void DataWrite(char *dataname,u32 *data,u32 datalen)//
{	
	u16 index=0;//,i=0
	if(strlen(dataname)<2||strlen(dataname)>DATANAMELEN)return;
	if(datalen<1)         return;
  for(index=TABLESTARTADDR;index<TABLELEN;index++){
	   if(strcmp(datatable[index].chr,dataname)==0){  //有这个文件，不需要更新文件索引表
		    if(datatable[index].len!=datalen){
				   USEVOLUMN=USEVOLUMN+datalen-datatable[index].len;//更新总长度
				   datatable[index].len=datalen;                    //更新节点长度
				}		
	      dataStore(index,data);                              //存储数据
				return;
		 }
	}
	 //更新文件索引表，并存储数据  
   
	strcpy(datatable[DATAPACKAGECOUNT].chr,dataname);//数据名，用于查找数据结构体
	datatable[DATAPACKAGECOUNT].len=datalen;         //数据长度与数据地址用于查找数据
	if(DATAPACKAGECOUNT==TABLESTARTADDR)datatable[DATAPACKAGECOUNT].addr=0;//相对地址
	else
	datatable[DATAPACKAGECOUNT].addr=datatable[DATAPACKAGECOUNT-1].addr +datatable[DATAPACKAGECOUNT-1].len; 
 
	DATAPACKAGECOUNT++;   //数据包加一 
	USEVOLUMN+=datalen;   //计算数据总长度
	strcpy(datatable[0].chr,TABLENAME); 
	datatable[0].len=DATAPACKAGECOUNT;  //装载数据包数
	datatable[0].addr= USEVOLUMN;        //装载数据总长度
	STMFLASH_Write(TABLESTORE_ADD,(u32*)datatable,sizeof(DataStoreInode)*DATAPACKAGECOUNT);	//开机使用 更新索引表
	
	
	pDatatemp=(u32*)malloc(sizeof(u32)*USEVOLUMN);        //分配内存
	STMFLASH_Read(DATASTORE_ADD,pDatatemp,USEVOLUMN);  //读取老的数据
	memcpy(&pDatatemp[datatable[DATAPACKAGECOUNT-1].addr],data,datatable[DATAPACKAGECOUNT-1].len*4);//数据拷贝
	
	STMFLASH_Write(DATASTORE_ADD,pDatatemp,USEVOLUMN);
	free(pDatatemp);//释放空间
	OLDUSEVOLUMN=USEVOLUMN;
}	


void DataRead(char *dataname,u32 *data)
{
	u16 index=0;
	if(strncmp(datatable[0].chr,TABLENAME,TABLENAMELEN)!=0){
		STMFLASH_Read(TABLESTORE_ADD,(u32*)datatable,sizeof(DataStoreInode));//读取表统计包
		if(strncmp(datatable[0].chr,TABLENAME,TABLENAMELEN)==0){//读取其余的表数据
			 DATAPACKAGECOUNT=datatable[0].len;  //加载数据包数
		   USEVOLUMN=datatable[0].addr;        //加载数据总长度
		   OLDUSEVOLUMN=USEVOLUMN;
		   STMFLASH_Read(TABLESTORE_ADD,(u32*)datatable,sizeof(DataStoreInode)*DATAPACKAGECOUNT);//如果没有读取过数据表，读取数据表
		}
		else return;  //没有存储数据
	}
	if(strlen(dataname)<2||strlen(dataname)>DATANAMELEN)return;
	
	for(index=TABLESTARTADDR;index<DATAPACKAGECOUNT;index++){
	    if(strcmp(datatable[index].chr,dataname)==0){     //比较数据名 //搜索数据名
				STMFLASH_Read((datatable[index].addr*4)+DATASTORE_ADD,data,datatable[index].len);//通过数据长度和相地址 获取数据
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
		ptcp->tcp_thread_open=0;//默认关闭
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

//从指定地址开始写入指定长度的数据

//该函数对OTP区域也有效!可以用来写OTP区!
//OTP区域地址范围:0X1FFF7800~0X1FFF7A0F
//WriteAddr:起始地址(此地址必须为4的倍数!!)
//pBuffer:数据指针
//NumToWrite:字(32位)数(就是要写入的32位数据的个数.) 

//#define STM_SECTOR_SIZE	(1024*128)

//void STMFLASH_Write(DataStoreTable *pDataList)	
//{ 
//  FLASH_Status status = FLASH_COMPLETE;
//	u32 addrx=0;
//  u8 size=0;	
//  if(pDataList->addr<STM32_FLASH_BASE||pDataList->addr%4)return;	//非法地址
//	FLASH_Unlock();									//解锁 
//  FLASH_DataCacheCmd(DISABLE);//FLASH擦除期间,必须禁止数据缓存
// 		
//	addrx=pDataList->addr;				//写入的起始地址
//	if(addrx<0X1FFF0000){
//		status=FLASH_EraseSector(STMFLASH_GetFlashSector(addrx),VoltageRange_3);//VCC=2.7~3.6V之间!! 
//	 if(status!=FLASH_COMPLETE)return;	//发生错误了
//	}

//	if(status==FLASH_COMPLETE)
//	{
//		for(;((pDataList!=NULL)&&(pDataList->addr<(DATASTORE_ADD+STM_SECTOR_SIZE)));pDataList=pDataList->next){
//			for(size=0;size<DATANAMELEN;size++)
//			{
//			   if(FLASH_ProgramWord(pDataList->addr,pDataList->chr[size])!=FLASH_COMPLETE)break;
//			}
//			if(FLASH_ProgramWord(pDataList->addr,pDataList->data)!=FLASH_COMPLETE)break;//写入数据 //写入异常退出
//		} 
//	}
//  FLASH_DataCacheCmd(ENABLE);	//FLASH擦除结束,开启数据缓存
//	FLASH_Lock();//上锁
//} 
