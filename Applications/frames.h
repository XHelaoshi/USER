#ifndef _FRAME_H_
#define _FRAME_H_

//#include "EZ_GUI.H"
//#include "EZ_BF.H"
#include <rtthread.h>

#define ID_FRAME_HOME   0x10
extern EFRAME HomeFrame;

#define ID_FRAME_SEL   0x11
extern EFRAME SelFrame;

#define ID_FRAME_USB    0x12
extern EFRAME UsbFrame;

#define ID_FRAME_FLA    0x13
extern EFRAME FlaFrame;

#define ID_FRAME_MON    0x14
extern EFRAME MonFrame;
// #define ID_FRAME_SET    0x13
// extern struct PGMER_SETTING
// {
//   unsigned char target_index;
//   /*0->LQFP48*/
//   /*1->LQFP64*/
//   /*2->LQFP100*/
//   unsigned char pgm_mode;
//   /*0->∆’Õ®*/
//   /*1->º”√‹*/
//   unsigned char bin_source[2][16];
// }pgmer_settings;

// extern EFRAME SetFrame;



#endif
