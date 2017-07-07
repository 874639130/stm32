#include "includes.h"

OS_EVENT* Sem_UART_Rx_Buf[UARTX_NUM];
OS_EVENT* Sem_UART_Tx_Buf[UARTX_NUM];
static unsigned char gUartRevDmaBuf[UARTX_NUM][UARTX_REV_DMA_SIZE];
static unsigned char gUartSendDmadBuf[UARTX_NUM][UARTX_SEND_DMA_SIZE];

typedef void (*RCC_Fun_Type)(uint32_t,FunctionalState);
//void RCC_AHB1PeriphClockCmd(uint32_t RCC_AHB1Periph, FunctionalState NewState)
//#pragma pack(8)
typedef struct
{
	RCC_Fun_Type Rcc_Fun;
	uint32_t RCC_Periph;
	GPIO_TypeDef* GPIOx;
	uint16_t GPIO_PinSource;
	uint8_t  GPIO_AF;
	uint32_t GPIO_Pin;
}InitGpioxStru;

typedef struct
{
	unsigned char sign;
	RCC_Fun_Type Rcc_Fun;
	uint32_t RCC_Periph;
	GPIO_TypeDef* GPIOx;
	uint16_t GPIO_PinSource;
	uint8_t GPIO_AF;
	uint32_t GPIO_Pin;
}InitRs485EnStru;

typedef struct
{
	RCC_Fun_Type Rcc_Fun;
	uint32_t RCC_Periph;
	USART_TypeDef* USARTx;
	uint8_t IRQChannel;                 
  uint8_t IRQChannelPreemptionPriority;
  uint8_t IRQChannelSubPriority;       
}UartxInitStru;

typedef struct
{
	RCC_Fun_Type Rcc_Fun;
	uint32_t RCC_Periph;
	uint32_t DMA_Channel;
	DMA_Stream_TypeDef* DMAy_Streamx;
	uint8_t IRQChannel;                 
  uint8_t IRQChannelPreemptionPriority;
  uint8_t IRQChannelSubPriority;  
  uint32_t DMA_IT;	
	uint32_t DMA_FLAG;
}InitDmaStru;

typedef struct
{
  InitGpioxStru RxGpio;
	InitGpioxStru TxGpio;
	UartxInitStru Uartx;
	InitDmaStru RxDma;
	InitDmaStru TxDma;
	InitRs485EnStru Rs485En;
}UartxDmaInitStru;


const UartxDmaInitStru UartxDmaInitBuf[]=
{
	{ 
		{RCC_AHB1PeriphClockCmd,  RCC_AHB1Periph_GPIOA, GPIOA,GPIO_PinSource10, GPIO_AF_USART1,GPIO_Pin_10},//
		{RCC_AHB1PeriphClockCmd,  RCC_AHB1Periph_GPIOA, GPIOA,GPIO_PinSource9,  GPIO_AF_USART1,GPIO_Pin_9},
		{RCC_APB2PeriphClockCmd,  RCC_APB2Periph_USART1,USART1,USART1_IRQn,0,0},
		{RCC_AHB1PeriphClockCmd,  RCC_AHB1Periph_DMA2,  DMA_Channel_4,DMA2_Stream5,DMA2_Stream5_IRQn,0,1,DMA_IT_TCIF5,DMA_FLAG_TCIF5},
		{RCC_AHB1PeriphClockCmd,  RCC_AHB1Periph_DMA2  ,DMA_Channel_4,DMA2_Stream7, DMA2_Stream7_IRQn,0,2,DMA_IT_TCIF7,DMA_FLAG_TCIF7},
	  {0,RCC_AHB1PeriphClockCmd,  RCC_AHB1Periph_GPIOA, GPIOA,GPIO_PinSource10, GPIO_AF_USART1,GPIO_Pin_10}
  },
	{
    {RCC_AHB1PeriphClockCmd,  RCC_AHB1Periph_GPIOD, GPIOD,GPIO_PinSource6,  GPIO_AF_USART2,GPIO_Pin_6},
		{RCC_AHB1PeriphClockCmd,  RCC_AHB1Periph_GPIOD, GPIOD,GPIO_PinSource5, GPIO_AF_USART2,GPIO_Pin_5},
		
		{RCC_APB1PeriphClockCmd,  RCC_APB1Periph_USART2,USART2,USART2_IRQn,1,0},
		{RCC_AHB1PeriphClockCmd,  RCC_AHB1Periph_DMA1,  DMA_Channel_4,DMA1_Stream5, DMA1_Stream5_IRQn,1,1,DMA_IT_TCIF5,DMA_FLAG_TCIF5},
		{RCC_AHB1PeriphClockCmd,  RCC_AHB1Periph_DMA1  ,DMA_Channel_4,DMA1_Stream6, DMA1_Stream6_IRQn,1,2,DMA_IT_TCIF6,DMA_FLAG_TCIF6},
	  {1,RCC_AHB1PeriphClockCmd,  RCC_AHB1Periph_GPIOE, GPIOE,GPIO_PinSource4, GPIO_AF_USART2,GPIO_Pin_4}//485
	},
	{
	  {RCC_AHB1PeriphClockCmd,  RCC_AHB1Periph_GPIOC, GPIOC,GPIO_PinSource7,  GPIO_AF_USART6,GPIO_Pin_7},
	  {RCC_AHB1PeriphClockCmd,  RCC_AHB1Periph_GPIOC, GPIOC,GPIO_PinSource6,  GPIO_AF_USART6,GPIO_Pin_6},
		{RCC_APB2PeriphClockCmd,  RCC_APB2Periph_USART6,USART6,USART6_IRQn,2,0},
		{RCC_AHB1PeriphClockCmd,  RCC_AHB1Periph_DMA2,  DMA_Channel_5,DMA2_Stream1,DMA2_Stream1_IRQn,2,1,DMA_IT_TCIF1,DMA_FLAG_TCIF1},//接收
		{RCC_AHB1PeriphClockCmd,  RCC_AHB1Periph_DMA2  ,DMA_Channel_5,DMA2_Stream6, DMA2_Stream6_IRQn,2,2,DMA_IT_TCIF6,DMA_FLAG_TCIF6},//发送
	  {0,RCC_AHB1PeriphClockCmd,  RCC_AHB1Periph_GPIOA, GPIOA,GPIO_PinSource10, GPIO_AF_USART1,GPIO_Pin_10}
	}
};

