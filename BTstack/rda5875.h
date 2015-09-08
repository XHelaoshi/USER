#ifndef _RDA5875_H_
#define _RDA5875_H_

#define BT_RING_SIZE (1024)
#include "stm32f4xx.h"
int RDA5875_Write(const u8 * dat, u32 len);
int RDA5875_Read(u8 * dat, u32 len);
int RDA5875_ReadRxLen(void);
int RDA5875_Init(void);
void Basic_Receive(void);
//void RDA5875_BasicCheck(void * para);
int RDA5875_IOConfig(int br);
void RDA5875_DelayMs(u32 ms);

#endif
