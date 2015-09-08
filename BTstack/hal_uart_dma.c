#include <rtthread.h>
#include "stm32f4xx_usart.h"
#include "stm32f4xx_gpio.h"
#include <stm32f4xx.h>
#include <stm32f4xx_dma.h>

#include "hal_uart_dma.h"
#include "rda5875.h"

//#define UART_IO_PERIPH    RCC_AHB1Periph_GPIOG
//#define UART_IO_PORT      GPIOG
//#define UART_IO_TX        GPIO_Pin_14
//#define UART_IO_RX        GPIO_Pin_9
//#define UART_PERIPH       RCC_APB2Periph_GPIOA
#define TX_DMA_CHANNEL DMA1_Stream3
#define BT_USART USART3
#define RX_DMA_CHANNEL DMA1_Stream1

void bt_usart_init(u32 br){

//	GPIO_InitTypeDef  GPIO_InitStructure;
//	USART_InitTypeDef USART_InitStructure;  
//	if(br==0)br = 115200;
//	USART_Cmd(BT_USART,DISABLE);
//	USART_DMACmd(BT_USART,USART_DMAReq_Tx,DISABLE);  
//	//IO
//	///RX
//	RCC_APB2PeriphClockCmd(UART_IO_PERIPH,ENABLE);
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
//  GPIO_InitStructure.GPIO_Pin = UART_IO_RX;
//  GPIO_Init(UART_IO_PORT, &GPIO_InitStructure);
//	///TX
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
//  GPIO_InitStructure.GPIO_Pin = UART_IO_TX;
//  GPIO_Init(UART_IO_PORT, &GPIO_InitStructure);
//	
//	//UART
//	/* Enable UART clock */
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
//	USART_InitStructure.USART_BaudRate = br;
//	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
//	USART_InitStructure.USART_StopBits = USART_StopBits_1; 
//	USART_InitStructure.USART_Parity = USART_Parity_No; 
//	USART_InitStructure.USART_HardwareFlowControl = 
//	USART_HardwareFlowControl_None; 
//	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
//	USART_Init(BT_USART, &USART_InitStructure);

//	USART_Cmd(BT_USART,ENABLE);
	USART_InitTypeDef USART_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
	if(br==0)br = 115200;	
	USART_Cmd(BT_USART,DISABLE);
	USART_DMACmd(BT_USART,USART_DMAReq_Tx,DISABLE);  
	RCC_AHB1PeriphClockCmd(USARTx_TX_GPIO_CLK | USARTx_RX_GPIO_CLK, ENABLE);  
  /* Enable USART clock */
  USARTx_CLK_INIT(USARTx_CLK, ENABLE);
  /* Enable the DMA clock */
//  RCC_AHB1PeriphClockCmd(USARTx_DMAx_CLK, ENABLE);
  
  /* USARTx GPIO configuration -----------------------------------------------*/ 
  /* Connect USART pins to AF7 */
  GPIO_PinAFConfig(USARTx_TX_GPIO_PORT, USARTx_TX_SOURCE, USARTx_TX_AF);
  GPIO_PinAFConfig(USARTx_RX_GPIO_PORT, USARTx_RX_SOURCE, USARTx_RX_AF);
  
  /* Configure USART Tx and Rx as alternate function push-pull */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  
  GPIO_InitStructure.GPIO_Pin = USARTx_TX_PIN;
  GPIO_Init(USARTx_TX_GPIO_PORT, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = USARTx_RX_PIN;
  GPIO_Init(USARTx_RX_GPIO_PORT, &GPIO_InitStructure);
 
  /* USARTx configuration ----------------------------------------------------*/
  /* Enable the USART OverSampling by 8 */
  //USART_OverSampling8Cmd(USARTx, ENABLE); 
  
  /* USARTx configured as follows:
        - BaudRate = 5250000 baud
		   - Maximum BaudRate that can be achieved when using the Oversampling by 8
		     is: (USART APB Clock / 8) 
			 Example: 
			    - (USART3 APB1 Clock / 8) = (42 MHz / 8) = 5250000 baud
			    - (USART1 APB2 Clock / 8) = (84 MHz / 8) = 10500000 baud
		   - Maximum BaudRate that can be achieved when using the Oversampling by 16
		     is: (USART APB Clock / 16) 
			 Example: (USART3 APB1 Clock / 16) = (42 MHz / 16) = 2625000 baud
			 Example: (USART1 APB2 Clock / 16) = (84 MHz / 16) = 5250000 baud
        - Word Length = 8 Bits
        - one Stop Bit
        - No parity
        - Hardware flow control disabled (RTS and CTS signals)
        - Receive and transmit enabled
  */ 
  USART_InitStructure.USART_BaudRate = br;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  /* When using Parity the word length must be configured to 9 bits */
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USARTx, &USART_InitStructure);
	USART_ClearFlag(USARTx,USART_FLAG_TC);
	USART_Cmd(BT_USART,ENABLE);
	}

