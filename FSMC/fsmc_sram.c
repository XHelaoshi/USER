/**
  ******************************************************************************
  * @file    stm324xg_eval_fsmc_sram.c
  * @author  MCD Application Team
  * @version V1.0.2
  * @date    09-March-2012
  * @brief   This file provides a set of functions needed to drive the
  *          IS61WV102416BLL SRAM memory mounted on STM324xG-EVAL evaluation
  *          board(MB786) RevB.    
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "fsmc_sram.h"
#include "stm32f4xx_fsmc.h"
#include <dfs_posix.h>
#include "rtthread.h"
/** @addtogroup Utilities
  * @{
  */

/** @addtogroup STM32_EVAL
  * @{
  */

/** @addtogroup STM324xG_EVAL
  * @{
  */
#define Bank1_SRAM1_ADDR  ((uint32_t)0x60040000)  
/** @addtogroup STM324xG_EVAL_FSMC_SRAM
  * @brief     This file provides a set of functions needed to drive the 
  *            CY7C1071DV33-12BAXI SRAM memory mounted on STM324xG-EVAL board.
  * @{
  */


/**
  * @brief  Configures the FSMC and GPIOs to interface with the SRAM memory.
  *         This function must be called before any write/read operation
  *         on the SRAM.
  * @param  None
  * @retval None
  */
#define BUFFER_SIZE         ((uint32_t)0x10)
#define WRITE_READ_ADDR     ((uint32_t)0x0800)
uint8_t RAM_Buffer[1024*1024] __attribute__((at(0x60000000))); //0x60040000
uint32_t aTxBuffer[BUFFER_SIZE];
uint8_t aRxBuffer[BUFFER_SIZE];
uint8_t BUFFER[1024];
static void Fill_Buffer(uint32_t *pBuffer, uint32_t uwBufferLenght, uint32_t uwOffset);
//DMA test
#define DMA_STREAM               DMA2_Stream0
#define DMA_CHANNEL              DMA_Channel_0
#define DMA_STREAM_CLOCK         RCC_AHB1Periph_DMA2 
#define DMA_STREAM_IRQ           DMA2_Stream0_IRQn
#define DMA_IT_TCIF              DMA_IT_TCIF0
#define DMA_STREAM_IRQHANDLER    DMA2_Stream0_IRQHandler
//end of DMA test
//#define BUFFER_SIZE              32
extern void DMA_Config(void);
int flag=1;
int flag1=1;
extern struct rt_semaphore rt_sram_sem;
void Sram_thread_entry(void* parameter)
{
	rt_tick_t t1,t2;
	SRAM_Init();
	Fill_Buffer(aTxBuffer, BUFFER_SIZE, 0xA244250F);  
	//DMA_Config();
//	while(flag)
//	{
//		rt_thread_delay(1);
//	}
	rt_sem_take(&rt_sram_sem, RT_WAITING_FOREVER);
	while(flag1)
	{
		int fd;
		//int i=1024;
		fd = open("/ud/text.txt", O_WRONLY | O_CREAT,0);	
			if (fd >= 0)
			{
				t1 = rt_tick_get();
//				while(i>0)
//				{
					write(fd, RAM_Buffer, sizeof(RAM_Buffer));
//					i--;
//				}
				t2 = rt_tick_get();
				rt_kprintf("%d\n\r",t2-t1);
				close(fd);
			}
			rt_thread_delay(100);
		//SRAM_ReadBuffer(aTxBuffer,0,BUFFER_SIZE);
	}
	while(1);
	//Fill_Buffer(aTxBuffer, BUFFER_SIZE, 0xA244250F); 
}
void SRAM_Init(void)
{
  FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
  FSMC_NORSRAMTimingInitTypeDef  p;
  GPIO_InitTypeDef GPIO_InitStructure; 
  
  /* Enable GPIOs clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOF |
                         RCC_AHB1Periph_GPIOG, ENABLE);

  /* Enable FSMC clock */
  RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FSMC, ENABLE); 
  
