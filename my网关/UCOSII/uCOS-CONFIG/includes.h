
#ifndef __INCLUDES_H__
#define __INCLUDES_H__


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdlib.h>
#include "ucos_ii.h"
#include "os_cpu.h"
#include "os_cfg.h"
#include <stm32f4xx.h>	 


   
#include "sdram.h"
#include "malloc.h"
#include "delay.h"
#include "stm32f4xx_it.h"
#include "usart.h"
#include "sys.h"
#include "bsp_led.h" 
#include "UartDriver.h"
#include "UserApp.h"
#include "stmflash.h"
#include "rtc.h"


extern INT8U DEBUGMESSAGE;
extern __IO u8 lwipCoreErrReboot;
#define MESSAGE    printf
//#define MESSAGE(message)  do {printf( message);} while(0)


//#define RS485ENABLE     digitalHi(DE1_GPIO_PORT,DE1_PIN)    
//#define RS485DISABLE    digitalLo(DE1_GPIO_PORT,DE1_PIN)      

//��������1
#define LOCK_COMM_TASK_PRIO            7
#define  LOCK_COMM_TASK_STK_SIZE       64
extern OS_STK gLockCommTaskStk[LOCK_COMM_TASK_STK_SIZE];


//485
#define CHARGING_485_TASK_PRIO            8
#define CHARGING_485_TASK_STK_SIZE        256
extern OS_STK gCharging485TaskStk[CHARGING_485_TASK_STK_SIZE];


//�ش�����
#define DICI_TASK_PRIO            9
#define DICI_TASK_STK_SIZE        256
extern OS_STK DICITaskStk[DICI_TASK_STK_SIZE];


//ϵͳ��������
#define  RUN_TASK_PRIO                6
#define TASK_STACK_SIZE               64
extern OS_STK   STK_RUN_TASK[TASK_STACK_SIZE];     

//lwip DHCP����
#define LWIP_DHCP_TASK_PRIO       	 10//�����������ȼ�
#define LWIP_DHCP_STK_SIZE  		    128//���������ջ��С
//extern  OS_STK  LWIP_DHCP_TASK_STK[LWIP_DHCP_STK_SIZE];//�����ջ�������ڴ����ķ�ʽ��������

//tcp�ͻ���
#define TCPCLIENT_PRIO		            11     
#define TCPCLIENT_STK_SIZE	          64             //�����ջ��С
extern OS_STK TCPCLIENT_TASK_STK[TCPCLIENT_STK_SIZE];//�����ջ

#define TCPCLIENTM_PRIO		            12     
#define TCPCLIENTM_STK_SIZE	          64             //�����ջ��С
extern OS_STK TCPCLIENTM_TASK_STK[TCPCLIENT_STK_SIZE];//�����ջ

//�������ݽ��մ���
#define SERVERRECVE_PRIO              13
#define Server_Recve_STACK_SIZE       64
extern  OS_STK STK_SERVER_COMM_RECVE[Server_Recve_STACK_SIZE];

//�������ݷ���
#define SERVERSEND_PRIO               14
#define Server_Send_STACK_SIZE        64
extern OS_STK STK_SERVER_COMM_SEND[Server_Send_STACK_SIZE];



//lwip�ں�
//#ifndef TCPIP_THREAD_PRIO

#define TCPIP_THREAD_PRIO		           5	//�����ں���������ȼ�Ϊ5
#define TCPIP_THREAD_STACKSIZE         1000	//�ں������ջ��С
//extern OS_STK  TCPIP_THREAD_TASK_STK[TCPIP_THREAD_STACKSIZE];	



//lwip ���ݽ���
//#define netifINTERFACE_TASK_PRIORITY		         15
//#define netifINTERFACE_TASK_STK_SIZE	           64  //�����ջ��С
//extern OS_STK netifINTERFACE_TASK_STK[netifINTERFACE_TASK_STK_SIZE];//�����ջ



extern OS_FLAG_GRP *semFlag;
extern INT8U semFlagErr;  
extern void WaitNetReady(void);
extern void NetReady(void);
#define WAITNETREADY  WaitNetReady()
#define NETREADY      NetReady()


















#endif