#define RST_CMD(x)  ((x)?GPIO_ResetBits(GPIOD,GPIO_Pin_12):GPIO_SetBits(GPIOD,GPIO_Pin_12))
	//ldo
void rst_init(){
//	GPIO_InitTypeDef  GPIO_InitStructure;
//	 /* GPIOD Periph clock enable */
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

//	/* Configure PB1 in output pushpull mode */
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
//	GPIO_Init(GPIOA, &GPIO_InitStructure);
//	
	GPIO_InitTypeDef  GPIO_InitStructure;
	  /* GPIOG Peripheral clock enable */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

  /* Configure PG6 and PG8 in output pushpull mode */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
}

void dma_nvic_init()
{
	
	 NVIC_InitTypeDef NVIC_InitStructure;
//	 /*DMA1 open*/
//	 RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);  
//	 // Enable the DMA1_CH5 Interrupt 
//	 NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel5_IRQn;
//	 NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
//	 NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//	 NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//	 NVIC_Init(&NVIC_InitStructure);
//	 // Enable the DMA1_CH4 Interrupt 
//	 NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_IRQn;
//	 NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
//	 NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//	 NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//	 NVIC_Init(&NVIC_InitStructure);
	/* Configure DMA controller to manage USART TX and RX DMA request ----------*/ 

	 RCC_AHB1PeriphClockCmd(USARTx_DMAx_CLK, ENABLE);
/* Configure two bits for preemption priority */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
	NVIC_InitStructure.NVIC_IRQChannel = USARTx_DMA_RX_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	
  NVIC_InitStructure.NVIC_IRQChannel = USARTx_DMA_TX_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
	  /* Enable the USARTx Interrupt */
//  NVIC_InitStructure.NVIC_IRQChannel = USART6_IRQn;
//  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
//  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//  NVIC_Init(&NVIC_InitStructure);
 
}

static void dummy_handler(void);
static void dummy_handler(void){};

uint8_t send_end=1;
static void rda5875_send_handle(void)
{
	send_end=1;
}

// handlers
static void (*rx_done_handler)(void) = dummy_handler;
static void (*tx_done_handler)(void) = dummy_handler;


void hal_uart_dma_init(void){
	bt_usart_init(921600);
	dma_nvic_init();
}
void hal_uart_dma_set_block_received( void (*the_block_handler)(void)){
    rx_done_handler = the_block_handler;
}

void hal_uart_dma_set_block_sent( void (*the_block_handler)(void)){
    tx_done_handler = the_block_handler;
}

int  hal_uart_dma_set_baud(uint32_t baud){
	USART_Cmd(USARTx,DISABLE);
	
	USART_Cmd(USARTx,ENABLE);
	return 0;
}


DMA_InitTypeDef dma_init;
void hal_uart_dma_send_block(const uint8_t *data, uint16_t size){

	// printf("hal_uart_dma_send_block size %u\n", size);
	/*
	 * USART1_TX Using DMA_CHANNEL4 
	 */
	//DMA_Cmd(TX_DMA_CHANNEL,DISABLE);
	/* Reset DMA channel*/
	DMA_DeInit(TX_DMA_CHANNEL);
	dma_init.DMA_Channel = USARTx_TX_DMA_CHANNEL;
	dma_init.DMA_PeripheralBaseAddr = (uint32_t)& BT_USART->DR;
	dma_init.DMA_Memory0BaseAddr = (uint32_t)data;
	dma_init.DMA_BufferSize = size;
	dma_init.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	dma_init.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	dma_init.DMA_MemoryInc = DMA_MemoryInc_Enable;
	dma_init.DMA_PeripheralDataSize =
			DMA_PeripheralDataSize_Byte;
	dma_init.DMA_MemoryDataSize =
			DMA_MemoryDataSize_Byte;
	dma_init.DMA_Mode = DMA_Mode_Normal;
	dma_init.DMA_Priority = DMA_Priority_VeryHigh;
	dma_init.DMA_FIFOMode = DMA_FIFOMode_Disable;
	//dma_init.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	dma_init.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	dma_init.DMA_MemoryBurst = DMA_MemoryBurst_Single;
//	dma_init.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(TX_DMA_CHANNEL, &dma_init);
	DMA_ITConfig(TX_DMA_CHANNEL,DMA_IT_TC,ENABLE);
	DMA_ClearITPendingBit(TX_DMA_CHANNEL,DMA_IT_TCIF3);
	USART_DMACmd(BT_USART,USART_DMAReq_Tx,ENABLE);
	DMA_Cmd(TX_DMA_CHANNEL,ENABLE);
	//rt_kprintf("send %d\n",size);	
}

