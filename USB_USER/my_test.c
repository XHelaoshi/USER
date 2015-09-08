#include "my_test.h"
#include <rtthread.h>
#include "integer.h"
#include "diskio.h"
#include <dfs_posix.h>
#include "stm32f4xx.h"                  // Device header
#define TIM_USEC 0x01
#define TIM_MSEC 0x02
//__IO uint32_t count = 0;
//BYTE	mytest_buff[512]={0};
BYTE buffer[1024*64];
//struct rt_semaphore SpeedTest_Sem;
rt_tick_t t1,t2;
void mytest_entry(void* parameter)
{
			int fd;
			int i=1024;
     // BYTE buffer[2048];
			rt_memset(buffer, 0x55 , sizeof(buffer));
			//SpeedTest_TimeInit();
			buffer[0] = 0xff;
			buffer[1] = 0xd8;
	
//      rt_sem_init(&SpeedTest_Sem, "SpeedTest_Sem", 0, RT_IPC_FLAG_FIFO);
	    rt_kprintf("start write \n");
			fd = open("/ud/text.txt", O_WRONLY | O_CREAT,0);	
			if (fd >= 0)
			{
				//TIM_Cmd(TIM3,ENABLE);
				//while(i>0)
				//{
					write(fd, buffer, 2);//sizeof(buffer)
				//	i--;
					//rt_kprintf("%d\n", count);
				//}
				
				close(fd);
			}
			 rt_kprintf("write done\n");
//		  rt_sem_release(&SpeedTest_Sem);
			rt_thread_delay(5);

}

void mytest_entry_1(void* parameter)
{
//	while(1)
//	{
//		if(disk_read(0,mytest_buff,17601673,1)==RES_ERROR)
//			continue;
//		else
//			rt_thread_delay(5);
//	}
			int fd, size;
			char s[] = "RT-Thread Programmer!\n", buffer[80];

			fd = open("/ud/text.txt", O_WRONLY | O_CREAT,0);
			if (fd >= 0)
			{
				write(fd, s, sizeof(s));
				close(fd);
			}

			fd = open("/ud/text.txt", O_RDONLY,0);
			if (fd >= 0)
			{
				size=read(fd, buffer, sizeof(buffer));
				close(fd);
			}
			rt_kprintf("%s", buffer);
}


#include "finsh.h"
void sdd()
{
	mytest_entry(NULL);
}
FINSH_FUNCTION_EXPORT(sdd, mytest_entry)

