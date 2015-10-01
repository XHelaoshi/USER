#include "spi_bus.h"

static rt_err_t configure(struct rt_spi_device* device, struct rt_spi_configuration* configuration);
static rt_uint32_t xfer(struct rt_spi_device* device, struct rt_spi_message* message);
static struct rt_spi_device *spi_user;
 static struct rt_spi_ops stm32_spi_ops =
 {
     configure,
     xfer
 };
 
 static rt_err_t configure(struct rt_spi_device* device,
                           struct rt_spi_configuration* configuration)
 {
     struct stm32_spi_bus * stm32_spi_bus = (struct stm32_spi_bus *)device->bus;
     SPI_InitTypeDef SPI_InitStructure;
 
     SPI_StructInit(&SPI_InitStructure);
		
     /* data_width */
     if(configuration->data_width <= 8)
     {
         SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
     }
     else if(configuration->data_width <= 16)
     {
         SPI_InitStructure.SPI_DataSize = SPI_DataSize_16b;
     }
     else
     {
         return RT_EIO;
     }
 
     /* baudrate */
     {
         uint32_t SPI_APB_CLOCK;
         uint32_t stm32_spi_max_clock;
         uint32_t max_hz;
 
         stm32_spi_max_clock = 42000000;
         max_hz = configuration->max_hz;
 #ifdef STM32F4XX
         stm32_spi_max_clock = 37500000;
 #elif STM32F2XX
         stm32_spi_max_clock = 30000000;
 #endif
 
         if(max_hz > stm32_spi_max_clock)
         {
             max_hz = stm32_spi_max_clock;
         }
 
         SPI_APB_CLOCK = SystemCoreClock / 4;
 
         /* STM32F2xx SPI MAX 30Mhz */
         /* STM32F4xx SPI MAX 37.5Mhz */
         if(max_hz >= SPI_APB_CLOCK/2 && SPI_APB_CLOCK/2 <= 42000000)
         {
             SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
         }
         else if(max_hz >= SPI_APB_CLOCK/4)
         {
             SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
         }
         else if(max_hz >= SPI_APB_CLOCK/8)
         {
             SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
         }
         else if(max_hz >= SPI_APB_CLOCK/16)
         {
             SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
         }
         else if(max_hz >= SPI_APB_CLOCK/32)
         {
             SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;
         }
         else if(max_hz >= SPI_APB_CLOCK/64)
         {
             SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
         }
         else if(max_hz >= SPI_APB_CLOCK/128)
         {
             SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;
         }
         else
         {
             /*  min prescaler 256 */
             SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
         }
     } /* baudrate */
 
     /* CPOL */
     if(configuration->mode & RT_SPI_CPOL)
     {
         SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
     }
     else
     {
         SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
     }
     /* CPHA */
     if(configuration->mode & RT_SPI_CPHA)
     {
         SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
     }
     else
     {
         SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
     }
     /* MSB or LSB */
     if(configuration->mode & RT_SPI_MSB)
     {
         SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
     }
     else
     {
         SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_LSB;
     }
     SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
     SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
     SPI_InitStructure.SPI_NSS  = SPI_NSS_Soft;
 
     /* init SPI */
     SPI_I2S_DeInit(stm32_spi_bus->SPI);
     SPI_Init(stm32_spi_bus->SPI, &SPI_InitStructure);
     /* Enable SPI_MASTER */
     SPI_Cmd(stm32_spi_bus->SPI, ENABLE);
     SPI_CalculateCRC(stm32_spi_bus->SPI, DISABLE);
 
     return RT_EOK;
 };
 
 static rt_uint32_t xfer(struct rt_spi_device* device, struct rt_spi_message* message)
 {
     struct stm32_spi_bus * stm32_spi_bus = (struct stm32_spi_bus *)device->bus;
     struct rt_spi_configuration * config = &device->config;
     SPI_TypeDef * SPI = stm32_spi_bus->SPI;
     struct stm32_spi_cs * stm32_spi_cs = device->parent.user_data;
     rt_uint32_t size = message->length;
 
     /* take CS */
     if(message->cs_take)
     {
         GPIO_ResetBits(stm32_spi_cs->GPIOx, stm32_spi_cs->GPIO_Pin);
     }
       {
         if(config->data_width <= 8)
         {
             const rt_uint8_t * send_ptr = message->send_buf;
             rt_uint8_t * recv_ptr = message->recv_buf;
 
             while(size--)
             {
                 rt_uint8_t data = 0xFF;
 
                 if(send_ptr != RT_NULL)
                 {
                     data = *send_ptr  ;
                 }
 
                 //Wait until the transmit buffer is empty
                 while (SPI_I2S_GetFlagStatus(SPI, SPI_I2S_FLAG_TXE) == RESET);
                 // Send the byte
                 SPI_I2S_SendData(SPI, data);
 
                 //Wait until a data is received
                 while (SPI_I2S_GetFlagStatus(SPI, SPI_I2S_FLAG_RXNE) == RESET);
                 // Get the received data
                 data = SPI_I2S_ReceiveData(SPI);
 
                 if(recv_ptr != RT_NULL)
                 {
                     *recv_ptr   = data;
                 }
             }
         }
         else if(config->data_width <= 16)
         {
             const rt_uint16_t * send_ptr = message->send_buf;
             rt_uint16_t * recv_ptr = message->recv_buf;
 
             while(size--)
             {
                 rt_uint16_t data = 0xFF;
 
                 if(send_ptr != RT_NULL)
                 {
                     data = *send_ptr  ;
                 }
 
                 //Wait until the transmit buffer is empty
                 while (SPI_I2S_GetFlagStatus(SPI, SPI_I2S_FLAG_TXE) == RESET);
                 // Send the byte
                 SPI_I2S_SendData(SPI, data);
 
                 //Wait until a data is received
                 while (SPI_I2S_GetFlagStatus(SPI, SPI_I2S_FLAG_RXNE) == RESET);
                 // Get the received data
                 data = SPI_I2S_ReceiveData(SPI);
 
                 if(recv_ptr != RT_NULL)
                 {
                     *recv_ptr   = data;
                 }
             }
         }
     }
 
     /* release CS */
     if(message->cs_release)
     {
         GPIO_SetBits(stm32_spi_cs->GPIOx, stm32_spi_cs->GPIO_Pin);
     }
 
     return message->length;
 };
 
 /** \brief init and register stm32 spi bus.
  *
  * \param SPI: STM32 SPI, e.g: SPI1,SPI2,SPI3.
  * \param stm32_spi: stm32 spi bus struct.
  * \param spi_bus_name: spi bus name, e.g: "spi1"
  * \return
  *
  */
 rt_err_t stm32_spi_register(SPI_TypeDef * SPI,
                             struct stm32_spi_bus * stm32_spi,
                             const char * spi_bus_name)
 {
     if(SPI == SPI1)
     {
     	stm32_spi->SPI = SPI1;
         RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
     }
     else if(SPI == SPI2)
     {
         stm32_spi->SPI = SPI2;
         RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
     }
     else if(SPI == SPI3)
     {
     	stm32_spi->SPI = SPI3;
         RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);
     }
     else
     {
         return RT_ENOSYS;
     }
 
     return rt_spi_bus_register(&stm32_spi->parent, spi_bus_name, &stm32_spi_ops);
 }
 
