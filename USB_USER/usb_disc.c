#include "usbh_usr.h"
#include <dfs_fs.h>
#include <rtthread.h>

#include "usb_disc.h"
#include "usbh_msc_fs_interface.h"

static struct rt_device uds_device;
static struct dfs_partition part;
#define SECTOR_SIZE 512
extern USB_OTG_CORE_HANDLE      USB_OTG_Core;
extern USBH_HOST								USB_Host;
/* RT-Thread Device Driver Interface */
static rt_err_t rt_uds_init(rt_device_t dev)
{
	return RT_EOK;
}

static rt_err_t rt_uds_open(rt_device_t dev, rt_uint16_t oflag)
{
	return RT_EOK;
}

static rt_err_t rt_uds_close(rt_device_t dev)
{
	return RT_EOK;
}

static rt_size_t rt_uds_read(rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size)
{
	rt_uint8_t status;
	rt_uint32_t i;

	status = EFS_PASS;
	// rt_kprintf("read: 0x%x, size %d\n", pos, size);
  if(HCD_IsDeviceConnected(&USB_OTG_Core))
  {  
    
    do
    {
      status = USBH_MSC_Read10(&USB_OTG_Core, buffer,pos,512*size);
      USBH_MSC_HandleBOTXfer(&USB_OTG_Core, &USB_Host);
    }
    while((status == USBH_MSC_BUSY ) && (HCD_IsDeviceConnected(&USB_OTG_Core)));
    
  }
  if (status == EFS_PASS) return size;
  return 0;
	/* read all sectors */
//	for (i = 0; i < size; i ++)
//	{
//		status = if_readBuf(0,
//			(part.offset + i + pos),
//			(rt_uint8_t*)((rt_uint8_t*)buffer + i * SECTOR_SIZE));
//		if (status != EFS_PASS)
//		{
//			rt_kprintf("usb disk read failed\n");
//			return 0;
//		}
//    //else
//      //rt_kprintf("usb disk read success\n");
//	}

//	if (status == EFS_PASS) return size;

//	rt_kprintf("read failed: %d\n", status);
//	return 0;
}

static rt_size_t rt_uds_write (rt_device_t dev, rt_off_t pos, const void* buffer, rt_size_t size)
{
	rt_int8_t status;
	rt_uint32_t i;

	status = EFS_ERROR;
	// rt_kprintf("write: 0x%x, size %d\n", pos, size);
if(HCD_IsDeviceConnected(&USB_OTG_Core))
  {  
    do
    {					
//			rt_kprintf("start write secter  %d\n", count);
			
      status = USBH_MSC_Write10(&USB_OTG_Core, (BYTE*)buffer,pos,USBH_MSC_PAGE_LENGTH*size);
			
//			rt_kprintf("status  %d\n", status);
//			rt_kprintf("write secter over  %d\n", count);
			
      USBH_MSC_HandleBOTXfer(&USB_OTG_Core, &USB_Host);
    }
    while((status == USBH_MSC_BUSY ) && \
      (HCD_IsDeviceConnected(&USB_OTG_Core)));
  }
	if (status == EFS_PASS) return size;
  return 0;
	/* read all sectors */
//	for (i = 0; i < size ; i ++)
//	{
//		status = if_writeBuf(0,
//			(part.offset + i + pos),
//			(rt_uint8_t*)((rt_uint8_t*)buffer + i * SECTOR_SIZE));
//		if (status != EFS_PASS)
//		{
//			rt_kprintf("usb disc write failed\n");
//			return 0;
//		}
//	}

//	if (status == EFS_PASS) return size;

//	rt_kprintf("write failed: %d\n", status);
//	return 0;
}

static rt_err_t rt_uds_control(rt_device_t dev, rt_uint8_t cmd, void *args)
{
	return RT_EOK;
}

void rt_hw_uds_init()
{
  rt_uint8_t status;
  rt_uint8_t *sector;

  /* register sdcard device */
  uds_device.type  = RT_Device_Class_Block;
  uds_device.init 	= rt_uds_init;
  uds_device.open 	= rt_uds_open;
  uds_device.close = rt_uds_close;
  uds_device.read 	= rt_uds_read;
  uds_device.write = rt_uds_write;
  uds_device.control = rt_uds_control;

  /* no private */
  uds_device.user_data = RT_NULL;
  /* get the first sector to read partition table */
  sector = (rt_uint8_t*) rt_malloc (512);
  if (sector == RT_NULL)
  {
    rt_kprintf("allocate partition sector buffer failed\n");
    return;
  }

  status = if_readBuf(0, 0,sector);
  if (status == EFS_PASS)
  {
    /* get the first partition */
    status = dfs_filesystem_get_partition(&part, sector, 0);
    if (status != RT_EOK)
    {
      /* there is no partition table */
      part.offset = 0;
      part.size   = 0;
    }
  }
  else
  {
    /* there is no partition table */
    part.offset = 0;
    part.size   = 0;
  }

  /* release sector buffer */
  rt_free(sector);

  rt_device_register(&uds_device, "ud0",
    RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_REMOVABLE | RT_DEVICE_FLAG_STANDALONE);
}

