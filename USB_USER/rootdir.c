#include <dfs_romfs.h>
#include "stdint.h"
extern int SMART_CHECK_01(void);
extern void ezgui_key_hw_init(void);

const uint32_t tcal_dat[]=
{
	0x02940C9E,
	0x02A40374,
	0x0CAE0C98,
	0x0C9E0360,
	0x0781080C,
	0x00280028,
	0x011700C7,
	0x00000000
};

const struct romfs_dirent _root_dirent[] = {
  {ROMFS_DIRENT_FILE,"HELLO.TXT","Hello,RT-Thread!",17},
	{ROMFS_DIRENT_FILE,"TCAL.DAT",(const rt_uint8_t *)tcal_dat,sizeof(tcal_dat)},
  {ROMFS_DIRENT_DIR,"sd0",    0   , 0},
  {ROMFS_DIRENT_DIR, "ud",    0   , 0},
};

const struct romfs_dirent romfs_root = {ROMFS_DIRENT_DIR, "/", (rt_uint8_t*) _root_dirent, sizeof(_root_dirent)/sizeof(_root_dirent[0])};