/*-- GPIOs Configuration -----------------------------------------------------*/
/*
 +-------------------+--------------------+------------------+------------------+
 | PD0  <-> FSMC_D2  | PE0  <-> FSMC_NBL0 | PF0 <-> FSMC_A0  | PG0 <-> FSMC_A10 |
 | PD1  <-> FSMC_D3  | PE1  <-> FSMC_NBL1 | PF1 <-> FSMC_A1  | PG1 <-> FSMC_A11 |
 | PD4  <-> FSMC_NOE | PE2  <-> FSMC_A23  | PF2 <-> FSMC_A2  | PG2 <-> FSMC_A12 |
 | PD5  <-> FSMC_NWE | PE3  <-> FSMC_A19  | PF3 <-> FSMC_A3  | PG3 <-> FSMC_A13 |
 | PD8  <-> FSMC_D13 | PE4  <-> FSMC_A20  | PF4 <-> FSMC_A4  | PG4 <-> FSMC_A14 |
 | PD9  <-> FSMC_D14 | PE5  <-> FSMC_A21  | PF5 <-> FSMC_A5  | PG5 <-> FSMC_A15 |
 | PD10 <-> FSMC_D15 | PE6  <-> FSMC_A22  | PF12 <-> FSMC_A6 | PG9 <-> FSMC_NE2 |
 | PD11 <-> FSMC_A16 | PE7  <-> FSMC_D4   | PF13 <-> FSMC_A7 |------------------+
 | PD12 <-> FSMC_A17 | PE8  <-> FSMC_D5   | PF14 <-> FSMC_A8 |
 | PD13 <-> FSMC_A18 | PE9  <-> FSMC_D6   | PF15 <-> FSMC_A9 |
 | PD14 <-> FSMC_D0  | PE10 <-> FSMC_D7   |------------------+
 | PD15 <-> FSMC_D1  | PE11 <-> FSMC_D8   |
 +-------------------| PE12 <-> FSMC_D9   |
   PD7 <-> FSMC_NE1  | PE13 <-> FSMC_D10  |
                     | PE14 <-> FSMC_D11  |
                     | PE15 <-> FSMC_D12  |
                     +--------------------+
*/

  /* GPIOD configuration */
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource0, GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource1, GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource4, GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource10, GPIO_AF_FSMC);
  //GPIO_PinAFConfig(GPIOD, GPIO_PinSource11, GPIO_AF_FSMC); 
  //GPIO_PinAFConfig(GPIOD, GPIO_PinSource12, GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource13, GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource14, GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource15, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource7, GPIO_AF_FSMC);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0  | GPIO_Pin_1  | GPIO_Pin_4  | GPIO_Pin_5  | GPIO_Pin_7 |
                                GPIO_Pin_8  | GPIO_Pin_9  | GPIO_Pin_10 | 
                                 GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;

  GPIO_Init(GPIOD, &GPIO_InitStructure);


  /* GPIOE configuration */
  //GPIO_PinAFConfig(GPIOE, GPIO_PinSource0 , GPIO_AF_FSMC);
  //GPIO_PinAFConfig(GPIOE, GPIO_PinSource1 , GPIO_AF_FSMC);
  //GPIO_PinAFConfig(GPIOE, GPIO_PinSource2 , GPIO_AF_FSMC);
  //GPIO_PinAFConfig(GPIOE, GPIO_PinSource3 , GPIO_AF_FSMC);
  //GPIO_PinAFConfig(GPIOE, GPIO_PinSource4 , GPIO_AF_FSMC);
  //GPIO_PinAFConfig(GPIOE, GPIO_PinSource5 , GPIO_AF_FSMC);
  //GPIO_PinAFConfig(GPIOE, GPIO_PinSource6 , GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource7 , GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource8 , GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource9 , GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource10 , GPIO_AF_FSMC);
