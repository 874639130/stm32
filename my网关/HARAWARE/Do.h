#ifndef __DO_H
#define	__DO_H
#include <stm32f4xx.h>	  
#define  	DO_ERROR_INIT_FAIL  0
#define     DO_SUCCESS			1




#define LP_INDEX                  0
#define DO_OUT_LED_INDEX          1
#define DO_LED2_INDEX             3
#define DO_LED_RUN                2
#define DO_MOTOR_INDEX1           4
#define DO_MOTOR_INDEX2           5
#define DO_ZIGBEE_REST            6
	
typedef struct
{
	uint16_t          GPIOPin;
	GPIO_InitTypeDef*  GPIOx;
	uint32_t 	        RCCGPIOx;
	GPIOMode_TypeDef GPIOMode;
}TDOGpio;
//extern TDOGpio DOGpioBuf[];


#define DO_NUM             7//( sizeof(DOGpioBuf)/sizeof(TDOGpio))
	
int DO_Init (const unsigned char DONum, const unsigned char DOType);
int DO_Write(unsigned char *pDOData);
int DO_Write_Bit(unsigned char value,unsigned char bitNum);
#endif



