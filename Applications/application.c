/*
 * File      : application.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 */

/**
 * @addtogroup STM32
 */
/*@{*/

#include <stdio.h>

#include "stm32f4xx.h"
#include <board.h>
#include <rtthread.h>

#include "sysdelay.h"

#ifdef RT_USING_DFS
/* dfs init */
#include <dfs_init.h>
/* dfs filesystem:ELM filesystem init */
#include <dfs_elm.h>
/* dfs Filesystem APIs */
#include <dfs_fs.h>
#include "my_usb.h"
#include "my_test.h"
#endif
#include <dfs_romfs.h>

#ifdef RT_USING_LWIP
#include <lwip/sys.h>
#include <lwip/api.h>
#include <netif/ethernetif.h>
#include "stm32_eth.h"
#endif
struct rt_semaphore rt_tim3_sem;
extern void rt_bt_thread_entry(void * para);
void rt_user_mb_init(void)
{
	rt_sem_init(&rt_tim3_sem, "tim3", 0, RT_IPC_FLAG_FIFO);
	//rt_sem_init(&rt_camera_sem, "camera", 0, RT_IPC_FLAG_FIFO);
	
}

void rt_init_thread_entry(void* parameter)
{
	#ifdef RT_USING_DFS
	{
		/* init the device filesystem */
		dfs_init();
		dfs_romfs_init();
		elm_init();
		if (dfs_mount(0, "/", "rom", 0, (void*)&romfs_root) == 0)
		{
			rt_kprintf("rom_fs initialized!\n");
		}
		else
			rt_kprintf("rom_fs initialzation failed!\n");

	}
#endif
    /* LwIP Initialization */
#ifdef RT_USING_LWIP
    {
        extern void lwip_sys_init(void);

        /* register ethernetif device */
        eth_system_device_init();

        rt_hw_stm32_eth_init();

        /* init lwip system */
        lwip_sys_init();
        rt_kprintf("TCP/IP initialized!\n");
    }
#endif

//FS

//GUI
}

int rt_application_init()
{
    rt_thread_t init_thread;

#if (RT_THREAD_PRIORITY_MAX == 32)
    init_thread = rt_thread_create("init",
                                   rt_init_thread_entry, RT_NULL,
                                   2048, 8, 20);
#else
    init_thread = rt_thread_create("init",
                                   rt_init_thread_entry, RT_NULL,
                                   2048, 80, 20);
#endif
/*
    if (init_thread != RT_NULL)
        rt_thread_startup(init_thread);

    //------- init led1 thread
    rt_thread_init(&thread_led1,
                   "led1",
                   rt_thread_entry_led1,
                   RT_NULL,
                   &thread_led1_stack[0],
                   sizeof(thread_led1_stack),11,5);
    rt_thread_startup(&thread_led1);

    //------- init led2 thread
    rt_thread_init(&thread_led2,
                   "led2",
                   rt_thread_entry_led2,
                   RT_NULL,
                   &thread_led2_stack[0],
                   sizeof(thread_led2_stack),11,5);
    rt_thread_startup(&thread_led2);
*/
/*
		if (init_thread != RT_NULL)
        rt_thread_startup(init_thread);
		//usb host
		init_thread = rt_thread_create("usb_host",
                                   usb_thread_entry, RT_NULL,
                                   2048, 0x9, 20);
		if (init_thread != RT_NULL)
       rt_thread_startup(init_thread);
		
		init_thread = rt_thread_create("TIM3_init",
                                   Tim3_thread_entry, RT_NULL,
																	2048, 0x12, 20);		
    if (init_thread != RT_NULL)
        rt_thread_startup(init_thread);                              
		rt_user_mb_init();
*/		
		
			if (init_thread != RT_NULL)
        rt_thread_startup(init_thread);
		init_thread = rt_thread_create("bt",
                                   rt_bt_thread_entry, RT_NULL,
                                   2048, 0x5, 20);
		if (init_thread != RT_NULL)
		 rt_thread_startup(init_thread);
    return 0;
}

/*@}*/
