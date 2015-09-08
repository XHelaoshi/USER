/**
  ******************************************************************************
  * @file    usbh_usr.c
  * @author  MCD Application Team
  * @version V2.1.0
  * @date    19-March-2012
  * @brief   This file includes the usb host library user callbacks
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
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
#include <string.h>
#include "usbh_usr.h"
//#include "lcd_log.h"
#include "ff.h"       /* FATFS */
#include "usbh_msc_core.h"
#include "usbh_msc_scsi.h"
#include "usbh_msc_bot.h"
#include "usbh_msc_fs_interface.h"
#include "usb_disc.h"
#include <rtthread.h>
#include <dfs_fs.h>
/** @addtogroup USBH_USER
* @{
*/

/** @addtogroup USBH_MSC_DEMO_USER_CALLBACKS
* @{
*/

/** @defgroup USBH_USR 
* @brief    This file includes the usb host stack user callbacks
* @{
*/ 

/** @defgroup USBH_USR_Private_TypesDefinitions
* @{
*/ 
/**
* @}
*/ 


/** @defgroup USBH_USR_Private_Defines
* @{
*/ 
#define IMAGE_BUFFER_SIZE    512
/**
* @}
*/ 


/** @defgroup USBH_USR_Private_Macros
* @{
*/ 
extern USB_OTG_CORE_HANDLE          USB_OTG_Core;
/**
* @}
*/ 


/** @defgroup USBH_USR_Private_Variables
* @{
*/ 
uint8_t USBH_USR_ApplicationState = USH_USR_FS_INIT;
uint8_t filenameString[15]  = {0};

FATFS fatfs;
FIL file;
uint8_t Image_Buf[IMAGE_BUFFER_SIZE];
uint8_t line_idx = 0;   

/*  Points to the DEVICE_PROP structure of current device */
/*  The purpose of this register is to speed up the execution */
uint8_t USBH_STATE=0;
struct rt_mailbox MSC_MailBox_Bot;
struct rt_mailbox MSC_MailBox_Top;
unsigned long MSC_MailPool[2][8];

USBH_Usr_cb_TypeDef USR_cb =
{
  USBH_USR_Init,
  USBH_USR_DeInit,
  USBH_USR_DeviceAttached,
  USBH_USR_ResetDevice,
  USBH_USR_DeviceDisconnected,
  USBH_USR_OverCurrentDetected,
  USBH_USR_DeviceSpeedDetected,
  USBH_USR_Device_DescAvailable,
  USBH_USR_DeviceAddressAssigned,
  USBH_USR_Configuration_DescAvailable,
  USBH_USR_Manufacturer_String,//ÖÆÔìÉÌ
  USBH_USR_Product_String,
  USBH_USR_SerialNum_String,
  USBH_USR_EnumerationDone,
  USBH_USR_UserInput,
  USBH_USR_MSC_Application,
  USBH_USR_DeviceNotSupported,
  USBH_USR_UnrecoveredError
    
};

/**
* @}
*/

/** @defgroup USBH_USR_Private_Constants
* @{
*/ 
/*--------------- LCD Messages ---------------*/
const uint8_t MSG_HOST_INIT[]        = "> Host Library Initialized\n";
const uint8_t MSG_DEV_ATTACHED[]     = "> Device Attached \n";
const uint8_t MSG_DEV_DISCONNECTED[] = "> Device Disconnected\n";
const uint8_t MSG_DEV_ENUMERATED[]   = "> Enumeration completed \n";
const uint8_t MSG_DEV_HIGHSPEED[]    = "> High speed device detected\n";
const uint8_t MSG_DEV_FULLSPEED[]    = "> Full speed device detected\n";
const uint8_t MSG_DEV_LOWSPEED[]     = "> Low speed device detected\n";
const uint8_t MSG_DEV_ERROR[]        = "> Device fault \n";

