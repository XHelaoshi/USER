/*
 * Copyright (C) 2011 by Matthias Ringwald
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the names of
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY MATTHIAS RINGWALD AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL MATTHIAS
 * RINGWALD OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

/*
 *  hal_uart_dma.h
 *
 *  Hardware abstraction layer that provides
 *  - blockwise IRQ-driven read/write
 *  - CSR IRQs
 *
 */

#ifndef __HAL_UART_DMA_H
#define __HAL_UART_DMA_H

#include <stdint.h>
#include "stm32f4xx.h"
#if defined __cplusplus
extern "C" {
#endif
/* Definition for USARTx resources ******************************************/
  #define USARTx                           USART3
  #define USARTx_CLK                       RCC_APB1Periph_USART3
  #define USARTx_CLK_INIT                  RCC_APB1PeriphClockCmd
  #define USARTx_IRQn                      USART3_IRQn
  #define USARTx_IRQHandler                USART3_IRQHandler

  #define USARTx_TX_PIN                    GPIO_Pin_8	                
  #define USARTx_TX_GPIO_PORT              GPIOD                     
  #define USARTx_TX_GPIO_CLK               RCC_AHB1Periph_GPIOD
  #define USARTx_TX_SOURCE                 GPIO_PinSource8
  #define USARTx_TX_AF                     GPIO_AF_USART3

  #define USARTx_RX_PIN                    GPIO_Pin_9                
  #define USARTx_RX_GPIO_PORT              GPIOD                    
  #define USARTx_RX_GPIO_CLK               RCC_AHB1Periph_GPIOD
  #define USARTx_RX_SOURCE                 GPIO_PinSource9
  #define USARTx_RX_AF                     GPIO_AF_USART3

  /* Definition for DMAx resources ********************************************/
  #define USARTx_DR_ADDRESS                ((uint32_t)USART3 + 0x04) 

  #define USARTx_DMA                       DMA1
  #define USARTx_DMAx_CLK                  RCC_AHB1Periph_DMA1
     
  #define USARTx_TX_DMA_CHANNEL            DMA_Channel_4
  #define USARTx_TX_DMA_STREAM             DMA1_Stream3
  #define USARTx_TX_DMA_FLAG_FEIF          DMA_FLAG_FEIF3
  #define USARTx_TX_DMA_FLAG_DMEIF         DMA_FLAG_DMEIF3
  #define USARTx_TX_DMA_FLAG_TEIF          DMA_FLAG_TEIF3
  #define USARTx_TX_DMA_FLAG_HTIF          DMA_FLAG_HTIF3
  #define USARTx_TX_DMA_FLAG_TCIF          DMA_FLAG_TCIF3
	#define USARTx_TX_DMA_IT_TCIF          	 DMA_IT_TCIF3
              
  #define USARTx_RX_DMA_CHANNEL            DMA_Channel_4
  #define USARTx_RX_DMA_STREAM             DMA1_Stream1
  #define USARTx_RX_DMA_FLAG_FEIF          DMA_FLAG_FEIF1
  #define USARTx_RX_DMA_FLAG_DMEIF         DMA_FLAG_DMEIF1
  #define USARTx_RX_DMA_FLAG_TEIF          DMA_FLAG_TEIF1
  #define USARTx_RX_DMA_FLAG_HTIF          DMA_FLAG_HTIF1
  #define USARTx_RX_DMA_FLAG_TCIF          DMA_FLAG_TCIF1
	#define USARTx_RX_DMA_IT_TCIF          	 DMA_IT_TCIF1

  #define USARTx_DMA_TX_IRQn               DMA1_Stream3_IRQn
  #define USARTx_DMA_RX_IRQn               DMA1_Stream1_IRQn
  #define USARTx_DMA_TX_IRQHandler         DMA1_Stream3_IRQHandler
  #define USARTx_DMA_RX_IRQHandler         DMA1_Stream1_IRQHandler
void hal_uart_dma_init(void);
void hal_uart_dma_set_block_received( void (*block_handler)(void));
void hal_uart_dma_set_block_sent( void (*block_handler)(void));
void hal_uart_dma_set_csr_irq_handler( void (*csr_irq_handler)(void));
int  hal_uart_dma_set_baud(uint32_t baud);
void hal_uart_dma_send_block(const uint8_t *buffer, uint16_t length);
void hal_uart_dma_receive_block(uint8_t *buffer, uint16_t len);
void hal_uart_dma_set_sleep(uint8_t sleep);

	
#if defined __cplusplus
}
#endif
#endif // __HAL_UART_DMA_H
