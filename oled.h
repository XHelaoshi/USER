#ifndef _OLED_H
#define _OLED_H

#include "stm32f4xx.h"

extern void OLED_Init(void);
extern void OLED_Clear(void);
extern void OLED_String(uint8_t page, uint8_t col, uint8_t *str);

#endif
