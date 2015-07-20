#include "stdarg.h"
#include "stdio.h"
#include "string.h"
#include "uart4.h"

u8 UART4_TX_BUF[UART4_MAX_SEND_LEN];
u8 UART4_RX_BUF[UART4_MAX_RECV_LEN];
//vu16 UART4_RX_STA=0;
//u8 gps_uart_en = 0;
//u8 gps_tim_cnt = 0;
u8 gps_uart_flag = 0;
u8 GPS_DATA[UART4_MAX_RECV_LEN+1];

void Init_Uart4(u32 baudrate)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  USART_InitTypeDef USART_InitStructure;
  
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4,ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  USART_DeInit(UART4);
  
  GPIO_PinAFConfig(GPIOA,GPIO_PinSource1,GPIO_AF_UART4);
  GPIO_PinAFConfig(GPIOA,GPIO_PinSource0,GPIO_AF_UART4);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOA,&GPIO_InitStructure);
  
  NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0 ;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  USART_InitStructure.USART_BaudRate = baudrate;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  
  USART_Init(UART4, &USART_InitStructure);
  //USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
  USART_ITConfig(UART4, USART_IT_IDLE, ENABLE);

  
  USART_Cmd(UART4, ENABLE);
  Init_Uart4_RxDMA();
}

void Init_Uart4_RxDMA()
{
  NVIC_InitTypeDef NVIC_InitStructure;
  DMA_InitTypeDef  DMA_InitStructure;
    
  /* Enable DMA clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
  
  /* Reset DMA Stream registers (for debug purpose) */
  DMA_DeInit(DMA1_Stream2);
  
  /* Configure DMA Stream */
  DMA_InitStructure.DMA_Channel = DMA_Channel_4;  
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)& UART4->DR;
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)UART4_RX_BUF;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStructure.DMA_BufferSize = (uint32_t)UART4_MAX_RECV_LEN;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;//DMA_Mode_Circular; //DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA1_Stream2, &DMA_InitStructure);
    
  /* Enable DMA Stream Transfer Complete interrupt */
//  DMA_ITConfig(DMA1_Stream2, DMA_IT_TC , ENABLE); //DMA_IT_HT
//  DMA_ClearITPendingBit(DMA1_Stream2, DMA_IT_TCIF2);
  
  USART_DMACmd(UART4, USART_DMAReq_Rx, ENABLE);
  
  /* DMA Stream enable */
  DMA_Cmd(DMA1_Stream2, ENABLE);

  /* Enable the DMA Stream IRQ Channel */
//  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream2_IRQn;
//  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
//  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//  NVIC_Init(&NVIC_InitStructure);  
}

void DMA1_Stream2_IRQHandler(void)
{
  if(DMA_GetITStatus(DMA1_Stream2,DMA_IT_TCIF2))
  {
    DMA_ClearITPendingBit(DMA1_Stream2, DMA_IT_TCIF2);
//    DMA1_Stream2->NDTR = UART4_MAX_RECV_LEN;
//    DMA_Cmd(DMA1_Stream2, ENABLE);
    USART_ITConfig(UART4, USART_IT_IDLE, ENABLE);
    //Init_Uart4_RxDMA();
  }
}

void UART4_IRQHandler(void)
{
//  if(USART_GetITStatus(UART4, USART_IT_RXNE) != RESET)
//  {
//    USART_ClearFlag(UART4, USART_IT_RXNE);
//    USART_ClearFlag(UART4, USART_IT_IDLE);
//    USART_ITConfig(UART4, USART_IT_IDLE, ENABLE);
//    USART_ITConfig(UART4, USART_IT_RXNE, DISABLE);
//    
////    DMA_ClearFlag(DMA1_Stream2,DMA_IT_TCIF2);
////    DMA1_Stream2->NDTR = UART4_MAX_RECV_LEN;
////    DMA_Cmd(DMA1_Stream2, ENABLE);
//    Init_Uart4_RxDMA();
//    USART_Cmd(UART4, ENABLE);
    

//  }
  if(USART_GetITStatus(UART4, USART_IT_IDLE) != RESET)
  {
    u32 len;
    u32 i;
    
    
//    USART_ITConfig(UART4, USART_IT_IDLE, DISABLE);
//    USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
    
    DMA_Cmd(DMA1_Stream2, DISABLE);
    len = UART4_MAX_RECV_LEN - DMA_GetCurrDataCounter(DMA1_Stream2);
    if(len > 0)
    {
      for(i=0;i<len;i++) GPS_DATA[i] = UART4_RX_BUF[i];
      GPS_DATA[i] = 0;
      gps_uart_flag = 1;
    }
    DMA_ClearFlag(DMA1_Stream2,DMA_IT_TCIF2);
    DMA1_Stream2->NDTR = UART4_MAX_RECV_LEN;
    DMA_Cmd(DMA1_Stream2, ENABLE);
    
    USART1->DR = '#';
    
    i = UART4->SR;
    i = UART4->DR;

  }
}



void u4_printf(char* fmt,...)  
{  
  u16 i,j;
  va_list ap;
  va_start(ap,fmt);
  vsprintf((char*)UART4_TX_BUF,fmt,ap);
  va_end(ap);
  i=strlen((const char*)UART4_TX_BUF);
  for(j=0;j<i;j++)
  {
    while(USART_GetFlagStatus(UART4,USART_FLAG_TC)==RESET);
    USART_SendData(UART4,UART4_TX_BUF[j]);  
  }
}