const uint8_t MSG_MSC_CLASS[]        = "> Mass storage device connected\n";
const uint8_t MSG_HID_CLASS[]        = "> HID device connected\n";
const uint8_t MSG_DISK_SIZE[]        = "> Size of the disk in MBytes: \n";
const uint8_t MSG_LUN[]              = "> LUN Available in the device:\n";
const uint8_t MSG_ROOT_CONT[]        = "> Exploring disk flash ...\n";
const uint8_t MSG_WR_PROTECT[]       = "> The disk is write protected\n";
const uint8_t MSG_UNREC_ERROR[]      = "> UNRECOVERED ERROR STATE\n";

/**
* @}
*/


/** @defgroup USBH_USR_Private_FunctionPrototypes
* @{
*/
static uint8_t Explore_Disk (char* path , uint8_t recu_level);
static uint8_t Image_Browser (char* path);
static void     Show_Image(void);
static void     Toggle_Leds(void);
/**
* @}
*/ 


/** @defgroup USBH_USR_Private_Functions
* @{
*/ 


/**
* @brief  USBH_USR_Init 
*         Displays the message on LCD for host lib initialization
* @param  None
* @retval None
*/
void USBH_USR_Init(void)
{
#ifdef USE_USB_OTG_HS 
		rt_kprintf("USB OTG HS MSC Host\n");
#else
    rt_kprintf(" USB OTG FS MSC Host");
#endif
}

/**
* @brief  USBH_USR_DeviceAttached 
*         Displays the message on LCD on device attached
* @param  None
* @retval None
*/
void USBH_USR_DeviceAttached(void)
{
  rt_kprintf((void *)MSG_DEV_ATTACHED);
}


/**
* @brief  USBH_USR_UnrecoveredError
* @param  None
* @retval None
*/
void USBH_USR_UnrecoveredError (void)
{
	rt_kprintf((void *)MSG_UNREC_ERROR); 
}


/**
* @brief  USBH_DisconnectEvent
*         Device disconnect event
* @param  None
* @retval Staus
*/
void USBH_USR_DeviceDisconnected (void)
{
  rt_device_t pdev;
  rt_kprintf("USBH_USR_DeviceDisconnected\n");
  pdev=rt_device_find("ud0");
  if(pdev!=RT_NULL)
  {
    if(dfs_unmount("/ud")==0)
      rt_kprintf("ud0 fileystem unmounted\n");
    else
      rt_kprintf("ud0 filesystem unmount failed\n");
    if(rt_device_unregister(pdev)==RT_EOK)
    {
      rt_kprintf("ud0 device unregistered\n");
    }
  }
}
/**
* @brief  USBH_USR_ResetUSBDevice 
* @param  None
* @retval None
*/
void USBH_USR_ResetDevice(void)
{
  /* callback for USB-Reset */
}


/**
* @brief  USBH_USR_DeviceSpeedDetected 
*         Displays the message on LCD for device speed
* @param  Device speed
* @retval None
*/
void USBH_USR_DeviceSpeedDetected(uint8_t DeviceSpeed)
{
	if(DeviceSpeed == HPRT0_PRTSPD_HIGH_SPEED)
  {
    rt_kprintf((void *)MSG_DEV_HIGHSPEED);
  }  
  else if(DeviceSpeed == HPRT0_PRTSPD_FULL_SPEED)
  {
    rt_kprintf((void *)MSG_DEV_FULLSPEED);
  }
  else if(DeviceSpeed == HPRT0_PRTSPD_LOW_SPEED)
  {
    rt_kprintf((void *)MSG_DEV_LOWSPEED);
  }
  else
  {
    rt_kprintf((void *)MSG_DEV_ERROR);
  }
}

/**
* @brief  USBH_USR_Device_DescAvailable 
*         Displays the message on LCD for device descriptor
* @param  device descriptor
* @retval None
*/
void USBH_USR_Device_DescAvailable(void *DeviceDesc)
{ 
	USBH_DevDesc_TypeDef *hs;
  hs = DeviceDesc;   
  rt_kprintf("VID : %04Xh\n" , (uint32_t)(*hs).idVendor); 
  rt_kprintf("PID : %04Xh\n" , (uint32_t)(*hs).idProduct); 
}

