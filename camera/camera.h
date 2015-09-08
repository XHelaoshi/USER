#ifndef __camera_h__
#define __camera_h__
#include "stm32f4xx.h"
#define JPEG_BUFFER_SIZE	1024*60-1//dma 1 burst max ,256KByte
#define JPEG_BUFFER_CNT	16
extern __IO uint32_t RAM_Buffer[];
#define Camera_key_PIN                      GPIO_Pin_3
#define Camera_key_GPIO_PORT                GPIOE
#define Camera_key_GPIO_CLK                 RCC_AHB1Periph_GPIOE  

#define Camera_key_EXTI_LINE                EXTI_Line3
#define Camera_key_EXTI_PROT                EXTI_PortSourceGPIOE
#define Camera_key_EXTI_PinSource           EXTI_PinSource3
void Camera_thread_entry (void* parameter);
void Camera_key(void);
void mywrite_usb(char *str);
#endif
