#include "rtthread.h"
#include "drivers/spi.h"
#include "spi_bus.h"
#include "spi_user.h"
#include "stm32f4xx.h"
#include <rthw.h>
uint8_t addt;
uint8_t address = 0x3c<<1;
uint8_t address1 = 0x3c<<1;
uint8_t address2 = 0x3c<<1;
uint32_t data = 0x12345678;
uint32_t data1 = 0x24568412;
uint32_t data2 = 0x87261595;
uint32_t recvdata;
void spi_thread_entry()
{
	while(1)
	{
		rt_base_t level;                                                          \
    level = rt_hw_interrupt_disable();
		data++;
		spi_user_sendbytes(&address,&data,4);
		addt = address | 1;
		spi_user_readbytes(&addt,&recvdata,4);
		if(data != recvdata)
		{
			rt_kprintf("write data: 0x%08x\n",data);
			rt_kprintf("recv data: 0x%08x\n\r",recvdata);
		}
		
//		spi_user_sendbytes(&address1,&data1,4);
//		addt = address1 | 1;
//		spi_user_readbytes(&addt,&recvdata,4);
//		rt_kprintf("write data1: 0x%08x\n",data1);
//		rt_kprintf("recv data1: 0x%08x\n\r",recvdata);
//		
//		spi_user_sendbytes(&address2,&data2,4);
//		addt = address2 | 1;
//		spi_user_readbytes(&addt,&recvdata,4);
//		rt_kprintf("write data2: 0x%08x\n",data2);
//		rt_kprintf("recv data2: 0x%08x\n\r",recvdata);
		
	//	rt_thread_delay(1);   
		rt_hw_interrupt_enable(level);
	}
}