/**
* @brief  USBH_USR_DeviceAddressAssigned 
*         USB device is successfully assigned the Address 
* @param  None
* @retval None
*/
void USBH_USR_DeviceAddressAssigned(void)
{
  
}


/**
* @brief  USBH_USR_Conf_Desc 
*         Displays the message on LCD for configuration descriptor
* @param  Configuration descriptor
* @retval None
*/
void USBH_USR_Configuration_DescAvailable(USBH_CfgDesc_TypeDef * cfgDesc,
                                          USBH_InterfaceDesc_TypeDef *itfDesc,
                                          USBH_EpDesc_TypeDef *epDesc)
{
  USBH_InterfaceDesc_TypeDef *id;
  
  id = itfDesc;  
  
  if((*id).bInterfaceClass  == 0x08)
  {
    rt_kprintf((void *)MSG_MSC_CLASS);
  }
  else if((*id).bInterfaceClass  == 0x03)
  {
    rt_kprintf((void *)MSG_HID_CLASS);
  }    
}

/**
* @brief  USBH_USR_Manufacturer_String 
*         Displays the message on LCD for Manufacturer String 
* @param  Manufacturer String 
* @retval None
*/
void USBH_USR_Manufacturer_String(void *ManufacturerString)
{
  rt_kprintf("\n");
  rt_kprintf(ManufacturerString);
  rt_kprintf("\n");
}

/**
* @brief  USBH_USR_Product_String 
*         Displays the message on LCD for Product String
* @param  Product String
* @retval None
*/
void USBH_USR_Product_String(void *ProductString)
{
  rt_kprintf("Product : %s\n", (char *)ProductString); 
}

/**
* @brief  USBH_USR_SerialNum_String 
*         Displays the message on LCD for SerialNum_String 
* @param  SerialNum_String 
* @retval None
*/
void USBH_USR_SerialNum_String(void *SerialNumString)
{
  rt_kprintf( "Serial Number : %s\n", (char *)SerialNumString);    
} 



/**
* @brief  EnumerationDone 
*         User response request is displayed to ask application jump to class
* @param  None
* @retval None
*/
void USBH_USR_EnumerationDone(void)
{
	//rt_kprintf("USBH_USR_EnumerationDone\n");
  //while(USBH_MSC_ReadCapacity10()!=USBH_MSC_OK);
  MSC_MAIL* pmsc_mail=(MSC_MAIL*)rt_malloc(sizeof(MSC_MAIL));
  if(pmsc_mail==RT_NULL)return;
  pmsc_mail->Type=MSC_MAIL_TYPE_MOUNT;
  rt_kprintf("USBH_USR_EnumerationDone\n");
  rt_mb_send(&MSC_MailBox_Bot,(rt_uint32_t)pmsc_mail);
  USBH_STATE=1;
} 


/**
* @brief  USBH_USR_DeviceNotSupported
*         Device is not supported
* @param  None
* @retval None
*/
void USBH_USR_DeviceNotSupported(void)
{
  rt_kprintf ("> Device not supported."); 
}  


/**
* @brief  USBH_USR_UserInput
*         User Action for application state entry
* @param  None
* @retval USBH_USR_Status : User response for key button
*/
USBH_USR_Status USBH_USR_UserInput(void)
{
  //USBH_USR_Status usbh_usr_status=USBH_USR_RESP_OK;
  
  //if(USBH_STATE==1)
  //{
    //usbh_usr_status = USBH_USR_RESP_OK;
    //USBH_STATE=0;
  //}  
  return USBH_USR_RESP_OK;
}  

/**
* @brief  USBH_USR_OverCurrentDetected
*         Over Current Detected on VBUS
* @param  None
* @retval Staus
*/
void USBH_USR_OverCurrentDetected (void)
{
  rt_kprintf ("Overcurrent detected.");
}


