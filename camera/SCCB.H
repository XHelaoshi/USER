#ifndef _SCCB_H
#define _SCCB_H

#include "stdint.h"
#define SCCB_SIO_C                          GPIO_Pin_6                //SCL
#define SCCB_SCL6_GPIO_PORT                 GPIOE
#define SCCB_SCL6_GPIO_CLK                  RCC_AHB1Periph_GPIOE

#define SCCB_SIO_D                          GPIO_Pin_5                //SDA
#define SCCB_SDA_GPIO_PORT                  GPIOE
#define SCCB_SDA_GPIO_CLK                   RCC_AHB1Periph_GPIOE


#define SCCB_SIC_H()     GPIOE->BSRRL = SCCB_SIO_C           //SCK   PE6
#define SCCB_SIC_L()     GPIOE->BSRRH = SCCB_SIO_C
#define SCCB_SID_H()     GPIOE->BSRRL = SCCB_SIO_D           //SDA   PE5
#define SCCB_SID_L()     GPIOE->BSRRH = SCCB_SIO_D

#define SCCB_SID_STATE	 GPIOE->IDR&(SCCB_SIO_D)

void SCCB_GPIO_Init(void);
void SCCB_SID_DATA_OUT(void);//设置SCCB_SID为输出
void SCCB_SID_DATA_IN(void);//设置SCCB_SID为输入
void SCCB_Start(void);
void SCCB_Stop(void);
uint8_t SCCB_Write(uint8_t m_data);
uint8_t SCCB_Read(void);
void noAck(void);
#endif
