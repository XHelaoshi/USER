#include "rtthread.h"
#include "drivers/spi.h"
#include "spi_bus.h"
#include "spi_user.h"
#include "stm32f4xx.h"
uint8_t address = 0xaa;
//uint32_t data = 0x12345678;
uint8_t data[] = {0x78, 0x56, 0x34, 0x12};
void spi_thread_entry()
{
	while(1)
	{
		spi_user_sendbytes(&address,data,4);
		rt_thread_delay(10);   
	}
}