/*mode:1  发送  0：接收*/

void RS485En(unsigned char port,unsigned char mode)
{
	if(UartxDmaInitBuf[port].Rs485En.sign!=1)return;
	if(mode!=1){
		GPIO_ResetBits(UartxDmaInitBuf[port].Rs485En.GPIOx,UartxDmaInitBuf[port].Rs485En.GPIO_Pin);
	}
	else{
		GPIO_SetBits(UartxDmaInitBuf[port].Rs485En.GPIOx,UartxDmaInitBuf[port].Rs485En.GPIO_Pin);
	}	 				
}

static UartxPortStru gUartxPortBuf[UARTX_NUM];


int Init_Uartx(unsigned char port,unsigned int opration,unsigned int baud,unsigned short revtimeout)
{
	if(port>UARTX_NUM)
	{
	  return -1;
	}
	UartxPortStru *pUartxPort=&gUartxPortBuf[port];
	
  pUartxPort->buad=baud;
	pUartxPort->portOpration=opration;
	pUartxPort->port=port;
	pUartxPort->pSendDataHandle=Uart_Send_Data;//register Uart_Send_Data
	pUartxPort->revTimeOut=revtimeout;
	//gUartxPortBuf[port].pSend=gUartSendDmadBuf[port];
	
	
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;  
	NVIC_InitTypeDef NVIC_InitStructure ; 


	//发送DMA初始化
  UartxDmaInitBuf[pUartxPort->port].TxDma.Rcc_Fun(UartxDmaInitBuf[pUartxPort->port].TxDma.RCC_Periph,ENABLE);
	
	NVIC_InitStructure.NVIC_IRQChannel = UartxDmaInitBuf[pUartxPort->port].TxDma.IRQChannel;  
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = UartxDmaInitBuf[pUartxPort->port].TxDma.IRQChannelPreemptionPriority;  
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = UartxDmaInitBuf[pUartxPort->port].TxDma.IRQChannelSubPriority;  
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
	NVIC_Init(&NVIC_InitStructure);  
 
	DMA_DeInit(UartxDmaInitBuf[pUartxPort->port].TxDma.DMAy_Streamx);  
	DMA_InitStructure.DMA_Channel = UartxDmaInitBuf[pUartxPort->port].TxDma.DMA_Channel;   
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&UartxDmaInitBuf[pUartxPort->port].Uartx.USARTx->DR);  
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)gUartSendDmadBuf[pUartxPort->port];  
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;  
	DMA_InitStructure.DMA_BufferSize = UARTX_SEND_DMA_SIZE;  
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;   
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  
	DMA_InitStructure.DMA_MemoryDataSize = DMA_PeripheralDataSize_Byte;  
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;  
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;      
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;          
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;         	
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;   		 
	DMA_Init(UartxDmaInitBuf[pUartxPort->port].TxDma.DMAy_Streamx, &DMA_InitStructure);    
	DMA_ITConfig(UartxDmaInitBuf[pUartxPort->port].TxDma.DMAy_Streamx,DMA_IT_TC,ENABLE);   
	
	
  //接收DMA初始化
  UartxDmaInitBuf[pUartxPort->port].RxDma.Rcc_Fun(UartxDmaInitBuf[pUartxPort->port].RxDma.RCC_Periph,ENABLE);
  DMA_DeInit(UartxDmaInitBuf[pUartxPort->port].RxDma.DMAy_Streamx);  
  DMA_InitStructure.DMA_Channel = UartxDmaInitBuf[pUartxPort->port].RxDma.DMA_Channel;     
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&UartxDmaInitBuf[pUartxPort->port].Uartx.USARTx->DR);  
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)gUartRevDmaBuf[pUartxPort->port];  
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;  
  DMA_InitStructure.DMA_BufferSize = UARTX_REV_DMA_SIZE;    
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;  
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  
  DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;  
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;       
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;            
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;   
  DMA_Init(UartxDmaInitBuf[pUartxPort->port].RxDma.DMAy_Streamx, &DMA_InitStructure);   
  DMA_Cmd(UartxDmaInitBuf[pUartxPort->port].RxDma.DMAy_Streamx,ENABLE);  
	
	
	  
 
	//串口引脚初始化
	UartxDmaInitBuf[pUartxPort->port].RxGpio.Rcc_Fun(UartxDmaInitBuf[pUartxPort->port].RxGpio.RCC_Periph,ENABLE);
  UartxDmaInitBuf[pUartxPort->port].TxGpio.Rcc_Fun(UartxDmaInitBuf[pUartxPort->port].TxGpio.RCC_Periph,ENABLE);
	GPIO_PinAFConfig(UartxDmaInitBuf[pUartxPort->port].RxGpio.GPIOx,UartxDmaInitBuf[pUartxPort->port].RxGpio.GPIO_PinSource,UartxDmaInitBuf[pUartxPort->port].RxGpio.GPIO_AF);
	GPIO_PinAFConfig(UartxDmaInitBuf[pUartxPort->port].TxGpio.GPIOx,UartxDmaInitBuf[pUartxPort->port].TxGpio.GPIO_PinSource,UartxDmaInitBuf[pUartxPort->port].TxGpio.GPIO_AF);

  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;

  GPIO_InitStructure.GPIO_Pin = UartxDmaInitBuf[pUartxPort->port].TxGpio.GPIO_Pin ;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(UartxDmaInitBuf[pUartxPort->port].TxGpio.GPIOx, &GPIO_InitStructure);

 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; 
  GPIO_InitStructure.GPIO_Pin = UartxDmaInitBuf[pUartxPort->port].RxGpio.GPIO_Pin;
  GPIO_Init(UartxDmaInitBuf[pUartxPort->port].RxGpio.GPIOx, &GPIO_InitStructure);
	
	
	//串口通信初始化
	//串口属性
	UartxDmaInitBuf[pUartxPort->port].Uartx.Rcc_Fun(UartxDmaInitBuf[pUartxPort->port].Uartx.RCC_Periph,ENABLE);
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);		
  /* USART1 mode config */
  USART_InitStructure.USART_BaudRate = pUartxPort->buad;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No ;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(UartxDmaInitBuf[pUartxPort->port].Uartx.USARTx, &USART_InitStructure); 
  USART_Cmd(UartxDmaInitBuf[pUartxPort->port].Uartx.USARTx, ENABLE);
	//使能串口DMA
	USART_DMACmd(UartxDmaInitBuf[pUartxPort->port].Uartx.USARTx,USART_DMAReq_Tx,ENABLE); 
	USART_DMACmd(UartxDmaInitBuf[pUartxPort->port].Uartx.USARTx,USART_DMAReq_Rx,ENABLE); 

	//使能串口中断
	NVIC_InitStructure.NVIC_IRQChannel = UartxDmaInitBuf[pUartxPort->port].Uartx.IRQChannel;                  
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = UartxDmaInitBuf[pUartxPort->port].Uartx.IRQChannelPreemptionPriority;        
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = UartxDmaInitBuf[pUartxPort->port].Uartx.IRQChannelSubPriority;                
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                  
  NVIC_Init(&NVIC_InitStructure);   
	USART_ITConfig(UartxDmaInitBuf[pUartxPort->port].Uartx.USARTx,USART_IT_TC,DISABLE);  
  USART_ITConfig(UartxDmaInitBuf[pUartxPort->port].Uartx.USARTx,USART_IT_RXNE,DISABLE);  
  USART_ITConfig(UartxDmaInitBuf[pUartxPort->port].Uartx.USARTx,USART_IT_TXE,DISABLE);  
  USART_ITConfig(UartxDmaInitBuf[pUartxPort->port].Uartx.USARTx,USART_IT_IDLE,ENABLE);  
	//初始RS485使能引脚
	if(UartxDmaInitBuf[pUartxPort->port].Rs485En.sign==1)
	{
		UartxDmaInitBuf[pUartxPort->port].Rs485En.Rcc_Fun(UartxDmaInitBuf[pUartxPort->port].Rs485En.RCC_Periph,ENABLE);
		GPIO_InitStructure.GPIO_Pin = UartxDmaInitBuf[pUartxPort->port].Rs485En.GPIO_Pin;	
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;   
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; 
		GPIO_Init(UartxDmaInitBuf[pUartxPort->port].Rs485En.GPIOx, &GPIO_InitStructure);	
		RS485En(pUartxPort->port,0);
	}
  
	//初始化接收与发送事件
	Sem_UART_Rx_Buf[pUartxPort->port]=OSSemCreate(0);
	Sem_UART_Tx_Buf[pUartxPort->port]=OSSemCreate(1);
	return 0;

}



