#include "CF5642C.H"
#include "sysdelay.h"
#include "tm_stm32f4_ili9341.h"
#include "camera.h"
#include <rtthread.h>
#include "integer.h"
#include "diskio.h"
#include <dfs_posix.h>
#include "string.h"
extern __IO uint32_t RAM_Buffer[JPEG_BUFFER_CNT*(JPEG_BUFFER_SIZE+1)];

extern struct rt_semaphore rt_tim3_sem;
extern struct rt_semaphore rt_camera_sem;
extern u8 camera_status;
int dma_freebuf_ok;
int cam_capture_ok=0;
uint8_t temp1,temp2,temp3,temp4,tempp;
int jpeg_soi_flag=0;
int jpeg_eoi_flag=0;
unsigned int soi_index,eoi_index,tail_index;
int file_index=0;
char file_string[5];
int fenbianlv=0;
void Camera_thread_entry (void* parameter)
{
		uint8_t ret=0;
	  int i,fd;
	int ttt;
	//等待TIM3 初始化完毕
	 rt_sem_take(&rt_tim3_sem, RT_WAITING_FOREVER);
		rt_memset((void*)RAM_Buffer,0,JPEG_BUFFER_CNT*(JPEG_BUFFER_SIZE+1));
		//RAM_Buffer[JPEG_BUFFER_CNT*(JPEG_BUFFER_SIZE+1)-1]=0xff;
	  ret=CF5642C_init();
	  if(1==ret)
		{
			//Prepair for Camera to SPI LCD

		  Cam_Stop();
		  Camera_key();
			
			while(1)
			{
				
				CF5642C_DMA_Init();
				write_i2c(0x3503, 0x0);//to enable AGC/AEC
				write_i2c(0x3b07 ,0x0a);//return to rolling strobe
				rt_memset((void*)RAM_Buffer,0,4*JPEG_BUFFER_CNT*(JPEG_BUFFER_SIZE+1));
				//rt_memset((void*)RAM_Buffer,0,1);
				rt_kprintf("waiting for shutter...\n");
				while(!cam_capture_ok){}
					cam_capture_ok=0;
					soi_index=0;
				eoi_index=0;
				tail_index=0;
				jpeg_soi_flag=0;
				jpeg_eoi_flag=0;
				Cam_Capture();
				rt_sem_take(&rt_camera_sem, RT_WAITING_FOREVER);
				while(!dma_freebuf_ok){}
					dma_freebuf_ok = 0;
					for(i=0;i<JPEG_BUFFER_CNT*(JPEG_BUFFER_SIZE+1);i++)
					{
						
						temp1 = RAM_Buffer[i]&0xff;
						temp2 = RAM_Buffer[i]>>8&0xff;
						temp3 = RAM_Buffer[i]>>16&0xff;
						temp4 = RAM_Buffer[i]>>24&0xff;
						
						if((temp1 == 0xff)&&(temp2 == 0xd8))
						{
							jpeg_soi_flag	=1;
							soi_index	= i;						
						}
						else if((temp2 == 0xff)&&(temp3 == 0xd8))
						{
							jpeg_soi_flag	=1;	
							soi_index	= i;
						}
						else if((temp3 == 0xff)&&(temp4 == 0xd8))
						{
							jpeg_soi_flag	=1;	
							soi_index	= i;
						}					
						if(jpeg_soi_flag == 1){
//							rt_kprintf("%02x",temp1);
//							rt_kprintf("%02x",temp2);
//							rt_kprintf("%02x",temp3);
//							rt_kprintf("%02x",temp4);					
							//search for eoi
							if((temp1 == 0xff)&&(temp2 == 0xd9))//[][][2][1]
						{
//							if(jpeg_off_flag == 1)
//								goto next;
//							else
//								jpeg_off_flag++;	
							jpeg_eoi_flag	=1;
							eoi_index	= i;
							tail_index = 2;
							goto next;
						}
						else if((temp2 == 0xff)&&(temp3 == 0xd9))//[][3][2][]
						{
							jpeg_eoi_flag	=1;	
							eoi_index	= i;
							tail_index = 1;
							goto next;
						}
						else if((temp3 == 0xff)&&(temp4 == 0xd9))//[4][3][][]
						{
							jpeg_eoi_flag	=1;	
							eoi_index	= i;
							tail_index = 0;
							goto next;
						}	
						else if((tempp == 0xff)&&(temp1 == 0xd9))//[][][][1] | [4]
						{
							jpeg_eoi_flag	=1;	
							eoi_index	= i;
							tail_index = 3;
							goto next;
						}
						tempp = temp4;
					}
					}
		next:	Cam_Stop();		
					rt_kprintf("start write \n");
					
					sprintf(file_string,"%d",file_index);
					file_index++;
					mywrite_usb(file_string);
//			fd = open("/ud/text.text", O_WRONLY | O_CREAT,0);	
//			if (fd >= 0)
//			{
//				//TIM_Cmd(TIM3,ENABLE);
//			
//					write(fd,(uint8_t *)&RAM_Buffer[soi_index-1], eoi_index-soi_index+2);					
//					//rt_kprintf("%d\n", count);
//				
//				
//				close(fd);
//			}
//			rt_kprintf("write over\n");
										
				
/******				
				TM_ILI9341_Rotate(TM_ILI9341_Orientation_Landscape_2);
				TM_ILI9341_SetCursorPosition(0, 0, picture_x - 1, picture_y - 1);
				TM_ILI9341_SendCommand(ILI9341_GRAM);	
						//Wirte GRAM
				ILI9341_WRX_SET;
				for (i = 0; i < picture_x*picture_y; i++) 
				{
					ILI9341_CS_RESET;
					ILI9341_SPI->DR=(RAM_Buffer[i])&0xff;
					while (!SPI_I2S_GetFlagStatus(ILI9341_SPI, SPI_I2S_FLAG_TXE));
					ILI9341_CS_SET;
					
					ILI9341_CS_RESET;		
					ILI9341_SPI->DR=(RAM_Buffer[i]>>8)&0xff;			
					while (!SPI_I2S_GetFlagStatus(ILI9341_SPI, SPI_I2S_FLAG_TXE));
					ILI9341_CS_SET;
				}
				if(camera_status==0)
				{
					TM_ILI9341_Rotate(TM_ILI9341_Orientation_Portrait_1);
					LCD_SetCursor(0,0);
				}
				*******/
			}
		}

}
void Camera_key()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	/* 使能管脚时钟 */
	RCC_AHB1PeriphClockCmd(Camera_key_GPIO_CLK  , ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	
	GPIO_InitStructure.GPIO_Mode= GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Pin= Camera_key_PIN;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_DOWN;
	GPIO_Init(Camera_key_GPIO_PORT, &GPIO_InitStructure);

	SYSCFG_EXTILineConfig(Camera_key_EXTI_PROT,Camera_key_EXTI_PinSource);
	
	EXTI_InitStructure.EXTI_Line = Camera_key_EXTI_LINE;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	
  NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	
	NVIC_Init(&NVIC_InitStructure);
}
#include "finsh.h"

