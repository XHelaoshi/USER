#include "stdarg.h"
#include "stdio.h"
#include "string.h"
#include "usart1.h"

u8 USART1_TX_BUF[1024];

void Init_Usart1(u32 baudrate)
{
  GPIO_InitTypeDef GPIO_InitStructure;
//  NVIC_InitTypeDef NVIC_InitStructure;
  USART_InitTypeDef USART_InitStructure;
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  USART_DeInit(USART1);
  
  GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1);
  GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOA,&GPIO_InitStructure);
  
//  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
//  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0 ;
//  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
//  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//  NVIC_Init(&NVIC_InitStructure);

  USART_InitStructure.USART_BaudRate = baudrate;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  
  USART_Init(USART1, &USART_InitStructure);
//  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
  USART_Cmd(USART1, ENABLE);
}

//void USART1_IRQHandler(void)
//{
//  if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
//  {
//  }                     
//}

void u1_printf(char* fmt,...)  
{  
  u16 i,j;
  va_list ap;
  va_start(ap,fmt);
  vsprintf((char*)USART1_TX_BUF,fmt,ap);
  va_end(ap);
  i=strlen((const char*)USART1_TX_BUF);
  for(j=0;j<i;j++)
  {
    USART_SendData(USART1,USART1_TX_BUF[j]);
    while(USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET);
  }
}