void Irq_Uart_Dma_Tx(unsigned char port)  
{  
    if(DMA_GetITStatus(UartxDmaInitBuf[port].TxDma.DMAy_Streamx,UartxDmaInitBuf[port].TxDma.DMA_IT) != RESET)   
    {  
        DMA_ClearFlag(UartxDmaInitBuf[port].TxDma.DMAy_Streamx,UartxDmaInitBuf[port].TxDma.DMA_FLAG);  
        USART_ITConfig(UartxDmaInitBuf[port].Uartx.USARTx,USART_IT_TC,ENABLE);  
    }  
}
//---------------------------------------------------------------------------
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//---------------------------------------------------------------------------
uint8_t Irq_Uart_Rx_End(unsigned char port)  
{     
    uint16_t len = 0;  
    if(USART_GetITStatus(UartxDmaInitBuf[port].Uartx.USARTx, USART_IT_IDLE) != RESET)  
    {  
        UartxDmaInitBuf[port].Uartx.USARTx->SR;  
        UartxDmaInitBuf[port].Uartx.USARTx->DR;  
        
        DMA_Cmd(UartxDmaInitBuf[port].RxDma.DMAy_Streamx,DISABLE);  
        
        DMA_ClearFlag(UartxDmaInitBuf[port].RxDma.DMAy_Streamx,UartxDmaInitBuf[port].RxDma.DMA_FLAG);  
          
        len = UARTX_REV_DMA_SIZE - DMA_GetCurrDataCounter(UartxDmaInitBuf[port].RxDma.DMAy_Streamx);  
			  if(len!=0)
				{
					memcpy(gUartxPortBuf[port].revBuf,gUartRevDmaBuf[port],len);
					gUartxPortBuf[port].revLen=len;
				}
        DMA_SetCurrDataCounter(UartxDmaInitBuf[port].RxDma.DMAy_Streamx,UARTX_REV_DMA_SIZE);  
       
        DMA_Cmd(UartxDmaInitBuf[port].RxDma.DMAy_Streamx,ENABLE);  
				
        OSSemPost(Sem_UART_Rx_Buf[port]);
        
    } 
    return 0;		
}
uint8_t Uartx_Tx_End(unsigned char port)  
{  
    if(USART_GetITStatus(UartxDmaInitBuf[port].Uartx.USARTx, USART_IT_TC) != RESET)  
    {  
			 DMA_Cmd(UartxDmaInitBuf[port].TxDma.DMAy_Streamx,DISABLE); 
       USART_ITConfig(UartxDmaInitBuf[port].Uartx.USARTx,USART_IT_TC,DISABLE);  
			 RS485En(port,0);
			 OSSemPost(Sem_UART_Tx_Buf[port]);
    }  
    return 0;		
} 
void Irq_Uartx_Handler(unsigned char port)                                
{     
    Uartx_Tx_End(port);
	  Irq_Uart_Rx_End(port);  
}  
//---------------------------------------------------------------------------
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//---------------------------------------------------------------------------