/**
* @brief  USBH_USR_MSC_Application 
*         Demo application for mass storage
* @param  None
* @retval Staus
*/
int USBH_USR_MSC_Application(void)
{
  //while(USBH_MSC_ReadCapacity10()!=USBH_MSC_OK);
  //if_readBuf(0,0,sec);
  rt_err_t mscdev_err;
  pMSC_MAIL pmsc_mail;
  mscdev_err=rt_mb_recv(&MSC_MailBox_Bot,(unsigned long *)&pmsc_mail,100);
  if(mscdev_err==RT_EOK)
  {
    if(pmsc_mail!=0)
    {
      switch(pmsc_mail->Type)
      {
        case MSC_MAIL_TYPE_INT:break;
        case MSC_MAIL_TYPE_READ:
        {
          mscdev_err=if_readBuf(0,pmsc_mail->Attr,(unsigned char *)(pmsc_mail->Ext));
          rt_mb_send_wait(&MSC_MailBox_Top,mscdev_err,RT_WAITING_FOREVER);
          break;
        }
        case MSC_MAIL_TYPE_WRITE:
        {
          mscdev_err=if_writeBuf(0,pmsc_mail->Attr,(unsigned char *)(pmsc_mail->Ext));
          rt_mb_send_wait(&MSC_MailBox_Top,mscdev_err,RT_WAITING_FOREVER);
          break;
        }
        case MSC_MAIL_TYPE_MOUNT:
        {
          rt_free((void*)pmsc_mail);
          rt_kprintf("DiskMount\n");
          rt_hw_uds_init();
          if (dfs_mount("ud0", "/ud", "elm", 0, 0) == 0)
            rt_kprintf("UDS File System initialized!\n");
          else
            rt_kprintf("UDS File System init failed!\n");
          break;
        }
        default:break;
      }
    }
  }
  return(0);
}

/**
* @brief  Explore_Disk 
*         Displays disk content
* @param  path: pointer to root path
* @retval None
*/
static uint8_t Explore_Disk (char* path , uint8_t recu_level)
{

//  FRESULT res;
//  FILINFO fno;
//  DIR dir;
//  char *fn;
//  char tmp[14];
//  
//  res = f_opendir(&dir, path);
//  if (res == FR_OK) {
//    while(HCD_IsDeviceConnected(&USB_OTG_Core)) 
//    {
//      res = f_readdir(&dir, &fno);
//      if (res != FR_OK || fno.fname[0] == 0) 
//      {
//        break;
//      }
//      if (fno.fname[0] == '.')
//      {
//        continue;
//      }

//      fn = fno.fname;
//      strcpy(tmp, fn); 

//      line_idx++;
//      if(line_idx > 9)
//      {
//        line_idx = 0;
//        LCD_SetTextColor(Green);
//        LCD_DisplayStringLine( LCD_PIXEL_HEIGHT - 42, "                                              ");
//        LCD_DisplayStringLine( LCD_PIXEL_HEIGHT - 30, "Press Key to continue...");
//        LCD_SetTextColor(LCD_LOG_DEFAULT_COLOR); 
//        
//        /*Key B3 in polling*/
//        while((HCD_IsDeviceConnected(&USB_OTG_Core)) && \
//          (STM_EVAL_PBGetState (BUTTON_KEY) == SET))
//        {
//          Toggle_Leds();
//          
//        }
//      } 
//      
//      if(recu_level == 1)
//      {
//        LCD_DbgLog("   |__");
//      }
//      else if(recu_level == 2)
//      {
//        LCD_DbgLog("   |   |__");
//      }
//      if((fno.fattrib & AM_MASK) == AM_DIR)
//      {
//        strcat(tmp, "\n"); 
//        LCD_UsrLog((void *)tmp);
//      }
//      else
//      {
//        strcat(tmp, "\n"); 
//        LCD_DbgLog((void *)tmp);
//      }

//      if(((fno.fattrib & AM_MASK) == AM_DIR)&&(recu_level == 1))
//      {
//        Explore_Disk(fn, 2);
//      }
//    }
//  }
//  return res;
}