void hal_uart_dma_receive_block(uint8_t *data, uint16_t size){
	/*
	 * USART1_RX is on DMA_CHANNEL5
	 */
	/* Reset DMA channel*/
	//DMA_Cmd(RX_DMA_CHANNEL,DISABLE);
	RST_CMD(0);
	DMA_DeInit(RX_DMA_CHANNEL);
	dma_init.DMA_Channel = USARTx_RX_DMA_CHANNEL;
	dma_init.DMA_PeripheralBaseAddr = (uint32_t)& BT_USART->DR;
	dma_init.DMA_Memory0BaseAddr = (uint32_t)data;
	dma_init.DMA_BufferSize = size;
	dma_init.DMA_DIR = DMA_DIR_PeripheralToMemory;
	dma_init.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	dma_init.DMA_MemoryInc = DMA_MemoryInc_Enable;
	dma_init.DMA_PeripheralDataSize =
			DMA_PeripheralDataSize_Byte;
	dma_init.DMA_MemoryDataSize =
			DMA_MemoryDataSize_Byte;
	dma_init.DMA_Mode = DMA_Mode_Normal;
	dma_init.DMA_Priority = DMA_Priority_High;
	dma_init.DMA_FIFOMode = DMA_FIFOMode_Disable;
	dma_init.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	dma_init.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	dma_init.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	//dma_init.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(RX_DMA_CHANNEL, &dma_init);
	DMA_ITConfig(RX_DMA_CHANNEL,DMA_IT_TC,ENABLE);
	DMA_ClearITPendingBit(RX_DMA_CHANNEL,DMA_IT_TCIF1);
	USART_DMACmd(BT_USART,USART_DMAReq_Rx,ENABLE);
	DMA_Cmd(RX_DMA_CHANNEL,ENABLE);
	RST_CMD(1);
	//rt_kprintf("need %d\n",size);
}

// end of hal_uart
extern rt_sem_t nw_bt_sem;
// DMA1_CHANNEL4 UART1_TX
void USARTx_DMA_TX_IRQHandler(void) {
	if(DMA_GetITStatus(TX_DMA_CHANNEL,USARTx_TX_DMA_IT_TCIF)) {
//		rt_interrupt_enter();
		DMA_ClearITPendingBit(TX_DMA_CHANNEL,USARTx_TX_DMA_IT_TCIF);
		DMA_ITConfig(TX_DMA_CHANNEL,DMA_IT_TC,DISABLE);
		USART_DMACmd(BT_USART,USART_DMAReq_Tx,DISABLE);
		DMA_Cmd(TX_DMA_CHANNEL,DISABLE);
		DMA_ClearFlag(TX_DMA_CHANNEL,USARTx_TX_DMA_FLAG_TCIF);
		(*tx_done_handler)();

//		rt_sem_release(nw_bt_sem);
//		rt_interrupt_leave();
	}
}

// DMA1_CHANNEL5 UART1_RX
void USARTx_DMA_RX_IRQHandler(void){
		RST_CMD(0);
	if(DMA_GetITStatus(RX_DMA_CHANNEL,USARTx_RX_DMA_IT_TCIF)) {
//		rt_interrupt_enter();
		
		DMA_ClearITPendingBit(RX_DMA_CHANNEL,USARTx_RX_DMA_IT_TCIF);
		DMA_ITConfig(RX_DMA_CHANNEL,DMA_IT_TC,DISABLE);
		USART_DMACmd(BT_USART,USART_DMAReq_Rx,DISABLE);
		DMA_Cmd(RX_DMA_CHANNEL,DISABLE);
		DMA_ClearFlag(RX_DMA_CHANNEL,USARTx_RX_DMA_FLAG_TCIF);
		(*rx_done_handler)();

//		rt_sem_release(nw_bt_sem);
//		rt_interrupt_leave();
	}
}
//void USART6_IRQHandler(void){
//	if(USART_GetITStatus(USART6, USART_IT_RXNE) != RESET)
//  {
//    /* Read one byte from the receive data register */    

//      /* Disable the EVAL_COM1 Receive interrupt */
//      USART_ITConfig(USART6, USART_IT_RXNE, DISABLE);  
//  }
//}