void mywrite_usb(char *str)
{
	int fd;
	char name[20];
//	//char* folder="/ud/";
//	char* postfix=".jpg";
//	strcat(name,str);
//	strcat(name,postfix);
	
	sprintf(name,"/ud/%s.jpg",str);
	//name = strcat(name,".jpg");
	fd = open(name, O_WRONLY | O_CREAT,0);	//text.jpg
			if (fd >= 0)
			{rt_kprintf("start write\n");
				//TIM_Cmd(TIM3,ENABLE);
			
					write(fd,(uint8_t *)&RAM_Buffer[soi_index], 4*(eoi_index-soi_index+1)-tail_index);					
					//rt_kprintf("%d\n", count);
				// fd = rename("/ud/test.txt","/ud/test.jpeg");
				
				close(fd);
//				fd = open("/ud/test.txt", O_WRONLY | O_CREAT,0);
//				dfs_file_rename("/ud/test.txt","/ud/test.jpeg");
//				close(fd);
				rt_kprintf("write over\n");
			}
			rt_kprintf("write over\n");
}
void ca()
{
	cam_capture_ok=1;
}
void fb(int t)
{
	fenbianlv = t;
	switch(fenbianlv)
	{
		case 0:
			rt_kprintf("qvga:320*240\n");break;
		case 1:
			rt_kprintf("vga:640*480\n");break;
		case 2:
			rt_kprintf("xga:1024*768\n");break;
		case 3:
			rt_kprintf("qxga:2048*1536\n");break;
		default:break;
	}
	
}
FINSH_FUNCTION_EXPORT(mywrite_usb, mywrite_usb)
FINSH_FUNCTION_EXPORT(ca, ca)
FINSH_FUNCTION_EXPORT(fb, fb(int t))
