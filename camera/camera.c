#include "CF5642C.H"
#include "sysdelay.h"

#include "camera.h"
#include <rtthread.h>
#include "integer.h"
#include "diskio.h"
#include <dfs_posix.h>
#include "string.h"

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
	
		ret = CF5642C_init();	
		if(ret == 1)
		{	
			rt_kprintf("init camera ok!\n");
		}
		else
		{
			rt_kprintf("init camera failed!\n");
		}
		while(1);
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
	rt_tick_t t1,t2;
	sprintf(name,"/ud/%s.jpg",str);
	//name = strcat(name,".jpg");
	fd = open(name, O_WRONLY | O_CREAT,0);	//text.jpg
			if (fd >= 0)
			{rt_kprintf("start write\n");
				//TIM_Cmd(TIM3,ENABLE);
					t1 = rt_tick_get();
					write(fd,(uint8_t *)&RAM_Buffer[soi_index], 4*(eoi_index-soi_index+1)-tail_index);					
					//rt_kprintf("%d\n", count);
				// fd = rename("/ud/test.txt","/ud/test.jpeg");
					t2 = rt_tick_get();
				rt_kprintf("%d\n\r",t2-t1);
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
		case 4:
			rt_kprintf("qsxga:\n");break;
		default:break;
	}
	
}
FINSH_FUNCTION_EXPORT(mywrite_usb, mywrite_usb)
FINSH_FUNCTION_EXPORT(ca, ca)
FINSH_FUNCTION_EXPORT(fb, fb(int t))
