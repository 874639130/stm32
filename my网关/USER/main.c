/**
  ******************************************************************************
  * @file    main.c
  * @author  
  * @version V1.0
  * @date    2015-xx-xx
  * @brief  
  ******************************************************************************
  * @attention
  *

  *
  ******************************************************************************
  */



#include "includes.h"
#include "lwip_comm.h"
#include "lwipopts.h"
#include "lwip/opt.h"
#include "lwip/lwip_sys.h"
#include "lwip/api.h"
#include "tcp_client_demo.h"


//��������

OS_STK   STK_RUN_TASK[TASK_STACK_SIZE];
OS_FLAG_GRP *semFlag;
INT8U semFlagErr;  
INT8U DEBUGMESSAGE=1;
/* Private functions ---------------------------------------------------------*/
/**
  * @brief  ������
  * @param  ��
  * @retval ��
	
  */

void System_Run(void *p_arg);
int main(void)
{
	Stm32_Clock_Init(384,25,2,8);   //����ʱ��,180Mhz   
	RTC_CLK_Config();	
	delay_init(192);                  //��ʼ����ʱ����
	LED_GPIO_Config();
	
	SDRAM_Init(); 	//��ʼ��SDRAM
	//PCF8574_Init();				//PCF8574��ʼ��
	my_mem_init(SRAMIN);		    //��ʼ���ڲ��ڴ��
	my_mem_init(SRAMEX);		    //��ʼ���ⲿ�ڴ��
	my_mem_init(SRAMCCM);		    //��ʼ��CCM�ڴ��

	OSInit();
	OSTaskCreate(System_Run,(void*) 0,	(OS_STK* )&STK_RUN_TASK[TASK_STACK_SIZE - 1],RUN_TASK_PRIO);
	OSStart(); 
}

/**********************************************************************************/

//u8 cha[]="Explorer STM32F407 NETCONN TCP Client send data\r\n";	//TCP�ͻ��˷������ݻ�����
//u8 buf[50];
//void test(void )
//{ 
//	u8 *pData=NULL;
//	u32 recvLen=0;
//	//GetNetDataForMirroringServer(pData,&recvLen,1);
//	if(!GetNetDataForServer((u8**)&pData,&recvLen,1))MESSAGE("%d %s\n",recvLen,pData);
//	buf[0]=buf[0]++;
//	memcpy(&buf[1],cha,strlen((char *)cha));
//	if(SendNetDataToServer(buf,strlen((char*)buf)))printf("send TO service fail\n");
//	
//	if(SendNetDataToMirroringServer(buf,strlen((char*)buf)))printf("send  TO Mirroringservice fail\n");
//}
/**********************************************************************************/

void System_Run(void *p_arg)
{
	//u8 temp[8];
	u16 ledontime=0;
	u16 ledofftime=0;
	
	Init_Uartx(0,0,115200,254);
	//Init_Uartx(1,0,19200,254);
	//Init_Uartx(2,0,115200,254);
	ConfigurationLoaded("all");
  Creat_Lock_Comm_Task();
	while(lwip_comm_init()) 	    //lwip��ʼ��
	{
		printf("Lwip Init failed!\r\n");//lwip��ʼ��ʧ��
		delay_ms(500);
	}
	
	semFlag=OSFlagCreate (0, &semFlagErr);
	lwip_comm_dhcp_creat();
	tcp_client_init(); 
	NETREADY;//�Ѿ����ú�Զ��ip	
	MESSAGE("System_Run\r\n");
	while(1)
	{
		if(getIpFlag())
		{ 
			ledontime=ledofftime=800;
		}
		else
		{
		  ledontime=1500;ledofftime=100;
		}
		LED_RED;
//		//LED1_ON;
		MESSAGE("fdsafdsa\r\n");
//		SendNetDataToServer("fdsafdsa",5);
		OSTimeDly(ledontime);	
//	
    LED_GREEN;		
		MESSAGE("123456\r\n");
//		SendNetDataToServer("fdsafdsa",5);
		OSTimeDly(ledofftime);
	}
}






