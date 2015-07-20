#ifndef _UART4_H
#define _UART4_H

#include "stm32f4xx.h"

#define UART4_MAX_SEND_LEN 1024
#define UART4_MAX_RECV_LEN 256

extern u8 UART4_TX_BUF[UART4_MAX_SEND_LEN];
extern u8 UART4_RX_BUF[UART4_MAX_RECV_LEN];
//extern vu16 UART4_RX_STA;
//extern u8 gps_uart_en;
//extern u8 gps_tim_cnt;
extern u8 gps_uart_flag;
extern u8 GPS_DATA[];

void Init_Uart4(u32 baudrate);
void u4_printf(char* fmt,...) ;
void Init_Uart4_RxDMA(void);

#endif