void rt_hw_spi3_init(void)
{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);
    /* register spi bus */
    {
        static struct stm32_spi_bus stm32_spi;
        GPIO_InitTypeDef GPIO_InitStructure;

        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

        /*!< SPI SCK pin configuration */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12;
        GPIO_Init(GPIOC, &GPIO_InitStructure);

        /* Connect alternate function */
        GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_SPI3);
        GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_SPI3);
        GPIO_PinAFConfig(GPIOC, GPIO_PinSource12, GPIO_AF_SPI3);

        stm32_spi_register(SPI3, &stm32_spi, "spi3");
    }

    /* attach cs */
    {
        static struct rt_spi_device spi_device;
        static struct stm32_spi_cs  spi_cs;

        GPIO_InitTypeDef GPIO_InitStructure;

        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

        /* spi20: PG6 */
        spi_cs.GPIOx = GPIOA;
        spi_cs.GPIO_Pin = GPIO_Pin_15;
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

        GPIO_InitStructure.GPIO_Pin = spi_cs.GPIO_Pin;
        GPIO_SetBits(spi_cs.GPIOx, spi_cs.GPIO_Pin);
        GPIO_Init(spi_cs.GPIOx, &GPIO_InitStructure);

        rt_spi_bus_attach_device(&spi_device, "spi20", "spi3", (void*)&spi_cs);
    }
		
		{
			static struct rt_spi_device * rt_spi_device;
			struct rt_spi_configuration cfg;
	
			rt_spi_device = (struct rt_spi_device *)rt_device_find("spi20");
			spi_user = rt_spi_device;
			rt_spi_device->bus->owner = rt_spi_device;
			cfg.data_width = 8;
			cfg.mode = RT_SPI_MODE_3 | RT_SPI_MSB; /* SPI Compatible: Mode 0 and Mode 3 */
			cfg.max_hz = 42 * 1000 * 1000; /* 50M */
			rt_spi_configure(rt_spi_device, &cfg);		
		}
}
void spi_user_sendbytes(uint8_t *address,const void *date,uint16_t length)
{
		rt_spi_send_then_send(spi_user,address,1,date,length);
}
void spi_user_readbytes(uint8_t *address,uint32_t *date,uint16_t length)
{
	rt_spi_send_then_recv(spi_user, &address, 1, &date, length);
}
//SPI_InitTypeDef  SPI_InitStructure;
/**
  * @brief  Configures the SPI Peripheral.
  * @param  None
  * @retval None
  */
