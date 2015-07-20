/**
  ******************************************************************************
  * @file    Project/STM32F4xx_StdPeriph_Templates/main.c 
  * @author  MCD Application Team
  * @version V1.5.0
  * @date    06-March-2015
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2015 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stdio.h"
#include "main.h"
#include "oled.h"
#include "mpu9250.h"
#include "uart4.h"
#include "usart1.h"
#include "gps\gps.h"
#include "AHRS\AHRS.h"
//#include "IMU\IMU.h"

/** @addtogroup Template_Project
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static __IO uint32_t uwTimingDelay;

char str[20];

uint8_t motion_tim_cnt = 0;
float sum_pitch = 0;
float sum_roll  = 0;
float sum_yaw   = 0;
float final_pitch = 0;
float final_roll  = 0;
float final_yaw   = 0;
#define AHRS_N 10

nmea_msg gpsx = {0};
double gps_longitude = 0;
double gps_latitude = 0;

/* Private function prototypes -----------------------------------------------*/
void DelayUS(__IO uint32_t nTime);
void DelayMS(__IO uint32_t nTime);
void Init_Tim(void);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
  /*!< At this stage the microcontroller clock setting is already configured, 
       this is done through SystemInit() function which is called from startup
       files (startup_stm32f40_41xxx.s/startup_stm32f427_437xx.s/
       startup_stm32f429_439xx.s/startup_stm32f401xx.s/startup_stm32f411xe.s or
       startup_stm32f446xx.s)
       before to branch to application main.
       To reconfigure the default setting of SystemInit() function, 
       refer to system_stm32f4xx.c file */

  RCC_ClocksTypeDef RCC_Clocks;
  GPIO_InitTypeDef GPIO_InitStructure;
  
  /* SysTick end of count event each 1ms */
  RCC_GetClocksFreq(&RCC_Clocks);
  SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000000);
    /* GPIOG Peripheral clock enable */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  GPIO_ResetBits(GPIOA, GPIO_Pin_2);
  
//    /* Enable the PWR clock */
//  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

//  /* Allow access to RTC */
//  PWR_BackupAccessCmd(ENABLE);

//  /* Reset RTC Domain */
//  RCC_BackupResetCmd(ENABLE);
//  RCC_BackupResetCmd(DISABLE);

//  /* Enable the LSE OSC */
//  RCC_LSEConfig(RCC_LSE_ON);

//  /* Wait till LSE is ready */  
//  while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
//  {
//  }
//  
//  /* Configure MCO1 pin(PA8) in alternate function */
//  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
//  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
//  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
//  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;  
//  GPIO_Init(GPIOA, &GPIO_InitStructure);
//    
//  /* HSE clock selected to output on MCO1 pin(PA8)*/
//  RCC_MCO1Config(RCC_MCO1Source_LSE, RCC_MCO1Div_1);

  
  /* Add your application code here */
  Init_Usart1(115200);
  OLED_Init();
  Init_MPU9250();
//  OLED_String(0,0,"MPU-9250 9-Axis Sensor");
//  init_quaternion();
//  OLED_Clear();
  //OLED_String(0,0,"MPU-9250 9-Axis Sensor");
  OLED_String(1,10,"Pitch:");
  OLED_String(2,10,"Roll: ");
  OLED_String(3,10,"Yaw:  ");
  Init_Tim();
//  Init_Uart4(9600);

  Init_Uart4(9600);
