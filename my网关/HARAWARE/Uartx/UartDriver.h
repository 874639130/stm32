#ifndef __UARTX_DRIVER_H
#define __UARTX_DRIVER_H

#define UARTX_NUM                   5
#define UARTX_REV_DMA_SIZE           1000
#define UARTX_SEND_DMA_SIZE          1000
#define UARTX_FRAM_NULL              (int)0

typedef  int (*UartSend)(const unsigned  char *data,unsigned short size,unsigned char port);

typedef struct
{
	unsigned char port;
	unsigned int portOpration;
	unsigned int buad;
	unsigned int sendDelay;
	unsigned char *pSend;
	unsigned int sendLen;  //len
	unsigned int sendTime;
	unsigned char revBuf[UARTX_REV_DMA_SIZE];
	unsigned char sendBuf[UARTX_SEND_DMA_SIZE]; //
	unsigned char revTimeOut;
	unsigned int revLen;
	unsigned int revTime;
	UartSend      pSendDataHandle;
}UartxPortStru;



int Init_Uartx(unsigned char port,unsigned int opration,unsigned int baud,unsigned short revtimeout);
int Uart_Send_Data(const unsigned  char *data,unsigned short size,unsigned char port);
UartxPortStru *Get_Uartx_Port_Fram(unsigned char port);

#endif