// GPIO_PinAFConfig(GPIOE, GPIO_PinSource11 , GPIO_AF_FSMC);
// GPIO_PinAFConfig(GPIOE, GPIO_PinSource12 , GPIO_AF_FSMC);
// GPIO_PinAFConfig(GPIOE, GPIO_PinSource13 , GPIO_AF_FSMC);
// GPIO_PinAFConfig(GPIOE, GPIO_PinSource14 , GPIO_AF_FSMC);
// GPIO_PinAFConfig(GPIOE, GPIO_PinSource15 , GPIO_AF_FSMC);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 |GPIO_Pin_8  | GPIO_Pin_9  | GPIO_Pin_10 ;

  GPIO_Init(GPIOE, &GPIO_InitStructure);


  /* GPIOF configuration */
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource0 , GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource1 , GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource2 , GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource3 , GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource4 , GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource5 , GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource12 , GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource13 , GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource14 , GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource15 , GPIO_AF_FSMC);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0  | GPIO_Pin_1  | GPIO_Pin_2  | GPIO_Pin_3  | 
                                GPIO_Pin_4  | GPIO_Pin_5  | GPIO_Pin_12 | GPIO_Pin_13 |
                                GPIO_Pin_14 | GPIO_Pin_15;      

  GPIO_Init(GPIOF, &GPIO_InitStructure);


  /* GPIOG configuration */
//  GPIO_PinAFConfig(GPIOG, GPIO_PinSource0 , GPIO_AF_FSMC);
//  GPIO_PinAFConfig(GPIOG, GPIO_PinSource1 , GPIO_AF_FSMC);
//  GPIO_PinAFConfig(GPIOG, GPIO_PinSource2 , GPIO_AF_FSMC);
//  GPIO_PinAFConfig(GPIOG, GPIO_PinSource3 , GPIO_AF_FSMC);
//  GPIO_PinAFConfig(GPIOG, GPIO_PinSource4 , GPIO_AF_FSMC);
//  GPIO_PinAFConfig(GPIOG, GPIO_PinSource5 , GPIO_AF_FSMC);
//  GPIO_PinAFConfig(GPIOG, GPIO_PinSource9 , GPIO_AF_FSMC);

//  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0  | GPIO_Pin_1  | GPIO_Pin_2  | GPIO_Pin_3 | 
//                                GPIO_Pin_4  | GPIO_Pin_5  |GPIO_Pin_9;      

//  GPIO_Init(GPIOG, &GPIO_InitStructure);

/*-- FSMC Configuration ------------------------------------------------------*/
  p.FSMC_AddressSetupTime = 0;//
  p.FSMC_AddressHoldTime = 0;
  p.FSMC_DataSetupTime = 0;//
  p.FSMC_BusTurnAroundDuration = 0;
  p.FSMC_CLKDivision = 0;
  p.FSMC_DataLatency = 0;
  p.FSMC_AccessMode = FSMC_AccessMode_A;

  FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM1;
  FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
  FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_PSRAM;
  FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_8b;
  FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
  FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;  
  FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
  FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
  FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
  FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
  FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
  FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
  FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
  FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &p;
  FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &p;

  FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure); 

  /*!< Enable FSMC Bank1_SRAM2 Bank */
  FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM1, ENABLE); 

}

/**
  * @brief  Writes a Half-word buffer to the FSMC SRAM memory.
  * @param  pBuffer : pointer to buffer.
  * @param  WriteAddr : SRAM memory internal address from which the data will be
  *         written.
  * @param  NumHalfwordToWrite : number of half-words to write.
  * @retval None
  */
//void SRAM_WriteBuffer(uint8_t* pBuffer, uint32_t WriteAddr, uint32_t NumHalfwordToWrite)
//{
//  for (; NumHalfwordToWrite != 0; NumHalfwordToWrite--) /* while there is data to write */
//  {
//    /* Transfer data to the memory */
//    *(uint16_t *) (Bank1_SRAM1_ADDR + WriteAddr) = *pBuffer++;

//    /* Increment the address*/
//    WriteAddr += 0;//2
//  }
//}

