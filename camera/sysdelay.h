#ifndef __SYSDELAY_H
#define __SYSDELAY_H
#include "stm32f4xx.h"
extern __IO uint32_t count ;
static void Tim3_Config(uint8_t unit);
static void TIM3_NVIC_Config ( void );
void TIM3_IRQHandler(void);
void Tim3_thread_entry(void* parameter);
//--------------Delay_Nus()---------------
void Delay_Nus(u32 Nus);
//-------------Delay_Nms()-------------------
void Delay_Nms(u32 Nms);
#endif