UartxPortStru *Get_Uartx_Port_Fram(unsigned char port)
{
	unsigned char err;
	
	UartxPortStru *pUartxPort=&gUartxPortBuf[port];
	
	OSSemPend(Sem_UART_Rx_Buf[pUartxPort->port],pUartxPort->revTimeOut,&err);

	if(err!=OS_ERR_NONE)
	{
		pUartxPort->revLen=UARTX_FRAM_NULL ;
	}
	return (pUartxPort);
}

int Uart_Send_Data(const unsigned  char *data,unsigned short size,unsigned char port)  
{  
	unsigned char err;
	UartxPortStru *pUartxPort=&gUartxPortBuf[port];
	if(size>UARTX_SEND_DMA_SIZE)
	   return -1;
	OSSemPend(Sem_UART_Tx_Buf[pUartxPort->port],5000,&err);
	if(err!=OS_ERR_NONE)
	{
		 return -1;
	}
	
	RS485En(pUartxPort->port,1);
	memcpy(gUartSendDmadBuf[pUartxPort->port],data,size);
	DMA_SetCurrDataCounter(UartxDmaInitBuf[pUartxPort->port].TxDma.DMAy_Streamx,size); 
	UartxDmaInitBuf[pUartxPort->port].TxDma.DMAy_Streamx->M0AR=(uint32_t)gUartSendDmadBuf[pUartxPort->port];
  DMA_Cmd(UartxDmaInitBuf[pUartxPort->port].TxDma.DMAy_Streamx,ENABLE); 
  USART_DMACmd(UartxDmaInitBuf[pUartxPort->port].Uartx.USARTx, USART_DMAReq_Tx, ENABLE);
	return 0;
} 



