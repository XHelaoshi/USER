#ifndef _USART1_H
#define _USART1_H

#include "stm32f4xx.h"

void Init_Usart1(u32 baudrate);
void u1_printf(char* fmt,...) ;

#endif
