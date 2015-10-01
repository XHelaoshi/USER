#include "rtthread.h"
#include "drivers/spi.h"
#include "spi_bus.h"
#include "spi_user.h"
#include "stm32f4xx.h"
uint8_t address = 0x88;
uint32_t data = 0x55555555;
void spi_thread_entry()
{
	while(1)
	{
		spi_user_sendbytes(&address,&data,4);
		rt_thread_delay(10);   
	}
}