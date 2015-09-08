#include <rtthread.h>
#include <rthw.h>
#include "hal_cpu.h"

static rt_base_t level;
extern rt_sem_t nw_bt_sem;
void hal_cpu_enable_irqs(void)
{

	rt_hw_interrupt_enable(level);
}

void hal_cpu_disable_irqs(void)
{
	level=rt_hw_interrupt_disable();
}

void hal_cpu_enable_irqs_and_sleep()
{
	rt_hw_interrupt_enable(level);
	rt_sem_take(nw_bt_sem, RT_WAITING_FOREVER);
}