//  if(Ublox_Cfg_Rate(250,1)!=0)
//  {
//    while((Ublox_Cfg_Rate(250,1)!=0)&&key)
//    {
//      Init_Uart4(9600);
//      Ublox_Cfg_Prt(115200);
//      Init_Uart4(115200);
//      Ublox_Cfg_Tp(1000000,100000,1);  
//      key=Ublox_Cfg_Cfg_Save();
//    }
//  }
//  
    
  /* Infinite loop */
  while (1)
  {
    if(motion_tim_cnt == AHRS_N)
    {
      motion_tim_cnt = 0;
      final_pitch = sum_pitch / 10;
      final_roll  = sum_roll  / 10;
      final_yaw   = sum_yaw   / 10;
      sum_pitch = 0;
      sum_roll  = 0;
      sum_yaw   = 0;
      sprintf(str, "% 3.1f   ", final_pitch);
      OLED_String(1,40,(u8 *)str);
      sprintf(str, "% 3.1f   ", final_roll);
      OLED_String(2,40,(u8 *)str);
      sprintf(str, "% 3.1f   ", final_yaw);
      OLED_String(3,40,(u8 *)str);
    }
    if(gps_uart_flag)
    {
      float tp;
      GPS_Analysis(&gpsx,(u8*)GPS_DATA);
      if((gpsx.ewhemi == 'E')||(gpsx.ewhemi == 'W'))
      {
        if(gpsx.longitude != gps_longitude)
        {
          gps_longitude = gpsx.longitude;
          tp = gps_longitude / 100000.0f;
          sprintf(str, "%3.4f%1c   ", tp, gpsx.ewhemi);
          OLED_String(0,0,(u8 *)str);
        }
      }
      if((gpsx.nshemi == 'N')||(gpsx.nshemi == 'S'))
      {
        if(gpsx.latitude != gps_latitude) 
        {
          gps_latitude = gpsx.latitude;
          tp = gps_latitude / 100000.0f;
          sprintf(str, "%3.4f%1c   ", tp, gpsx.nshemi);
          OLED_String(0,68,(u8 *)str);
        }
      }
      gps_uart_flag = 0;
      //u1_printf(GPS_DATA);
      //u1_printf("\r\n\r\n\r\n");
    }
  }
}

/**
  * @brief  Inserts a delay time.
  * @param  nTime: specifies the delay time length, in milliseconds.
  * @retval None
  */
void DelayUS(__IO uint32_t nTime)
{ 
  uwTimingDelay = nTime;

  while(uwTimingDelay != 0);
}

void DelayMS(__IO uint32_t nTime)
{ 
  uwTimingDelay = nTime * 1000;

  while(uwTimingDelay != 0);
}

/**
  * @brief  Decrements the TimingDelay variable.
  * @param  None
  * @retval None
  */
void TimingDelay_Decrement(void)
{
  if (uwTimingDelay != 0x00)
  { 
    uwTimingDelay--;
  }
}

void Init_Tim(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  
  /* TIM3 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

  /* Enable the TIM3 global Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 999;
  TIM_TimeBaseStructure.TIM_Prescaler = 1679;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
  
  /* TIM Interrupts enable */
  TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

  /* TIM3 enable counter */
  TIM_Cmd(TIM3, ENABLE);
  
  
//  /* TIM4 clock enable */
//  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

//  /* Enable the TIM4 global Interrupt */
//  NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
//  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
//  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
//  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//  NVIC_Init(&NVIC_InitStructure);

//  /* Time base configuration */
//  TIM_TimeBaseStructure.TIM_Period = 99;
//  TIM_TimeBaseStructure.TIM_Prescaler = 1679;
//  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
//  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

//  TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
//  
//  /* TIM Interrupts enable */
//  TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);

//  /* TIM4 enable counter */
//  TIM_Cmd(TIM4, ENABLE);
}

void TIM3_IRQHandler(void)
{
  if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
  {
    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    READ_MPU9250_ACCEL();
    READ_MPU9250_GYRO();
    READ_MPU9250_MAG();
    AHRSupdate(GYRO_DATA[0], GYRO_DATA[1], GYRO_DATA[2],
               ACCEL_DATA[0],ACCEL_DATA[1],ACCEL_DATA[2],
               MAG_DATA[0],  MAG_DATA[1],  MAG_DATA[2]);
    sum_pitch += pitch;
    sum_roll  += roll;
    sum_yaw   += yaw;
    motion_tim_cnt++;
//    if(gps_uart_en)
//    {
//      gps_tim_cnt++;
//      if(gps_tim_cnt>5)
//      {
//        UART4_RX_STA |= 1<<15;
//        gps_tim_cnt = 0;
//        gps_uart_en = 0;
//      }
//    }
  }
}

//void TIM4_IRQHandler(void)
//{
//  if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
//  {
//    vu32 i;
//    TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
////    i = 1024 - DMA_GetCurrDataCounter(DMA1_Stream2);
////    if(gps_uart_dmacnt != i)
////    {
////      gps_uart_dmacnt = i;
////      gps_uart_timeout = 0;
////    }
////    else
////    {
////      if(((gps_uart_timeout & 0x80000000) == 0) && (i > 0))
////      {
////        gps_uart_timeout++;
////        if(gps_uart_timeout == 5)
////        {
////          for(i=0;i<gps_uart_dmacnt;i++) GPS_DATA[i] = UART4_RX_BUF[i];
////          GPS_DATA[i] = 0;
////          gps_uart_dmacnt = 0;
////          gps_uart_timeout |= 0x80000000;
////        }
////      }
////    }
//  }
//}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
