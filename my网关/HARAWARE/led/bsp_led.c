/**
  ******************************************************************************
  * @file    bsp_led.c
  * @author  fire
  * @version V1.0
  * @date    2015-xx-xx
  * @brief   ledӦ�ú����ӿ�
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */
  
#include "includes.h" 

 /**
  * @brief  ��ʼ������LED��IO
  * @param  ��
  * @retval ��
  */
//typedef struct
//{
//	uint16_t           GPIOPin;  //����
//	GPIO_TypeDef*      GPIOx;    //��������
//	uint32_t 	         RCCGPIOx; //ʱ��
//	GPIOMode_TypeDef   GPIOMode; //ģʽ
//	GPIOOType_TypeDef  GPIO_OType;//�������
//	GPIOPuPd_TypeDef   GPIO_PuPd;//����������
//}TDOGpio;
//�������Ŷ���
/*******************************************************/
//#define     KEY1          	PI0
//#define     KEY2	          PH15
//#define     KEY3	          PG10
//#define     KEY4          	PH13
//#define     KEY5          	PG6
//#define     KEY6          	PA12
//#define     KEY7          	PB0
//#define     KEY8          	PB1
/*******************************************************/
const TDIGpio TDIGpioBuf[]=
{
  {GPIO_Pin_0,GPIOI,RCC_AHB1Periph_GPIOI},
  {GPIO_Pin_15,GPIOH,RCC_AHB1Periph_GPIOH},
  {GPIO_Pin_10,GPIOG,RCC_AHB1Periph_GPIOG},
  {GPIO_Pin_13,GPIOH,RCC_AHB1Periph_GPIOH},
  {GPIO_Pin_6,GPIOG,RCC_AHB1Periph_GPIOG},
	{GPIO_Pin_12,GPIOA,RCC_AHB1Periph_GPIOA},
	{GPIO_Pin_0,GPIOB,RCC_AHB1Periph_GPIOB},
	{GPIO_Pin_1,GPIOB,RCC_AHB1Periph_GPIOB},
};

int DI_Init(const unsigned char DINum)
{
	unsigned char index;
	GPIO_InitTypeDef GPIO_InitStructure;
  if(DINum>(sizeof(TDIGpioBuf)/sizeof(TDIGpio)))
	{
	   return 1;
	}	
	
	for(index=0;index<DINum;index++)
	{
		RCC_AHB1PeriphClockCmd(TDIGpioBuf[index].RCCGPIOx ,ENABLE);/*��������GPIO�ڵ�ʱ��*/
		 
		GPIO_InitStructure.GPIO_Pin = TDIGpioBuf[index].GPIOPin; /*ѡ�񰴼�������*/
		 
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN; /*��������Ϊ����ģʽ*/
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//GPIO_PuPd_NOPULL;/*�������Ų�����Ҳ������*/
		
  	GPIO_Init(TDIGpioBuf[index].GPIOx , &GPIO_InitStructure);  /*ʹ������Ľṹ���ʼ������*/
  }
	return 0;
}


#define STATICID  6   //��̬��ַ


static u8 keybuf[sizeof(TDIGpioBuf)/sizeof(TDIGpio)];
static u8 gLockID=0;
u8* DI_Read(void)
{
  int index;
	int gDiNum=sizeof(TDIGpioBuf)/sizeof(TDIGpio);
	gLockID=0;
	for(index=0;index<gDiNum;index++)
	{
		if((TDIGpioBuf[index].GPIOx)->IDR  & TDIGpioBuf[index].GPIOPin)
		{
		   keybuf[index]=1;
		}
		else 
		{  
		   keybuf[index]=0;
 			 gLockID|=(1<<(7-index));
		}
	}
	gLockID=STATICID;
	return &gLockID;      
}
	
	
	
	 
void LED_GPIO_Config(void)
{		
		/*����һ��GPIO_InitTypeDef���͵Ľṹ��*/
		GPIO_InitTypeDef GPIO_InitStructure;

		/*����LED��ص�GPIO����ʱ��*/
		RCC_AHB1PeriphClockCmd ( LED1_GPIO_CLK|LED2_GPIO_CLK|LED3_GPIO_CLK|LED4_GPIO_CLK|DE1_GPIO_CLK, ENABLE); 
		GPIO_InitStructure.GPIO_Pin = LED1_PIN;		/*ѡ��Ҫ���Ƶ�GPIO����*/		
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;  /*��������ģʽΪ���ģʽ*/ 
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; /*�������ŵ��������Ϊ�������*/
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; /*��������Ϊ����ģʽ��Ĭ��LED��*/
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; /*������������Ϊ50MHz */   

		/*���ÿ⺯����ʹ���������õ�GPIO_InitStructure��ʼ��GPIO*/
		GPIO_Init(LED1_GPIO_PORT, &GPIO_InitStructure);	
    
    /*ѡ��Ҫ���Ƶ�GPIO����*/															   
		GPIO_InitStructure.GPIO_Pin = LED2_PIN;	
    GPIO_Init(LED2_GPIO_PORT, &GPIO_InitStructure);	
    
    /*ѡ��Ҫ���Ƶ�GPIO����*/															   
		GPIO_InitStructure.GPIO_Pin = LED3_PIN;	
    GPIO_Init(LED3_GPIO_PORT, &GPIO_InitStructure);	
		
		/*ѡ��Ҫ���Ƶ�GPIO����*/															   
		GPIO_InitStructure.GPIO_Pin = LED4_PIN;	
    GPIO_Init(LED4_GPIO_PORT, &GPIO_InitStructure);	
		
		GPIO_InitStructure.GPIO_Pin = DE1_PIN;	
    GPIO_Init(DE1_GPIO_PORT, &GPIO_InitStructure);	
		
		/*�ر�RGB��*/
		//LED_RGBOFF;
		
		/*ָʾ��Ĭ�Ͽ���*/
		LED4(ON);
		LED1_ON;
		//DI_Init(8);
		
}
/*********************************************END OF FILE**********************/


