#ifndef __my_test_h__
#define __my_test_h__

#include "usb_bsp.h"
void mytest_entry(void* parameter);
static void SpeedTest_TimeInit ( void );
void  SpeedTest_TimerIRQ(void);
static void Tim3_Config(uint8_t unit);
#endif