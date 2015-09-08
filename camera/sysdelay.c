#include "stm32f4xx.h"
#include <rtthread.h>
#define TIM_USEC 0x01
#define TIM_MSEC 0x02
__IO uint32_t count = 0;
static void Tim3_Config(uint8_t unit)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); 
	
  TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
  TIM_Cmd(TIM3,DISABLE);
  TIM_ITConfig(TIM3, TIM_IT_Update, DISABLE); 
  
  if(unit == TIM_USEC)
  {  
    TIM_TimeBaseStructure.TIM_Period = 10-1;
  }
  else if(unit == TIM_MSEC)
  {
    TIM_TimeBaseStructure.TIM_Period = 9999;
  }
  TIM_TimeBaseStructure.TIM_Prescaler = 18;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0x0000;
	
  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
  TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
  
  TIM_ARRPreloadConfig(TIM3, ENABLE);
  
  /* TIM IT enable */
  TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
} 
static void TIM3_NVIC_Config ( void )
{
 
  NVIC_InitTypeDef NVIC_InitStructure;
  
  /* Set the Vector Table base address at 0x08000000 */
  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x00);
  
  /* Configure the Priority Group to 2 bits */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  
  /* Enable the TIM2 gloabal Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  
  NVIC_Init(&NVIC_InitStructure);
  
 

}

void TIM3_IRQHandler(void)
{
	    /* enter interrupt */
		rt_interrupt_enter();

		if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
		{
			TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
			count++;
		}
	
    /* leave interrupt */
    rt_interrupt_leave();

}
extern struct rt_semaphore rt_tim3_sem;
void Tim3_thread_entry(void* parameter)
{
	Tim3_Config(TIM_USEC);
	TIM3_NVIC_Config();
	rt_sem_release(&rt_tim3_sem);
}

//--------------Delay_Nus()---------------

void Delay_Nus(u32 Nus)
{
	count=0;
	TIM_Cmd(TIM3,ENABLE);
	while(count<Nus);
	TIM_Cmd(TIM3,DISABLE);
} 
//-------------Delay_Nms()-------------------
void Delay_Nms(u32 Nms)
{
	count=0;
	TIM_Cmd(TIM3,ENABLE);
	while(count<1000*Nms);		
	TIM_Cmd(TIM3,DISABLE);
}
//-----------------------------------------------
