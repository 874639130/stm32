#ifndef __STMFLASH_H__
#define __STMFLASH_H__
#include "includes.h"   




//FLASH��ʼ��ַ
#define STM32_FLASH_BASE 0x08000000 	//STM32 FLASH����ʼ��ַ
 

//FLASH ��������ʼ��ַ
#define ADDR_FLASH_SECTOR_0     ((u32)0x08000000) 	//����0��ʼ��ַ, 16 Kbytes  
#define ADDR_FLASH_SECTOR_1     ((u32)0x08004000) 	//����1��ʼ��ַ, 16 Kbytes  
#define ADDR_FLASH_SECTOR_2     ((u32)0x08008000) 	//����2��ʼ��ַ, 16 Kbytes  
#define ADDR_FLASH_SECTOR_3     ((u32)0x0800C000) 	//����3��ʼ��ַ, 16 Kbytes  
#define ADDR_FLASH_SECTOR_4     ((u32)0x08010000) 	//����4��ʼ��ַ, 64 Kbytes  
#define ADDR_FLASH_SECTOR_5     ((u32)0x08020000) 	//����5��ʼ��ַ, 128 Kbytes  
#define ADDR_FLASH_SECTOR_6     ((u32)0x08040000) 	//����6��ʼ��ַ, 128 Kbytes  
#define ADDR_FLASH_SECTOR_7     ((u32)0x08060000) 	//����7��ʼ��ַ, 128 Kbytes  
#define ADDR_FLASH_SECTOR_8     ((u32)0x08080000) 	//����8��ʼ��ַ, 128 Kbytes  
#define ADDR_FLASH_SECTOR_9     ((u32)0x080A0000) 	//����9��ʼ��ַ, 128 Kbytes  
#define ADDR_FLASH_SECTOR_10    ((u32)0x080C0000) 	//����10��ʼ��ַ,128 Kbytes  
#define ADDR_FLASH_SECTOR_11    ((u32)0x080E0000) 	//����11��ʼ��ַ,128 Kbytes  


#define TABLESTORE_ADD      ADDR_FLASH_SECTOR_10
#define DATASTORE_ADD       ADDR_FLASH_SECTOR_11



//�û��洢
#define DATA1_ADD           DATASTORE_ADD+8
#define DATA2_ADD           DATA1_ADD+8
#define DATA3_ADD           DATA2_ADD+8
#define DATA4_ADD           DATA3_ADD+8
#define DATA5_ADD           DATA4_ADD+8
/******************************************************************************/

#define DATA1CH        "DEBUG"

#define RIP           "rip:"
#define RPORT         "rport:"
#define RMIP          "rmip:"
#define RMPORT        "rmport:"
#define LOCALIP       "localip:"
#define LOCALNETMASK  "localnetmask:"

#define SYSERROR      "syserror"
#define SYSEEORCOUNT  "syserrorcount"
#define HEADSEND      "headsend"
#define	HEADACK       "headack"
#define	STATESEND     "statesend"
#define	STATEACK      "stateack"
#define	FINISHSEND    "finishsend"
#define	FINISHACK     "finishack"
#define	STARTCOUNT    "startcount"
#define	STOPCOUNT     "stopcount"
/******************************************************************************/
#define DATANAMELEN        24
#define TABLELEN           20
typedef struct storetable
{
  u32   addr;                //ָ�����ݵ�ָ��(��Ե�ַ)
	u32   len;                 //���ݵĳ���
	char  chr[DATANAMELEN];    //���ݵ�����
}DataStoreInode;
extern DataStoreInode datatable[TABLELEN]; 


//u32 STMFLASH_ReadWord(u32 faddr);		  	//������  
//void STMFLASH_Write(u32 WriteAddr,u32 *pBuffer,u32 NumToWrite);		//��ָ����ַ��ʼд��ָ�����ȵ�����
void setSysError(u8 temp);
//void STMFLASH_Read(u32 ReadAddr,u32 *pBuffer,u32 NumToRead);   		//��ָ����ַ��ʼ����ָ�����ȵ�����
void ConfigurationLoaded(char *ch);		
//void DataStore(char *dataname,u32 *data,u32 datalen);
void DataStore(char *dataname);


#endif

