/**
  * @brief  Reads a block of data from the FSMC SRAM memory.
  * @param  pBuffer : pointer to the buffer that receives the data read from the
  *         SRAM memory.
  * @param  ReadAddr : SRAM memory internal address to read from.
  * @param  NumHalfwordToRead : number of half-words to read.
  * @retval None
  */
void SRAM_ReadBuffer(uint32_t* pBuffer, uint32_t ReadAddr, uint32_t NumHalfwordToRead)
{
  for (; NumHalfwordToRead != 0; NumHalfwordToRead--) /* while there is data to read */
  {
    /* Read a half-word from the memory */
    *pBuffer++ = *(__IO uint32_t*) (Bank1_SRAM1_ADDR + ReadAddr);

    /* Increment the address*/
    ReadAddr += 0;//2
  }
}
static void Fill_Buffer(uint32_t *pBuffer, uint32_t uwBufferLenght, uint32_t uwOffset)
{
  uint32_t tmpIndex = 0;

  /* Put in global buffer different values */
  for (tmpIndex = 0; tmpIndex < uwBufferLenght; tmpIndex++ )
  {
    pBuffer[tmpIndex] = tmpIndex + uwOffset;
  }
} 
void DMA_Config()
{
  NVIC_InitTypeDef NVIC_InitStructure;
  DMA_InitTypeDef  DMA_InitStructure;
 // __IO uint32_t    Timeout = TIMEOUT_MAX;
    
  /* Enable DMA clock */
  RCC_AHB1PeriphClockCmd(DMA_STREAM_CLOCK, ENABLE);
  
  /* Reset DMA Stream registers (for debug purpose) */
  DMA_DeInit(DMA_STREAM);

  /* Check if the DMA Stream is disabled before enabling it.
     Note that this step is useful when the same Stream is used multiple times:
     enabled, then disabled then re-enabled... In this case, the DMA Stream disable
     will be effective only at the end of the ongoing data transfer and it will 
     not be possible to re-configure it before making sure that the Enable bit 
     has been cleared by hardware. If the Stream is used only once, this step might 
     be bypassed. */
  while (DMA_GetCmdStatus(DMA_STREAM) != DISABLE)
  {
  }
  
  /* Configure DMA Stream */
  DMA_InitStructure.DMA_Channel = DMA_CHANNEL;  
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)Bank1_SRAM1_ADDR;
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)aTxBuffer;
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToMemory;
  DMA_InitStructure.DMA_BufferSize = (uint32_t)BUFFER_SIZE;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA_STREAM, &DMA_InitStructure);
    
  /* Enable DMA Stream Transfer Complete interrupt */
  //DMA_ITConfig(DMA_STREAM, DMA_IT_TC, ENABLE);

  /* DMA Stream enable */
  DMA_Cmd(DMA_STREAM, ENABLE);

  /* Check if the DMA Stream has been effectively enabled.
     The DMA Stream Enable bit is cleared immediately by hardware if there is an 
     error in the configuration parameters and the transfer is no started (ie. when
     wrong FIFO threshold is configured ...) */
//  Timeout = TIMEOUT_MAX;
//  while ((DMA_GetCmdStatus(DMA_STREAM) != ENABLE) && (Timeout-- > 0))
//  {
//  }
//   
//  /* Check if a timeout condition occurred */
//  if (Timeout == 0)
//  {
//    /* Manage the error: to simplify the code enter an infinite loop */
//    while (1)
//    {
//    }
//  }

//  /* Enable the DMA Stream IRQ Channel */
//  NVIC_InitStructure.NVIC_IRQChannel = DMA_STREAM_IRQ;
//  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
//  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//  NVIC_Init(&NVIC_InitStructure);     
}
#include "finsh.h"
void start()
{
//	flag=0;
	rt_sem_release(&rt_sram_sem);
}
void stop()
{
	flag1=0;
}
FINSH_FUNCTION_EXPORT(start, start)
FINSH_FUNCTION_EXPORT(stop, stop)
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