static uint8_t Image_Browser (char* path)
{
//  FRESULT res;
//  uint8_t ret = 1;
//  FILINFO fno;
//  DIR dir;
//  char *fn;
//  
//  res = f_opendir(&dir, path);
//  if (res == FR_OK) {
//    
//    for (;;) {
//      res = f_readdir(&dir, &fno);
//      if (res != FR_OK || fno.fname[0] == 0) break;
//      if (fno.fname[0] == '.') continue;

//      fn = fno.fname;
// 
//      if (fno.fattrib & AM_DIR) 
//      {
//        continue;
//      } 
//      else 
//      {
//        if((strstr(fn, "bmp")) || (strstr(fn, "BMP")))
//        {
//          res = f_open(&file, fn, FA_OPEN_EXISTING | FA_READ);
//          Show_Image();
//          USB_OTG_BSP_mDelay(100);
//          ret = 0;
//          while((HCD_IsDeviceConnected(&USB_OTG_Core)) && \
//            (STM_EVAL_PBGetState (BUTTON_KEY) == SET))
//          {
//            Toggle_Leds();
//          }
//          f_close(&file);
//          
//        }
//      }
//    }  
//  }
//  
//  #ifdef USE_USB_OTG_HS 
//  LCD_LOG_SetHeader(" USB OTG HS MSC Host");
//#else
//  LCD_LOG_SetHeader(" USB OTG FS MSC Host");
//#endif
//  LCD_LOG_SetFooter ("     USB Host Library v2.1.0" );
//  LCD_UsrLog("> Disk capacity : %d Bytes\n", USBH_MSC_Param.MSCapacity * \
//      USBH_MSC_Param.MSPageLength); 
//  USBH_USR_ApplicationState = USH_USR_FS_READLIST;
//  return ret;
}

/**
* @brief  Show_Image 
*         Displays BMP image
* @param  None
* @retval None
*/
static void Show_Image(void)
{
  
//  uint16_t i = 0;
//  uint16_t numOfReadBytes = 0;
//  FRESULT res; 
//  
//  LCD_SetDisplayWindow(239, 319, 240, 320);
//  LCD_WriteReg(0x03, 0x1008);
//  LCD_WriteRAM_Prepare(); /* Prepare to write GRAM */
//  
//  /* Bypass Bitmap header */ 
//  f_lseek (&file, 54);
//  
//  while (HCD_IsDeviceConnected(&USB_OTG_Core))
//  {
//    res = f_read(&file, Image_Buf, IMAGE_BUFFER_SIZE, (void *)&numOfReadBytes);
//    if((numOfReadBytes == 0) || (res != FR_OK)) /*EOF or Error*/
//    {
//      break; 
//    }
//    for(i = 0 ; i < IMAGE_BUFFER_SIZE; i+= 2)
//    {
//      LCD_WriteRAM(Image_Buf[i+1] << 8 | Image_Buf[i]); 
//    } 
//  }
  
}

/**
* @brief  Toggle_Leds
*         Toggle leds to shows user input state
* @param  None
* @retval None
*/
static void Toggle_Leds(void)
{
  static uint32_t i;
//  if (i++ == 0x10000)
//  {
//    STM_EVAL_LEDToggle(LED1);
//    STM_EVAL_LEDToggle(LED2);
//    STM_EVAL_LEDToggle(LED3);
//    STM_EVAL_LEDToggle(LED4);
//    i = 0;
//  }  
}
/**
* @brief  USBH_USR_DeInit
*         Deint User state and associated variables
* @param  None
* @retval None
*/
void USBH_USR_DeInit(void)
{
  USBH_STATE=0;
}


/**
* @}
*/ 

/**
* @}
*/ 

/**
* @}
*/

/**
* @}
*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

