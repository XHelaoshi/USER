#include "my_usb.h"
#include "usbh_core.h"
#include "usbh_usr.h"
#include "usbh_msc_core.h"


#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
#if defined ( __ICCARM__ ) /*!< IAR Compiler */
#pragma data_alignment=4   
#endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */
__ALIGN_BEGIN USB_OTG_CORE_HANDLE      USB_OTG_Core __ALIGN_END;

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
#if defined ( __ICCARM__ ) /*!< IAR Compiler */
#pragma data_alignment=4   
#endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */
__ALIGN_BEGIN USBH_HOST                USB_Host __ALIGN_END;

void usb_thread_entry(void* parameter)
{

	
	 rt_mb_init(&MSC_MailBox_Bot, "MSC_BOT", (void*)MSC_MailPool[0], 8, RT_IPC_FLAG_FIFO);
   rt_mb_init(&MSC_MailBox_Top, "MSC_TOP", (void*)MSC_MailPool[1], 8, RT_IPC_FLAG_FIFO);
	
	//config usb xi for 24 Mhz
	 MCO2_GPIO_Init();
	 RCC_MCO2Config(RCC_MCO2Source_HSE,RCC_MCO2Div_1);
	
	 USBH_Init(&USB_OTG_Core, USB_OTG_HS_CORE_ID,&USB_Host,&USBH_MSC_cb,&USR_cb);
	
	 while (1)
  {
    /* Host Task handler */
    USBH_Process(&USB_OTG_Core, &USB_Host);
    //rt_thread_delay(1);   
  }
}