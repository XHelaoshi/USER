#ifndef __SPI_BUS_H__
#define __SPI_BUS_H__

#include "stm32f4xx.h"
#include "rtthread.h"
#include "drivers/spi.h"
//#define SPI_USE_DMA

struct stm32_spi_bus
{
    struct rt_spi_bus parent;
    SPI_TypeDef * SPI;
};

struct stm32_spi_cs
{
    GPIO_TypeDef * GPIOx;
    uint16_t GPIO_Pin;
};

/* public function */
rt_err_t stm32_spi_register(SPI_TypeDef * SPI,
                            struct stm32_spi_bus * stm32_spi,
                            const char * spi_bus_name);
void rt_hw_spi3_init(void);
void spi_user_sendbytes(uint8_t *address,const void *date,uint16_t length);
void spi_user_readbytes(uint8_t *address,void *date,uint16_t length);
#endif
