#ifndef _CF5642C_H
#define _CF5642C_H
#include "stm32f4xx.h"   
#include "SCCB.h"
//管脚定义
#define CF5642C_HSYNC_PIN                      GPIO_Pin_4
#define CF5642C_HSYNC_GPIO_PORT                GPIOA
#define CF5642C_HSYNC_GPIO_CLK                 RCC_AHB1Periph_GPIOA 

#define CF5642C_PIXCLK_PIN                      GPIO_Pin_6
#define CF5642C_PIXCLK_GPIO_PORT                GPIOA
#define CF5642C_PIXCLK_GPIO_CLK                 RCC_AHB1Periph_GPIOA 

#define CF5642C_VSYNC_PIN                      GPIO_Pin_7
#define CF5642C_VSYNC_GPIO_PORT                GPIOB
#define CF5642C_VSYNC_GPIO_CLK                 RCC_AHB1Periph_GPIOB

#define CF5642C_D6_PIN                      GPIO_Pin_8
#define CF5642C_D6_GPIO_PORT                GPIOB
#define CF5642C_D6_GPIO_CLK                 RCC_AHB1Periph_GPIOB

#define CF5642C_D7_PIN                      GPIO_Pin_9
#define CF5642C_D7_GPIO_PORT                GPIOB
#define CF5642C_D7_GPIO_CLK                 RCC_AHB1Periph_GPIOB

#define CF5642C_D0_PIN                      GPIO_Pin_6
#define CF5642C_D0_GPIO_PORT                GPIOC
#define CF5642C_D0_GPIO_CLK                 RCC_AHB1Periph_GPIOC

#define CF5642C_D1_PIN                      GPIO_Pin_7
#define CF5642C_D1_GPIO_PORT                GPIOC
#define CF5642C_D1_GPIO_CLK                 RCC_AHB1Periph_GPIOC

#define CF5642C_D2_PIN                      GPIO_Pin_8
#define CF5642C_D2_GPIO_PORT                GPIOC
#define CF5642C_D2_GPIO_CLK                 RCC_AHB1Periph_GPIOC

#define CF5642C_D4_PIN                      GPIO_Pin_11
#define CF5642C_D4_GPIO_PORT                GPIOC
#define CF5642C_D4_GPIO_CLK                 RCC_AHB1Periph_GPIOC

#define CF5642C_D5_PIN                      GPIO_Pin_3
#define CF5642C_D5_GPIO_PORT                GPIOD
#define CF5642C_D5_GPIO_CLK                 RCC_AHB1Periph_GPIOD

#define CF5642C_D3_PIN                      GPIO_Pin_12
#define CF5642C_D3_GPIO_PORT                GPIOH
#define CF5642C_D3_GPIO_CLK                 RCC_AHB1Periph_GPIOH

#define CF5642C_STROBE_PIN                  GPIO_Pin_6
#define CF5642C_STROBE_GPIO_PORT            GPIOE
#define CF5642C_STROBE_GPIO_CLK             RCC_AHB1Periph_GPIOE

#define CF5642C_FREX_PIN                 		GPIO_Pin_8
#define CF5642C_FREX_GPIO_PORT              GPIOI
#define CF5642C_FREX_GPIO_CLK               RCC_AHB1Periph_GPIOI
//私有定义
#define picture_x 320
#define picture_y 240


void CF5642C_GPIO_Init(void);
void CF5642C_DCMI_Init(void);
void CF5642C_DMA_Init(void);
void Cam_Start(void);
void Cam_Stop(void);
void Cam_Capture(void);
void Cam_Frex(void);
void OverTrans_DMA_Init(uint32_t *Buffer);
unsigned char write_i2c(unsigned int regID, unsigned char regDat);  
unsigned char Read_i2c(unsigned int regID, unsigned char *regDat);
unsigned char CF5642C_init(void);


unsigned char QVGA_rgb565_Preview(void);//QVGA 320*240
unsigned char VGA_jpeg_Preview(void);//VGA 640*480
unsigned char XGA_jpeg_Preview(void);//XGA  800*600 或者1024*768
#endif 