DMA_InitTypeDef dma_init;
int RDA5875_Write(const u8 * dat, u32 len){
  int val = 0;
  while(1)
  {
    while(!(BT_USART->SR&USART_SR_TC));
    BT_USART->DR=*(dat++);
		val++;
    if( val== len )
		{
			RDA5875_DelayMs(1);
			return val;
		}
  }
}

void bt_usart_conf(u32 br){
	
//  GPIO_InitTypeDef  GPIO_InitStructure;
//  USART_InitTypeDef USART_InitStructure;  
//	if(br==0)br = 115200;
//	/*DMA1 open*/
//	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);  

//  /* Enable UART GPIO clocks */
//  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
//  /* Enable UART clock */
//  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
//  //UART
//  USART_InitStructure.USART_BaudRate = br;
//  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
//  USART_InitStructure.USART_StopBits = USART_StopBits_1; 
//  USART_InitStructure.USART_Parity = USART_Parity_No; 
//  USART_InitStructure.USART_HardwareFlowControl = 
//    USART_HardwareFlowControl_None; 
//  USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
//  USART_Init(BT_USART, &USART_InitStructure);
//  USART_Cmd(BT_USART,ENABLE);
//  ///TX
//  RCC_APB2PeriphClockCmd(UART_IO_PERIPH,ENABLE);
//  GPIO_SetBits(UART_IO_PORT,UART_IO_TX);
//  GPIO_InitStructure.GPIO_Pin = UART_IO_TX;
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
//  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
//  GPIO_Init(UART_IO_PORT, &GPIO_InitStructure);
//  ///RX
//  RCC_APB2PeriphClockCmd(UART_IO_PERIPH,ENABLE);
//  GPIO_InitStructure.GPIO_Pin=UART_IO_RX;
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
//  GPIO_Init(UART_IO_PORT, &GPIO_InitStructure);
	
	USART_InitTypeDef USART_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
	if(br==0)br = 115200;
  /* Peripheral Clock Enable -------------------------------------------------*/
  /* Enable GPIO clock */
  RCC_AHB1PeriphClockCmd(USARTx_TX_GPIO_CLK | USARTx_RX_GPIO_CLK, ENABLE);
  
  /* Enable USART clock */
  USARTx_CLK_INIT(USARTx_CLK, ENABLE);
  
  /* Enable the DMA clock */
  RCC_AHB1PeriphClockCmd(USARTx_DMAx_CLK, ENABLE);
  
  /* USARTx GPIO configuration -----------------------------------------------*/ 
  /* Connect USART pins to AF7 */
  GPIO_PinAFConfig(USARTx_TX_GPIO_PORT, USARTx_TX_SOURCE, USARTx_TX_AF);
  GPIO_PinAFConfig(USARTx_RX_GPIO_PORT, USARTx_RX_SOURCE, USARTx_RX_AF);
  
  /* Configure USART Tx and Rx as alternate function push-pull */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  
  GPIO_InitStructure.GPIO_Pin = USARTx_TX_PIN;
  GPIO_Init(USARTx_TX_GPIO_PORT, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = USARTx_RX_PIN;
  GPIO_Init(USARTx_RX_GPIO_PORT, &GPIO_InitStructure);
 
  /* USARTx configuration ----------------------------------------------------*/
  /* Enable the USART OverSampling by 8 */
 // USART_OverSampling8Cmd(USARTx, ENABLE); 
  
  /* USARTx configured as follows:
        - BaudRate = 5250000 baud
		   - Maximum BaudRate that can be achieved when using the Oversampling by 8
		     is: (USART APB Clock / 8) 
			 Example: 
			    - (USART3 APB1 Clock / 8) = (42 MHz / 8) = 5250000 baud
			    - (USART1 APB2 Clock / 8) = (84 MHz / 8) = 10500000 baud
		   - Maximum BaudRate that can be achieved when using the Oversampling by 16
		     is: (USART APB Clock / 16) 
			 Example: (USART3 APB1 Clock / 16) = (42 MHz / 16) = 2625000 baud
			 Example: (USART1 APB2 Clock / 16) = (84 MHz / 16) = 5250000 baud
        - Word Length = 8 Bits
        - one Stop Bit
        - No parity
        - Hardware flow control disabled (RTS and CTS signals)
        - Receive and transmit enabled
  */ 
  USART_InitStructure.USART_BaudRate = br;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  /* When using Parity the word length must be configured to 9 bits */
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USARTx, &USART_InitStructure);
	USART_Cmd(BT_USART,ENABLE);
}

int RDA5875_IOConfig(int br){
	rst_init();
	RST_CMD(1);
	
	bt_usart_conf(br);

	
  return 0;
}

