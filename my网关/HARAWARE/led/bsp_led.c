/**
  ******************************************************************************
  * @file    bsp_led.c
  * @author  fire
  * @version V1.0
  * @date    2015-xx-xx
  * @brief   led应用函数接口
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */
  
#include "includes.h" 

 /**
  * @brief  初始化控制LED的IO
  * @param  无
  * @retval 无
  */
//typedef struct
//{
//	uint16_t           GPIOPin;  //引脚
//	GPIO_TypeDef*      GPIOx;    //引脚组名
//	uint32_t 	         RCCGPIOx; //时钟
//	GPIOMode_TypeDef   GPIOMode; //模式
//	GPIOOType_TypeDef  GPIO_OType;//输出类型
//	GPIOPuPd_TypeDef   GPIO_PuPd;//设置上下啦
//}TDOGpio;
//输入引脚定义
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
		RCC_AHB1PeriphClockCmd(TDIGpioBuf[index].RCCGPIOx ,ENABLE);/*开启按键GPIO口的时钟*/
		 
		GPIO_InitStructure.GPIO_Pin = TDIGpioBuf[index].GPIOPin; /*选择按键的引脚*/
		 
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN; /*设置引脚为输入模式*/
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//GPIO_PuPd_NOPULL;/*设置引脚不上拉也不下拉*/
		
  	GPIO_Init(TDIGpioBuf[index].GPIOx , &GPIO_InitStructure);  /*使用上面的结构体初始化按键*/
  }
	return 0;
}


#define STATICID  6   //静态地址


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
		/*定义一个GPIO_InitTypeDef类型的结构体*/
		GPIO_InitTypeDef GPIO_InitStructure;

		/*开启LED相关的GPIO外设时钟*/
		RCC_AHB1PeriphClockCmd ( LED1_GPIO_CLK|LED2_GPIO_CLK|LED3_GPIO_CLK|LED4_GPIO_CLK|DE1_GPIO_CLK, ENABLE); 
		GPIO_InitStructure.GPIO_Pin = LED1_PIN;		/*选择要控制的GPIO引脚*/		
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;  /*设置引脚模式为输出模式*/ 
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; /*设置引脚的输出类型为推挽输出*/
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; /*设置引脚为上拉模式，默认LED亮*/
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; /*设置引脚速率为50MHz */   

		/*调用库函数，使用上面配置的GPIO_InitStructure初始化GPIO*/
		GPIO_Init(LED1_GPIO_PORT, &GPIO_InitStructure);	
    
    /*选择要控制的GPIO引脚*/															   
		GPIO_InitStructure.GPIO_Pin = LED2_PIN;	
    GPIO_Init(LED2_GPIO_PORT, &GPIO_InitStructure);	
    
    /*选择要控制的GPIO引脚*/															   
		GPIO_InitStructure.GPIO_Pin = LED3_PIN;	
    GPIO_Init(LED3_GPIO_PORT, &GPIO_InitStructure);	
		
		/*选择要控制的GPIO引脚*/															   
		GPIO_InitStructure.GPIO_Pin = LED4_PIN;	
    GPIO_Init(LED4_GPIO_PORT, &GPIO_InitStructure);	
		
		GPIO_InitStructure.GPIO_Pin = DE1_PIN;	
    GPIO_Init(DE1_GPIO_PORT, &GPIO_InitStructure);	
		
		/*关闭RGB灯*/
		//LED_RGBOFF;
		
		/*指示灯默认开启*/
		LED4(ON);
		LED1_ON;
		//DI_Init(8);
		
}
/*********************************************END OF FILE**********************/


