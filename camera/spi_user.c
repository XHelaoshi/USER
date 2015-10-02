#include "rtthread.h"
#include "drivers/spi.h"
#include "spi_bus.h"
#include "spi_user.h"
#include "stm32f4xx.h"
#include <rthw.h>
uint8_t address = 0x71;
//uint32_t data = 0x12345678;
uint8_t data[] = {0x78, 0x56, 0x34, 0x12};
uint32_t recvdata;
void spi_thread_entry()
{
	while(1)
	{
		rt_base_t level;                                                          \
    level = rt_hw_interrupt_disable();
		//spi_user_sendbytes(&address,data,4);
		spi_user_readbytes(&address,&recvdata,4);
		rt_kprintf("recv data: 0x%08x\n\r",recvdata);
		rt_thread_delay(100);   
		rt_hw_interrupt_enable(level);
	}
}