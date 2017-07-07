
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

//串口任务1
#define LOCK_COMM_TASK_PRIO            7
#define  LOCK_COMM_TASK_STK_SIZE       64
extern OS_STK gLockCommTaskStk[LOCK_COMM_TASK_STK_SIZE];


//485
#define CHARGING_485_TASK_PRIO            8
#define CHARGING_485_TASK_STK_SIZE        256
extern OS_STK gCharging485TaskStk[CHARGING_485_TASK_STK_SIZE];


//地磁数据
#define DICI_TASK_PRIO            9
#define DICI_TASK_STK_SIZE        256
extern OS_STK DICITaskStk[DICI_TASK_STK_SIZE];


//系统运行任务
#define  RUN_TASK_PRIO                6
#define TASK_STACK_SIZE               64
extern OS_STK   STK_RUN_TASK[TASK_STACK_SIZE];     

//lwip DHCP任务
#define LWIP_DHCP_TASK_PRIO       	 10//设置任务优先级
#define LWIP_DHCP_STK_SIZE  		    128//设置任务堆栈大小
//extern  OS_STK  LWIP_DHCP_TASK_STK[LWIP_DHCP_STK_SIZE];//任务堆栈，采用内存管理的方式控制申请

//tcp客户端
#define TCPCLIENT_PRIO		            11     
#define TCPCLIENT_STK_SIZE	          64             //任务堆栈大小
extern OS_STK TCPCLIENT_TASK_STK[TCPCLIENT_STK_SIZE];//任务堆栈

#define TCPCLIENTM_PRIO		            12     
#define TCPCLIENTM_STK_SIZE	          64             //任务堆栈大小
extern OS_STK TCPCLIENTM_TASK_STK[TCPCLIENT_STK_SIZE];//任务堆栈

//网络数据接收处理
#define SERVERRECVE_PRIO              13
#define Server_Recve_STACK_SIZE       64
extern  OS_STK STK_SERVER_COMM_RECVE[Server_Recve_STACK_SIZE];

//网络数据发送
#define SERVERSEND_PRIO               14
#define Server_Send_STACK_SIZE        64
extern OS_STK STK_SERVER_COMM_SEND[Server_Send_STACK_SIZE];



//lwip内核
//#ifndef TCPIP_THREAD_PRIO

#define TCPIP_THREAD_PRIO		           5	//定义内核任务的优先级为5
#define TCPIP_THREAD_STACKSIZE         1000	//内核任务堆栈大小
//extern OS_STK  TCPIP_THREAD_TASK_STK[TCPIP_THREAD_STACKSIZE];	



//lwip 数据接收
//#define netifINTERFACE_TASK_PRIORITY		         15
//#define netifINTERFACE_TASK_STK_SIZE	           64  //任务堆栈大小
//extern OS_STK netifINTERFACE_TASK_STK[netifINTERFACE_TASK_STK_SIZE];//任务堆栈



extern OS_FLAG_GRP *semFlag;
extern INT8U semFlagErr;  
extern void WaitNetReady(void);
extern void NetReady(void);
#define WAITNETREADY  WaitNetReady()
#define NETREADY      NetReady()


















#endif