//static void SPI_Config(void)
//{
//  GPIO_InitTypeDef GPIO_InitStructure;
//  NVIC_InitTypeDef NVIC_InitStructure;

//  /* Peripheral Clock Enable -------------------------------------------------*/
//  /* Enable the SPI clock */
//  SPIx_CLK_INIT(SPIx_CLK, ENABLE);
//  
//  /* Enable GPIO clocks */
//  RCC_AHB1PeriphClockCmd(SPIx_SCK_GPIO_CLK | SPIx_MISO_GPIO_CLK | SPIx_MOSI_GPIO_CLK, ENABLE);

//  /* SPI GPIO Configuration --------------------------------------------------*/
//  /* Connect SPI pins to AF5 */  
//  GPIO_PinAFConfig(SPIx_SCK_GPIO_PORT, SPIx_SCK_SOURCE, SPIx_SCK_AF);
//  GPIO_PinAFConfig(SPIx_MOSI_GPIO_PORT, SPIx_MOSI_SOURCE, SPIx_MOSI_AF);

//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
//  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
//  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;

//  /* SPI SCK pin configuration */
//  GPIO_InitStructure.GPIO_Pin = SPIx_SCK_PIN;
//  GPIO_Init(SPIx_SCK_GPIO_PORT, &GPIO_InitStructure);

//  /* SPI  MOSI pin configuration */
//  GPIO_InitStructure.GPIO_Pin =  SPIx_MOSI_PIN;
//  GPIO_Init(SPIx_MOSI_GPIO_PORT, &GPIO_InitStructure);
// 
//  /* SPI configuration -------------------------------------------------------*/
//  SPI_I2S_DeInit(SPIx);
//  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
//  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
//  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
//  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
//  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
//  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
//  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
//  SPI_InitStructure.SPI_CRCPolynomial = 7;
//  
//	/* Initializes the SPI communication */
//  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
//  SPI_Init(SPIx, &SPI_InitStructure);
//  
//  /* The Data transfer is performed in the SPI interrupt routine */
//  /* Enable the SPI peripheral */
//  SPI_Cmd(SPIx, ENABLE);
////  /* Configure the Priority Group to 1 bit */                
////  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
////  
////  /* Configure the SPI interrupt priority */
////  NVIC_InitStructure.NVIC_IRQChannel = SPIx_IRQn;
////  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
////  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
////  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
////  NVIC_Init(&NVIC_InitStructure);
//	
//}