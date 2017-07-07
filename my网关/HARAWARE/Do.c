/*********************************************************************************************************

*********************************************************************************************************/

#include "Do.h"
unsigned char gDONum=0;


//DO�ܽŽṹ���ʼ��
const TDOGpio DOGpioBuf[]=
{                    
  {GPIO_Pin_5 ,GPIOB,RCC_AHB1Periph_GPIOD,GPIO_OType_PP},		   //LP���   0
  {GPIO_Pin_4 ,GPIOB,RCC_AHB1Periph_GPIOD,GPIO_OType_PP}, 		 // ����    1
 
  {GPIO_Pin_11,GPIOB,RCC_AHB1Periph_GPIOD,GPIO_OType_PP}, 		//�ڲ�LED1   2  **
	{GPIO_Pin_4 ,GPIOA,RCC_AHB1Periph_GPIOD,GPIO_OType_PP}, 		//�ڲ�LED2   3  **

  {GPIO_Pin_6 ,GPIOA,RCC_AHB1Periph_GPIOD,GPIO_OType_PP},         //���Ƶ��1   4
  {GPIO_Pin_7 ,GPIOA,RCC_AHB1Periph_GPIOD,GPIO_OType_PP},          //���Ƶ��2  5	
	{GPIO_Pin_1 ,GPIOA,RCC_AHB1Periph_GPIOD,GPIO_OType_PP},          //���Ƶ��2  5	
	
};

int DO_Init (const unsigned char DONum, const unsigned char DOType) 
{
	unsigned char index; 
	GPIO_InitTypeDef GPIO_InitStructure;
	if(DONum>(sizeof(DOGpioBuf)/sizeof(TDOGpio)))
	{
	    return 	DO_ERROR_INIT_FAIL;
	}
	for(index=0;index<DONum;index++)
	{	  		
	  RCC_AHB1PeriphClockCmd(DOGpioBuf[index].RCCGPIOx, ENABLE);
		GPIO_InitStructure.GPIO_Pin=DOGpioBuf[index].GPIOPin;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	  GPIO_InitStructure.GPIO_Mode = DOGpioBuf[index].GPIOMode ;
		GPIO_Init(DOGpioBuf[index].GPIOx, &GPIO_InitStructure);
		GPIO_SetBits(DOGpioBuf[index].GPIOx,DOGpioBuf[index].GPIOPin);
	}
	gDONum=DONum;
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	
	
	
	//GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	//GPIO_SetBits(DOGpioBuf[DO_ZIGBEE_REST].GPIOx,DOGpioBuf[DO_ZIGBEE_REST].GPIOPin);
	
	
	return DO_SUCCESS;
} 


int DO_Write(unsigned char *pDOData)
{	
	int index; 
	for(index=0;index<gDONum;index++)
	{
		
			if(pDOData[index]!=1)											   
			{
					GPIO_ResetBits(DOGpioBuf[index].GPIOx,DOGpioBuf[index].GPIOPin);
			}
			else
			{
					GPIO_SetBits(DOGpioBuf[index].GPIOx,DOGpioBuf[index].GPIOPin);
			}	
	
	}
	return 0;
}
int DO_Write_Bit(unsigned char value,unsigned char bitNum)
{
	if(value!=0)											   
	{
			GPIO_SetBits(DOGpioBuf[bitNum].GPIOx,DOGpioBuf[bitNum].GPIOPin);
	}
	else
	{
			GPIO_ResetBits(DOGpioBuf[bitNum].GPIOx,DOGpioBuf[bitNum].GPIOPin);
	}	
	return 0;
}