//const unsigned char Modbus_auchCRCHi[] = 
//{ 
//0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 
//0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
//0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 
//0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
//0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 
//0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 
//0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 
//0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
//0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 
//0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 
//0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 
//0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
//0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 
//0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 
//0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 
//0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
//0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 
//0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
//0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 
//0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
//0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 
//0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 
//0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 
//0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
//0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 
//0x80, 0x41, 0x00, 0xC1, 0x81, 0x40 
//} ; 
///*
//*********************************************************************************************************

//*********************************************************************************************************
//*/
//const unsigned char  Modbus_auchCRCLo[] = 
//{ 
//0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 
//0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD, 
//0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 
//0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 
//0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4, 
//0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3, 
//0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 
//0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4, 
//0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 
//0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29, 
//0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED, 
//0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26, 
//0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 
//0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67, 
//0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 
//0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 
//0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 
//0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5, 
//0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 
//0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92, 
//0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C, 
//0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 
//0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B, 
//0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C, 
//0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 
//0x43, 0x83, 0x41, 0x81, 0x80, 0x40 
//};



//unsigned short Modbus_CRC16(unsigned char *puchMsg, unsigned int usDataLen) 
//{ 
//unsigned char uchCRCHi = 0xFF;
//unsigned char uchCRCLo = 0xFF;
//unsigned int  uIndex;
// while (usDataLen--)
// { 
//  uIndex = uchCRCHi ^ *puchMsg++;
//  uchCRCHi = uchCRCLo ^ Modbus_auchCRCHi[uIndex]; 
//  uchCRCLo = Modbus_auchCRCLo[uIndex]; 
// } 
// return (uchCRCHi << 8 | uchCRCLo); 
//}
///*
//*********************************************************************************************************

//*********************************************************************************************************
//*/
//int Modbus_CRCVerify(unsigned char *pBuf,unsigned int Len)
//{
//unsigned short TempCRC;
//	if(Len<3){return -1;}

//	TempCRC=Modbus_CRC16(pBuf,Len-2);
// 	if(pBuf[Len-2]!=(unsigned char)(TempCRC>>8))
//	{
//	 	return -2;
//	}

// 	if(pBuf[Len-1]!=(unsigned char)(TempCRC&0xff))
//	{
//	 	return -3;
//	}

//	return 0;

//}












